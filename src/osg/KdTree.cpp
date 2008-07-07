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

#include <osg/KdTree>
#include <osg/Geode>
#include <osg/io_utils>
#include <osg/TriangleIndexFunctor>

using namespace osg;

//#define VERBOSE_OUTPUT

////////////////////////////////////////////////////////////////////////////////
//
// Functor for collecting triangle indices from Geometry

struct TriangleIndicesCollector
{
    TriangleIndicesCollector():
        _kdTree(0)
    {
    }

    inline void operator () (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        unsigned int i = _kdTree->_triangles.size();
        _kdTree->_triangles.push_back(KdTree::Triangle(p1,p2,p3));
        
        osg::BoundingBox bb;
        bb.expandBy((*(_kdTree->_vertices))[p1]);
        bb.expandBy((*(_kdTree->_vertices))[p2]);
        bb.expandBy((*(_kdTree->_vertices))[p3]);
        _kdTree->_boundingBoxes.push_back(bb);
        
        _kdTree->_centers.push_back(bb.center());

        _kdTree->_primitiveIndices.push_back(i);
        
    }
    
    KdTree* _kdTree;

};

////////////////////////////////////////////////////////////////////////////////
//
// KdTree

KdTree::KdTree()
{
}

KdTree::KdTree(const KdTree& rhs, const osg::CopyOp& copyop):
    Shape(rhs)
{
}

bool KdTree::build(BuildOptions& options, osg::Geometry* geometry)
{
#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree()"<<std::endl;
#endif

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return false;
    
    if (vertices->size() <= options._targetNumTrianglesPerLeaf) return false;

    _geometry = geometry;
    _bb = _geometry->getBound();
    _vertices = vertices;
    
    unsigned int estimatedSize = (unsigned int)(2.0*float(vertices->size())/float(options._targetNumTrianglesPerLeaf));

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"kdTree->_kdNodes.reserve()="<<estimatedSize<<std::endl<<std::endl;
#endif

    _kdNodes.reserve(estimatedSize);
    _kdLeaves.reserve(estimatedSize);
    
    computeDivisions(options);

    options._numVerticesProcessed += vertices->size();

    unsigned int estimatedNumTriangles = vertices->size()*2;
    _primitiveIndices.reserve(estimatedNumTriangles);
    _boundingBoxes.reserve(estimatedNumTriangles);
    _triangles.reserve(estimatedNumTriangles);
    _centers.reserve(estimatedNumTriangles);



    osg::TriangleIndexFunctor<TriangleIndicesCollector> collectTriangleIndices;
    collectTriangleIndices._kdTree = this;
    geometry->accept(collectTriangleIndices);

    _primitiveIndices.reserve(vertices->size());



    KdLeaf leaf(0, _primitiveIndices.size());

    int leafNum = addLeaf(leaf);

    osg::BoundingBox bb = _bb;
    int nodeNum = divide(options, bb, leafNum, 0);

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"Root nodeNum="<<nodeNum<<std::endl;
#endif
    
    
//    osg::notify(osg::NOTICE)<<"_kdNodes.size()="<<k_kdNodes.size()<<"  estimated size = "<<estimatedSize<<std::endl;
//    osg::notify(osg::NOTICE)<<"_kdLeaves.size()="<<_kdLeaves.size()<<"  estimated size = "<<estimatedSize<<std::endl<<std::endl;


    return true;
}

void KdTree::computeDivisions(BuildOptions& options)
{
    osg::Vec3 dimensions(_bb.xMax()-_bb.xMin(),
                         _bb.yMax()-_bb.yMin(),
                         _bb.zMax()-_bb.zMin());

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"computeDivisions("<<options._maxNumLevels<<") "<<dimensions<< " { "<<std::endl;
#endif

    _axisStack.reserve(options._maxNumLevels);
 
    for(int level=0; level<options._maxNumLevels; ++level)
    {
        int axis = 0;
        if (dimensions[0]>=dimensions[1])
        {
            if (dimensions[0]>=dimensions[2]) axis = 0;
            else axis = 2;
        }
        else if (dimensions[1]>=dimensions[2]) axis = 1;
        else axis = 2;

        _axisStack.push_back(axis);
        dimensions[axis] /= 2.0f;

#ifdef VERBOSE_OUTPUT    
        osg::notify(osg::NOTICE)<<"  "<<level<<", "<<dimensions<<", "<<axis<<std::endl;
#endif
    }

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"}"<<std::endl;
#endif
}

int KdTree::divide(BuildOptions& options, osg::BoundingBox& bb, int nodeIndex, unsigned int level)
{
    if (_axisStack.size()<=level) return nodeIndex;

    int axis = _axisStack[level];

#ifdef VERBOSE_OUTPUT    
    //osg::notify(osg::NOTICE)<<"divide("<<nodeIndex<<", "<<level<< "), axis="<<axis<<std::endl;
#endif

    if (nodeIndex>=0)
    {
#ifdef VERBOSE_OUTPUT    
        osg::notify(osg::NOTICE)<<"  divide node"<<std::endl;
#endif
        KdNode& node = getNode(nodeIndex);
        return nodeIndex;
    }
    else
    {    

        if (getLeaf(nodeIndex).second<=options._targetNumTrianglesPerLeaf) return nodeIndex;

        //osg::notify(osg::NOTICE)<<"  divide leaf"<<std::endl;
        
        int nodeNum = addNode(KdNode());

        float original_min = bb._min[axis];
        float original_max = bb._max[axis];

        float mid = (original_min+original_max)*0.5f;

        {
            KdLeaf& leaf = getLeaf(nodeIndex);

            //osg::Vec3Array* vertices = kdTree._vertices.get();
            int end = leaf.first+leaf.second-1;
            int left = leaf.first;
            int right = leaf.first+leaf.second-1;
            
            while(left<right)
            {
                while(left<right && (_centers[_primitiveIndices[left]][axis]<=mid)) { ++left; }

                while(left<right && (_centers[_primitiveIndices[right]][axis]>mid)) { --right; }
                
                while(left<right && (_centers[_primitiveIndices[right]][axis]>mid)) { --right; }

                if (left<right)
                {
                    std::swap(_primitiveIndices[left], _primitiveIndices[right]);
                    ++left;
                    --right;
                }
            }
            
            if (left==right)
            {
                if (_centers[_primitiveIndices[left]][axis]<=mid) ++left;
                else --right;
            }
            
            KdLeaf leftLeaf(leaf.first, (right-leaf.first)+1);
            KdLeaf rightLeaf(left, (end-left)+1);

#if 0            
            osg::notify(osg::NOTICE)<<"In  leaf.first     ="<<leaf.first     <<" leaf.second     ="<<leaf.second<<std::endl;
            osg::notify(osg::NOTICE)<<"    leftLeaf.first ="<<leftLeaf.first <<" leftLeaf.second ="<<leftLeaf.second<<std::endl;
            osg::notify(osg::NOTICE)<<"    rightLeaf.first="<<rightLeaf.first<<" rightLeaf.second="<<rightLeaf.second<<std::endl;
            osg::notify(osg::NOTICE)<<"    left="<<left<<" right="<<right<<std::endl;

            if (leaf.second != (leftLeaf.second +rightLeaf.second))
            {
                osg::notify(osg::NOTICE)<<"*** Error in size, leaf.second="<<leaf.second
                                        <<", leftLeaf.second="<<leftLeaf.second
                                        <<", rightLeaf.second="<<rightLeaf.second<<std::endl;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Size OK, leaf.second="<<leaf.second
                                        <<", leftLeaf.second="<<leftLeaf.second
                                        <<", rightLeaf.second="<<rightLeaf.second<<std::endl;
            }
#endif
            if (leftLeaf.second<=0)
            {
                //osg::notify(osg::NOTICE)<<"LeftLeaf empty"<<std::endl;
                getNode(nodeNum).first = 0;
                getNode(nodeNum).second = replaceLeaf(nodeIndex, rightLeaf);
            }
            else if (rightLeaf.second<=0)
            {
                //osg::notify(osg::NOTICE)<<"RightLeaf empty"<<std::endl;
                getNode(nodeNum).first = replaceLeaf(nodeIndex, leftLeaf);
                getNode(nodeNum).second = 0;
            }
            else
            {
                getNode(nodeNum).first = replaceLeaf(nodeIndex, leftLeaf);
                getNode(nodeNum).second = addLeaf(rightLeaf);
            }
        }

        
        int originalLeftChildIndex = getNode(nodeNum).first;
        int originalRightChildIndex = getNode(nodeNum).second;


        float restore = bb._max[axis];
        bb._max[axis] = mid;

        //osg::notify(osg::NOTICE)<<"  divide leftLeaf "<<kdTree.getNode(nodeNum).first<<std::endl;
        int leftChildIndex = divide(options, bb, originalLeftChildIndex, level+1);

        bb._max[axis] = restore;
        
        restore = bb._min[axis];
        bb._min[axis] = mid;

        //osg::notify(osg::NOTICE)<<"  divide rightLeaf "<<kdTree.getNode(nodeNum).second<<std::endl;
        int rightChildIndex = divide(options, bb, originalRightChildIndex, level+1);
        
        bb._min[axis] = restore;
        
        getNode(nodeNum).first = leftChildIndex;
        getNode(nodeNum).second = rightChildIndex; 
        
        
        return nodeNum;        
    }
}


bool KdTree::intersect(const osg::Vec3& start, const osg::Vec3& end, LineSegmentIntersections& intersections)
{
    osg::notify(osg::NOTICE)<<"KdTree::intersect("<<start<<","<<end<<")"<<std::endl;
    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
// KdTreeBuilder
KdTreeBuilder::KdTreeBuilder():
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
{            
    _kdTreePrototype = new osg::KdTree;
}

KdTreeBuilder::KdTreeBuilder(const KdTreeBuilder& rhs):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _kdTreePrototype(rhs._kdTreePrototype),
    _buildOptions(rhs._buildOptions)
{
}

void KdTreeBuilder::apply(osg::Geode& geode)
{
    for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
    {            

        osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
        if (geom)
        {
            osg::KdTree* previous = dynamic_cast<osg::KdTree*>(geom->getShape());
            if (previous) continue;

            osg::ref_ptr<osg::KdTree> kdTree = dynamic_cast<osg::KdTree*>(_kdTreePrototype->cloneType());

            if (kdTree->build(_buildOptions, geom))
            {
                geom->setShape(kdTree.get());
            }
        }   
    }
}
