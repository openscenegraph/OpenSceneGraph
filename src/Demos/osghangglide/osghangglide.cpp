#include <osg/Group>
#include <osg/Notify>
#include <osg/Depth>
#include <osg/StateSet>
#include <osg/ClearNode>
#include <osg/Transform>

#include <osgUtil/CullVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/AnimationPathManipulator>

#include <osgGLUT/Viewer>

#include "GliderManipulator.h"

extern osg::Node *makeTerrain( void );
extern osg::Node *makeTrees( void );
extern osg::Node *makeTank( void );
extern osg::Node *makeWindsocks( void );
extern osg::Node *makeGliders( void );
extern osg::Node *makeGlider( void );
extern osg::Node *makeSky( void );
extern osg::Node *makeBase( void );
extern osg::Node *makeClouds( void );

struct MoveEarthySkyWithEyePointCallback : public osg::Transform::ComputeTransformCallback
{
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,const osg::Transform*, osg::NodeVisitor* nv) const 
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.preMult(osg::Matrix::translate(eyePointLocal.x(),eyePointLocal.y(),0.0f));
        }
        return true;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,const osg::Transform*, osg::NodeVisitor* nv) const
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
        if (cv)
        {
            osg::Vec3 eyePointLocal = cv->getEyeLocal();
            matrix.postMult(osg::Matrix::translate(-eyePointLocal.x(),-eyePointLocal.y(),0.0f));
        }
        return true;
    }
};

osg::Group* createModel()
{
    // no database loaded so automatically create Ed Levin Park..
    osg::Group* group = new osg::Group;

    // the base and sky subgraphs go to set the earth sky of the
    // model and clear the color and depth buffer for us, by using
    // osg::Depth, and setting their bin numbers to less than 0,
    // to force them to draw before the rest of the scene.

    osg::ClearNode* clearNode = new osg::ClearNode;
    clearNode->setRequiresClear(false); // we've got base and sky to do it.

    // use a transform to make the sky and base around with the eye point.
    osg::Transform* transform = new osg::Transform;

    // transform's value isn't knowm until in the cull traversal so its bounding
    // volume is can't be determined, therefore culling will be invalid,
    // so switch it off, this cause all our paresnts to switch culling
    // off as well. But don't worry culling will be back on once underneath
    // this node or any other branch above this transform.
    transform->setCullingActive(false);

    // set the compute transform callback to do all the work of
    // determining the transform according to the current eye point.
    transform->setComputeTransformCallback(new MoveEarthySkyWithEyePointCallback);

    // add the sky and base layer.
    transform->addChild(makeSky());  // bin number -2 so drawn first.
    transform->addChild(makeBase()); // bin number -1 so draw second.      

    // add the transform to the earth sky.
    clearNode->addChild(transform);

    // add to earth sky to the scene.
    group->addChild(clearNode);

    // the rest of the scene drawn after the base and sky above.
    group->addChild(makeTrees()); // will drop into a transparent, depth sorted bin (1)
    group->addChild(makeTerrain()); // will drop into default bin - state sorted 0
    group->addChild(makeTank()); // will drop into default bin - state sorted 0
    // add the following in the future...
    // makeGliders
    // makeClouds

    return group;
}

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
    if (!rootnode) rootnode = createModel();

    viewer.addViewport( rootnode );

    unsigned int pos = viewer.registerCameraManipulator(new GliderManipulator());

    // Open window so camera manipulator's warp pointer request will succeed
    viewer.open();

    viewer.selectCameraManipulator(pos);

    viewer.run();

    return 0;
}

