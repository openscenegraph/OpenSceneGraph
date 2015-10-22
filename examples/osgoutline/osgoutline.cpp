// -*-c++-*-

/*
 * Draw an outline around a model.
 */

#include <osg/Group>
#include <osg/PositionAttitudeTransform>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgFX/Outline>


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc,argv);
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] <file>");
    arguments.getApplicationUsage()->addCommandLineOption("--testOcclusion","Test occlusion by other objects");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    bool testOcclusion = false;
    while (arguments.read("--testOcclusion")) { testOcclusion = true; }

    // load outlined object
    std::string modelFilename = arguments.argc() > 1 ? arguments[1] : "dumptruck.osgt";
    osg::ref_ptr<osg::Node> outlineModel = osgDB::readRefNodeFile(modelFilename);
    if (!outlineModel)
    {
        osg::notify(osg::FATAL) << "Unable to load model '" << modelFilename << "'\n";
        return -1;
    }

    // create scene
    osg::ref_ptr<osg::Group> root = new osg::Group;

    {
        // create outline effect
        osg::ref_ptr<osgFX::Outline> outline = new osgFX::Outline;
        root->addChild(outline);

        outline->setWidth(8);
        outline->setColor(osg::Vec4(1,1,0,1));
        outline->addChild(outlineModel);
    }

    if (testOcclusion)
    {
        // load occluder
        std::string occludedModelFilename = "cow.osgt";
        osg::ref_ptr<osg::Node> occludedModel = osgDB::readRefNodeFile(occludedModelFilename);
        if (!occludedModel)
        {
            osg::notify(osg::FATAL) << "Unable to load model '" << occludedModelFilename << "'\n";
            return -1;
        }

        // occluder offset
        const osg::BoundingSphere& bsphere = outlineModel->getBound();
        const osg::Vec3 occluderOffset = osg::Vec3(0,1,0) * bsphere.radius() * 1.2f;

        // occluder behind outlined model
        osg::ref_ptr<osg::PositionAttitudeTransform> modelTransform0 = new osg::PositionAttitudeTransform;
        modelTransform0->setPosition(bsphere.center() + occluderOffset);
        modelTransform0->addChild(occludedModel);
        root->addChild(modelTransform0);

        // occluder in front of outlined model
        osg::ref_ptr<osg::PositionAttitudeTransform> modelTransform1 = new osg::PositionAttitudeTransform;
        modelTransform1->setPosition(bsphere.center() - occluderOffset);
        modelTransform1->addChild(occludedModel);
        root->addChild(modelTransform1);
    }

    // must have stencil buffer...
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(1);

    // construct the viewer
    osgViewer::Viewer viewer;
    viewer.setSceneData(root);

    // must clear stencil buffer...
    unsigned int clearMask = viewer.getCamera()->getClearMask();
    viewer.getCamera()->setClearMask(clearMask | GL_STENCIL_BUFFER_BIT);
    viewer.getCamera()->setClearStencil(0);

    return viewer.run();
}
