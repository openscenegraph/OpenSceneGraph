#include <osgViewer/Viewer>

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

#include <iostream>

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
        virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* drawable) const
        {
            std::cout<<"draw call back - pre drawImplementation"<<drawable<<std::endl;
            drawable->drawImplementation(renderInfo);
            std::cout<<"draw call back - post drawImplementation"<<drawable<<std::endl;
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
            apply((osg::Node&)node);
        }
};

class MyReadFileCallback : public osgDB::Registry::ReadFileCallback
{
public:
    virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
    {
        std::cout<<"before readNode"<<std::endl;
        // note when calling the Registry to do the read you have to call readNodeImplementation NOT readNode, as this will
        // cause on infinite recusive loop.
        osgDB::ReaderWriter::ReadResult result = osgDB::Registry::instance()->readNodeImplementation(fileName,options);
        std::cout<<"after readNode"<<std::endl;
        return result;
    }
};

class CameraUpdateCallback : public osg::NodeCallback
{
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { 
        std::cout<<"Camera update callback - pre traverse"<<node<<std::endl;
        traverse(node,nv);
        std::cout<<"Camera update callback - post traverse"<<node<<std::endl;
    }
};

class CameraEventCallback : public osg::NodeCallback
{
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { 
        std::cout<<"Camera event callback - pre traverse"<<node<<std::endl;
        traverse(node,nv);
        std::cout<<"Camera event callback - post traverse"<<node<<std::endl;
    }
};

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
   
    // set the osgDB::Registy the read file callback to catch all requests for reading files.
    osgDB::Registry::instance()->setReadFileCallback(new MyReadFileCallback());
   
    // initialize the viewer.
    osgViewer::Viewer viewer;

    // load the nodes from the commandline arguments.
    osg::Node* rootnode = osgDB::readNodeFiles(arguments);
    if (!rootnode)
    {
        osg::notify(osg::NOTICE)<<"Please specify a file on the command line"<<std::endl;

        return 1;
    }
    
    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // insert all the callbacks
    InsertCallbacksVisitor icv;
    rootnode->accept(icv);

    viewer.getCamera()->setUpdateCallback(new CameraUpdateCallback());
    viewer.getCamera()->setEventCallback(new CameraEventCallback());

    // set the scene to render
    viewer.setSceneData(rootnode);

    return viewer.run();
}
