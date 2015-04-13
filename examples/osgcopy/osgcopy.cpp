/* OpenSceneGraph example, osgcopy.
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

#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>

#include <osgUtil/Optimizer>

#include <iostream>

// Customize the CopyOp so that we add our own verbose 
// output of what's being copied.
class MyCopyOp : public osg::CopyOp
{
    public:
    
        inline MyCopyOp(CopyFlags flags=SHALLOW_COPY):
            osg::CopyOp(flags),
            _indent(0),
            _step(4) {}

        inline void moveIn() const { _indent += _step; }
        inline void moveOut() const { _indent -= _step; }
        inline void writeIndent() const 
        {
            for(int i=0;i<_indent;++i) std::cout << " ";
        }
    
        virtual osg::Referenced*     operator() (const osg::Referenced* ref) const
        {
            writeIndent(); std::cout << "copying Referenced "<<ref<<std::endl;
            moveIn();
            osg::Referenced* ret_ref = CopyOp::operator()(ref);
            moveOut();
            return ret_ref;
        }
        
        virtual osg::Object*         operator() (const osg::Object* obj) const
        {
            writeIndent(); std::cout << "copying Object "<<obj;
            if (obj) std::cout<<" "<<obj->className();
            std::cout<<std::endl;
            moveIn();
            osg::Object* ret_obj = CopyOp::operator()(obj);
            moveOut();
            return ret_obj;
        }
        
        virtual osg::Node*           operator() (const osg::Node* node) const
        {
            writeIndent(); std::cout << "copying Node "<<node;
            if (node) std::cout<<" "<<node->className()<<" '"<<node->getName()<<"'";
            std::cout<<std::endl;
            moveIn();
            osg::Node* ret_node = CopyOp::operator()(node);
            moveOut();
            return ret_node;
        }

        virtual osg::Drawable*       operator() (const osg::Drawable* drawable) const
        {
            writeIndent(); std::cout << "copying Drawable "<<drawable;
            if (drawable) std::cout<<" "<<drawable->className();
            std::cout<<std::endl;
            moveIn();
            osg::Drawable* ret_drawable = CopyOp::operator()(drawable);
            moveOut();
            return ret_drawable;
        }

        virtual osg::StateSet*       operator() (const osg::StateSet* stateset) const
        {
            writeIndent(); std::cout << "copying StateSet "<<stateset;
            if (stateset) std::cout<<" "<<stateset->className();
            std::cout<<std::endl;
            moveIn();
            osg::StateSet* ret_stateset = CopyOp::operator()(stateset);
            moveOut();
            return ret_stateset;
        }

        virtual osg::StateAttribute* operator() (const osg::StateAttribute* attr) const
        {
            writeIndent(); std::cout << "copying StateAttribute "<<attr;
            if (attr) std::cout<<" "<<attr->className();
            std::cout<<std::endl;
            moveIn();
            osg::StateAttribute* ret_attr = CopyOp::operator()(attr);
            moveOut();
            return ret_attr;
        }

        virtual osg::Texture*        operator() (const osg::Texture* text) const
        {
            writeIndent(); std::cout << "copying Texture "<<text;
            if (text) std::cout<<" "<<text->className();
            std::cout<<std::endl;
            moveIn();
            osg::Texture* ret_text = CopyOp::operator()(text);
            moveOut();
            return ret_text;
        }

        virtual osg::Image*          operator() (const osg::Image* image) const
        {
            writeIndent(); std::cout << "copying Image "<<image;
            if (image) std::cout<<" "<<image->className();
            std::cout<<std::endl;
            moveIn();
            osg::Image* ret_image = CopyOp::operator()(image);
            moveOut();
            return ret_image;
        }
        
    protected:
    
        // must be mutable since CopyOp is passed around as const to
        // the various clone/copy constructors.
        mutable int _indent;
        mutable int _step;
};

// this CopyOp class will preserve the multi-parent structure of the copied 
// object, instead of expanding it into a tree. Works with the 
// DEEP_COPY_NODES flag.
class GraphCopyOp : public osg::CopyOp
{
    public:
    
        inline GraphCopyOp(CopyFlags flags=SHALLOW_COPY):
            osg::CopyOp(flags) { _nodeCopyMap.clear();}
            
        virtual osg::Node* operator() (const osg::Node* node) const
        {
            if (node && _flags&DEEP_COPY_NODES)
            {
                if ( node->getNumParents() > 1 )
                {
                    if ( _nodeCopyMap.find(node) != _nodeCopyMap.end() )
                    {
                        std::cout<<"Copy of node "<<node<<" "<<node->getName()<<", "
                            << _nodeCopyMap[node]<<", will be reused"<<std::endl;
                        return (osg::Node*)(_nodeCopyMap[node]);
                    }
                    else
                    {
                        osg::Node* newNode = dynamic_cast<osg::Node*>( node->clone(*this) );
                        _nodeCopyMap[node] = newNode;
                        return newNode;
                    }
                }
                else
                    return dynamic_cast<osg::Node*>( node->clone(*this) );
            }
            else
                return const_cast<osg::Node*>(node);
        }
         
    protected:
    
        // must be mutable since CopyOp is passed around as const to
        // the various clone/copy constructors.
        mutable std::map<const osg::Node*,osg::Node*> _nodeCopyMap;

};

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // initialize the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
    
// -------------    Start of copy specific code -------------------------------------------------------
    
    // do a deep copy, using MyCopyOp to reveal whats going on under the hood,
    // in your own code you'd typically just use the basic osg::CopyOp something like
    osg::ref_ptr<osg::Node> mycopy = dynamic_cast<osg::Node*>(rootnode->clone(osg::CopyOp::DEEP_COPY_ALL));
    std::cout << "Doing a deep copy of scene graph"<<std::endl;

    // note, we need the dyanmic_cast because MS Visual Studio can't handle covarient
    // return types, so that clone has return just Object*.  bahh hum bug
    osg::ref_ptr<osg::Node> deep_copy = dynamic_cast<osg::Node*>(rootnode->clone(MyCopyOp(osg::CopyOp::DEEP_COPY_ALL)));
    
    std::cout << "----------------------------------------------------------------"<<std::endl;

    // do a graph preserving deep copy.
    std::cout << "Doing a graph preserving deep copy of scene graph nodes"<<std::endl;
    osg::ref_ptr<osg::Node> graph_copy = dynamic_cast<osg::Node*>(rootnode->clone(GraphCopyOp(osg::CopyOp::DEEP_COPY_NODES)));


    // do a shallow copy.
    std::cout << "Doing a shallow copy of scene graph"<<std::endl;
    osg::ref_ptr<osg::Node> shallow_copy = dynamic_cast<osg::Node*>(rootnode->clone(MyCopyOp(osg::CopyOp::SHALLOW_COPY)));


    // write out the various scene graphs so that they can be browsed, either
    // in an editor or using a graphics diff tool gdiff/xdiff/xxdiff.
    std::cout << std::endl << "Writing out the original scene graph as 'original.osgt'"<<std::endl;
    osgDB::writeNodeFile(*rootnode,"original.osgt");

    std::cout << std::endl << "Writing out the graph preserving scene graph as 'graph_copy.osgt'"<<std::endl;
    osgDB::writeNodeFile(*graph_copy,"graph_copy.osgt");

    std::cout << "Writing out the deep copied scene graph as 'deep_copy.osgt'"<<std::endl;
    osgDB::writeNodeFile(*deep_copy,"deep_copy.osgt");

    std::cout << "Writing out the shallow copied scene graph as 'shallow_copy.osgt'"<<std::endl;
    osgDB::writeNodeFile(*shallow_copy,"shallow_copy.osgt");


    // You can use a bit mask to control which parts of the scene graph are shallow copied
    // vs deep copied.  The options are (from include/osg/CopyOp) :
    // enum Options {
    //        SHALLOW_COPY = 0,
    //        DEEP_COPY_OBJECTS = 1,
    //        DEEP_COPY_NODES = 2,
    //        DEEP_COPY_DRAWABLES = 4,
    //        DEEP_COPY_STATESETS = 8,
    //        DEEP_COPY_STATEATTRIBUTES = 16,
    //        DEEP_COPY_TEXTURES = 32,
    //        DEEP_COPY_IMAGES = 64,
    //        DEEP_COPY_ALL = 0xffffffff
    // };
    // 
    // These options you can use together such as :
    //    osg::Node* mycopy = dynamic_cast<osg::Node*>(rootnode->clone(osg::CopyOp::DEEP_COPY_NODES | DEEP_COPY_DRAWABLES));
    // Which shares state but creates copies of all nodes and drawables (which contain the geometry).
    // 
    // You may also want to subclass from CopyOp to provide finer grained control of what gets shared (shallow copy) vs
    // cloned (deep copy).
    


// -------------    End of copy specific code -------------------------------------------------------
     
    // set the scene to render
    viewer.setSceneData(rootnode);

    // viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    return viewer.run();
}
