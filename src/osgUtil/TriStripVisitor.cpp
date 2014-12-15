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
#include <osg/Notify>
#include <osg/TriangleIndexFunctor>
#include <osg/io_utils>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>

#include <stdio.h>
#include <algorithm>
#include <map>
#include <iterator>

#include "tristripper/include/tri_stripper.h"

using namespace osg;
using namespace osgUtil;

typedef std::vector<unsigned int> IndexList;

class WriteValue : public osg::ConstValueVisitor
{
    public:
        WriteValue(std::ostream& o):_o(o) {}

        std::ostream& _o;

        virtual void apply(const GLbyte& v) { _o << v; }
        virtual void apply(const GLshort& v) { _o << v; }
        virtual void apply(const GLint& v) { _o << v; }
        virtual void apply(const GLushort& v) { _o << v; }
        virtual void apply(const GLubyte& v) { _o << v; }
        virtual void apply(const GLuint& v) { _o << v; }
        virtual void apply(const GLfloat& v) { _o << v; }
        virtual void apply(const Vec4ub& v) { _o << v; }
        virtual void apply(const Vec2& v) { _o << v; }
        virtual void apply(const Vec3& v) { _o << v; }
        virtual void apply(const Vec4& v) { _o << v; }

    protected:

        WriteValue& operator = (const WriteValue&) { return *this; }
};


struct VertexAttribComparitor
{
    VertexAttribComparitor(osg::Geometry& geometry)
    {
        add(geometry.getVertexArray());
        add(geometry.getNormalArray());
        add(geometry.getColorArray());
        add(geometry.getSecondaryColorArray());
        add(geometry.getFogCoordArray());
        unsigned int i;
        for(i=0;i<geometry.getNumTexCoordArrays();++i)
        {
            add(geometry.getTexCoordArray(i));
        }
        for(i=0;i<geometry.getNumVertexAttribArrays();++i)
        {
            add(geometry.getVertexAttribArray(i));
        }
    }

    void add(osg::Array* array)
    {
        if (array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            for(ArrayList::const_iterator itr=_arrayList.begin();
                itr!=_arrayList.end();
                ++itr)
            {
                if ((*itr) == array)
                    return;
            }
            _arrayList.push_back(array);
        }
    }

    typedef std::vector<osg::Array*> ArrayList;

    ArrayList _arrayList;

    bool operator() (unsigned int lhs, unsigned int rhs) const
    {
        for(ArrayList::const_iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            int compare = (*itr)->compare(lhs,rhs);
            if (compare==-1) return true;
            if (compare==1) return false;
        }
        return false;
    }

    int compare(unsigned int lhs, unsigned int rhs)
    {
        for(ArrayList::iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            int compare = (*itr)->compare(lhs,rhs);
            if (compare==-1) return -1;
            if (compare==1) return 1;
        }
//
//         WriteValue wv(std::cout);
//
//         std::cout<<"Values equal"<<std::endl;
//         for(ArrayList::iterator itr=_arrayList.begin();
//             itr!=_arrayList.end();
//             ++itr)
//         {
//             std::cout<<"   lhs["<<lhs<<"]="; (*itr)->accept(lhs,wv);
//             std::cout<<"   rhs["<<rhs<<"]="; (*itr)->accept(rhs,wv);
//             std::cout<<std::endl;
//         }


        return 0;
    }

    void accept(osg::ArrayVisitor& av)
    {
        for(ArrayList::iterator itr=_arrayList.begin();
            itr!=_arrayList.end();
            ++itr)
        {
            (*itr)->accept(av);
        }
    }

protected:

    VertexAttribComparitor& operator = (const VertexAttribComparitor&) { return *this; }

};

class RemapArray : public osg::ArrayVisitor
{
    public:
        RemapArray(const IndexList& remapping):_remapping(remapping) {}

        const IndexList& _remapping;

        template<class T>
        inline void remap(T& array)
        {
            for(unsigned int i=0;i<_remapping.size();++i)
            {
                if (i!=_remapping[i])
                {
                    array[i] = array[_remapping[i]];
                }
            }
            array.erase(array.begin()+_remapping.size(),array.end());
        }

        virtual void apply(osg::Array&) {}
        virtual void apply(osg::ByteArray& array) { remap(array); }
        virtual void apply(osg::ShortArray& array) { remap(array); }
        virtual void apply(osg::IntArray& array) { remap(array); }
        virtual void apply(osg::UByteArray& array) { remap(array); }
        virtual void apply(osg::UShortArray& array) { remap(array); }
        virtual void apply(osg::UIntArray& array) { remap(array); }
        virtual void apply(osg::Vec4ubArray& array) { remap(array); }
        virtual void apply(osg::FloatArray& array) { remap(array); }
        virtual void apply(osg::Vec2Array& array) { remap(array); }
        virtual void apply(osg::Vec3Array& array) { remap(array); }
        virtual void apply(osg::Vec4Array& array) { remap(array); }

protected:

        RemapArray& operator = (const RemapArray&) { return *this; }
};

struct MyTriangleOperator
{

    IndexList                                _remapIndices;
    triangle_stripper::indices _in_indices;

    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        if (_remapIndices.empty())
        {
            _in_indices.push_back(p1);
            _in_indices.push_back(p2);
            _in_indices.push_back(p3);
        }
        else
        {
            _in_indices.push_back(_remapIndices[p1]);
            _in_indices.push_back(_remapIndices[p2]);
            _in_indices.push_back(_remapIndices[p3]);
        }
    }

};
typedef osg::TriangleIndexFunctor<MyTriangleOperator> MyTriangleIndexFunctor;

void TriStripVisitor::stripify(Geometry& geom)
{
    if (geom.containsDeprecatedData()) geom.fixDeprecatedData();

    if (osg::getBinding(geom.getNormalArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getSecondaryColorArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    if (osg::getBinding(geom.getFogCoordArray())==osg::Array::BIND_PER_PRIMITIVE_SET) return;

    // no point tri stripping if we don't have enough vertices.
    if (!geom.getVertexArray() || geom.getVertexArray()->getNumElements()<3) return;

    // check for the existence of surface primitives
    unsigned int numSurfacePrimitives = 0;
    unsigned int numNonSurfacePrimitives = 0;
    Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    Geometry::PrimitiveSetList::iterator itr;
    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                ++numSurfacePrimitives;
                break;
            default:
                ++numNonSurfacePrimitives;
                break;

        }
    }

    // nothitng to tri strip leave.
    if (!numSurfacePrimitives) return;

    // compute duplicate vertices

    typedef std::vector<unsigned int> IndexList;
    unsigned int numVertices = geom.getVertexArray()->getNumElements();
    IndexList indices(numVertices);
    unsigned int i,j;
    for(i=0;i<numVertices;++i)
    {
        indices[i] = i;
    }

    VertexAttribComparitor arrayComparitor(geom);
    std::sort(indices.begin(),indices.end(),arrayComparitor);

    unsigned int lastUnique = 0;
    unsigned int numUnique = 1;
    unsigned int numDuplicate = 0;
    for(i=1;i<numVertices;++i)
    {
        if (arrayComparitor.compare(indices[lastUnique],indices[i])==0)
        {
            //std::cout<<"  found duplicate "<<indices[lastUnique]<<" and "<<indices[i]<<std::endl;
            ++numDuplicate;
        }
        else
        {
            //std::cout<<"  unique "<<indices[i]<<std::endl;
            lastUnique = i;
            ++numUnique;
        }

    }
//     std::cout<<"  Number of duplicates "<<numDuplicate<<std::endl;
//     std::cout<<"  Number of unique "<<numUnique<<std::endl;
//     std::cout<<"  Total number of vertices required "<<numUnique<<" vs original "<<numVertices<<std::endl;
//     std::cout<<"  % size "<<(float)numUnique/(float)numVertices*100.0f<<std::endl;

    IndexList remapDuplicatesToOrignals(numVertices);
    lastUnique = 0;
    for(i=1;i<numVertices;++i)
    {
        if (arrayComparitor.compare(indices[lastUnique],indices[i])!=0)
        {
            // found a new vertex entry, so previous run of duplicates needs
            // to be put together.
            unsigned int min_index = indices[lastUnique];
            for(j=lastUnique+1;j<i;++j)
            {
                min_index = osg::minimum(min_index,indices[j]);
            }
            for(j=lastUnique;j<i;++j)
            {
                remapDuplicatesToOrignals[indices[j]]=min_index;
            }
            lastUnique = i;
        }

    }
    unsigned int min_index = indices[lastUnique];
    for(j=lastUnique+1;j<i;++j)
    {
        min_index = osg::minimum(min_index,indices[j]);
    }
    for(j=lastUnique;j<i;++j)
    {
        remapDuplicatesToOrignals[indices[j]]=min_index;
    }


    // copy the arrays.
    IndexList finalMapping(numVertices);
    IndexList copyMapping;
    copyMapping.reserve(numUnique);
    unsigned int currentIndex=0;
    for(i=0;i<numVertices;++i)
    {
        if (remapDuplicatesToOrignals[i]==i)
        {
            finalMapping[i] = currentIndex;
            copyMapping.push_back(i);
            currentIndex++;
        }
    }

    for(i=0;i<numVertices;++i)
    {
        if (remapDuplicatesToOrignals[i]!=i)
        {
            finalMapping[i] = finalMapping[remapDuplicatesToOrignals[i]];
        }
    }


    MyTriangleIndexFunctor taf;
    taf._remapIndices.swap(finalMapping);

    Geometry::PrimitiveSetList new_primitives;
    new_primitives.reserve(primitives.size());

    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                (*itr)->accept(taf);
                break;
            default:
                new_primitives.push_back(*itr);
                break;

        }
    }

    float minimum_ratio_of_indices_to_unique_vertices = 1;
    float ratio_of_indices_to_unique_vertices = ((float)taf._in_indices.size()/(float)numUnique);

    OSG_INFO<<"TriStripVisitor::stripify(Geometry&): Number of indices"<<taf._in_indices.size()<<" numUnique"<< numUnique << std::endl;
    OSG_INFO<<"TriStripVisitor::stripify(Geometry&):     ratio indices/numUnique"<< ratio_of_indices_to_unique_vertices << std::endl;

    // only tri strip if there is point in doing so.
    if (!taf._in_indices.empty() && ratio_of_indices_to_unique_vertices>=minimum_ratio_of_indices_to_unique_vertices)
    {
        OSG_INFO<<"TriStripVisitor::stripify(Geometry&):     doing tri strip"<< std::endl;

        unsigned int in_numVertices = 0;
        for(triangle_stripper::indices::iterator itr=taf._in_indices.begin();
            itr!=taf._in_indices.end();
            ++itr)
        {
            if (*itr>in_numVertices) in_numVertices=*itr;
        }
        // the largest indice is in_numVertices, but indices start at 0
        // so increment to give to the corrent number of verticies.
        ++in_numVertices;

        // remap any shared vertex attributes
        RemapArray ra(copyMapping);
        arrayComparitor.accept(ra);

        triangle_stripper::tri_stripper stripifier(taf._in_indices);
        stripifier.SetCacheSize(_cacheSize);
        stripifier.SetMinStripSize(_minStripSize);

        triangle_stripper::primitive_vector outPrimitives;
        stripifier.Strip(&outPrimitives);
        if (outPrimitives.empty())
        {
            OSG_WARN<<"Error: TriStripVisitor::stripify(Geometry& geom) failed."<<std::endl;
            return;
        }

        triangle_stripper::primitive_vector::iterator pitr;
        if (_generateFourPointPrimitivesQuads)
        {
            OSG_INFO<<"Collecting all quads"<<std::endl;

            typedef triangle_stripper::primitive_vector::iterator prim_iterator;
            typedef std::multimap<unsigned int,prim_iterator> QuadMap;
            QuadMap quadMap;

            // pick out quads and place them in the quadMap, and also look for the max
            for(pitr=outPrimitives.begin();
                pitr!=outPrimitives.end();
                ++pitr)
            {
                if (pitr->Indices.size()==4)
                {
                    std::swap(pitr->Indices[2],pitr->Indices[3]);
                    unsigned int minValue = *(std::max_element(pitr->Indices.begin(),pitr->Indices.end()));
                    quadMap.insert(QuadMap::value_type(minValue,pitr));
                }
            }


            // handle the quads
            if (!quadMap.empty())
            {
                IndexList indices;
                indices.reserve(4*quadMap.size());

                // adds all the quads into the quad primitive, in ascending order
                // and the QuadMap stores the quad's in ascending order.
                for(QuadMap::iterator qitr=quadMap.begin();
                    qitr!=quadMap.end();
                    ++qitr)
                {
                    pitr = qitr->second;

                    unsigned int min_pos = 0;
                    for(i=1;i<4;++i)
                    {
                        if (pitr->Indices[min_pos]>pitr->Indices[i])
                            min_pos = i;
                    }
                    indices.push_back(pitr->Indices[min_pos]);
                    indices.push_back(pitr->Indices[(min_pos+1)%4]);
                    indices.push_back(pitr->Indices[(min_pos+2)%4]);
                    indices.push_back(pitr->Indices[(min_pos+3)%4]);
                }

                unsigned int maxValue = *(std::max_element(indices.begin(),indices.end()));
                if (maxValue>=65536)
                {
                    osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_QUADS);
                    std::copy(indices.begin(),indices.end(),std::back_inserter(*elements));
                    new_primitives.push_back(elements);
                }
                else
                {
                    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(GL_QUADS);
                    std::copy(indices.begin(),indices.end(),std::back_inserter(*elements));
                    new_primitives.push_back(elements);
                }
            }
        }

        // handle non quad primitives
        for(pitr=outPrimitives.begin();
            pitr!=outPrimitives.end();
            ++pitr)
        {
            if (!_generateFourPointPrimitivesQuads || pitr->Indices.size()!=4)
            {
                unsigned int maxValue = *(std::max_element(pitr->Indices.begin(),pitr->Indices.end()));
                if (maxValue>=65536)
                {
                    osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(pitr->Type);
                    elements->reserve(pitr->Indices.size());
                    std::copy(pitr->Indices.begin(),pitr->Indices.end(),std::back_inserter(*elements));
                    new_primitives.push_back(elements);
                }
                else
                {
                    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(pitr->Type);
                    elements->reserve(pitr->Indices.size());
                    std::copy(pitr->Indices.begin(),pitr->Indices.end(),std::back_inserter(*elements));
                    new_primitives.push_back(elements);
                }
            }
        }

        if(_mergeTriangleStrips) {
            mergeTriangleStrips(new_primitives);
        }

        geom.setPrimitiveSetList(new_primitives);

        #if 0
        // debugging code for indentifying the tri-strips.
                osg::Vec4Array* colors = new osg::Vec4Array(new_primitives.size());
                for(i=0;i<colors->size();++i)
                {
                    (*colors)[i].set(((float)rand()/(float)RAND_MAX),
                                     ((float)rand()/(float)RAND_MAX),
                                     ((float)rand()/(float)RAND_MAX),
                                     1.0f);
                }
                geom.setColorArray(colors);
                geom.setColorBinding(osg::Array::BIND_PER_PRIMITIVE_SET);
        #endif
    }
    else
    {
        OSG_INFO<<"TriStripVisitor::stripify(Geometry&):     not doing tri strip *****************"<< std::endl;
    }

}


void TriStripVisitor::mergeTriangleStrips(osg::Geometry::PrimitiveSetList& primitives)
{
    int nbtristrip = 0;
    int nbtristripVertexes = 0;

    for (unsigned int i = 0; i < primitives.size(); ++ i)
    {
        osg::PrimitiveSet* ps = primitives[i].get();
        osg::DrawElements* de = ps->getDrawElements();
        if (de && de->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
        {
            ++ nbtristrip;
            nbtristripVertexes += de->getNumIndices();
        }
    }

    if (nbtristrip > 0) {
        osg::notify(osg::NOTICE) << "found " << nbtristrip << " tristrip, "
                                 << "total indices " << nbtristripVertexes
                                 << " should result to " << nbtristripVertexes + nbtristrip * 2
                                 << " after connection" << std::endl;

        osg::DrawElementsUInt* ndw = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP);
        for (unsigned int i = 0; i < primitives.size(); ++ i)
        {
            osg::PrimitiveSet* ps = primitives[i].get();
            if (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)
            {
                osg::DrawElements* de = ps->getDrawElements();
                if (de)
                {
                    // if connection needed insert degenerate triangles
                    if (ndw->getNumIndices() != 0 && ndw->back() != de->getElement(0))
                    {
                        // duplicate last vertex
                        ndw->addElement(ndw->back());
                        // insert first vertex of next strip
                        ndw->addElement(de->getElement(0));
                    }

                    if (ndw->getNumIndices() % 2 != 0 ) {
                        // add a dummy vertex to reverse the strip
                        ndw->addElement(de->getElement(0));
                    }

                    for (unsigned int j = 0; j < de->getNumIndices(); j++) {
                        ndw->addElement(de->getElement(j));
                    }
                }
            }
        }

        for (int i = primitives.size() - 1 ; i >= 0 ; -- i)
        {
            osg::PrimitiveSet* ps = primitives[i].get();
            // remove null primitive sets and all primitives that have been merged
            // (i.e. all TRIANGLE_STRIP DrawElements)
            if (!ps || (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP))
            {
                primitives.erase(primitives.begin() + i);
            }
        }
        primitives.insert(primitives.begin(), ndw);
    }
}

void TriStripVisitor::stripify()
{
    for(GeometryList::iterator itr=_geometryList.begin();
        itr!=_geometryList.end();
        ++itr)
    {
        stripify(*(*itr));

        // osgUtil::SmoothingVisitor sv;
        // sv.smooth(*(*itr));
    }
}

void TriStripVisitor::apply(Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); ++i )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) _geometryList.insert(geom);
    }
}
