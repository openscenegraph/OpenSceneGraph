#include <osg/ArgumentParser>

#include <osg/LightModel>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Stencil>
#include <osg/CullFace>
#include <osg/Geometry>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>

#include <osgShadow/OccluderGeometry>

#include <osgDB/ReadFile>

#include <iostream>

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
    osgViewer::Viewer viewer;

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
    while (arguments.read("--addOccluderToScene")) addOccluderToScene = true;

    bool updateLightPosition = true;
    while (arguments.read("--noUpdate")) updateLightPosition = false;

    bool createBase = false;
    while (arguments.read("--base")) createBase = true;

    bool doShadow = true;
    while (arguments.read("--noShadow")) doShadow = false;

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

    if (createBase)
    {
        osg::ref_ptr<osg::Group> newGroup = new osg::Group;
        newGroup->addChild(model.get());
        
        osg::Geode* geode = new osg::Geode;
        
        osg::Vec3 widthVec(bb.radius(), 0.0f, 0.0f);
        osg::Vec3 depthVec(0.0f, bb.radius(), 0.0f);
        osg::Vec3 centerBase( (bb.xMin()+bb.xMax())*0.5f, (bb.yMin()+bb.yMax())*0.5f, bb.zMin()-bb.radius()*0.1f );
        
        geode->addDrawable( osg::createTexturedQuadGeometry( centerBase-widthVec*1.5f-depthVec*1.5f, 
                                                             widthVec*3.0f, depthVec*3.0f) );
        newGroup->addChild(geode);
        
        model = newGroup.get();
    }

    // get the bounds of the model.
    cbbv.reset();
    model->accept(cbbv);
    bb = cbbv.getBoundingBox();
    
    osg::ref_ptr<osg::Group> group = new osg::Group;

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
        lightpos.set(0.5f,0.25f,0.8f,0.0f);
    }



    osg::ref_ptr<osg::Light> light = new osg::Light;

    if (!doShadow)
    {
        group->addChild(model.get());

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
        geode->addDrawable(shadowVolume.get());
        group->addChild(geode.get());

        osg::StateSet* ss = geode->getOrCreateStateSet();
        ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    }
    else
    {
        osg::Vec4 ambient(0.2,0.2,0.2,1.0);
        osg::Vec4 diffuse(0.8,0.8,0.8,1.0);
        osg::Vec4 zero_colour(0.0,0.0,0.0,1.0);
    
        // first group
        {

            osg::Group* first_model_group = new osg::Group;
            first_model_group->addChild(model.get());
            group->addChild(first_model_group);

            osg::StateSet* ss1 = first_model_group->getOrCreateStateSet();

            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(ambient);
            ss1->setAttribute(lm1);

            osg::Light* light1 = new osg::Light;
            light1->setAmbient(ambient);
            light1->setDiffuse(zero_colour);
            light1->setPosition(lightpos);
            ss1->setAttributeAndModes(light1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            ss1->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        
        }    
    
        // second group
        {
            // use a camera here just to implement a seperate post rendering stage.
            osg::Camera* camera = new osg::Camera;
            camera->setRenderOrder(osg::Camera::POST_RENDER);
            camera->setClearMask(GL_STENCIL_BUFFER_BIT);
            group->addChild(camera);

            osg::StateSet* ss_camera = camera->getOrCreateStateSet();

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            ss_camera->setAttribute(depth);

            {
                osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
                geode->addDrawable(shadowVolume.get());

                // switch off the writing to the color bit planes.
                osg::ColorMask* colourMask = new osg::ColorMask;
                colourMask->setMask(false,false,false,false);

                osg::Stencil* stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::INCR);

                osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                ss_sv1->setRenderBinDetails(0, "RenderBin");
                ss_sv1->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                ss_sv1->setAttribute(colourMask);
                
                ss_sv1->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                camera->addChild(geode.get());
            }
            
            if (true)
            {
                osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
                geode->addDrawable(shadowVolume.get());

                // switch off the writing to the color bit planes.
                osg::ColorMask* colourMask = new osg::ColorMask;
                colourMask->setMask(false,false,false,false);

                osg::Stencil* stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::DECR);

                osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                ss_sv1->setRenderBinDetails(1, "RenderBin");
                ss_sv1->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                ss_sv1->setAttribute(colourMask);
                
                ss_sv1->setAttributeAndModes(new osg::CullFace(osg::CullFace::FRONT), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                camera->addChild(geode.get());
            }

            {
                osg::Group* second_model_group = new osg::Group;
                second_model_group->addChild(model.get());


                osg::StateSet* ss1 = second_model_group->getOrCreateStateSet();
                ss1->setRenderBinDetails(5, "RenderBin");

                osg::LightModel* lm1 = new osg::LightModel;
                lm1->setAmbientIntensity(zero_colour);
                ss1->setAttribute(lm1);


                osg::LightSource* lightsource = new osg::LightSource;
                lightsource->setLight(light.get());
                light->setAmbient(zero_colour);
                light->setDiffuse(diffuse);
                light->setPosition(lightpos);
                second_model_group->addChild(lightsource);

                ss1->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                // set up the stencil ops so that only operator on this mirrors stencil value.
                osg::Stencil* stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::EQUAL,0,~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                ss1->setAttributeAndModes(stencil,osg::StateAttribute::ON);


                osg::BlendFunc* blend = new osg::BlendFunc;
                blend->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::ONE);
                ss1->setAttributeAndModes(blend, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                ss1->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

                camera->addChild(second_model_group);
            }
                    
        }    

    }

    osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

    viewer.setSceneData(group.get());


    viewer.setCameraManipulator(new osgGA::TrackballManipulator());


    osg::notify(osg::NOTICE)<<"Warning: Stencil buffer required, but not yet switched on."<<std::endl;


    // create the windows and run the threads.
    viewer.realize();

    while (!viewer.done())
    {
        if (updateLightPosition)
        {
            float t = viewer.getFrameStamp()->getReferenceTime();
            if (postionalLight)
            {
                lightpos.set(bb.center().x()+sinf(t)*bb.radius(), bb.center().y() + cosf(t)*bb.radius(), bb.zMax() + bb.radius()  ,1.0f);
            }
            else
            {
                lightpos.set(sinf(t),cosf(t),0.8f,0.0f);
            }
            light->setPosition(lightpos);
            occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
        }

        viewer.frame();
    }
    
    return 0;
}
