#include <osg/ArgumentParser>

#include <osg/LightModel>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osg/Camera>
#include <osg/Stencil>
#include <osg/StencilTwoSided>
#include <osg/CullFace>
#include <osg/Geometry>
#include <osg/Switch>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/StateSetManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/StatsHandler>

#include <osgShadow/OccluderGeometry>
#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

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
        
    void getBase(osg::Polytope& polytope, float margin=0.1) const
    {
        float delta = _bb.radius()*margin;
        polytope.add( osg::Plane(0.0, 0.0, 1.0, -(_bb.zMin()-delta)) );
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

enum Faces
{
    FRONT_FACE = 1,
    BACK_FACE = 2,
    LEFT_FACE = 4,
    RIGHT_FACE = 8,
    TOP_FACE = 16,
    BOTTOM_FACE = 32        
};

osg::Node* createCube(unsigned int mask)
{
    osg::Geode* geode = new osg::Geode;
    
    osg::Geometry* geometry = new osg::Geometry;
    geode->addDrawable(geometry);
    
    osg::Vec3Array* vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices);
    
    osg::Vec3Array* normals = new osg::Vec3Array;
    geometry->setNormalArray(normals);
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    
    osg::Vec4Array* colours = new osg::Vec4Array;
    geometry->setColorArray(colours);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    
    
    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 dx(2.0f,0.0f,0.0f);
    osg::Vec3 dy(0.0f,1.0f,0.0f);
    osg::Vec3 dz(0.0f,0.0f,1.0f);
    
    osg::Vec3 px(1.0f,0.0,0.0f);
    osg::Vec3 nx(-1.0f,0.0,0.0f);
    osg::Vec3 py(0.0f,1.0f,0.0f);
    osg::Vec3 ny(0.0f,-1.0f,0.0f);
    osg::Vec3 pz(0.0f,0.0f,1.0f);
    osg::Vec3 nz(0.0f,0.0f,-1.0f);

    if (mask & FRONT_FACE)
    {
        // front face    
        vertices->push_back(origin);
        vertices->push_back(origin+dx);
        vertices->push_back(origin+dx+dz);
        vertices->push_back(origin+dz);
        normals->push_back(ny);
        normals->push_back(ny);
        normals->push_back(ny);
        normals->push_back(ny);
    }

    if (mask & BACK_FACE)
    {
        // back face    
        vertices->push_back(origin+dy);
        vertices->push_back(origin+dy+dz);
        vertices->push_back(origin+dy+dx+dz);
        vertices->push_back(origin+dy+dx);
        normals->push_back(py);
        normals->push_back(py);
        normals->push_back(py);
        normals->push_back(py);
    }

    if (mask & LEFT_FACE)
    {
        // left face    
        vertices->push_back(origin+dy);
        vertices->push_back(origin);
        vertices->push_back(origin+dz);
        vertices->push_back(origin+dy+dz);
        normals->push_back(nx);
        normals->push_back(nx);
        normals->push_back(nx);
        normals->push_back(nx);
    }

    if (mask & RIGHT_FACE)
    {
        // right face    
        vertices->push_back(origin+dx+dy);
        vertices->push_back(origin+dx+dy+dz);
        vertices->push_back(origin+dx+dz);
        vertices->push_back(origin+dx);
        normals->push_back(px);
        normals->push_back(px);
        normals->push_back(px);
        normals->push_back(px);
    }

    if (mask & TOP_FACE)
    {
        // top face    
        vertices->push_back(origin+dz);
        vertices->push_back(origin+dz+dx);
        vertices->push_back(origin+dz+dx+dy);
        vertices->push_back(origin+dz+dy);
        normals->push_back(pz);
        normals->push_back(pz);
        normals->push_back(pz);
        normals->push_back(pz);
    }

    if (mask & BOTTOM_FACE)
    {
        // bottom face    
        vertices->push_back(origin);
        vertices->push_back(origin+dy);
        vertices->push_back(origin+dx+dy);
        vertices->push_back(origin+dx);
        normals->push_back(nz);
        normals->push_back(nz);
        normals->push_back(nz);
        normals->push_back(nz);
    }
    
    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, vertices->size()));

    return geode;
}

class SwitchHandler : public osgGA::GUIEventHandler
{
public:

    SwitchHandler():
        _childNum(0) {}
    
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* /*nv*/)
    {
        osg::Switch* sw = dynamic_cast<osg::Switch*>(object);
        if (!sw) return false;

        if (ea.getHandled()) return false;
        
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYDOWN):
            {
                if (ea.getKey()=='n')
                {
                    ++_childNum;
                    if (_childNum >= sw->getNumChildren()) _childNum = 0;

                    sw->setSingleChildOn(_childNum);
                    return true;
                }                
                break;
            }
            default:
                break;
        }
        return false;
    }

protected:

    virtual ~SwitchHandler() {}
    unsigned int    _childNum;

};

osg::Node* createTestModel()
{
    osg::Switch* sw = new osg::Switch;
    sw->setEventCallback(new SwitchHandler);
    
    sw->addChild(createCube(FRONT_FACE), true);
    sw->addChild(createCube(FRONT_FACE | BACK_FACE), false);
    sw->addChild(createCube(FRONT_FACE | BACK_FACE | LEFT_FACE), false);
    sw->addChild(createCube(FRONT_FACE | BACK_FACE | LEFT_FACE | RIGHT_FACE), false);
    sw->addChild(createCube(FRONT_FACE | BACK_FACE | LEFT_FACE | RIGHT_FACE | TOP_FACE), false);
    sw->addChild(createCube(FRONT_FACE | BACK_FACE | LEFT_FACE | RIGHT_FACE | TOP_FACE | BOTTOM_FACE), false);
    
    return sw;
}



class ShadowCallback : public osg::NodeCallback
{
public:

    osg::ref_ptr<osg::Light>    _ambientLight;
    osg::ref_ptr<osg::Light>    _diffuseLight;
    
    osg::ref_ptr<osg::StateSet> _ss1;
    osg::ref_ptr<osg::StateSet> _mainShadowStateSet;
    osg::ref_ptr<osg::StateSet> _shadowVolumeStateSet;
    osg::ref_ptr<osg::StateSet> _shadowedSceneStateSet;

    ShadowCallback(osgShadow::ShadowVolumeGeometry::DrawMode drawMode)
    {
        // first group, render the depth buffer + ambient light contribution
        {
            _ss1 = new osg::StateSet;

#if 0
            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(ambient);
            _ss1->setAttribute(lm1);

            osg::Light* light1 = new osg::Light;
            light1->setAmbient(ambient);
            light1->setDiffuse(zero_colour);
            light1->setPosition(_lightpos);
            _ss1->setAttributeAndModes(light1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
#endif            
            
            _ss1->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        {
            _mainShadowStateSet = new osg::StateSet;

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _mainShadowStateSet->setAttribute(depth);
            _mainShadowStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        {
            _shadowVolumeStateSet = new osg::StateSet;
            
            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _shadowVolumeStateSet->setAttribute(depth);
            _shadowVolumeStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE);
            _shadowVolumeStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);


            if (drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
            {
                osg::StencilTwoSided* stencil = new osg::StencilTwoSided;
                stencil->setFunction(osg::StencilTwoSided::BACK, osg::StencilTwoSided::ALWAYS,0,~0u);
                stencil->setOperation(osg::StencilTwoSided::BACK, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::DECR_WRAP);
                stencil->setFunction(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::ALWAYS,0,~0u);
                stencil->setOperation(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::INCR_WRAP);

                osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                _shadowVolumeStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
                _shadowVolumeStateSet->setAttribute(colourMask, osg::StateAttribute::OVERRIDE);
                _shadowVolumeStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);


            }
            else
            {
                osg::Stencil* stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

                osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                _shadowVolumeStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                _shadowVolumeStateSet->setAttribute(colourMask);
                _shadowVolumeStateSet->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
            }
        }


        {
            _shadowedSceneStateSet = new osg::StateSet;

            osg::Depth* depth = new osg::Depth;
            depth->setWriteMask(false);
            depth->setFunction(osg::Depth::LEQUAL);
            _shadowedSceneStateSet->setAttribute(depth);
            _shadowedSceneStateSet->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
            // _shadowedSceneStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

#if 0
            osg::LightModel* lm1 = new osg::LightModel;
            lm1->setAmbientIntensity(zero_colour);
            _shadowedSceneStateSet->setAttribute(lm1);

            osg::Light* light = new osg::Light;
            light->setAmbient(zero_colour);
            light->setDiffuse(diffuse);
            light->setPosition(_lightpos);

            _shadowedSceneStateSet->setMode(GL_LIGHT0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            _shadowedSceneStateSet->setAttribute(light);
#endif

            // set up the stencil ops so that only operator on this mirrors stencil value.
            osg::Stencil* stencil = new osg::Stencil;
            stencil->setFunction(osg::Stencil::EQUAL,0,~0u);
            stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
            _shadowedSceneStateSet->setAttributeAndModes(stencil,osg::StateAttribute::ON);

            osg::BlendFunc* blend = new osg::BlendFunc;
            blend->setFunction(osg::BlendFunc::ONE, osg::BlendFunc::ONE);
            _shadowedSceneStateSet->setAttributeAndModes(blend, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
            // _shadowedSceneStateSet->setMode(GL_LIGHTING, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        }

        _ss1->setThreadSafeRefUnref(true);
        _mainShadowStateSet->setThreadSafeRefUnref(true);
        _shadowVolumeStateSet->setThreadSafeRefUnref(true);
        _shadowedSceneStateSet->setThreadSafeRefUnref(true);

    }
    
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { 
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (!cv) 
        {
            traverse(node,nv);
            return;
        }

        // osg::notify(osg::NOTICE)<<std::endl<<std::endl<<"Cull callback"<<std::endl;

        
        osg::ref_ptr<osgUtil::RenderBin> original_bin = cv->getCurrentRenderBin();
        
        osg::ref_ptr<osgUtil::RenderBin> new_bin = original_bin->find_or_insert(0,"RenderBin");

        cv->setCurrentRenderBin(new_bin.get());

        traverse(node,nv);
        
        cv->setCurrentRenderBin(original_bin.get());

        osgUtil::RenderBin::RenderBinList::iterator itr =  new_bin->getRenderBinList().find(1000);
        osg::ref_ptr<osgUtil::RenderBin> shadowVolumeBin;
        if (itr != new_bin->getRenderBinList().end())
        {
            shadowVolumeBin = itr->second;
            
            if (shadowVolumeBin.valid())
            {
                // osg::notify(osg::NOTICE)<<"Found shadow volume bin, now removing it"<<std::endl;
                new_bin->getRenderBinList().erase(itr);
            }
        }
        
        if (shadowVolumeBin.valid())
        {        
            original_bin->setStateSet(_ss1.get());

            osgUtil::RenderStage* orig_rs = cv->getRenderStage();
            osgUtil::RenderStage* new_rs = new osgUtil::RenderStage;
            orig_rs->addPostRenderStage(new_rs);

            new_rs->setViewport(orig_rs->getViewport());
            new_rs->setClearColor(orig_rs->getClearColor());
            new_rs->setClearMask(GL_STENCIL_BUFFER_BIT);
            new_rs->setDrawBuffer(orig_rs->getDrawBuffer());
            new_rs->setReadBuffer(orig_rs->getReadBuffer());
            new_rs->setColorMask(orig_rs->getColorMask());
            
            new_rs->setPositionalStateContainer(orig_rs->getPositionalStateContainer());

            if (shadowVolumeBin.valid())
            {
                // new_rs->setStateSet(_mainShadowStateSet.get());
                new_rs->getRenderBinList()[0] = shadowVolumeBin;
                shadowVolumeBin->setStateSet(_shadowVolumeStateSet.get());

                osg::ref_ptr<osgUtil::RenderBin> nested_bin = new_rs->find_or_insert(1,"RenderBin");
                nested_bin->getRenderBinList()[0] = new_bin;            
                nested_bin->setStateSet(_shadowedSceneStateSet.get());
            }
        }
    }
};



int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates using of GL_ARB_shadow extension implemented in osg::Texture class");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName());
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--positionalLight", "Use a positional light.");
    arguments.getApplicationUsage()->addCommandLineOption("--directionalLight", "Use a direction light.");
    arguments.getApplicationUsage()->addCommandLineOption("--addOccluderToScene", "Add the occluders geometry.");
    arguments.getApplicationUsage()->addCommandLineOption("--noUpdate", "Disable the updating the of light source.");
    arguments.getApplicationUsage()->addCommandLineOption("--base", "Add a base geometry to test shadows.");
    arguments.getApplicationUsage()->addCommandLineOption("--noShadow", "Disable the shadows.");
    arguments.getApplicationUsage()->addCommandLineOption("--two-sided", "Use two-sided stencil extension for shadow volumes.");
    arguments.getApplicationUsage()->addCommandLineOption("--two-pass", "Use two-pass stencil for shadow volumes.");

    // hint to tell viewer to request stencil buffer when setting up windows
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

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
    
    bool cullCallback = false;
    while (arguments.read("-c")) cullCallback = true;

    int screenNum = -1;
    while (arguments.read("--screen", screenNum)) viewer.setUpViewOnSingleScreen(screenNum);

    osgShadow::ShadowVolumeGeometry::DrawMode drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED;
    while (arguments.read("--two-sided")) drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED;
    while (arguments.read("--two-pass")) drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_PASS;
    
    bool ShadowVolume = false;
    while (arguments.read("--ShadowVolume")) ShadowVolume = true;


    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid()) 
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }


    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add stats
    viewer.addEventHandler( new osgViewer::StatsHandler() );

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
        model = createTestModel();
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
//    cbbv.getPolytope(occluder->getBoundingPolytope(),0.001);
    cbbv.getBase(occluder->getBoundingPolytope(),0.001);

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
    light->setPosition(lightpos);

    if (ShadowVolume)
    {
        osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
        
        shadowedScene->setShadowTechnique(new osgShadow::ShadowVolume);

        shadowedScene->addChild(model.get());
        
        group->addChild(shadowedScene.get());
    }
    else if (!doShadow)
    {
        group->addChild(model.get());

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
        geode->addDrawable(shadowVolume.get());
        group->addChild(geode.get());

        osg::StateSet* ss = geode->getOrCreateStateSet();
        ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    }
    else if (cullCallback)
    {
    
        int shadowVolumeBin = 1000;
    
        group->setCullCallback(new ShadowCallback(drawMode));

        group->addChild(model.get());

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->setLight(light.get());
        group->addChild(ls.get());

        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            group->addChild(geode.get());

            occluder->computeShadowVolumeGeometry(lightpos, *shadowVolume);
            shadowVolume->setDrawMode(drawMode);

            if (drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
            {
                osg::notify(osg::NOTICE)<<"STENCIL_TWO_SIDED seleteced"<<std::endl;

                osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
                geode->addDrawable(shadowVolume.get());
            }
            else
            {
                osg::notify(osg::NOTICE)<<"STENCIL_TWO_PASSES seleteced"<<std::endl;

                osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                ss_sv1->setRenderBinDetails(shadowVolumeBin, "RenderBin");
                geode->addDrawable(shadowVolume.get());
            }

        }

    }    
    else
    {
        osg::Vec4 ambient(0.2,0.2,0.2,1.0);
        osg::Vec4 diffuse(0.8,0.8,0.8,1.0);
        osg::Vec4 zero_colour(0.0,0.0,0.0,1.0);
    
        // first group, render the depth buffer + ambient light contribution
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
                shadowVolume->setDrawMode(drawMode);
                

                if (drawMode == osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED)
                {
                    osg::notify(osg::NOTICE)<<"STENCIL_TWO_SIDED seleteced"<<std::endl;

                    osg::StencilTwoSided* stencil = new osg::StencilTwoSided;
                    stencil->setFunction(osg::StencilTwoSided::BACK, osg::StencilTwoSided::ALWAYS,0,~0u);
                    stencil->setOperation(osg::StencilTwoSided::BACK, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::DECR_WRAP);
                    stencil->setFunction(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::ALWAYS,0,~0u);
                    stencil->setOperation(osg::StencilTwoSided::FRONT, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::KEEP, osg::StencilTwoSided::INCR_WRAP);


                    osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                    osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                    ss_sv1->setRenderBinDetails(0, "RenderBin");
                    ss_sv1->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                    ss_sv1->setAttribute(colourMask);
                    ss_sv1->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
                    
                    geode->addDrawable(shadowVolume.get());
                    
                    camera->addChild(geode.get());

                }
                else
                {
                    osg::notify(osg::NOTICE)<<"STENCIL_TWO_PASSES seleteced"<<std::endl;

                    osg::Stencil* stencil = new osg::Stencil;
                    stencil->setFunction(osg::Stencil::ALWAYS,0,~0u);
                    stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);

                    osg::ColorMask* colourMask = new osg::ColorMask(false, false, false, false);

                    osg::StateSet* ss_sv1 = geode->getOrCreateStateSet();
                    ss_sv1->setRenderBinDetails(0, "RenderBin");
                    ss_sv1->setAttributeAndModes(stencil,osg::StateAttribute::ON);
                    ss_sv1->setAttribute(colourMask);
                    ss_sv1->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
                    
                    geode->addDrawable(shadowVolume.get());
                    
                    camera->addChild(geode.get());
                }

            }


            // render scene graph adding contribution of light
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


    viewer.setSceneData(group.get());
    
    // create the windows and run the threads.
    viewer.realize();

    // osgDB::writeNodeFile(*group,"test.osg");

    while (!viewer.done())
    {
        if (updateLightPosition)
        {
            float t = viewer.getFrameStamp()->getSimulationTime();
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
