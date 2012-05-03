// -*-c++-*-

/*
 * Wavefront OBJ loader for Open Scene Graph
 *
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * Modified by Robert Osfield to support per Drawable coord, normal and
 * texture coord arrays, bug fixes, and support for texture mapping.
 *
 * Writing support added 2007 by Stephan Huber, http://digitalmind.de,
 * some ideas taken from the dae-plugin
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */

 #ifndef OBJ_WRITER_NODE_VISITOR_HEADER__
 #define OBJ_WRITER_NODE_VISITOR_HEADER__


#include <string>
#include <stack>
#include <sstream>

#include <osg/Notify>
#include <osg/Node>
#include <osg/MatrixTransform>
#include <osg/Geode>

#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexGen>
#include <osg/TexMat>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>




#include <map>
#include <set>

class OBJWriterNodeVisitor: public osg::NodeVisitor {

    public:
        OBJWriterNodeVisitor(std::ostream& fout, const std::string materialFileName = "") :
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
            _fout(fout),
            _currentStateSet(new osg::StateSet()),
            _lastVertexIndex(1),
            _lastNormalIndex(1),
            _lastTexIndex(1)
        {
            _fout << "# file written by OpenSceneGraph" << std::endl << std::endl;

            if (!materialFileName.empty()) {
                _fout << "mtllib " << materialFileName << std::endl << std::endl;
            }
        }

        virtual void apply(osg::Geode &node);

        virtual void apply(osg::Group &node)
        {
            _nameStack.push_back( node.getName().empty() ? node.className() : node.getName() );
            _fout << std::endl;
            _fout << "g " << getUniqueName() << std::endl;

            osg::NodeVisitor::traverse( node );
            _nameStack.pop_back();
        }

        void traverse (osg::Node &node)
        {
            pushStateSet(node.getStateSet());

            osg::NodeVisitor::traverse( node );

            popStateSet(node.getStateSet());
        }

        void pushStateSet(osg::StateSet* ss)
        {
          if (NULL!=ss) {
            // Save our current stateset
            _stateSetStack.push(_currentStateSet.get());

            // merge with node stateset
            _currentStateSet = static_cast<osg::StateSet*>(_currentStateSet->clone(osg::CopyOp::SHALLOW_COPY));
            _currentStateSet->merge(*ss);
          }
        }


        void popStateSet(osg::StateSet* ss)
        {
            if (NULL!=ss) {
              // restore the previous stateset
              _currentStateSet = _stateSetStack.top();
              _stateSetStack.pop();
            }
        }


        void writeMaterials(std::ostream& fout);



        class OBJMaterial {
            public:
                OBJMaterial() {}
                OBJMaterial(osg::Material* mat, osg::Texture* tex);

                osg::Vec4  diffuse, ambient, specular;
                std::string    image;
                std::string name;
        };

    protected:
        struct CompareStateSet
        {
            bool operator()(const osg::ref_ptr<osg::StateSet>& ss1, const osg::ref_ptr<osg::StateSet>& ss2) const
            {
                //std::cout << "CompareStateSet: " << ss1->compare(*ss2, false) << " " << ss1 << " " << ss2 << std::endl;
                return ss1->compare(*ss2, true) < 0;
            }
        };


    private:

        OBJWriterNodeVisitor& operator = (const OBJWriterNodeVisitor&) { return *this; }

        void processGeometry(osg::Geometry* geo, osg::Matrix& m);
        void processArray(const std::string& key, osg::Array* array, const osg::Matrix& m = osg::Matrix::identity(), bool isNormal = false);

        void processStateSet(osg::StateSet* stateset);

        std::string getUniqueName(const std::string& defaultValue = "");

        typedef std::stack<osg::ref_ptr<osg::StateSet> > StateSetStack;
        typedef std::map< osg::ref_ptr<osg::StateSet>, OBJMaterial, CompareStateSet> MaterialMap;


        std::ostream&                            _fout;
        std::list<std::string>                    _nameStack;
        StateSetStack                            _stateSetStack;
        osg::ref_ptr<osg::StateSet>                _currentStateSet;
        std::map<std::string, unsigned int>        _nameMap;
        unsigned int                            _lastVertexIndex, _lastNormalIndex, _lastTexIndex;
        MaterialMap                                _materialMap;

};

#endif
