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
            osg::notify(osg::NOTICE)<<"Ignoring transparent drawable."<<std::endl;
            return;
        }
        
        _oc->computeOccluderGeometry(drawable, (_matrixStack.empty() ? 0 : &_matrixStack.back()), _ratio);
        
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
    
    CollectOccludersVisitor cov(this, matrix, sampleRatio);
    subgraph->accept(cov);

    osg::notify(osg::NOTICE)<<"done"<<std::endl;
}

struct TriangleIndexCollector
{
    OccluderGeometry::Vec3List& _vertices;
    OccluderGeometry::UIntList& _triangleIndices;
    unsigned int                _baseIndex;
    osg::Matrix*                _matrix;

    TriangleIndexCollector(OccluderGeometry::Vec3List& vertices, OccluderGeometry::UIntList& triangleIndices, osg::Matrix* matrix):
            _vertices(vertices),
            _triangleIndices(triangleIndices)
    {
        _baseIndex = _vertices.size();
    }

    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        _triangleIndices.push_back(_baseIndex + p1);
        _triangleIndices.push_back(_baseIndex + p2);
        _triangleIndices.push_back(_baseIndex + p3);
    }

};
typedef osg::TriangleIndexFunctor<TriangleIndexCollector> TriangleIndexCollectorFunctor;

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
            osg::notify(osg::NOTICE)<<"Triangle temp ("<<v1<<") ("<<v2<<") ("<<v3<<")"<<std::endl;
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
        if (_vertexPointers.size()<3) return;
    
        const osg::Vec3* minVertex = _vertexPointers.front();
        const osg::Vec3* maxVertex = _vertexPointers.front();
        VertexPointers::iterator itr;
        for(itr = _vertexPointers.begin();
            itr != _vertexPointers.end();
            ++itr)
        {
            if (*itr < minVertex) minVertex = *itr;
            if (*itr > maxVertex) maxVertex = *itr;
        }

        unsigned int base = _vertices->size();
        unsigned int numberNewVertices = (maxVertex - minVertex);
        
        osg::notify(osg::NOTICE)<<"base = "<<base<<" numberNewVertices="<<numberNewVertices<<std::endl;

        _vertices->resize(base + numberNewVertices);
        
        for(itr = _vertexPointers.begin();
            itr != _vertexPointers.end();
            ++itr)
        {
            const osg::Vec3* vec = *itr;
            unsigned int index = base + (vec - minVertex);
            (*_vertices)[index] = *vec;
            _triangleIndices->push_back(index);
        }
        
    }

};
typedef osg::TriangleFunctor<TriangleCollector> TriangleCollectorFunctor;

void OccluderGeometry::computeOccluderGeometry(osg::Drawable* drawable, osg::Matrix* matrix, float sampleRatio)
{
    osg::notify(osg::NOTICE)<<"computeOccluderGeometry(osg::Node* subgraph, float sampleRatio)"<<std::endl;

    TriangleCollectorFunctor tc;
    tc.set(&_vertices, &_triangleIndices, matrix);

    drawable->accept(tc);
    
    tc.copyToLocalData();
    
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
}

void OccluderGeometry::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::notify(osg::NOTICE)<<"drawImplementation(osg::RenderInfo& renderInfo)"<<std::endl;
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
    return bb;
}
