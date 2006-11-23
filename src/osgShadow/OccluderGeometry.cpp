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

#include <osgShadow/OccluderGeometry>

#include <osg/Notify>
#include <osg/Geode>
#include <osg/io_utils>
#include <osg/TriangleFunctor>
#include <osg/TriangleIndexFunctor>
#include <osg/GL>
#include <osg/Timer>

#include <algorithm>

using namespace osgShadow;

OccluderGeometry::OccluderGeometry()
{
}

OccluderGeometry::OccluderGeometry(const OccluderGeometry& oc, const osg::CopyOp& copyop):
    osg::Drawable(oc,copyop)
{
    
}


class CollectOccludersVisitor : public osg::NodeVisitor
{
public:
    CollectOccludersVisitor(OccluderGeometry* oc, osg::Matrix* matrix, float ratio):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _oc(oc),
        _ratio(ratio)
        {
            if (matrix) pushMatrix(*matrix);
        }
        
    void apply(osg::Node& node)
    {
        if (node.getStateSet()) pushState(node.getStateSet());
        
        traverse(node);

        if (node.getStateSet()) popState();
    }
    
    void apply(osg::Transform& transform)
    {
        if (transform.getStateSet()) pushState(transform.getStateSet());
        
        osg::Matrix matrix;
        if (!_matrixStack.empty()) matrix = _matrixStack.back();
        
        transform.computeLocalToWorldMatrix(matrix,this);
        
        pushMatrix(matrix);
        
        traverse(transform);
        
        popMatrix();

        if (transform.getStateSet()) popState();
    }
    
    void apply(osg::Geode& geode)
    {
        if (geode.getStateSet()) pushState(geode.getStateSet());

        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            osg::Drawable* drawable = geode.getDrawable(i);

            if (drawable->getStateSet()) pushState(drawable->getStateSet());
            
            apply(geode.getDrawable(i));

            if (drawable->getStateSet()) popState();
        }
        
        if (geode.getStateSet()) popState();
    }
    
    void pushState(osg::StateSet* stateset)
    {
        osg::StateAttribute::GLModeValue prevBlendModeValue = _blendModeStack.empty() ? osg::StateAttribute::GLModeValue(osg::StateAttribute::INHERIT) : _blendModeStack.back();
        osg::StateAttribute::GLModeValue newBlendModeValue = stateset->getMode(GL_BLEND);
        
        if (!(newBlendModeValue & osg::StateAttribute::PROTECTED) && 
             (prevBlendModeValue & osg::StateAttribute::OVERRIDE) )
        {
            newBlendModeValue = prevBlendModeValue;
        }
        
        _blendModeStack.push_back(newBlendModeValue);
    }
    
    void popState()
    {
        _blendModeStack.pop_back();
    }
    
    void pushMatrix(osg::Matrix& matrix)
    {
        _matrixStack.push_back(matrix);
    }
    
    void popMatrix()
    {
        _matrixStack.pop_back();
    }

    void apply(osg::Drawable* drawable)
    {
        osg::StateAttribute::GLModeValue blendModeValue = _blendModeStack.empty() ? osg::StateAttribute::GLModeValue(osg::StateAttribute::INHERIT) : _blendModeStack.back();
        if (blendModeValue & osg::StateAttribute::ON)
        {
            // osg::notify(osg::NOTICE)<<"Ignoring transparent drawable."<<std::endl;
            return;
        }
        
        _oc->processGeometry(drawable, (_matrixStack.empty() ? 0 : &_matrixStack.back()), _ratio);
        
    }
    
protected:
    
    OccluderGeometry* _oc;
    
    typedef std::vector<osg::Matrix> MatrixStack;
    typedef std::vector<osg::StateAttribute::GLModeValue> ModeStack;
    
    float           _ratio;
    MatrixStack     _matrixStack;
    ModeStack       _blendModeStack;
    
};

void OccluderGeometry::computeOccluderGeometry(osg::Node* subgraph, osg::Matrix* matrix, float sampleRatio)
{
    osg::notify(osg::NOTICE)<<"computeOccluderGeometry(osg::Node* subgraph, float sampleRatio)"<<std::endl;

    osg::Timer_t startTick = osg::Timer::instance()->tick();
    
    CollectOccludersVisitor cov(this, matrix, sampleRatio);
    subgraph->accept(cov);
    
    setUpInternalStructures();

    osg::Timer_t endTick = osg::Timer::instance()->tick();

    osg::notify(osg::NOTICE)<<"done in "<<osg::Timer::instance()->delta_m(startTick, endTick)<<" ms"<<std::endl;
}

void OccluderGeometry::computeOccluderGeometry(osg::Drawable* drawable, osg::Matrix* matrix, float sampleRatio)
{
    processGeometry(drawable, matrix, sampleRatio);

    setUpInternalStructures();
}

struct TriangleCollector
{
    OccluderGeometry::Vec3List* _vertices;
    OccluderGeometry::UIntList* _triangleIndices;
    osg::Matrix*                _matrix;

    typedef std::vector<const osg::Vec3*> VertexPointers;
    VertexPointers _vertexPointers;
    
    OccluderGeometry::Vec3List _tempoaryTriangleVertices;

    TriangleCollector():_matrix(0) { }

    void set(OccluderGeometry::Vec3List* vertices, OccluderGeometry::UIntList* triangleIndices, osg::Matrix* matrix)
    {
        _vertices = vertices;
        _triangleIndices = triangleIndices;
        _matrix = matrix;
    }


    //   bool intersect(const Vec3& v1,const Vec3& v2,const Vec3& v3,float& r)
    inline void operator () (const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3, bool treatVertexDataAsTemporary)
    {
        if (treatVertexDataAsTemporary)
        {
            // osg::notify(osg::NOTICE)<<"Triangle temp ("<<v1<<") ("<<v2<<") ("<<v3<<")"<<std::endl;
            _tempoaryTriangleVertices.push_back(v1);
            _tempoaryTriangleVertices.push_back(v2);
            _tempoaryTriangleVertices.push_back(v3);
            
        }
        else
        {
            // osg::notify(osg::NOTICE)<<"Triangle ("<<v1<<") ("<<v2<<") ("<<v3<<")"<<std::endl;
            _vertexPointers.push_back(&v1);
            _vertexPointers.push_back(&v2);
            _vertexPointers.push_back(&v3);
        }

    }
    
    void copyToLocalData()
    {
        if ((_vertexPointers.size()+_tempoaryTriangleVertices.size())<3) return;
    
    
        const osg::Vec3* minVertex = _vertexPointers.empty() ? 0 : _vertexPointers.front();
        const osg::Vec3* maxVertex = _vertexPointers.empty() ? 0 : _vertexPointers.front();
    
        VertexPointers::iterator itr;
        for(itr = _vertexPointers.begin();
            itr != _vertexPointers.end();
            ++itr)
        {
            if (*itr < minVertex) minVertex = *itr;
            if (*itr > maxVertex) maxVertex = *itr;
        }

        unsigned int base = _vertices->size();
        unsigned int numberNewVertices = _vertexPointers.empty() ? 0 : (maxVertex - minVertex) + 1;
        
        // osg::notify(osg::NOTICE)<<"base = "<<base<<" numberNewVertices="<<numberNewVertices<<std::endl;

        _vertices->resize(base + numberNewVertices + _tempoaryTriangleVertices.size());
        
        for(itr = _vertexPointers.begin();
            itr != _vertexPointers.end();
            ++itr)
        {
            const osg::Vec3* vec = *itr;
            unsigned int index = base + (vec - minVertex);
            (*_vertices)[index] = *vec;
            _triangleIndices->push_back(index);
        }


        unsigned int pos = base + numberNewVertices;
        for(OccluderGeometry::Vec3List::iterator vitr = _tempoaryTriangleVertices.begin();
             vitr != _tempoaryTriangleVertices.end();
             ++vitr, ++pos)
        {
            (*_vertices)[pos] = *vitr;
            _triangleIndices->push_back(pos);
        }

        if (_matrix)
        {
            for(unsigned int i=base; i<_vertices->size(); ++i)
            {
                (*_vertices)[i] = (*_vertices)[i] * (*_matrix);
            }
        }
        
    }


};
typedef osg::TriangleFunctor<TriangleCollector> TriangleCollectorFunctor;

void OccluderGeometry::processGeometry(osg::Drawable* drawable, osg::Matrix* matrix, float sampleRatio)
{
    // osg::notify(osg::NOTICE)<<"computeOccluderGeometry(osg::Node* subgraph, float sampleRatio)"<<std::endl;

    TriangleCollectorFunctor tc;
    tc.set(&_vertices, &_triangleIndices, matrix);

    drawable->accept(tc);
    
    tc.copyToLocalData();
    
#if 0    
    for(Vec3List::iterator vitr = _vertices.begin();
        vitr != _vertices.end();
        ++vitr)
    {
        osg::notify(osg::NOTICE)<<"v "<<*vitr<<std::endl;
    }

    for(UIntList::iterator titr = _triangleIndices.begin();
        titr != _triangleIndices.end();
        )
    {
        osg::notify(osg::NOTICE)<<"t "<<*titr++<<" "<<*titr++<<" "<<*titr++<<std::endl;
    }
#endif
}

void OccluderGeometry::setUpInternalStructures()
{

    osg::Timer_t t0 = osg::Timer::instance()->tick();

    removeDuplicateVertices();
    
    osg::Timer_t t1 = osg::Timer::instance()->tick();

    removeNullTriangles();

    osg::Timer_t t2 = osg::Timer::instance()->tick();

    computeNormals();

    osg::Timer_t t3 = osg::Timer::instance()->tick();

    buildEdgeMaps();
    
    osg::Timer_t t4 = osg::Timer::instance()->tick();

    computeLightDirectionSlihouetteEdges(osg::Vec3(0.0,0.0,1.0f));

    osg::Timer_t t45 = osg::Timer::instance()->tick();

    computeLightDirectionSlihouetteEdges(osg::Vec3(0.0,0.0,1.0f));

    osg::Timer_t t5 = osg::Timer::instance()->tick();

    osg::notify(osg::NOTICE)<<"removeDuplicateVertices "<<osg::Timer::instance()->delta_m(t0,t1)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"removeNullTriangles "<<osg::Timer::instance()->delta_m(t1,t2)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"computeNormals "<<osg::Timer::instance()->delta_m(t2,t3)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"buildEdgeMaps "<<osg::Timer::instance()->delta_m(t3,t4)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"computeLightDirectionSlihouetteEdges "<<osg::Timer::instance()->delta_m(t4,t45)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"computeLightDirectionSlihouetteEdges "<<osg::Timer::instance()->delta_m(t45,t5)<<" ms"<<std::endl;
    osg::notify(osg::NOTICE)<<"setUpInternalStructures "<<osg::Timer::instance()->delta_m(t0,t5)<<" ms"<<std::endl;


    dirtyBound();
    
    dirtyDisplayList();
}

struct IndexVec3PtrPair
{
    IndexVec3PtrPair():
        vec(0),
        index(0) {}

    IndexVec3PtrPair(const osg::Vec3* v, unsigned int i):
        vec(v),
        index(i) {}

    inline bool operator < (const IndexVec3PtrPair& rhs) const
    {
        return *vec < *rhs.vec;
    }

    inline bool operator == (const IndexVec3PtrPair& rhs) const
    {
        return *vec == *rhs.vec;
    }

    const osg::Vec3* vec;
    unsigned int index;
};

void OccluderGeometry::removeDuplicateVertices()
{
    if (_vertices.empty()) return;
    
    // osg::notify(osg::NOTICE)<<"OccluderGeometry::removeDuplicates() before = "<<_vertices.size()<<std::endl;

    typedef std::vector<IndexVec3PtrPair> IndexVec3PtrPairs;
    IndexVec3PtrPairs indexVec3PtrPairs;
    indexVec3PtrPairs.reserve(_vertices.size());

    unsigned int i = 0;
    for(OccluderGeometry::Vec3List::iterator vitr = _vertices.begin();
         vitr != _vertices.end();
         ++vitr, ++i)
    {
        indexVec3PtrPairs.push_back(IndexVec3PtrPair(&(*vitr),i));
    }
    std::sort(indexVec3PtrPairs.begin(),indexVec3PtrPairs.end());;


    // compute size
    IndexVec3PtrPairs::iterator prev = indexVec3PtrPairs.begin();
    IndexVec3PtrPairs::iterator curr = prev;
    ++curr;

    unsigned int numDuplicates = 0;
    unsigned int numUnique = 1;

    for(; curr != indexVec3PtrPairs.end(); ++curr)
    {
        if (*prev==*curr) 
        {
            ++numDuplicates;
        }
        else
        {
            prev = curr; 
            ++numUnique;
        }
    }
    
    // osg::notify(osg::NOTICE)<<"Num diplicates = "<<numDuplicates<<std::endl;
    // osg::notify(osg::NOTICE)<<"Num unique = "<<numUnique<<std::endl;

    if (numDuplicates==0) return;

    // now assign the unique Vec3 to the newVertices arrays    
    typedef std::vector<unsigned int> IndexMap;
    IndexMap indexMap(indexVec3PtrPairs.size());

    Vec3List newVertices;
    newVertices.reserve(numUnique);
    unsigned int index = 0;

    prev = indexVec3PtrPairs.begin();
    curr = prev;

    // add first vertex
    indexMap[curr->index] = index;
    newVertices.push_back(*(curr->vec));

    ++curr;

    for(; curr != indexVec3PtrPairs.end(); ++curr)
    {
        if (*prev==*curr) 
        {
            indexMap[curr->index] = index;
        }
        else
        {
            ++index;            
            
            // add new unique vertex
            indexMap[curr->index] = index;
            newVertices.push_back(*(curr->vec));

            prev = curr; 
        }
    }

    // copy over need arrays and index values
    _vertices.swap(newVertices);

    for(UIntList::iterator titr = _triangleIndices.begin();
        titr != _triangleIndices.end();
        ++titr)
    {
        *titr = indexMap[*titr];
    }

    // osg::notify(osg::NOTICE)<<"OccluderGeometry::removeDuplicates() after = "<<_vertices.size()<<std::endl;
}

void OccluderGeometry::removeNullTriangles()
{
    // osg::notify(osg::NOTICE)<<"OccluderGeometry::removeNullTriangles()"<<std::endl;

    
    UIntList::iterator lastValidItr = _triangleIndices.begin();
    for(UIntList::iterator titr = _triangleIndices.begin();
        titr != _triangleIndices.end();
        )
    {
        UIntList::iterator currItr = titr;
        GLuint p1 = *titr++;
        GLuint p2 = *titr++;
        GLuint p3 = *titr++;
        if ((p1 != p2) && (p1 != p3) && (p2 != p3))
        {
            if (lastValidItr!=currItr)
            {
                *lastValidItr++ = p1;
                *lastValidItr++ = p2;
                *lastValidItr++ = p3;
            }
            else
            {
                lastValidItr = titr;
            }
        }
        else
        {
            // osg::notify(osg::NOTICE)<<"Null triangle"<<std::endl;
        }
    }
    if (lastValidItr != _triangleIndices.end())
    {
        // osg::notify(osg::NOTICE)<<"Pruning end - before "<<_triangleIndices.size()<<std::endl;
        _triangleIndices.erase(lastValidItr,_triangleIndices.end());
        // osg::notify(osg::NOTICE)<<"Pruning end - after "<<_triangleIndices.size()<<std::endl;
    }
}

void OccluderGeometry::computeNormals()
{
    // osg::notify(osg::NOTICE)<<"OccluderGeometry::computeNormals()"<<std::endl;
    
    unsigned int numTriangles = _triangleIndices.size() / 3;
    unsigned int redundentIndices = _triangleIndices.size() - numTriangles * 3;
    if (redundentIndices)
    {
        osg::notify(osg::NOTICE)<<"Warning OccluderGeometry::computeNormals() has found redundent trailing indices"<<std::endl;
        _triangleIndices.erase(_triangleIndices.begin() + numTriangles * 3, _triangleIndices.end());
    }
    
    _triangleNormals.clear();
    _triangleNormals.reserve(numTriangles);
    
    _normals.resize(_vertices.size());


    for(UIntList::iterator titr = _triangleIndices.begin();
        titr != _triangleIndices.end();
        )
    {
        GLuint p1 = *titr++;
        GLuint p2 = *titr++;
        GLuint p3 = *titr++;
        osg::Vec3 normal = (_vertices[p2] - _vertices[p1]) ^ (_vertices[p3] - _vertices[p2]);
        normal.normalize();

        _triangleNormals.push_back(normal);

        if (!_normals.empty())
        {        
            _normals[p1] += normal;
            _normals[p2] += normal;
            _normals[p3] += normal;
        }
    }
    
    for(Vec3List::iterator nitr = _normals.begin();
        nitr != _normals.end();
        ++nitr)
    {
        nitr->normalize();
    }

    
}

void OccluderGeometry::buildEdgeMaps()
{
    // osg::notify(osg::NOTICE)<<"OccluderGeometry::buildEdgeMaps()"<<std::endl;
    
    typedef std::set<Edge> EdgeSet;
    EdgeSet edgeSet;
    
    unsigned int numTriangleErrors = 0;
    unsigned int triNo=0;
    for(UIntList::iterator titr = _triangleIndices.begin();
        titr != _triangleIndices.end();
        ++triNo)
    {
        GLuint p1 = *titr++;
        GLuint p2 = *titr++;
        GLuint p3 = *titr++;
        
        {
            Edge edge12(p1,p2);
            EdgeSet::iterator itr = edgeSet.find(edge12);
            if (itr == edgeSet.end()) 
            {
                if (!edge12.addTriangle(triNo)) ++numTriangleErrors;
                edgeSet.insert(edge12);
            }
            else
            {
                if (!itr->addTriangle(triNo)) ++numTriangleErrors;
            }
        }

        {        
            Edge edge23(p2,p3);
            EdgeSet::iterator itr = edgeSet.find(edge23);
            if (itr == edgeSet.end()) 
            {
                if (!edge23.addTriangle(triNo)) ++numTriangleErrors;
                edgeSet.insert(edge23);
            }
            else
            {
                if (!itr->addTriangle(triNo)) ++numTriangleErrors;
            }
        }
        
        {
            Edge edge31(p3,p1);
            EdgeSet::iterator itr = edgeSet.find(edge31);
            if (itr == edgeSet.end()) 
            {
                if (!edge31.addTriangle(triNo)) ++numTriangleErrors;
                
                edgeSet.insert(edge31);
            }
            else
            {
                if (!itr->addTriangle(triNo)) ++numTriangleErrors;
            }
        }
    }

    _edges.clear();
    _edges.reserve(edgeSet.size());
    
    unsigned int numEdgesWithNoTriangles = 0;
    unsigned int numEdgesWithOneTriangles = 0;
    unsigned int numEdgesWithTwoTriangles = 0;
    
    for(EdgeSet::iterator eitr = edgeSet.begin();
        eitr != edgeSet.end();
        ++eitr)
    {
        _edges.push_back(*eitr);
        unsigned int numTriangles = ((eitr->_t1>=0)? 1:0) + ((eitr->_t2>=0)? 1:0);
        switch(numTriangles)
        {
            case(0): ++numEdgesWithNoTriangles;  break; 
            case(1): ++numEdgesWithOneTriangles;  break; 
            default: ++numEdgesWithTwoTriangles;  break; 
        }
    }

#if 0
    osg::notify(osg::NOTICE)<<"Num of indices "<<_triangleIndices.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of triangles "<<triNo<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of numTriangleErrors "<<numTriangleErrors<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of edges "<<edgeSet.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of numEdgesWithNoTriangles "<<numEdgesWithNoTriangles<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of numEdgesWithOneTriangles "<<numEdgesWithOneTriangles<<std::endl;
    osg::notify(osg::NOTICE)<<"Num of numEdgesWithTwoTriangles "<<numEdgesWithTwoTriangles<<std::endl;
#endif
}


void OccluderGeometry::computeLightPointSlihouetteEdges(const osg::Vec3& lightpos)
{
    _silhouetteIndices.clear();
    
    for(EdgeList::iterator eitr = _edges.begin();
        eitr != _edges.end();
        ++eitr)
    {
        Edge& edge = *eitr;
        if (testLightPointSilhouetteEdge(lightpos,edge)<=0.0f)
        {
            _silhouetteIndices.push_back(edge._p1);
            _silhouetteIndices.push_back(edge._p2);
        }
    }
}

void OccluderGeometry::computeLightDirectionSlihouetteEdges(const osg::Vec3& lightdirection)
{
    _silhouetteIndices.clear();
    
    for(EdgeList::iterator eitr = _edges.begin();
        eitr != _edges.end();
        ++eitr)
    {
        Edge& edge = *eitr;
        if (testLightDirectionSilhouetteEdge(lightdirection,edge)<=0.0f)
        {
            _silhouetteIndices.push_back(edge._p1);
            _silhouetteIndices.push_back(edge._p2);
        }
    }
}

void OccluderGeometry::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::notify(osg::NOTICE)<<"drawImplementation(osg::RenderInfo& renderInfo)"<<std::endl;

    renderInfo.getState()->disableAllVertexArrays();

    renderInfo.getState()->setVertexPointer( 3, GL_FLOAT, 0, &(_vertices.front()) );
    
    if (!_normals.empty())
    {
        renderInfo.getState()->setNormalPointer( GL_FLOAT, 0, &(_normals.front()) );
    }

    if (!_triangleIndices.empty())
    {
        glDrawElements(GL_TRIANGLES, _triangleIndices.size(), GL_UNSIGNED_INT, &(_triangleIndices.front()) );
    }
    
    if (!_silhouetteIndices.empty())
    {
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
        glDrawElements(GL_LINES, _silhouetteIndices.size(), GL_UNSIGNED_INT, &(_silhouetteIndices.front()) );
    }
    
}

osg::BoundingBox OccluderGeometry::computeBound() const
{
    osg::BoundingBox bb;
    for(Vec3List::const_iterator itr =  _vertices.begin();
        itr != _vertices.end();
        ++itr)
    {
        bb.expandBy(*itr);
    }

    osg::notify(osg::NOTICE)<<"computeBB "<<bb.xMin()<<", "<<bb.xMax()<<std::endl;

    return bb;
}
