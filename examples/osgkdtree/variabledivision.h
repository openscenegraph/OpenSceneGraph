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

#ifndef VARIABLEDIVISION_H
#define VARIABLEDIVISION_H

  
#include <osgDB/ReadFile>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Timer>
#include <osg/CoordinateSystemNode>
#include <osg/Notify>
#include <osg/io_utils>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor>


#include <osgUtil/IntersectionVisitor>
#include <osgUtil/LineSegmentIntersector>
#include <osgUtil/UpdateVisitor>

#include <osgSim/LineOfSight>
#include <osgSim/HeightAboveTerrain>
#include <osgSim/ElevationSlice>

#include <iostream>


namespace variabledivision
{

typedef int value_type;
typedef std::vector< value_type >   Indices;

//#define VERBOSE_OUTPUT

typedef std::pair< value_type, value_type> KDNode;
typedef std::pair< value_type, value_type> KDLeaf;

struct Triangle
{
    Triangle(unsigned int p1, unsigned int p2, unsigned int p3):
        _p1(p1), _p2(p2), _p3(p3) {}
        
    bool operator < (const Triangle& rhs) const
    {
        if (_p1<rhs._p1) return true;
        if (_p1>rhs._p1) return false;
        if (_p2<rhs._p2) return true;
        if (_p2>rhs._p2) return false;
        return _p3<rhs._p3;
    }
    
    unsigned int _p1;
    unsigned int _p2;
    unsigned int _p3;    
};

class KDTree : public osg::Shape
{
    public:
    
    
        KDTree() {}
        
        KDTree(const KDTree& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
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


        typedef std::vector< osg::BoundingBox >                 BoundingBoxList;
        typedef std::vector< Triangle >                         TriangleList;
        typedef std::vector< osg::Vec3 >                        CenterList;

        osg::observer_ptr<osg::Geometry>    _geometry;

        osg::BoundingBox                    _bb;

        AxisStack                           _axisStack;
        KDNodeList                          _kdNodes;
        KDLeafList                          _kdLeaves;

        osg::ref_ptr<osg::Vec3Array>        _vertices;
        
        Indices                             _primitiveIndices;
        
        BoundingBoxList                     _boundingBoxes;
        TriangleList                        _triangles;
        CenterList                          _centers;
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
                if (i==leaf.first) osg::notify(osg::NOTICE)<<tree._primitiveIndices[i];
                else osg::notify(osg::NOTICE)<<", "<<tree._primitiveIndices[i];
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
            _maxNumLevels(16),
            _targetNumTrianglesPerLeaf(4),
            _numVerticesProcessed(0),
            _processTriangles(true)
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
        unsigned int _targetNumTrianglesPerLeaf;
        
        unsigned int _numVerticesProcessed;
        bool         _processTriangles;
        
};


}

#endif
