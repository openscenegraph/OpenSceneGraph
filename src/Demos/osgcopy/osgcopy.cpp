#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Texture>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgUtil/TrackballManipulator>
#include <osgUtil/FlightManipulator>
#include <osgUtil/DriveManipulator>

#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osgUtil/Optimizer>

void write_usage(std::ostream& out,const std::string& name)
{
    out << std::endl;
    out <<"usage:"<< std::endl;
    out <<"    "<<name<<" [options] infile1 [infile2 ...]"<< std::endl;
    out << std::endl;
    out <<"options:"<< std::endl;
    out <<"    -l libraryName      - load plugin of name libraryName"<< std::endl;
    out <<"                          i.e. -l osgdb_pfb"<< std::endl;
    out <<"                          Useful for loading reader/writers which can load"<< std::endl;
    out <<"                          other file formats in addition to its extension."<< std::endl;
    out <<"    -e extensionName    - load reader/wrter plugin for file extension"<< std::endl;
    out <<"                          i.e. -e pfb"<< std::endl;
    out <<"                          Useful short hand for specifying full library name as"<< std::endl;
    out <<"                          done with -l above, as it automatically expands to"<< std::endl;
    out <<"                          the full library name appropriate for each platform."<< std::endl;
    out <<std::endl;
    out <<"    -stereo             - switch on stereo rendering, using the default of,"<< std::endl;
    out <<"                          ANAGLYPHIC or the value set in the OSG_STEREO_MODE "<< std::endl;
    out <<"                          environmental variable. See doc/stereo.html for "<< std::endl;
    out <<"                          further details on setting up accurate stereo "<< std::endl;
    out <<"                          for your system. "<< std::endl;
    out <<"    -stereo ANAGLYPHIC  - switch on anaglyphic(red/cyan) stereo rendering."<< std::endl;
    out <<"    -stereo QUAD_BUFFER - switch on quad buffered stereo rendering."<< std::endl;
    out <<std::endl;
    out <<"    -stencil            - use a visual with stencil buffer enabled, this "<< std::endl;
    out <<"                          also allows the depth complexity statistics mode"<< std::endl;
    out <<"                          to be used (press 'p' three times to cycle to it)."<< std::endl;
    out << std::endl;
}

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
    
        virtual Referenced*     operator() (const Referenced* ref) const
        {
            writeIndent(); std::cout << "copying Referenced "<<ref<<std::endl;
            moveIn();
            osg::Referenced* ret_ref = CopyOp::operator()(ref);
            moveOut();
            return ret_ref;
        }
        
        virtual Object*         operator() (const Object* obj) const
        {
            writeIndent(); std::cout << "copying Object "<<obj;
            if (obj) std::cout<<" "<<obj->className();
            std::cout<<std::endl;
            moveIn();
            osg::Object* ret_obj = CopyOp::operator()(obj);
            moveOut();
            return ret_obj;
        }
        
        virtual Node*           operator() (const Node* node) const
        {
            writeIndent(); std::cout << "copying Node "<<node;
            if (node) std::cout<<" "<<node->className()<<" '"<<node->getName()<<"'";
            std::cout<<std::endl;
            moveIn();
            osg::Node* ret_node = CopyOp::operator()(node);
            moveOut();
            return ret_node;
        }

        virtual Drawable*       operator() (const Drawable* drawable) const
        {
            writeIndent(); std::cout << "copying Drawable "<<drawable;
            if (drawable) std::cout<<" "<<drawable->className();
            std::cout<<std::endl;
            moveIn();
            osg::Drawable* ret_drawable = CopyOp::operator()(drawable);
            moveOut();
            return ret_drawable;
        }

        virtual StateSet*       operator() (const StateSet* stateset) const
        {
            writeIndent(); std::cout << "copying StateSet "<<stateset;
            if (stateset) std::cout<<" "<<stateset->className();
            std::cout<<std::endl;
            moveIn();
            osg::StateSet* ret_stateset = CopyOp::operator()(stateset);
            moveOut();
            return ret_stateset;
        }

        virtual StateAttribute* operator() (const StateAttribute* attr) const
        {
            writeIndent(); std::cout << "copying StateAttribute "<<attr;
            if (attr) std::cout<<" "<<attr->className();
            std::cout<<std::endl;
            moveIn();
            osg::StateAttribute* ret_attr = CopyOp::operator()(attr);
            moveOut();
            return ret_attr;
        }

        virtual Texture*        operator() (const Texture* text) const
        {
            writeIndent(); std::cout << "copying Texture "<<text;
            if (text) std::cout<<" "<<text->className();
            std::cout<<std::endl;
            moveIn();
            osg::Texture* ret_text = CopyOp::operator()(text);
            moveOut();
            return ret_text;
        }

        virtual Image*          operator() (const Image* image) const
        {
            writeIndent(); std::cout << "copying Image "<<image;
            if (image) std::cout<<" "<<image->className();
            std::cout<<std::endl;
            moveIn();
            Image* ret_image = CopyOp::operator()(image);
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

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(commandLine);
    if (!rootnode)
    {
        write_usage(osg::notify(osg::NOTICE),argv[0]);
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
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    viewer.registerCameraManipulator(new osgUtil::FlightManipulator);
    viewer.registerCameraManipulator(new osgUtil::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
