#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>


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

class AppCallback : public osg::NodeCallback
{
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 
            std::cout<<"app callback - pre traverse"<<node<<std::endl;
            traverse(node,nv);
            std::cout<<"app callback - post traverse"<<node<<std::endl;
        }
};

class CullCallback : public osg::NodeCallback
{
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 
            std::cout<<"cull callback - pre traverse"<<node<<std::endl;
            traverse(node,nv);
            std::cout<<"cull callback - post traverse"<<node<<std::endl;
        }
};

class DrawableCallback : public osg::Drawable::DrawCallback
{
        virtual void drawImmediateMode(osg::State& state,osg::Drawable* drawable) const
        {
            std::cout<<"draw call back - pre drawImmediateMode"<<drawable<<std::endl;
            drawable->drawImmediateMode(state);
            std::cout<<"draw call back - post drawImmediateMode"<<drawable<<std::endl;
        }
};

struct LODCallback : public osg::LOD::EvaluateLODCallback
{
    /** Compute the child to select.*/
    virtual const int evaluateLODChild(const osg::LOD* lod, const osg::Vec3& eye_local, const float bias) const
    {
        std::cout<<"evaluateLODChild callback - pre lod->evaluateLODChild"<<std::endl;
        int result = lod->evaluateLODChild(eye_local,bias);
        std::cout<<"evaluateLODChild callback - post lod->evaluateLODChild"<<std::endl;
        return result;
    }
};

struct TransformCallback : public osg::Transform::ComputeTransformCallback
{
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual const bool computeLocalToWorldMatrix(osg::Matrix& matrix,const osg::Transform* transform, osg::NodeVisitor* nv) const
    {
        std::cout<<"computeLocalToWorldMatrix - pre transform->computeLocalToWorldMatrix"<<std::endl;
        bool result = transform->computeLocalToWorldMatrix(matrix,nv);
        std::cout<<"computeLocalToWorldMatrix - post transform->computeLocalToWorldMatrix"<<std::endl;
        return result;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual const bool computeWorldToLocalMatrix(osg::Matrix& matrix,const osg::Transform* transform, osg::NodeVisitor* nv) const 
    {
        std::cout<<"computeWorldToLocalMatrix - pre transform->computeWorldToLocalMatrix"<<std::endl;
        bool result = transform->computeWorldToLocalMatrix(matrix,nv);
        std::cout<<"computeWorldToLocalMatrix - post transform->computeWorldToLocalMatrix"<<std::endl;
        return result;
    }
};


struct BillboardCallback : public osg::Billboard::ComputeBillboardCallback
{
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual const bool computeMatrix(osg::Matrix& modelview, const osg::Billboard* billboard, const osg::Vec3& eye_local, const osg::Vec3& pos_local) const
    {
        std::cout<<"ComputeBillboardCallback - pre billboard->computeMatrix"<<std::endl;
        bool result = billboard->computeMatrix(modelview,eye_local,pos_local);
        std::cout<<"ComputeBillboardCallback - post billboard->computeMatrix"<<std::endl;
        return result;
    }
};

struct DrawableCullCallback : public osg::Drawable::CullCallback
{
    /** do customized cull code.*/
    virtual bool cull(osg::NodeVisitor*, osg::Drawable* drawable, osg::State *state) const
    {
        std::cout<<"Drawable cull callback "<<drawable<<std::endl;
        return false;
    }
};

class InsertCallbacksVisitor : public osg::NodeVisitor
{

   public:
   
        InsertCallbacksVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
        }
        
        virtual void apply(osg::Node& node)
        {
             node.setAppCallback(new AppCallback());
             node.setCullCallback(new CullCallback());
             traverse(node);
        }

        virtual void apply(osg::Geode& geode)
        {
            geode.setAppCallback(new AppCallback());
            
            //note, it makes no sense to attach a cull callback to the node
            //at there are no nodes to traverse below the geode, only
            //drawables, and as such the Cull node callbacks is ignored.
            //If you wish to control the culling of drawables
            //then use a drawable cullback...

            for(int i=0;i<geode.getNumDrawables();++i)
            {
                geode.getDrawable(i)->setCullCallback(new DrawableCullCallback());
                geode.getDrawable(i)->setDrawCallback(new DrawableCallback());
            }
        }
        
        virtual void apply(osg::Billboard& node)
        {
            node.setComputeBillboardCallback(new BillboardCallback());
            apply((osg::Geode&)node);
        }

        virtual void apply(osg::Transform& node)
        {
            node.setComputeTransformCallback(new TransformCallback());
            apply((osg::Node&)node);
        }

        virtual void apply(osg::LOD& node)
        {
            node.setEvaluateLODCallback(new LODCallback());
            apply((osg::Node&)node);
        }
};

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );

    if (argc<2)
    {
        write_usage(std::cout,argv[0]);
        return 0;
    }
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

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
//        write_usage(osg::notify(osg::NOTICE),argv[0]);
        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // insert all the callbacks
    InsertCallbacksVisitor icv;
    rootnode->accept(icv);

    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
