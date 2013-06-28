/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <osg/GL>
#include <osg/GLU>

#include <osg/Notify>
#include <osg/io_utils>
#include <osgUtil/Tessellator>

using namespace osg;
using namespace osgUtil;


Tessellator::Tessellator() :
    _wtype(TESS_WINDING_ODD),
    _ttype(TESS_TYPE_POLYGONS),
    _boundaryOnly(false), _numberVerts(0)
{
    _tobj = gluNewTess();
    if (_tobj)
    {
        gluTessCallback(_tobj, GLU_TESS_VERTEX_DATA, (GLU_TESS_CALLBACK) vertexCallback);
        gluTessCallback(_tobj, GLU_TESS_BEGIN_DATA,  (GLU_TESS_CALLBACK) beginCallback);
        gluTessCallback(_tobj, GLU_TESS_END_DATA,    (GLU_TESS_CALLBACK) endCallback);
        gluTessCallback(_tobj, GLU_TESS_COMBINE_DATA,(GLU_TESS_CALLBACK) combineCallback);
        gluTessCallback(_tobj, GLU_TESS_ERROR_DATA,  (GLU_TESS_CALLBACK) errorCallback);
    }
    _errorCode = 0;
    _index=0;
}

Tessellator::~Tessellator()
{
    reset();
    if (_tobj)
    {
        gluDeleteTess(_tobj);
    }
}

void Tessellator::beginTessellation()
{
    reset();

    if (_tobj)
    {
        gluTessProperty(_tobj, GLU_TESS_WINDING_RULE, _wtype);
        gluTessProperty(_tobj, GLU_TESS_BOUNDARY_ONLY, _boundaryOnly);

        if (tessNormal.length()>0.0) gluTessNormal(_tobj, tessNormal.x(), tessNormal.y(), tessNormal.z());

        gluTessBeginPolygon(_tobj,this);
    }
}

void Tessellator::beginContour()
{
    if (_tobj)
    {
        gluTessBeginContour(_tobj);
    }
}

void Tessellator::addVertex(osg::Vec3* vertex)
{
    if (_tobj)
    {
        if (vertex && vertex->valid())
        {
            Vec3d* data = new Vec3d;
            _coordData.push_back(data);
            (*data)._v[0]=(*vertex)[0];
            (*data)._v[1]=(*vertex)[1];
            (*data)._v[2]=(*vertex)[2];
            gluTessVertex(_tobj,data->_v,vertex);
        }
        else
        {
            OSG_INFO<<"Tessellator::addVertex("<<*vertex<<") detected NaN, ignoring vertex."<<std::endl;
        }
    }
}

void Tessellator::endContour()
{
    if (_tobj)
    {
        gluTessEndContour(_tobj);
    }
}

void Tessellator::endTessellation()
{
    if (_tobj)
    {
        gluTessEndPolygon(_tobj);

        if (_errorCode!=0)
        {
           const GLubyte *estring = gluErrorString((GLenum)_errorCode);
           OSG_WARN<<"Tessellation Error: "<<estring<< std::endl;
        }
    }
}

void Tessellator::reset()
{
    for (Vec3dList::iterator i = _coordData.begin(); i != _coordData.end(); ++i)
    {
        delete (*i);
    }

    // We need to also free the vertex list as well otherwise we are leaking...
    for (NewVertexList::iterator j = _newVertexList.begin(); j != _newVertexList.end(); ++j)
    {
        NewVertex& newVertex = (*j);
        delete newVertex._vpos;
        newVertex._vpos = NULL;
    }

    _coordData.clear();
    _newVertexList.clear();
    _primList.clear();
    _errorCode = 0;
}


class InsertNewVertices : public osg::ArrayVisitor
{
    public:

        float _f1,_f2,_f3,_f4;
        unsigned int _i1,_i2,_i3,_i4;

        InsertNewVertices(float f1,unsigned int i1,
                          float f2,unsigned int i2,
                          float f3,unsigned int i3,
                          float f4,unsigned int i4):
                            _f1(f1),_f2(f2),_f3(f3),_f4(f4),
                            _i1(i1),_i2(i2),_i3(i3),_i4(i4){}

        template <class ARRAY,class TYPE>
        void apply_imp(ARRAY& array,TYPE initialValue)
        {
            TYPE val = initialValue;
            if (_f1) val += static_cast<TYPE>(array[_i1] * _f1);
            if (_f2) val += static_cast<TYPE>(array[_i2] * _f2);
            if (_f3) val += static_cast<TYPE>(array[_i3] * _f3);
            if (_f4) val += static_cast<TYPE>(array[_i4] * _f4);

            array.push_back(val);
        }

        virtual void apply(osg::ByteArray& ba) { apply_imp(ba,GLbyte(0)); }
        virtual void apply(osg::ShortArray& ba) { apply_imp(ba,GLshort(0)); }
        virtual void apply(osg::IntArray& ba) { apply_imp(ba,GLint(0)); }
        virtual void apply(osg::UByteArray& ba) { apply_imp(ba,GLubyte(0)); }
        virtual void apply(osg::UShortArray& ba) { apply_imp(ba,GLushort(0)); }
        virtual void apply(osg::UIntArray& ba) { apply_imp(ba,GLuint(0)); }
        virtual void apply(osg::Vec4ubArray& ba) { apply_imp(ba,Vec4ub()); }
        virtual void apply(osg::FloatArray& ba) { apply_imp(ba,float(0)); }
        virtual void apply(osg::Vec2Array& ba) { apply_imp(ba,Vec2()); }
        virtual void apply(osg::Vec3Array& ba) { apply_imp(ba,Vec3()); }
        virtual void apply(osg::Vec4Array& ba) { apply_imp(ba,Vec4()); }

};

void Tessellator::retessellatePolygons(osg::Geometry &geom)
{
    // turn the contour list into primitives, a little like Tessellator does but more generally
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());

    if (!vertices || vertices->empty() || geom.getPrimitiveSetList().empty()) return;

    if (_ttype==TESS_TYPE_POLYGONS || _ttype==TESS_TYPE_DRAWABLE) _numberVerts=0; // 09.04.04 GWM reset Tessellator
    // the reset is needed by the flt loader which reuses a Tessellator for triangulating polygons.
    // as such it might be reset by other loaders/developers in future.
    _index=0; // reset the counter for indexed vertices
    _extraPrimitives = 0;
    if (!_numberVerts) {
        _numberVerts=geom.getVertexArray()->getNumElements();
        // save the contours for complex (winding rule) tessellations
        _Contours=geom.getPrimitiveSetList();
    }

    // now cut out vertex attributes added on any previous tessellation
    reduceArray(geom.getVertexArray(), _numberVerts);
    reduceArray(geom.getColorArray(), _numberVerts);
    reduceArray(geom.getNormalArray(), _numberVerts);
    reduceArray(geom.getFogCoordArray(), _numberVerts);
    for(unsigned int unit1=0;unit1<geom.getNumTexCoordArrays();++unit1)
    {
        reduceArray(geom.getTexCoordArray(unit1), _numberVerts);
    }

    // remove the existing primitives.
    unsigned int nprimsetoriginal= geom.getNumPrimitiveSets();
    if (nprimsetoriginal) geom.removePrimitiveSet(0, nprimsetoriginal);

    // the main difference from osgUtil::Tessellator for Geometry sets of multiple contours is that the begin/end tessellation
    // occurs around the whole set of contours.
    if (_ttype==TESS_TYPE_GEOMETRY) {
        beginTessellation();
    }
    // process all the contours into the Tessellator
    int noContours = _Contours.size();
    int currentPrimitive = 0;
    for(int primNo=0;primNo<noContours;++primNo)
    {
        osg::ref_ptr<osg::PrimitiveSet> primitive = _Contours[primNo].get();
        if (_ttype==TESS_TYPE_POLYGONS || _ttype==TESS_TYPE_DRAWABLE)
        { // this recovers the 'old' tessellation which just retessellates single polygons.
            if (primitive->getMode()==osg::PrimitiveSet::POLYGON || _ttype==TESS_TYPE_DRAWABLE)
            {

                if (primitive->getType()==osg::PrimitiveSet::DrawArrayLengthsPrimitiveType)
                {
                    osg::DrawArrayLengths* drawArrayLengths = static_cast<osg::DrawArrayLengths*>(primitive.get());
                    unsigned int first = drawArrayLengths->getFirst();
                    for(osg::DrawArrayLengths::iterator itr=drawArrayLengths->begin();
                        itr!=drawArrayLengths->end();
                        ++itr)
                    {
                        beginTessellation();
                            unsigned int last = first + *itr;
                            addContour(primitive->getMode(),first,last,vertices);
                            first = last;
                        endTessellation();
                        collectTessellation(geom, currentPrimitive);
                        currentPrimitive++;
                    }
                }
                else
                {
                    if (primitive->getNumIndices()>3) { // April 2005 gwm only retessellate "complex" polygons
                        beginTessellation();
                        addContour(primitive.get(), vertices);
                        endTessellation();
                        collectTessellation(geom, currentPrimitive);
                        currentPrimitive++;
                    } else { // April 2005 gwm triangles don't need to be retessellated
                        geom.addPrimitiveSet(primitive.get());
                    }
                }

            }
            else
            { // copy the contour primitive as it is not being tessellated
                geom.addPrimitiveSet(primitive.get());
            }
        } else {
            if (primitive->getMode()==osg::PrimitiveSet::POLYGON ||
                primitive->getMode()==osg::PrimitiveSet::QUADS ||
                primitive->getMode()==osg::PrimitiveSet::TRIANGLES ||
                primitive->getMode()==osg::PrimitiveSet::LINE_LOOP ||
                primitive->getMode()==osg::PrimitiveSet::QUAD_STRIP ||
                primitive->getMode()==osg::PrimitiveSet::TRIANGLE_FAN ||
                primitive->getMode()==osg::PrimitiveSet::TRIANGLE_STRIP)
            {
                addContour(primitive.get(), vertices);
            } else { // copy the contour primitive as it is not being tessellated
                // in this case points, lines or line_strip
                geom.addPrimitiveSet(primitive.get());
            }
        }
    }
    if (_ttype==TESS_TYPE_GEOMETRY) {
        endTessellation();

        collectTessellation(geom, 0);
    }
}

void Tessellator::addContour(GLenum mode, unsigned int first, unsigned int last, osg::Vec3Array* vertices)
{
    beginContour();

    unsigned int idx=0;
    unsigned int nperprim=0; // number of vertices per primitive
    if (mode==osg::PrimitiveSet::QUADS) nperprim=4;
    else if (mode==osg::PrimitiveSet::TRIANGLES) nperprim=3;

    unsigned int i;
    switch (mode)
    {
    case osg::PrimitiveSet::QUADS:
    case osg::PrimitiveSet::TRIANGLES:
    case osg::PrimitiveSet::POLYGON:
    case osg::PrimitiveSet::LINE_LOOP:
    case osg::PrimitiveSet::TRIANGLE_FAN:
        {
            for(i=first;i<last;++i, idx++)
            {
                addVertex(&((*vertices)[i]));
                if (nperprim>0 && i<last-1 && idx%nperprim==nperprim-1) {
                    endContour();
                    beginContour();
                }
            }
        }
        break;
    case osg::PrimitiveSet::QUAD_STRIP:
        { // always has an even number of vertices
            for(i=first;i<last;i+=2)
            { // 0,2,4...
                addVertex(&((*vertices)[i]));
            }
            for(i=last-1;i>=first;i-=2)
            { // ...5,3,1
                addVertex(&((*vertices)[i]));
            }
        }
        break;
    case osg::PrimitiveSet::TRIANGLE_STRIP:
        {
            for( i=first;i<last;i+=2)
            {// 0,2,4,...
                addVertex(&((*vertices)[i]));
            }
            for(i=((last-first)%2)?(last-2):(last-1) ;i>first&& i<last;i-=2)
            {
                addVertex(&((*vertices)[i]));
            }
        }
        break;
    default: // lines, points, line_strip
        {
            for(i=first;i<last;++i, idx++)
            {
                addVertex(&((*vertices)[i]));
                if (nperprim>0 && i<last-1 && idx%nperprim==nperprim-1) {
                    endContour();
                    beginContour();
                }
            }
        }
        break;
    }

    endContour();
}

void Tessellator::addContour(osg::PrimitiveSet* primitive, osg::Vec3Array* vertices)
{
    // adds a single primitive as a contour.
    unsigned int nperprim=0; // number of vertices per primitive
    if (primitive->getMode()==osg::PrimitiveSet::QUADS) nperprim=4;
    if (primitive->getMode()==osg::PrimitiveSet::TRIANGLES) nperprim=3;
    unsigned int idx=0;

    switch(primitive->getType())
    {
    case(osg::PrimitiveSet::DrawArraysPrimitiveType):
        {
            osg::DrawArrays* drawArray = static_cast<osg::DrawArrays*>(primitive);
            unsigned int first = drawArray->getFirst();
            unsigned int last = first+drawArray->getCount();
            addContour(primitive->getMode(),first,last,vertices);
            break;
         }
    case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
        {
            beginContour();
            osg::DrawElementsUByte* drawElements = static_cast<osg::DrawElementsUByte*>(primitive);
            for(osg::DrawElementsUByte::iterator indexItr=drawElements->begin();
                indexItr!=drawElements->end();
                ++indexItr, idx++)
            {
                addVertex(&((*vertices)[*indexItr]));
                if (nperprim>0 && indexItr!=drawElements->end() && idx%nperprim==nperprim-1) {
                    endContour();
                    beginContour();
                }
            }
            endContour();
            break;
        }
    case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
        {
            beginContour();
            osg::DrawElementsUShort* drawElements = static_cast<osg::DrawElementsUShort*>(primitive);
            for(osg::DrawElementsUShort::iterator indexItr=drawElements->begin();
                indexItr!=drawElements->end();
                ++indexItr, idx++)
            {
                addVertex(&((*vertices)[*indexItr]));
                if (nperprim>0 && indexItr!=drawElements->end() && idx%nperprim==nperprim-1) {
                    endContour();
                    beginContour();
                }
            }
            endContour();
            break;
        }
    case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
        {
            beginContour();
            osg::DrawElementsUInt* drawElements = static_cast<osg::DrawElementsUInt*>(primitive);
            for(osg::DrawElementsUInt::iterator indexItr=drawElements->begin();
                indexItr!=drawElements->end();
                ++indexItr, idx++)
            {
                addVertex(&((*vertices)[*indexItr]));
                if (nperprim>0 && indexItr!=drawElements->end() && idx%nperprim==nperprim-1) {
                    endContour();
                    beginContour();
                }
            }
            endContour();
            break;
        }
    default:
        OSG_NOTICE<<"Tessellator::addContour(primitive, vertices) : Primitive type "<<primitive->getType()<<" not handled"<<std::endl;
        break;
    }

}

void Tessellator::handleNewVertices(osg::Geometry& geom,VertexPtrToIndexMap &vertexPtrToIndexMap)
{
    if (!_newVertexList.empty())
    {

        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
        osg::Vec3Array* normals = NULL;
        if (osg::getBinding(geom.getNormalArray())==osg::Array::BIND_PER_VERTEX)
        {
            normals = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray());
        }

        typedef std::vector<osg::Array*> ArrayList;
        ArrayList arrays;

        if (osg::getBinding(geom.getColorArray())==osg::Array::BIND_PER_VERTEX)
        {
            arrays.push_back(geom.getColorArray());
        }

        if (osg::getBinding(geom.getSecondaryColorArray())==osg::Array::BIND_PER_VERTEX)
        {
            arrays.push_back(geom.getSecondaryColorArray());
        }

        if (osg::getBinding(geom.getFogCoordArray())==osg::Array::BIND_PER_VERTEX)
        {
            arrays.push_back(geom.getFogCoordArray());
        }

        osg::Geometry::ArrayList& tcal = geom.getTexCoordArrayList();
        for(osg::Geometry::ArrayList::iterator tcalItr=tcal.begin();
            tcalItr!=tcal.end();
            ++tcalItr)
        {
            if (tcalItr->valid())
            {
                arrays.push_back(tcalItr->get());
            }
        }

        // now add any new vertices that are required.
        for(NewVertexList::iterator itr=_newVertexList.begin();
            itr!=_newVertexList.end();
            ++itr)
        {
            NewVertex& newVertex = (*itr);
            osg::Vec3* vertex = newVertex._vpos;

            // assign vertex.
            vertexPtrToIndexMap[vertex]=vertices->size();
            vertices->push_back(*vertex);

            // assign normals
            if (normals)
            {
                osg::Vec3 norm(0.0f,0.0f,0.0f);
                if (newVertex._v1) norm += (*normals)[vertexPtrToIndexMap[newVertex._v1]] * newVertex._f1;
                if (newVertex._v2) norm += (*normals)[vertexPtrToIndexMap[newVertex._v2]] * newVertex._f2;
                if (newVertex._v3) norm += (*normals)[vertexPtrToIndexMap[newVertex._v3]] * newVertex._f3;
                if (newVertex._v4) norm += (*normals)[vertexPtrToIndexMap[newVertex._v4]] * newVertex._f4;
                norm.normalize();
                normals->push_back(norm);
            }

            if (!arrays.empty())
            {
                InsertNewVertices inv(newVertex._f1,vertexPtrToIndexMap[newVertex._v1],
                    newVertex._f2,vertexPtrToIndexMap[newVertex._v2],
                    newVertex._f3,vertexPtrToIndexMap[newVertex._v3],
                    newVertex._f4,vertexPtrToIndexMap[newVertex._v4]);

                // assign the rest of the attributes.
                for(ArrayList::iterator aItr=arrays.begin();
                    aItr!=arrays.end();
                    ++aItr)
                {
                    (*aItr)->accept(inv);
                }
            }
        }

    }

}

void Tessellator::begin(GLenum mode)
{
    _primList.push_back(new Prim(mode));
}

void Tessellator::vertex(osg::Vec3* vertex)
{
    if (!_primList.empty())
    {
        Prim* prim = _primList.back().get();
        prim->_vertices.push_back(vertex);

    }
}

void Tessellator::combine(osg::Vec3* vertex,void* vertex_data[4],GLfloat weight[4])
{
    _newVertexList.push_back(NewVertex(vertex,
                                    weight[0],(Vec3*)vertex_data[0],
                                     weight[1],(Vec3*)vertex_data[1],
                                     weight[2],(Vec3*)vertex_data[2],
                                     weight[3],(Vec3*)vertex_data[3]));
}

void Tessellator::end()
{
    // no need to do anything right now...
}

void Tessellator::error(GLenum errorCode)
{
    _errorCode = errorCode;
}

void CALLBACK Tessellator::beginCallback(GLenum which, void* userData)
{
    ((Tessellator*)userData)->begin(which);
}

void CALLBACK Tessellator::endCallback(void* userData)
{
    ((Tessellator*)userData)->end();
}

void CALLBACK Tessellator::vertexCallback(GLvoid *data, void* userData)
{
    ((Tessellator*)userData)->vertex((Vec3*)data);
}

void CALLBACK Tessellator::combineCallback(GLdouble coords[3], void* vertex_data[4],
                              GLfloat weight[4], void** outData,
                              void* userData)
{
    Vec3* newData = new osg::Vec3(coords[0],coords[1],coords[2]);
    *outData = newData;
    ((Tessellator*)userData)->combine(newData,vertex_data,weight);
}

void CALLBACK Tessellator::errorCallback(GLenum errorCode, void* userData)
{
    ((Tessellator*)userData)->error(errorCode);
}

void Tessellator::reduceArray(osg::Array * cold, const unsigned int nnu)
{ // shrinks size of array to N
    if (cold && cold->getNumElements()>nnu) {
        osg::Vec2Array* v2arr = NULL;
        osg::Vec3Array* v3arr = NULL;
        osg::Vec4Array* v4arr = NULL;
        switch (cold->getType()) {
        case osg::Array::Vec2ArrayType: {
            v2arr = dynamic_cast<osg::Vec2Array*>(cold);
            osg::Vec2Array::iterator itr=v2arr->begin()+nnu;
            (*v2arr).erase(itr, v2arr->end());
                                        }
            break;
        case osg::Array::Vec3ArrayType: {
            v3arr = dynamic_cast<osg::Vec3Array*>(cold);
            osg::Vec3Array::iterator itr=v3arr->begin()+nnu;
            (*v3arr).erase(itr, v3arr->end());
                                        }
            break;
        case osg::Array::Vec4ArrayType: {
            v4arr = dynamic_cast<osg::Vec4Array*>(cold);
            osg::Vec4Array::iterator itr=v4arr->begin()+nnu;
            (*v4arr).erase(itr, v4arr->end());
                                        }
            break;
        default: // should also handle:ArrayType' ByteArrayType' ShortArrayType' IntArrayType'
        // `UShortArrayType'  `UIntArrayType'  `Vec4ubArrayType'  `FloatArrayType'
            break;
        }
    }
}

void Tessellator::collectTessellation(osg::Geometry &geom, unsigned int /*originalIndex*/)
{
    if (geom.containsDeprecatedData()) geom.fixDeprecatedData();

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    VertexPtrToIndexMap vertexPtrToIndexMap;

    // populate the VertexPtrToIndexMap.
    for(unsigned int vi=0;vi<vertices->size();++vi)
    {
        vertexPtrToIndexMap[&((*vertices)[vi])] = vi;
    }

    handleNewVertices(geom, vertexPtrToIndexMap);

    // we don't properly handle per primitive and per primitive_set bindings yet
    // will need to address this soon. Robert Oct 2002.
    {
        osg::Vec3Array* normals = NULL; // GWM Sep 2002 - add normals for extra facets
        int iprim=0;
        if (osg::getBinding(geom.getNormalArray())==osg::Array::BIND_PER_PRIMITIVE_SET)
        {
            normals = dynamic_cast<osg::Vec3Array*>(geom.getNormalArray()); // GWM Sep 2002
        }
        // GWM Dec 2003 - needed to add colours for extra facets
        osg::Vec4Array* cols4 = NULL; // GWM Dec 2003 colours are vec4
        osg::Vec3Array* cols3 = NULL; // GWM Dec 2003 colours are vec3
        if (osg::getBinding(geom.getColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET)
        {
              Array* colours = geom.getColorArray(); // GWM Dec 2003 - need to duplicate face colours
              switch (colours->getType()) {
              case osg::Array::Vec4ArrayType:
                  cols4=dynamic_cast<osg::Vec4Array *> (colours);
                  break;
              case osg::Array::Vec3ArrayType:
                  cols3=dynamic_cast<osg::Vec3Array *> (colours);
                  break;
              default:
                  break;
              }

        }
        // GWM Dec 2003 - these holders need to go outside the loop to
        // retain the flat shaded colour &/or normal for each tessellated polygon
        osg::Vec3 norm(0.0f,0.0f,0.0f);
        osg::Vec4 primCol4(0.0f,0.0f,0.0f,1.0f);
        osg::Vec3 primCol3(0.0f,0.0f,0.0f);

        for(PrimList::iterator primItr=_primList.begin();
        primItr!=_primList.end();
        ++primItr, ++_index)
        {
              Prim* prim=primItr->get();
              int ntris=0;

              if(vertexPtrToIndexMap.size() <= 255)
              {
                  osg::DrawElementsUByte* elements = new osg::DrawElementsUByte(prim->_mode);
                  for(Prim::VecList::iterator vitr=prim->_vertices.begin();
                  vitr!=prim->_vertices.end();
                  ++vitr)
                {
                    elements->push_back(vertexPtrToIndexMap[*vitr]);
                }

                  // add to the drawn primitive list.
                  geom.addPrimitiveSet(elements);
                  ntris=elements->getNumIndices()/3;
              }
              else if(vertexPtrToIndexMap.size() > 255 && vertexPtrToIndexMap.size() <= 65535)
              {
                  osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(prim->_mode);
                  for(Prim::VecList::iterator vitr=prim->_vertices.begin();
                    vitr!=prim->_vertices.end();
                    ++vitr)
                  {
                    elements->push_back(vertexPtrToIndexMap[*vitr]);
                  }

                  // add to the drawn primitive list.
                  geom.addPrimitiveSet(elements);
                  ntris=elements->getNumIndices()/3;
              }
              else
              {
                  osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(prim->_mode);
                  for(Prim::VecList::iterator vitr=prim->_vertices.begin();
                    vitr!=prim->_vertices.end();
                    ++vitr)
                  {
                    elements->push_back(vertexPtrToIndexMap[*vitr]);
                  }

                  // add to the drawn primitive list.
                  geom.addPrimitiveSet(elements);
                  ntris=elements->getNumIndices()/3;
              }

              if (primItr==_primList.begin())
              {   // first primitive so collect primitive normal & colour.
                  if (normals) {
                    norm=(*normals)[iprim]; // GWM Sep 2002 the flat shaded normal
                  }
                  if (cols4) {
                      primCol4=(*cols4)[iprim]; // GWM Dec 2003 the flat shaded rgba colour
                    if (_index>=cols4->size()) {
                        cols4->push_back(primCol4); // GWM Dec 2003 add flat shaded colour for new facet
                    }
                  }
                  if (cols3) {
                      primCol3=(*cols3)[iprim]; // GWM Dec 2003 flat shaded rgb colour
                    if (_index>=cols3->size()) {
                        cols3->push_back(primCol3); // GWM Dec 2003 add flat shaded colour for new facet
                    }
                  }
              }
              else
              { // later primitives use same colour
                  if (normals)
                  {
                    normals->push_back(norm); // GWM Sep 2002 add flat shaded normal for new facet
                  }
                  if (cols4 && _index>=cols4->size()) {
                    cols4->push_back(primCol4); // GWM Dec 2003 add flat shaded colour for new facet
                  }
                  if (cols3 && _index>=cols3->size()) {
                    if (cols3) cols3->push_back(primCol3); // GWM Dec 2003 add flat shaded colour for new facet
                  }
                  if (prim->_mode==GL_TRIANGLES) {
                      if (osg::getBinding(geom.getNormalArray())==osg::Array::BIND_PER_PRIMITIVE_SET) { // need one per triangle? Not one per set.
                          for (int ii=1; ii<ntris; ii++) {
                              if (normals) normals->push_back(norm); // GWM Sep 2002 add flat shaded normal for new facet
                          }
                      }
                      if (osg::getBinding(geom.getColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET) { // need one per triangle? Not one per set.
                          for (int ii=1; ii<ntris; ii++) {
                              if (cols3 && _index>=cols3->size()) {
                                  if (cols3) cols3->push_back(primCol3);
                              }
                              if (cols4 && _index>=cols4->size()) {
                                  if (cols4) cols4->push_back(primCol4);
                              }
                              _index++;
                          }
                      }
                  }
                  //        OSG_WARN<<"Add: "<< iprim << std::endl;
              }
              iprim++; // GWM Sep 2002 count which normal we should use
        }
    }
}
