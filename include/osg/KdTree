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

#ifndef OSG_KDTREE
#define OSG_KDTREE 1

#include <osg/Shape>
#include <osg/Geometry>

#include <map>

namespace osg
{

/** Implementation of a kdtree for Geometry leaves, to enable fast intersection tests.*/
class OSG_EXPORT KdTree : public osg::Shape
{
    public:


        KdTree();

        KdTree(const KdTree& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Shape(osg, KdTree)

        struct OSG_EXPORT BuildOptions
        {
            BuildOptions();

            unsigned int _numVerticesProcessed;
            unsigned int _targetNumTrianglesPerLeaf;
            unsigned int _maxNumLevels;
        };


        /** Build the kdtree from the specified source geometry object.
          * retun true on success. */
        virtual bool build(BuildOptions& buildOptions, osg::Geometry* geometry);


        void setVertices(osg::Vec3Array* vertices) { _vertices = vertices; }
        const osg::Vec3Array* getVertices() const { return _vertices.get(); }


        typedef std::vector< unsigned int > Indices;

        // index in the VertexIndices vector
        void setPrimitiveIndices(const Indices& indices) { _primitiveIndices = indices; }
        Indices& getPrimitiveIndices() { return _primitiveIndices; }
        const Indices& getPrimitiveIndices() const { return _primitiveIndices; }

        // vector containing the primitive vertex index data packed as no_vertice_indices then vertex indices ie. for points it's (1, p0), for lines (2, p0, p1) etc.
        void setVertexIndices(const Indices& indices) { _vertexIndices = indices; }
        Indices& getVertexIndices() { return _vertexIndices; }
        const Indices& getVertexIndices() const { return _vertexIndices; }


        inline unsigned int addPoint(unsigned int p0)
        {
            unsigned int i = _vertexIndices.size();
            _vertexIndices.push_back(_primitiveIndices.size() + _degenerateCount);
            _vertexIndices.push_back(1);
            _vertexIndices.push_back(p0);
            _primitiveIndices.push_back(i);
            return i;
        }
        inline unsigned int addLine(unsigned int p0, unsigned int p1)
        {
            unsigned int i = _vertexIndices.size();
            _vertexIndices.push_back(_primitiveIndices.size() + _degenerateCount);
            _vertexIndices.push_back(2);
            _vertexIndices.push_back(p0);
            _vertexIndices.push_back(p1);
            _primitiveIndices.push_back(i);
            return i;
        }

        inline unsigned int addTriangle(unsigned int p0, unsigned int p1, unsigned int p2)
        {
            unsigned int i = _vertexIndices.size();
            _vertexIndices.push_back(_primitiveIndices.size() + _degenerateCount);
            _vertexIndices.push_back(3);
            _vertexIndices.push_back(p0);
            _vertexIndices.push_back(p1);
            _vertexIndices.push_back(p2);
            _primitiveIndices.push_back(i);
            return i;
        }

        inline unsigned int addQuad(unsigned int p0, unsigned int p1, unsigned int p2, unsigned int p3)
        {
            unsigned int i = _vertexIndices.size();
            _vertexIndices.push_back(_primitiveIndices.size() + _degenerateCount);
            _vertexIndices.push_back(4);
            _vertexIndices.push_back(p0);
            _vertexIndices.push_back(p1);
            _vertexIndices.push_back(p2);
            _vertexIndices.push_back(p3);
            _primitiveIndices.push_back(i);
            return i;
        }



        typedef int value_type;

        struct KdNode
        {
            KdNode():
                first(0),
                second(0) {}

            KdNode(value_type f, value_type s):
                first(f),
                second(s) {}

            osg::BoundingBox bb;

            value_type first;
            value_type second;
        };
        typedef std::vector< KdNode >       KdNodeList;

        int addNode(const KdNode& node)
        {
            int num = static_cast<int>(_kdNodes.size());
            _kdNodes.push_back(node);
            return num;
        }

        KdNode& getNode(int nodeNum) { return _kdNodes[nodeNum]; }
        const KdNode& getNode(int nodeNum) const { return _kdNodes[nodeNum]; }

        KdNodeList& getNodes() { return _kdNodes; }
        const KdNodeList& getNodes() const { return _kdNodes; }


        template<class IntersectFunctor>
        void intersect(IntersectFunctor& functor, const KdNode& node) const
        {
            if (node.first<0)
            {
                // treat as a leaf
                int istart = -node.first-1;
                int iend = istart + node.second;

                for(int i=istart; i<iend; ++i)
                {
                    unsigned int primitiveIndex = _primitiveIndices[i];
                    unsigned int originalPIndex = _vertexIndices[primitiveIndex++];
                    unsigned int numVertices = _vertexIndices[primitiveIndex++];
                    switch(numVertices)
                    {
                        case(1): functor.intersect(_vertices.get(), originalPIndex, _vertexIndices[primitiveIndex]); break;
                        case(2): functor.intersect(_vertices.get(), originalPIndex, _vertexIndices[primitiveIndex], _vertexIndices[primitiveIndex+1]); break;
                        case(3): functor.intersect(_vertices.get(), originalPIndex, _vertexIndices[primitiveIndex], _vertexIndices[primitiveIndex+1], _vertexIndices[primitiveIndex+2]); break;
                        case(4): functor.intersect(_vertices.get(), originalPIndex, _vertexIndices[primitiveIndex], _vertexIndices[primitiveIndex+1], _vertexIndices[primitiveIndex+2], _vertexIndices[primitiveIndex+3]); break;
                        default : OSG_NOTICE<<"Warning: KdTree::intersect() encounted unsupported primitive size of "<<numVertices<<std::endl; break;
                    }
                }
            }
            else if (functor.enter(node.bb))
            {
                if (node.first>0) intersect(functor, _kdNodes[node.first]);
                if (node.second>0) intersect(functor, _kdNodes[node.second]);

                functor.leave();
            }
        }

        unsigned int _degenerateCount;

    protected:

        osg::ref_ptr<osg::Vec3Array>    _vertices;
        Indices                         _primitiveIndices;
        Indices                         _vertexIndices;
        KdNodeList                      _kdNodes;
};

class OSG_EXPORT KdTreeBuilder : public osg::NodeVisitor
{
    public:

        KdTreeBuilder();

        KdTreeBuilder(const KdTreeBuilder& rhs);

        META_NodeVisitor(osg, KdTreeBuilder)

        virtual KdTreeBuilder* clone() { return new KdTreeBuilder(*this); }

        void apply(Geometry& geometry);

        KdTree::BuildOptions _buildOptions;

        osg::ref_ptr<osg::KdTree> _kdTreePrototype;



    protected:

        virtual ~KdTreeBuilder() {}

};

}

#endif
