#include <osg/MatrixTransform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgProducer/Viewer>

#include <osgUtil/Optimizer>

using namespace osg;

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

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of deep vs shallow vs custom copying of scene graphs.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // initialize the viewer.
    osgProducer::Viewer viewer(arguments);

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
    
// -------------    Start of copy specific code -------------------------------------------------------
    
    // do a deep copy, using MyCopyOp to reveal whats going on under the good,
    // in your own code you'd typically just use the basic osg::CopyOp something like
    // osg::Node* mycopy = dynamic_cast<osg::Node*>(rootnode->clone(osg::CopyOp::DEEP_COPY_ALL));
    std::cout << "Doing a deep copy of scene graph"<<std::endl;
    // note, we need the dyanmic_cast because MS Visual Studio can't handle covarient
    // return types, so that clone has return just Object*.  bahh hum bug
    osg::Node* deep_copy = dynamic_cast<osg::Node*>(rootnode->clone(MyCopyOp(osg::CopyOp::DEEP_COPY_ALL)));
    
    std::cout << "----------------------------------------------------------------"<<std::endl;

    // do a shallow copy.
    std::cout << "Doing a shallow copy of scene graph"<<std::endl;
    osg::Node* shallow_copy = dynamic_cast<osg::Node*>(rootnode->clone(MyCopyOp(osg::CopyOp::SHALLOW_COPY)));


    // write out the various scene graphs so that they can be browsed, either
    // in an editor or using a graphics diff tool gdiff/xdiff/xxdiff.
    std::cout << std::endl << "Writing out the original scene graph as 'original.osg'"<<std::endl;
    osgDB::writeNodeFile(*rootnode,"original.osg");

    std::cout << "Writing out the deep copied scene graph as 'deep_copy.osg'"<<std::endl;
    osgDB::writeNodeFile(*deep_copy,"deep_copy.osg");

    std::cout << "Writing out the shallow copied scene graph as 'shallow_copy.osg'"<<std::endl;
    osgDB::writeNodeFile(*shallow_copy,"shallow_copy.osg");


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

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
