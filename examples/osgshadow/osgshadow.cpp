#include <osg/ArgumentParser>

#include <osgProducer/Viewer>

#include <osgShadow/OccluderGeometry>

#include <osgDB/ReadFile>


class ComputeBoundingBoxVisitor : public osg::NodeVisitor
{
public:
    ComputeBoundingBoxVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
    }
    
    virtual void reset()
    {
        _matrixStack.clear();
        _bb.init();
    }
    
    osg::BoundingBox& getBoundingBox() { return _bb; }

    void getPolytope(osg::Polytope& polytope, float margin=0.1) const
    {
        float delta = _bb.radius()*margin;
        polytope.add( osg::Plane(0.0, 0.0, 1.0, -(_bb.zMin()-delta)) );
        polytope.add( osg::Plane(0.0, 0.0, -1.0, (_bb.zMax()+delta)) );

        polytope.add( osg::Plane(1.0, 0.0, 0.0, -(_bb.xMin()-delta)) );
        polytope.add( osg::Plane(-1.0, 0.0, 0.0, (_bb.xMax()+delta)) );

        polytope.add( osg::Plane(0.0, 1.0, 0.0, -(_bb.yMin()-delta)) );
        polytope.add( osg::Plane(0.0, -1.0, 0.0, (_bb.yMax()+delta)) );
    }
        
    void apply(osg::Node& node)
    {
        traverse(node);
    }
    
    void apply(osg::Transform& transform)
    {
        osg::Matrix matrix;
        if (!_matrixStack.empty()) matrix = _matrixStack.back();
        
        transform.computeLocalToWorldMatrix(matrix,this);
        
        pushMatrix(matrix);
        
        traverse(transform);
        
        popMatrix();
    }
    
    void apply(osg::Geode& geode)
    {
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(geode.getDrawable(i));
        }
    }
    
    void pushMatrix(osg::Matrix& matrix)
    {
        _matrixStack.push_back(matrix);
    }
    
    void popMatrix()
    {
        _matrixStack.pop_back();
    }

    void apply(osg::Drawable* drawable)
    {
        if (_matrixStack.empty()) _bb.expandBy(drawable->getBound());
        else
        {
            osg::Matrix& matrix = _matrixStack.back();
            const osg::BoundingBox& dbb = drawable->getBound();
            if (dbb.valid())
            {
                _bb.expandBy(dbb.corner(0) * matrix);
                _bb.expandBy(dbb.corner(1) * matrix);
                _bb.expandBy(dbb.corner(2) * matrix);
                _bb.expandBy(dbb.corner(3) * matrix);
                _bb.expandBy(dbb.corner(4) * matrix);
                _bb.expandBy(dbb.corner(5) * matrix);
                _bb.expandBy(dbb.corner(6) * matrix);
                _bb.expandBy(dbb.corner(7) * matrix);
            }
        }
    }
    
protected:
    
    typedef std::vector<osg::Matrix> MatrixStack;

    MatrixStack         _matrixStack;
    osg::BoundingBox    _bb;
};



int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates using of GL_ARB_shadow extension implemented in osg::Texture class");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--with-base-texture", "Adde base texture to shadowed model.");
    arguments.getApplicationUsage()->addCommandLineOption("--no-base-texture", "Adde base texture to shadowed model.");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments. getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    bool postionalLight = false;
    while (arguments.read("--positionalLight")) postionalLight = true;
    while (arguments.read("--directionalLight")) postionalLight = false;

    bool addOccluderToScene = false;
    while (arguments.read("addOccluderToScene")) addOccluderToScene = true;

    bool updateLightPosition = true;

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
      arguments.writeErrorMessages(std::cout);
      return 1;
    }


    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);
    if (!model)
    {
        osg::notify(osg::NOTICE)<<"No model loaded, please specify a model to load."<<std::endl;
        return 1;
    }

    // get the bounds of the model.    
    ComputeBoundingBoxVisitor cbbv;
    model->accept(cbbv);
    osg::BoundingBox bb = cbbv.getBoundingBox();
    
    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(model.get());

    // set up the occluder
    osg::ref_ptr<osgShadow::OccluderGeometry> occluder = new osgShadow::OccluderGeometry;
    occluder->computeOccluderGeometry(model.get());
    cbbv.getPolytope(occluder->getBoundingPolytope(),0.001);

    if (addOccluderToScene)
    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        geode->addDrawable(occluder.get());
        group->addChild(geode.get());
    }
    
    osg::ref_ptr<osgShadow::ShadowVolumeGeometry> shadowVolume = new osgShadow::ShadowVolumeGeometry;

    shadowVolume->setUseDisplayList(!updateLightPosition);

    osg::Vec4 lightpos;
    
    if (postionalLight)
    {
        lightpos.set(bb.center().x(), bb.center().y(), bb.zMax() + bb.radius()  ,1.0f);
    }
    else
    {
        lightpos.set(0.5f,0.25f,-0.8f,0.0f);
    }

    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        occluder->comptueShadowVolumeGeometry(lightpos, *shadowVolume);
        geode->addDrawable(shadowVolume.get());
        group->addChild(geode.get());
    }



    viewer.setSceneData(group.get());

    // create the windows and run the threads.
    viewer.realize();

    while (!viewer.done())
    {
      // wait for all cull and draw threads to complete.
      viewer.sync();

        if (updateLightPosition)
        {
            float t = viewer.getFrameStamp()->getReferenceTime();
            if (postionalLight)
            {
                lightpos.set(bb.center().x()+sinf(t)*bb.radius(), bb.center().y() + cosf(t)*bb.radius(), bb.zMax() + bb.radius()  ,1.0f);
            }
            else
            {
                lightpos.set(sinf(t),cosf(t),-0.8f,0.0f);
            }
            occluder->comptueShadowVolumeGeometry(lightpos, *shadowVolume);
       }

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
