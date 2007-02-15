#include <osg/ArgumentParser>

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

    // default to single threaded during dev work.
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    
    while (arguments.read("--SingleThreaded")) viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    while (arguments.read("--CullDrawThreadPerContext")) viewer.setThreadingModel(osgViewer::Viewer::CullDrawThreadPerContext);
    while (arguments.read("--DrawThreadPerContext")) viewer.setThreadingModel(osgViewer::Viewer::DrawThreadPerContext);
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) viewer.setThreadingModel(osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext);


    bool postionalLight = false;
    while (arguments.read("--positionalLight")) postionalLight = true;
    while (arguments.read("--directionalLight")) postionalLight = false;

    bool updateLightPosition = true;
    while (arguments.read("--noUpdate")) updateLightPosition = false;

    bool createBase = false;
    while (arguments.read("--base")) createBase = true;


    int screenNum = -1;
    while (arguments.read("--screen", screenNum)) viewer.setUpViewOnSingleScreen(screenNum);

    osgShadow::ShadowVolumeGeometry::DrawMode drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED;
    while (arguments.read("--two-sided")) drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_SIDED;
    while (arguments.read("--two-pass")) drawMode = osgShadow::ShadowVolumeGeometry::STENCIL_TWO_PASS;


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

    osg::Vec4 lightpos;
    
    if (postionalLight)
    {
        lightpos.set(bb.center().x(), bb.center().y(), bb.zMax() + bb.radius()  ,1.0f);
    }
    else
    {
        lightpos.set(0.5f,0.25f,0.8f,0.0f);
    }


    osg::ref_ptr<osgShadow::ShadowedScene> shadowedScene = new osgShadow::ShadowedScene;
    
    osg::ref_ptr<osgShadow::ShadowVolume> shadowVolume = new osgShadow::ShadowVolume;
    shadowedScene->setShadowTechnique(shadowVolume.get());
    shadowVolume->setDynamicShadowVolumes(updateLightPosition);

    osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
    ls->getLight()->setPosition(lightpos);
    ls->getLight()->setAmbient(osg::Vec4(1.0,0.0,0.0,1.0));
    ls->getLight()->setDiffuse(osg::Vec4(0.0,1.0,0.0,1.0));

    shadowedScene->addChild(model.get());
    shadowedScene->addChild(ls.get());
    
    osgDB::writeNodeFile(*shadowedScene, "shadow.osg");
    

    viewer.setSceneData(shadowedScene.get());
    
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
                lightpos.set(sinf(t),cosf(t),1.0f,0.0f);
            }
            ls->getLight()->setPosition(lightpos);
        }

        viewer.frame();
    }
    
    return 0;
}
