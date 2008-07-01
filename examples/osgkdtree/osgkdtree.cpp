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

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Geometry>


#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/UpdateVisitor>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include <iostream>


namespace osg
{

typedef int value_type;
typedef std::vector< value_type >   Indices;

//#define VERBOSE_OUTPUT

typedef std::pair< value_type, value_type> KDNode;
typedef std::pair< value_type, value_type> KDLeaf;


class KDTree : public osg::Shape
{
    public:
    
    
        KDTree() {}
        
        KDTree(const KDTree& rhs, const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            Shape(rhs,copyop) {}

        META_Shape(osg, KDTree)
        
    
        typedef std::vector< unsigned int > AxisStack;
        typedef std::vector< KDNode >       KDNodeList;

        typedef std::vector< KDLeaf > KDLeafList;

        /// note, leafNum is negative to distinguish from nodeNum
        int addLeaf(const KDLeaf& leaf) { int num = _kdLeaves.size(); _kdLeaves.push_back(leaf); return -(num+1); }

        int replaceLeaf(int leafNum, const KDLeaf& leaf)
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
        KDLeaf& getLeaf(int leafNum)
        {
            int num = -leafNum-1;
            if (num<0 || num>_kdLeaves.size()-1)
            {
                osg::notify(osg::NOTICE)<<"Warning: getLeaf("<<leafNum<<", num = "<<num<<") _kdLeaves.size()="<<_kdLeaves.size()<<std::endl;
            }

            return _kdLeaves[num];
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


        osg::observer_ptr<osg::Geometry>    _geometry;

        osg::BoundingBox                    _bb;

        AxisStack                           _axisStack;
        KDNodeList                          _kdNodes;
        KDLeafList                          _kdLeaves;

        osg::ref_ptr<osg::Vec3Array>        _vertices;
        Indices                             _vertexIndices;
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
    

        void traverse(KDTree& tree, KDLeaf& leaf, unsigned int level)
        {
            output(level)<<"leaf("<<level<<") { ";

            unsigned int end = leaf.first+leaf.second;
            for(unsigned int i=leaf.first; i<end; ++i)
            {
                if (i==leaf.first) osg::notify(osg::NOTICE)<<tree._vertexIndices[i];
                else osg::notify(osg::NOTICE)<<", "<<tree._vertexIndices[i];                
            }

            osg::notify(osg::NOTICE)<<"}"<<std::endl;;
            
        }

        void traverse(KDTree& tree, value_type nodeIndex, unsigned int level)
        {
            output(level)<<"traverse("<<nodeIndex<<", "<< level<<") { "<<std::endl;
            
            if (nodeIndex>=0)
            {        
                KDNode& node = tree._kdNodes[nodeIndex];
                if (node.first) traverse(tree,node.first,level+1);
                else output(level+1)<<"empty left child()"<<std::endl;
                
                if (node.second) traverse(tree,node.second,level+1);
                else output(level+1)<<"empty right child()"<<std::endl;
            }
            else 
            {
                value_type leafIndex = -nodeIndex-1;

                KDLeaf& leaf = tree._kdLeaves[leafIndex];

                traverse(tree, leaf, level);
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
                traverse(tree, tree._kdLeaves.front(), 0);
            }
        }
    
};


class KDTreeBuilder : public osg::NodeVisitor
{
    public:
    
        KDTreeBuilder():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _maxNumLevels(24),
            _targetNumVerticesPerLeaf(8),
            _numVerticesProcessed(0)
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
        
        unsigned int _numVerticesProcessed;   

};


KDTree* KDTreeBuilder::createKDTree(osg::Geometry* geometry)
{
#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree()"<<std::endl;
#endif

    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return 0;

    osg::ref_ptr<KDTree> kdTree = new KDTree;
    kdTree->_geometry = geometry;
    kdTree->_bb = kdTree->_geometry->getBound();
    kdTree->_vertices = vertices;
    
    unsigned int estimatedSize = (unsigned int)(float(vertices->size())/float(_targetNumVerticesPerLeaf)*1.5);

#ifdef VERBOSE_OUTPUT    
    osg::notify(osg::NOTICE)<<"kdTree->_kdNodes.reserve()="<<estimatedSize<<std::endl<<std::endl;
#endif

    kdTree->_kdNodes.reserve(estimatedSize);
    kdTree->_kdLeaves.reserve(estimatedSize);
    
    computeDivisions(*kdTree);


    _numVerticesProcessed += vertices->size();

    kdTree->_vertexIndices.reserve(vertices->size());
    for(unsigned int i=0; i<vertices->size(); ++i)
    {
        kdTree->_vertexIndices.push_back(i);
    }
    
    KDLeaf leaf(0, kdTree->_vertexIndices.size());

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

        if (kdTree.getLeaf(nodeIndex).second<=_targetNumVerticesPerLeaf) return nodeIndex;

        //osg::notify(osg::NOTICE)<<"  divide leaf"<<std::endl;
        
        int nodeNum = kdTree.addNode(KDNode());

        float original_min = bb._min[axis];
        float original_max = bb._max[axis];

        float mid = (original_min+original_max)*0.5f;

        {
            KDLeaf& leaf = kdTree.getLeaf(nodeIndex);

            osg::Vec3Array* vertices = kdTree._vertices.get();

            //osg::notify(osg::NOTICE)<<"  divide leaf->_vertexIndices.size()="<<leaf->_vertexIndices.size()<<std::endl;

            unsigned int estimatedSize = leaf.second;

            int end = leaf.first+leaf.second-1;
            int left = leaf.first;
            int right = leaf.first+leaf.second-1;
            
            while(left<right)
            {
                while(left<right && ((*vertices)[kdTree._vertexIndices[left]])[axis]<=mid) { ++left; }
                
                while(left<right && ((*vertices)[kdTree._vertexIndices[right]])[axis]>mid) { --right; }

                if (left<right)
                {
                    std::swap(kdTree._vertexIndices[left], kdTree._vertexIndices[right]);
                    ++left;
                    --right;
                }
            }
            
            if (left==right)
            {
                if (((*vertices)[kdTree._vertexIndices[left]])[axis]<=mid) ++left;
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


    osgUtil::UpdateVisitor updateVisitor;
    updateVisitor.setFrameStamp(new osg::FrameStamp);
    scene->accept(updateVisitor);
    scene->getBound();


    osg::Timer_t start = osg::Timer::instance()->tick();
    
    osg::KDTreeBuilder builder;
    scene->accept(builder);
    
    osg::Timer_t end = osg::Timer::instance()->tick();
    double time = osg::Timer::instance()->delta_s(start,end);
    osg::notify(osg::NOTICE)<<"Time to build "<<time*1000.0<<"ms "<<builder._numVerticesProcessed<<std::endl;
    osg::notify(osg::NOTICE)<<"build speed "<<(double(builder._numVerticesProcessed)/time)/1000000.0<<"M vertices per second"<<std::endl;
    
    
    return 0;
}
