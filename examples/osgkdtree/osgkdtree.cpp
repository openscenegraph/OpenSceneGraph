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

#include <osgDB/ReadFile>

#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include <iostream>

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

class KDTree : public osg::Referenced
{
    public:
    
        typedef std::vector< unsigned int > AxisStack;
        typedef std::vector< KDNode > KDNodeList;
        typedef std::vector< osg::ref_ptr<KDLeaf> > KDLeafList;

        osg::BoundingBox    _bb;

        AxisStack           _axisStack;
        KDNodeList          _kdNodes;
        KDLeafList          _kdLeaves; 
};

class KDTreeTraverser
{
    public:
    
        void traverse(KDTree& tree, KDNode::value_type nodeIndex)
        {
            if (nodeIndex>=0)
            {        
                KDNode& node = tree._kdNodes[nodeIndex];
                traverse(tree,node._leftChild);
                traverse(tree,node._rightChild);
            }
            else 
            {
                KDNode::value_type leafIndex = -nodeIndex-1;
                KDLeaf& leaf = *(tree._kdLeaves[leafIndex]);
            }
        }


        void traverse(KDTree& tree)
        {
            if (!tree._kdNodes.empty()) traverse(tree,0);
        }
    
};

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
    
    
    return 0;
}
