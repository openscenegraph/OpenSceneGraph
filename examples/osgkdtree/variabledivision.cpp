/* OpenSceneGraph example, osgintersection.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include "variabledivision.h"

namespace variabledivision
{

struct TriangleIndicesCollector
{
    TriangleIndicesCollector():
        _kdTree(0)
    {
    }

    inline void operator () (unsigned int p1, unsigned int p2, unsigned int p3)
    {
        unsigned int i = _kdTree->_triangles.size();
        _kdTree->_triangles.push_back(Triangle(p1,p2,p3));
        
        osg::BoundingBox bb;
        bb.expandBy((*(_kdTree->_vertices))[p1]);
        bb.expandBy((*(_kdTree->_vertices))[p2]);
        bb.expandBy((*(_kdTree->_vertices))[p3]);
        _kdTree->_boundingBoxes.push_back(bb);
        
        _kdTree->_centers.push_back(bb.center());

        _kdTree->_primitiveIndices.push_back(i);
        
    }
    
    KDTree* _kdTree;

};

KDTree* KDTreeBuilder::createKDTree(osg::Geometry* geometry)
{
#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree()"<<std::endl;
#endif

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return 0;
    
    if (vertices->size() <= _targetNumTrianglesPerLeaf) return 0;

    osg::ref_ptr<KDTree> kdTree = new KDTree;
    kdTree->_geometry = geometry;
    kdTree->_bb = kdTree->_geometry->getBound();
    kdTree->_vertices = vertices;
    
    unsigned int estimatedSize = (unsigned int)(2.0*float(vertices->size())/float(_targetNumTrianglesPerLeaf));

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"kdTree->_kdNodes.reserve()="<<estimatedSize<<std::endl<<std::endl;
#endif

    kdTree->_kdNodes.reserve(estimatedSize);
    kdTree->_kdLeaves.reserve(estimatedSize);
    
    computeDivisions(*kdTree);


    _numVerticesProcessed += vertices->size();

    unsigned int estimatedNumTriangles = vertices->size()*2;
    kdTree->_primitiveIndices.reserve(estimatedNumTriangles);
    kdTree->_boundingBoxes.reserve(estimatedNumTriangles);
    kdTree->_triangles.reserve(estimatedNumTriangles);
    kdTree->_centers.reserve(estimatedNumTriangles);



    osg::TriangleIndexFunctor<TriangleIndicesCollector> collectTriangleIndices;
    collectTriangleIndices._kdTree = kdTree.get();
    geometry->accept(collectTriangleIndices);

    kdTree->_primitiveIndices.reserve(vertices->size());



    KDLeaf leaf(0, kdTree->_primitiveIndices.size());

    int leafNum = kdTree->addLeaf(leaf);

    osg::BoundingBox bb = kdTree->_bb;
    int nodeNum = divide(*kdTree, bb, leafNum, 0);

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"Root nodeNum="<<nodeNum<<std::endl;
#endif
    
    
#ifdef VERBOSE_OUTPUT    
    
    KDTreeTraverser traverser;
    traverser.traverse(*kdTree);
    
    
    osg::notify(osg::NOTICE)<<"Final kdTree->_kdNodes.size()="<<kdTree->_kdNodes.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"Final kdTree->_kdLeaves.size()="<<kdTree->_kdLeaves.size()<<std::endl;

    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree() completed"<<std::endl<<std::endl;
#endif

//    osg::notify(osg::NOTICE)<<"kdTree->_kdNodes.size()="<<kdTree->_kdNodes.size()<<"  estimated size = "<<estimatedSize<<std::endl;
//    osg::notify(osg::NOTICE)<<"kdTree->_kdLeaves.size()="<<kdTree->_kdLeaves.size()<<"  estimated size = "<<estimatedSize<<std::endl<<std::endl;



    return kdTree.release();
}    

void KDTreeBuilder::computeDivisions(KDTree& kdTree)
{


    osg::Vec3 dimensions(kdTree._bb.xMax()-kdTree._bb.xMin(),
                         kdTree._bb.yMax()-kdTree._bb.yMin(),
                         kdTree._bb.zMax()-kdTree._bb.zMin());

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"computeDivisions("<<_maxNumLevels<<") "<<dimensions<< " { "<<std::endl;
#endif

    kdTree._axisStack.reserve(_maxNumLevels);
 
    int level = 0;
    for(unsigned int level=0; level<_maxNumLevels; ++level)
    {
        int axis = 0;
        if (dimensions[0]>=dimensions[1])
        {
            if (dimensions[0]>=dimensions[2]) axis = 0;
            else axis = 2;
        }
        else if (dimensions[1]>=dimensions[2]) axis = 1;
        else axis = 2;

        kdTree._axisStack.push_back(axis);
        dimensions[axis] /= 2.0f;

#ifdef VERBOSE_OUTPUT    
        osg::notify(osg::NOTICE)<<"  "<<level<<", "<<dimensions<<", "<<axis<<std::endl;
#endif
    }

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"}"<<std::endl;
#endif
}



int KDTreeBuilder::divide(KDTree& kdTree, osg::BoundingBox& bb, int nodeIndex, unsigned int level)
{
    if (kdTree._axisStack.size()<=level) return nodeIndex;


    int axis = kdTree._axisStack[level];

#ifdef VERBOSE_OUTPUT    
    //osg::notify(osg::NOTICE)<<"divide("<<nodeIndex<<", "<<level<< "), axis="<<axis<<std::endl;
#endif

    if (nodeIndex>=0)
    {
#ifdef VERBOSE_OUTPUT    
        osg::notify(osg::NOTICE)<<"  divide node"<<std::endl;
#endif
        KDNode& node = kdTree.getNode(nodeIndex);
        return nodeIndex;
    }
    else
    {    

        if (kdTree.getLeaf(nodeIndex).second<=_targetNumTrianglesPerLeaf) return nodeIndex;

        //osg::notify(osg::NOTICE)<<"  divide leaf"<<std::endl;
        
        int nodeNum = kdTree.addNode(KDNode());

        float original_min = bb._min[axis];
        float original_max = bb._max[axis];

        float mid = (original_min+original_max)*0.5f;

        {
            KDLeaf& leaf = kdTree.getLeaf(nodeIndex);

            //osg::Vec3Array* vertices = kdTree._vertices.get();
            KDTree::CenterList& centers = kdTree._centers;

            int end = leaf.first+leaf.second-1;
            int left = leaf.first;
            int right = leaf.first+leaf.second-1;
            
            while(left<right)
            {
                while(left<right && (centers[kdTree._primitiveIndices[left]][axis]<=mid)) { ++left; }

                while(left<right && (centers[kdTree._primitiveIndices[right]][axis]>mid)) { --right; }
                
                while(left<right && (centers[kdTree._primitiveIndices[right]][axis]>mid)) { --right; }

                if (left<right)
                {
                    std::swap(kdTree._primitiveIndices[left], kdTree._primitiveIndices[right]);
                    ++left;
                    --right;
                }
            }
            
            if (left==right)
            {
                if (centers[kdTree._primitiveIndices[left]][axis]<=mid) ++left;
                else --right;
            }
            
            KDLeaf leftLeaf(leaf.first, (right-leaf.first)+1);
            KDLeaf rightLeaf(left, (end-left)+1);

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
                kdTree.getNode(nodeNum).first = 0;
                kdTree.getNode(nodeNum).second = kdTree.replaceLeaf(nodeIndex, rightLeaf);
            }
            else if (rightLeaf.second<=0)
            {
                //osg::notify(osg::NOTICE)<<"RightLeaf empty"<<std::endl;
                kdTree.getNode(nodeNum).first = kdTree.replaceLeaf(nodeIndex, leftLeaf);
                kdTree.getNode(nodeNum).second = 0;
            }
            else
            {
                kdTree.getNode(nodeNum).first = kdTree.replaceLeaf(nodeIndex, leftLeaf);
                kdTree.getNode(nodeNum).second = kdTree.addLeaf(rightLeaf);
            }
        }

        
        int originalLeftChildIndex = kdTree.getNode(nodeNum).first;
        int originalRightChildIndex = kdTree.getNode(nodeNum).second;


        float restore = bb._max[axis];
        bb._max[axis] = mid;

        //osg::notify(osg::NOTICE)<<"  divide leftLeaf "<<kdTree.getNode(nodeNum).first<<std::endl;
        int leftChildIndex = divide(kdTree, bb, originalLeftChildIndex, level+1);

        bb._max[axis] = restore;
        
        restore = bb._min[axis];
        bb._min[axis] = mid;

        //osg::notify(osg::NOTICE)<<"  divide rightLeaf "<<kdTree.getNode(nodeNum).second<<std::endl;
        int rightChildIndex = divide(kdTree, bb, originalRightChildIndex, level+1);
        
        bb._min[axis] = restore;
        
        kdTree.getNode(nodeNum).first = leftChildIndex;
        kdTree.getNode(nodeNum).second = rightChildIndex; 
        
        return nodeNum;        
    }


}

}
