#include <osgGLUT/Viewer>

#include <osg/Transform>
#include <osg/Billboard>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/Optimizer>


class UpdateCallback : public osg::NodeCallback
{
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        { 
            std::cout<<"update callback - pre traverse"<<node<<std::endl;
            traverse(node,nv);
            std::cout<<"update callback - post traverse"<<node<<std::endl;
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

class DrawableDrawCallback : public osg::Drawable::DrawCallback
{
        virtual void drawImplementation(osg::State& state,const osg::Drawable* drawable) const
        {
            std::cout<<"draw call back - pre drawImplementation"<<drawable<<std::endl;
            drawable->drawImplementation(state);
            std::cout<<"draw call back - post drawImplementation"<<drawable<<std::endl;
        }
};

struct TransformCallback : public osg::Transform::ComputeTransformCallback
{
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,const osg::Transform* transform, osg::NodeVisitor* nv) const
    {
        std::cout<<"computeLocalToWorldMatrix - pre transform->computeLocalToWorldMatrix"<<std::endl;
        bool result = transform->computeLocalToWorldMatrix(matrix,nv);
        std::cout<<"computeLocalToWorldMatrix - post transform->computeLocalToWorldMatrix"<<std::endl;
        return result;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,const osg::Transform* transform, osg::NodeVisitor* nv) const 
    {
        std::cout<<"computeWorldToLocalMatrix - pre transform->computeWorldToLocalMatrix"<<std::endl;
        bool result = transform->computeWorldToLocalMatrix(matrix,nv);
        std::cout<<"computeWorldToLocalMatrix - post transform->computeWorldToLocalMatrix"<<std::endl;
        return result;
    }
};

struct DrawableUpdateCallback : public osg::Drawable::UpdateCallback
{
    virtual void update(osg::NodeVisitor*, osg::Drawable* drawable)
    {
        std::cout<<"Drawable update callback "<<drawable<<std::endl;
    }
};

struct DrawableCullCallback : public osg::Drawable::CullCallback
{
    /** do customized cull code.*/
    virtual bool cull(osg::NodeVisitor*, osg::Drawable* drawable, osg::State* /*state*/) const
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
             node.setUpdateCallback(new UpdateCallback());
             node.setCullCallback(new CullCallback());
             traverse(node);
        }

        virtual void apply(osg::Geode& geode)
        {
            geode.setUpdateCallback(new UpdateCallback());
            
            //note, it makes no sense to attach a cull callback to the node
            //at there are no nodes to traverse below the geode, only
            //drawables, and as such the Cull node callbacks is ignored.
            //If you wish to control the culling of drawables
            //then use a drawable cullback...

            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                geode.getDrawable(i)->setUpdateCallback(new DrawableUpdateCallback());
                geode.getDrawable(i)->setCullCallback(new DrawableCullCallback());
                geode.getDrawable(i)->setDrawCallback(new DrawableDrawCallback());
            }
        }
        
        virtual void apply(osg::Transform& node)
        {
            node.setComputeTransformCallback(new TransformCallback());
            apply((osg::Node&)node);
        }
};

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
   
    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

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

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
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
