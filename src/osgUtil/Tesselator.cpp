/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osgUtil/Tesselator>

using namespace osg;
using namespace osgUtil;

Tesselator::Tesselator()
{
    _tobj = 0;
    _errorCode = 0;
}

Tesselator::~Tesselator()
{
    reset();
}

void Tesselator::beginTesselation()
{
    reset();

    if (!_tobj) _tobj = gluNewTess();
    
    gluTessCallback(_tobj, GLU_TESS_VERTEX_DATA, (GLvoid (CALLBACK*)()) vertexCallback);
    gluTessCallback(_tobj, GLU_TESS_BEGIN_DATA,  (GLvoid (CALLBACK*)()) beginCallback);
    gluTessCallback(_tobj, GLU_TESS_END_DATA,    (GLvoid (CALLBACK*)()) endCallback);
    gluTessCallback(_tobj, GLU_TESS_COMBINE_DATA,(GLvoid (CALLBACK*)()) combineCallback);
    gluTessCallback(_tobj, GLU_TESS_ERROR_DATA,  (GLvoid (CALLBACK*)()) errorCallback);

    gluTessBeginPolygon(_tobj,this);
}    
    
void Tesselator::beginContour()
{
    if (_tobj)
    {
        gluTessBeginContour(_tobj);
    }
}
      
void Tesselator::addVertex(osg::Vec3* vertex)
{
    if (_tobj)
    {
        Vec3d* data = new Vec3d;
        _coordData.push_back(data);
        (*data)._v[0]=(*vertex)[0];
        (*data)._v[1]=(*vertex)[1];
        (*data)._v[2]=(*vertex)[2];
        gluTessVertex(_tobj,data->_v,vertex);
    }
}

void Tesselator::endContour()
{
    if (_tobj)
    {
        gluTessEndContour(_tobj);
    }
}

void Tesselator::endTesselation()
{
    if (_tobj)
    {
        gluTessEndPolygon(_tobj);
        gluDeleteTess(_tobj);
        _tobj = 0;
        
        if (_errorCode!=0)
        {
           const GLubyte *estring = gluErrorString((GLenum)_errorCode);
           osg::notify(osg::WARN)<<"Tessellation Error: "<<estring<< std::endl;
        }
    }
}

void Tesselator::reset()
{
    if (_tobj)
    {
        gluDeleteTess(_tobj);
        _tobj = 0;
    }
    for (Vec3dList::iterator i = _coordData.begin(); i != _coordData.end(); ++i) {
      delete (*i);
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
        virtual void apply(osg::UByte4Array& ba) { apply_imp(ba,UByte4()); }
        virtual void apply(osg::FloatArray& ba) { apply_imp(ba,float(0)); }
        virtual void apply(osg::Vec2Array& ba) { apply_imp(ba,Vec2()); }
        virtual void apply(osg::Vec3Array& ba) { apply_imp(ba,Vec3()); }
        virtual void apply(osg::Vec4Array& ba) { apply_imp(ba,Vec4()); }

};
            

void Tesselator::retesselatePolygons(osg::Geometry& geom)
{
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geom.getVertexArray());
    if (!vertices || vertices->empty() || geom.getPrimitiveSetList().empty()) return;


    // we currently don't handle geometry which use indices...
    if (geom.getVertexIndices() ||
        geom.getNormalIndices() ||
        geom.getColorIndices() ||
        geom.getSecondaryColorIndices() ||
        geom.getFogCoordIndices()) return;
        
    // not even text coord indices don't handle geometry which use indices...
    for(unsigned int unit=0;unit<geom.getNumTexCoordArrays();++unit)
    {
        if (geom.getTexCoordIndices(unit)) return;
    }

    // process the primitives 
    int noPrimitiveAtStart = geom.getPrimitiveSetList().size();
    for(int primNo=0;primNo<noPrimitiveAtStart;++primNo)
    {
        osg::PrimitiveSet* primitive = geom.getPrimitiveSetList()[primNo].get();
        if (primitive->getMode()==osg::PrimitiveSet::POLYGON && primitive->getNumIndices()>3)
        {
            beginTesselation();
            beginContour();

            switch(primitive->getType())
            {
                case(osg::PrimitiveSet::DrawArraysPrimitiveType):
                {
                    osg::DrawArrays* drawArray = static_cast<osg::DrawArrays*>(primitive);
                    unsigned int first = drawArray->getFirst(); 
                    unsigned int last = first+drawArray->getCount();
                    for(unsigned int i=first;i<last;++i)
                    {
                        addVertex(&((*vertices)[i]));
                    }
                    break;
                }
                case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    osg::DrawElementsUByte* drawElements = static_cast<osg::DrawElementsUByte*>(primitive);
                    for(osg::DrawElementsUByte::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    osg::DrawElementsUShort* drawElements = static_cast<osg::DrawElementsUShort*>(primitive);
                    for(osg::DrawElementsUShort::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    osg::DrawElementsUInt* drawElements = static_cast<osg::DrawElementsUInt*>(primitive);
                    for(osg::DrawElementsUInt::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                default:
                    break;
            }
            
            endContour();
            endTesselation();
            
            typedef std::map<osg::Vec3*,unsigned int> VertexPtrToIndexMap;
            VertexPtrToIndexMap vertexPtrToIndexMap;
            
            // populate the VertexPtrToIndexMap.
            for(unsigned int vi=0;vi<vertices->size();++vi)
            {
                vertexPtrToIndexMap[&((*vertices)[vi])] = vi;
            }
            
            if (!_newVertexList.empty())
            {

                osg::Vec3Array* normals = NULL;
                if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)
                {
                    normals = geom.getNormalArray();
                }

                typedef std::vector<osg::Array*> ArrayList;
                ArrayList arrays;
    
                if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)
                {
                    arrays.push_back(geom.getColorArray());
                }
                
                if (geom.getSecondaryColorBinding()==osg::Geometry::BIND_PER_VERTEX)
                {
                    arrays.push_back(geom.getSecondaryColorArray());
                }

                if (geom.getFogCoordBinding()==osg::Geometry::BIND_PER_VERTEX)
                {
                    arrays.push_back(geom.getFogCoordArray());
                }

                osg::Geometry::ArrayList& tcal = geom.getTexCoordArrayList();
                for(osg::Geometry::ArrayList::iterator tcalItr=tcal.begin();
                    tcalItr!=tcal.end();
                    ++tcalItr)
                {
                    if (tcalItr->array.valid()) 
                    {
                        arrays.push_back(tcalItr->array.get());
                    }
                }

                // now add any new vertices that are required.
                for(NewVertexList::iterator itr=_newVertexList.begin();
                    itr!=_newVertexList.end();
                    ++itr)
                {
                    osg::Vec3* vertex = itr->first;
                    NewVertex& newVertex = itr->second;

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
            
            
            // we don't properly handle per primitive and per primitive bindings yet
            // will need to address this soon. Robert Oct 2002.
            {
                osg::Vec3Array* normals = NULL; // GWM Sep 2002 - add normals for extra facets
                int iprim=0;
                if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE ||
                    geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE_SET)
                {
                    normals = geom.getNormalArray(); // GWM Sep 2002
                }
                for(PrimList::iterator primItr=_primList.begin();
                primItr!=_primList.end();
                ++primItr)
                {
                    Prim* prim = primItr->get();
                    osg::Vec3 norm(0.0f,0.0f,0.0f);
                    
                    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(prim->_mode);
                    for(Prim::VecList::iterator vitr=prim->_vertices.begin();
                    vitr!=prim->_vertices.end();
                    ++vitr)
                    {
                        elements->push_back(vertexPtrToIndexMap[*vitr]);
                    }
                    
                    if (primItr==_primList.begin()) 
                    {
                        // first new primitive so overwrite the previous polygon.
                        geom.getPrimitiveSetList()[primNo] = elements;                    
                        if (normals) {
                            norm=(*normals)[iprim]; // GWM Sep 2002 the flat shaded normal
                        }
                    }
                    else
                    {
                        // subsequence primitives add to the back of the primitive list.
                        geom.addPrimitiveSet(elements);
                        if (normals) normals->push_back(norm); // GWM Sep 2002 add flat shaded normal for new facet
                    }
                    iprim++; // GWM Sep 2002 count which normal we should use
                }
            }
            
        }
        
    }
}

void Tesselator::begin(GLenum mode)
{
    _primList.push_back(new Prim(mode));
}

void Tesselator::vertex(osg::Vec3* vertex)
{
    if (!_primList.empty())
    {
        Prim* prim = _primList.back().get();
        prim->_vertices.push_back(vertex);

    }
}

void Tesselator::combine(osg::Vec3* vertex,void* vertex_data[4],GLfloat weight[4])
{
    _newVertexList[vertex]=NewVertex(weight[0],(Vec3*)vertex_data[0],
                                     weight[1],(Vec3*)vertex_data[1],
                                     weight[2],(Vec3*)vertex_data[2],
                                     weight[3],(Vec3*)vertex_data[3]);
}

void Tesselator::end()
{
    // no need to do anything right now...
}

void Tesselator::error(GLenum errorCode)
{
    _errorCode = errorCode;
}

void CALLBACK Tesselator::beginCallback(GLenum which, void* userData)
{
    ((Tesselator*)userData)->begin(which);
}

void CALLBACK Tesselator::endCallback(void* userData)
{
    ((Tesselator*)userData)->end();
}

void CALLBACK Tesselator::vertexCallback(GLvoid *data, void* userData)
{
    ((Tesselator*)userData)->vertex((Vec3*)data);
}

void CALLBACK Tesselator::combineCallback(GLdouble coords[3], void* vertex_data[4],
                              GLfloat weight[4], void** outData,
                              void* userData)
{
    Vec3* newData = new osg::Vec3(coords[0],coords[1],coords[2]);
    *outData = newData;
    ((Tesselator*)userData)->combine(newData,vertex_data,weight);
}

void CALLBACK Tesselator::errorCallback(GLenum errorCode, void* userData)
{
    ((Tesselator*)userData)->error(errorCode);
}
