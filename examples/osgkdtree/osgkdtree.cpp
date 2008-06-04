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

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Geometry>

#include <osgDB/ReadFile>

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
    
        typedef short value_type;

        value_type _leftChild;
        value_type _rightChild;
};

class KDLeaf : public osg::Referenced
{
    public:
    
        KDLeaf();
    
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

        osg::observer_ptr<osg::Geometry> _geometry;

        osg::BoundingBox                _bb;

        AxisStack                       _axisStack;
        KDNodeList                      _kdNodes;
        KDLeafList                      _kdLeaves; 
};

class KDTreeTraverser
{
    public:
    
        void traverse(KDTree& tree, KDNode::value_type nodeIndex, unsigned int level)
        {
            for(unsigned int i=0; i<level; ++i)
            {
                osg::notify(osg::NOTICE)<<"  ";
            }
            osg::notify(osg::NOTICE)<<"traverse("<<nodeIndex<<", "<< level<<") { "<<std::endl;
            
            if (nodeIndex>=0)
            {        
                KDNode& node = tree._kdNodes[nodeIndex];
                traverse(tree,node._leftChild,level+1);
                traverse(tree,node._rightChild,level+1);
            }
            else 
            {
                KDNode::value_type leafIndex = -nodeIndex-1;
                KDLeaf& leaf = *(tree._kdLeaves[leafIndex]);
            }

            for(unsigned int i=0; i<level; ++i)
            {
                osg::notify(osg::NOTICE)<<"  ";
            }
            osg::notify(osg::NOTICE)<<"}"<<std::endl;;
        }


        void traverse(KDTree& tree)
        {
            osg::notify(osg::NOTICE)<<"traverse(tree)"<<std::endl;
            if (!tree._kdNodes.empty()) traverse(tree,0,0);
        }
    
};


class KDTreeBuilder : public osg::NodeVisitor
{
    public:
    
        KDTreeBuilder():
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
    
    
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
};


KDTree* KDTreeBuilder::createKDTree(osg::Geometry* geometry)
{
    KDTree* kdTree = new KDTree;
    kdTree->_geometry = geometry;
    kdTree->_bb = kdTree->_geometry->getBound();
    
    osg::notify(osg::NOTICE)<<"osg::KDTreeBuilder::createKDTree()"<<std::endl;
    
    return kdTree;
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
