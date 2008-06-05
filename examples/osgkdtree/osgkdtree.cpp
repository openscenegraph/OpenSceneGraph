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

  
#include <osgDB/ReadFile>

#define _GLIBCXX_DEBUG

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Geometry>


#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include <iostream>


namespace osg
{

class KDNode
{
    public:
    
        KDNode():
            _leftChild(0),
            _rightChild(0) {}
    
        KDNode(const KDNode& rhs):
            _leftChild(rhs._leftChild),
            _rightChild(rhs._rightChild) {}

        KDNode& operator = (const KDNode& rhs)
        {
            _leftChild = rhs._leftChild;
            _rightChild = rhs._rightChild;
            return *this;
        }

        typedef int value_type;

        value_type _leftChild;
        value_type _rightChild;
};

class KDLeaf : public osg::Referenced
{
    public:
    
        KDLeaf() {}
    
        typedef unsigned int index_type;
        typedef std::vector< index_type > Indices;

        Indices _vertexIndices;        
    
    protected:
    
        virtual ~KDLeaf() {}
};

class KDTree : public osg::Shape
{
    public:
    
    
        KDTree() {}
        
        KDTree(const KDTree& rhs, const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            Shape(rhs,copyop) {}

        META_Shape(osg, KDTree)
    
        typedef std::vector< unsigned int > AxisStack;
        typedef std::vector< KDNode > KDNodeList;
        typedef std::vector< osg::ref_ptr<KDLeaf> > KDLeafList;
        
        
        /// note, leafNum is negative to distinguish from nodeNum
        int addLeaf(KDLeaf* leaf) { int num = _kdLeaves.size(); _kdLeaves.push_back(leaf); return -(num+1); }

        int replaceLeaf(int leafNum, KDLeaf* leaf)
        {
            int num = -leafNum-1; 
            
            if (num>_kdLeaves.size()-1)
            {
                osg::notify(osg::NOTICE)<<"Warning: replaceChild("<<leafNum<<", leaf), num = "<<num<<" _kdLeaves.size()="<<_kdLeaves.size()<<std::endl;
                return leafNum;
            }
            
            _kdLeaves[num] = leaf; return leafNum;
        }

        /// note, leafNum is negative to distinguish from nodeNum
        KDLeaf* getLeaf(int leafNum)
        {
            int num = -leafNum-1;
            if (num<0 || num>_kdLeaves.size()-1)
            {
                osg::notify(osg::NOTICE)<<"Warning: getLeaf("<<leafNum<<", num = "<<num<<") _kdLeaves.size()="<<_kdLeaves.size()<<std::endl;
                return 0;
            }

            return _kdLeaves[num].get();
        }
        
        int addNode(const KDNode& node)
        {
            int num = _kdNodes.size(); 
            _kdNodes.push_back(node); 
            return num;
        }

        /// note, nodeNum is positive to distinguish from leftNum
        KDNode& getNode(int nodeNum)
        {
            if (nodeNum<0 || nodeNum>_kdNodes.size()-1)
            {
                osg::notify(osg::NOTICE)<<"Warning: getNode("<<nodeNum<<") _kdNodes.size()="<<_kdNodes.size()<<std::endl;
            }
            return _kdNodes[nodeNum];
        }


        osg::observer_ptr<osg::Geometry> _geometry;

        osg::BoundingBox                _bb;

        AxisStack                       _axisStack;
        KDNodeList                      _kdNodes;
        KDLeafList                      _kdLeaves; 
};

class KDTreeTraverser
{
    public:
    
    
        std::ostream& output(unsigned int level)
        {
            for(unsigned int i=0; i<level; ++i)
            {
                osg::notify(osg::NOTICE)<<"  ";
            }
            return osg::notify(osg::NOTICE);
        }
    

        void traverse(KDLeaf& leaf, unsigned int level)
        {
            output(level)<<"leaf("<<level<<") { ";

            for(unsigned int i=0; i<leaf._vertexIndices.size(); ++i)
            {
                if (i==0) osg::notify(osg::NOTICE)<<leaf._vertexIndices[i];
                else osg::notify(osg::NOTICE)<<", "<<leaf._vertexIndices[i];                
            }

            osg::notify(osg::NOTICE)<<"}"<<std::endl;;
            
        }

        void traverse(KDTree& tree, KDNode::value_type nodeIndex, unsigned int level)
        {
            output(level)<<"traverse("<<nodeIndex<<", "<< level<<") { "<<std::endl;
            
            if (nodeIndex>=0)
            {        
                KDNode& node = tree._kdNodes[nodeIndex];
                if (node._leftChild) traverse(tree,node._leftChild,level+1);
                else output(level+1)<<"empty left child()"<<std::endl;
                
                if (node._rightChild) traverse(tree,node._rightChild,level+1);
                else output(level+1)<<"empty right child()"<<std::endl;
            }
            else 
            {
                KDNode::value_type leafIndex = -nodeIndex-1;
                KDLeaf& leaf = *(tree._kdLeaves[leafIndex]);
                traverse(leaf, level);
            }

            output(level)<<"}"<<std::endl;;
        }


        void traverse(KDTree& tree)
        {
            osg::notify(osg::NOTICE)<<"traverse(tree)"<<std::endl;
            if (!tree._kdNodes.empty()) 
            {
                traverse(tree,0,0);
            }
            else if (!tree._kdLeaves.empty())
            {
                traverse(*tree._kdLeaves.front(), 0);
            }
        }
    
};


class KDTreeBuilder : public osg::NodeVisitor
{
    public:
    
        KDTreeBuilder():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _maxNumLevels(24),
            _targetNumVerticesPerLeaf(8)            
        {            
        }
    
    
        void apply(osg::Geode& geode)
        {
            for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
            {
                osg::Geometry* geom = geode.getDrawable(i)->asGeometry();
                if (geom)
                {
                    geom->setShape(createKDTree(geom));
                }   
            }
        }
    
        KDTree* createKDTree(osg::Geometry* geometry);
        
        void computeDivisions(KDTree& kdTree);
        
        int divide(KDTree& kdTree, osg::BoundingBox& bb, int nodeIndex, unsigned int level);

        unsigned int _maxNumLevels;
        unsigned int _targetNumVerticesPerLeaf;        

};


KDTree* KDTreeBuilder::createKDTree(osg::Geometry* geometry)
{
    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree()"<<std::endl;

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return 0;

    osg::ref_ptr<KDTree> kdTree = new KDTree;
    kdTree->_geometry = geometry;
    kdTree->_bb = kdTree->_geometry->getBound();
    
    
    unsigned int estimatedSize = (unsigned int)(float(vertices->size())/float(_targetNumVerticesPerLeaf)*1.5);

    osg::notify(osg::NOTICE)<<"kdTree->_kdNodes.reserve()="<<estimatedSize<<std::endl<<std::endl;

    kdTree->_kdNodes.reserve(estimatedSize);
    kdTree->_kdLeaves.reserve(estimatedSize);
    
    computeDivisions(*kdTree);

    // create initial leaf list    
    osg::ref_ptr<KDLeaf> leaf = new KDLeaf;
    leaf->_vertexIndices.reserve(vertices->size());
    for(unsigned int i=0; i<vertices->size(); ++i)
    {
        leaf->_vertexIndices.push_back(i);
    }

    osg::BoundingBox bb = kdTree->_bb;
    
    int leafNum = kdTree->addLeaf(leaf.get());
    int nodeNum = divide(*kdTree, bb, leafNum, 0);
    
    osg::notify(osg::NOTICE)<<"Root nodeNum="<<nodeNum<<std::endl;
    
    
    KDTreeTraverser traverser;
    traverser.traverse(*kdTree);
    
    osg::notify(osg::NOTICE)<<"Final kdTree->_kdNodes.size()="<<kdTree->_kdNodes.size()<<std::endl;
    osg::notify(osg::NOTICE)<<"Final kdTree->_kdLeaves.size()="<<kdTree->_kdLeaves.size()<<std::endl;

    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree() completed"<<std::endl<<std::endl;

    return kdTree.release();
}    

void KDTreeBuilder::computeDivisions(KDTree& kdTree)
{


    osg::Vec3 dimensions(kdTree._bb.xMax()-kdTree._bb.xMin(),
                         kdTree._bb.yMax()-kdTree._bb.yMin(),
                         kdTree._bb.zMax()-kdTree._bb.zMin());

    osg::notify(osg::NOTICE)<<"computeDivisions("<<_maxNumLevels<<") "<<dimensions<< " { "<<std::endl;

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

        osg::notify(osg::NOTICE)<<"  "<<level<<", "<<dimensions<<", "<<axis<<std::endl;
    }

    osg::notify(osg::NOTICE)<<"}"<<std::endl;
}

int KDTreeBuilder::divide(KDTree& kdTree, osg::BoundingBox& bb, int nodeIndex, unsigned int level)
{
    if (kdTree._axisStack.size()<=level) return nodeIndex;


    int axis = kdTree._axisStack[level];

    osg::notify(osg::NOTICE)<<"divide("<<nodeIndex<<", "<<level<< "), axis="<<axis<<std::endl;

    if (nodeIndex>=0)
    {
        osg::notify(osg::NOTICE)<<"  divide node"<<std::endl;
        KDNode& node = kdTree.getNode(nodeIndex);
        return nodeIndex;
    }
    else
    {    
        if (kdTree.getLeaf(nodeIndex)->_vertexIndices.size()<=_targetNumVerticesPerLeaf) return nodeIndex;
    
        osg::notify(osg::NOTICE)<<"  divide leaf"<<std::endl;
        
        int nodeNum = kdTree.addNode(KDNode());

        float original_min = bb._min[axis];
        float original_max = bb._max[axis];

        float mid = (original_min+original_max)*0.5f;

        {
            osg::ref_ptr<KDLeaf> leaf = kdTree.getLeaf(nodeIndex);

            // create new node, and add two leaves to it.
            osg::ref_ptr<KDLeaf> leftLeaf = new KDLeaf;
            osg::ref_ptr<KDLeaf> rightLeaf = new KDLeaf;


            osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(kdTree._geometry->getVertexArray());

            osg::notify(osg::NOTICE)<<"  divide leaf->_vertexIndices.size()="<<leaf->_vertexIndices.size()<<std::endl;

            unsigned int estimatedSize = leaf->_vertexIndices.size();
            leftLeaf->_vertexIndices.reserve(estimatedSize);
            rightLeaf->_vertexIndices.reserve(estimatedSize);

            for(unsigned int i=0; i<leaf->_vertexIndices.size(); ++i)
            {
                unsigned int vi = leaf->_vertexIndices[i];
                osg::Vec3& v = (*vertices)[vi];
                if (v[axis] <= mid) leftLeaf->_vertexIndices.push_back(vi);
                else rightLeaf->_vertexIndices.push_back(vi);
            }

            if (leftLeaf->_vertexIndices.empty())
            {
                osg::notify(osg::NOTICE)<<"LeftLeaf empty"<<std::endl;
                kdTree.getNode(nodeNum)._leftChild = 0;
                kdTree.getNode(nodeNum)._rightChild = kdTree.replaceLeaf(nodeIndex, rightLeaf.get());
            }
            else if (rightLeaf->_vertexIndices.empty())
            {
                osg::notify(osg::NOTICE)<<"RightLeaf empty"<<std::endl;
                kdTree.getNode(nodeNum)._leftChild = kdTree.replaceLeaf(nodeIndex, leftLeaf.get());
                kdTree.getNode(nodeNum)._rightChild = 0;
            }
            else
            {
                kdTree.getNode(nodeNum)._leftChild = kdTree.replaceLeaf(nodeIndex, leftLeaf.get());
                kdTree.getNode(nodeNum)._rightChild = kdTree.addLeaf(rightLeaf.get());
            }


        }
        
        int originalLeftChildIndex = kdTree.getNode(nodeNum)._leftChild;
        int originalRightChildIndex = kdTree.getNode(nodeNum)._rightChild;


        float restore = bb._max[axis];
        bb._max[axis] = mid;

        osg::notify(osg::NOTICE)<<"  divide leftLeaf "<<kdTree.getNode(nodeNum)._leftChild<<std::endl;
        int leftChildIndex = divide(kdTree, bb, originalLeftChildIndex, level+1);

        bb._max[axis] = restore;
        
        restore = bb._min[axis];
        bb._min[axis] = mid;

        osg::notify(osg::NOTICE)<<"  divide rightLeaf "<<kdTree.getNode(nodeNum)._rightChild<<std::endl;
        int rightChildIndex = divide(kdTree, bb, originalRightChildIndex, level+1);
        
        bb._min[axis] = restore;
        
        kdTree.getNode(nodeNum)._leftChild = leftChildIndex;
        kdTree.getNode(nodeNum)._rightChild = rightChildIndex; 
        
        return nodeNum;        
    }


}

}

int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    osg::ref_ptr<osg::Node> scene = osgDB::readNodeFiles(arguments);
    
    if (!scene) 
    {
        std::cout<<"No model loaded, please specify a valid model on the command line."<<std::endl;
        return 0;
    }
    
    osg::KDTreeBuilder builder;
    scene->accept(builder);
    
    
    
    return 0;
}
