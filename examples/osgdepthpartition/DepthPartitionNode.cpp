#include "DepthPartitionNode.h"
#include <osgUtil/CullVisitor>

using namespace osg;

#define CURRENT_CLASS DepthPartitionNode

CURRENT_CLASS::CURRENT_CLASS()
{
    _distAccumulator = new DistanceAccumulator;
    init();
}

CURRENT_CLASS::CURRENT_CLASS(const CURRENT_CLASS& dpn, const osg::CopyOp& copyop)
    : osg::Group(dpn, copyop),
          _active(dpn._active),
          _renderOrder(dpn._renderOrder),
          _clearColorBuffer(dpn._clearColorBuffer)
{
    _distAccumulator = new DistanceAccumulator;
    _numCameras = 0;
}

CURRENT_CLASS::~CURRENT_CLASS() {}

void CURRENT_CLASS::init()
{
    _active = true;
    _numCameras = 0;
    setCullingActive(false);
    _renderOrder = osg::CameraNode::POST_RENDER;
    _clearColorBuffer = true;
}

void CURRENT_CLASS::setActive(bool active)
{
    if(_active == active) return;
    _active = active;
}

void CURRENT_CLASS::setClearColorBuffer(bool clear)
{
    _clearColorBuffer = clear;

    // Update the render order for the first CameraNode if it exists
    if(!_cameraList.empty())
    {
      if(clear) 
        _cameraList[0]->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      else  
        _cameraList[0]->setClearMask(GL_DEPTH_BUFFER_BIT);
    }
}

void CURRENT_CLASS::setRenderOrder(osg::CameraNode::RenderOrder order)
{
    _renderOrder = order;

    // Update the render order for existing CameraNodes
    unsigned int numCameras = _cameraList.size();
    for(unsigned int i = 0; i < numCameras; i++)
    {
      _cameraList[i]->setRenderOrder(_renderOrder);
    }
}

void CURRENT_CLASS::traverse(osg::NodeVisitor &nv)
{
    // If the scene hasn't been defined then don't do anything
    unsigned int numChildren = _children.size();
    if(numChildren == 0) return;

    // If the node is not active then don't analyze it
    if(!_active)
    {
      // Traverse the graph as usual
      Group::traverse(nv);
      return;
    }

    // If the visitor is not a cull visitor, pass it directly onto the scene.
    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if(!cv) 
    { 
      Group::traverse(nv);
      return; 
    }

    // We are in the cull traversal, so first collect information on the
    // current modelview and projection matrices and viewport.
    osg::RefMatrix& modelview = cv->getModelViewMatrix();
    osg::RefMatrix& projection = cv->getProjectionMatrix();
    osg::Viewport* viewport = cv->getViewport();

    // Prepare for scene traversal.
    _distAccumulator->setMatrices(modelview, projection);
    _distAccumulator->setNearFarRatio(cv->getNearFarRatio());
    _distAccumulator->reset();

    // Step 1: Traverse the children, collecting the near/far distances.
    unsigned int i;
    for(i = 0; i < numChildren; i++)
    {
      _children[i]->accept(*(_distAccumulator.get()));
    }

    // Step 2: Compute the near and far distances for every CameraNode that
    // should be used to render the scene.
    _distAccumulator->computeCameraPairs();

    // Step 3: Create the CameraNodes, and add them as children.
    DistanceAccumulator::PairList& camPairs = _distAccumulator->getCameraPairs();
    _numCameras = camPairs.size(); // Get the number of cameras

    // Create the CameraNodes, and add them as children.
    if(_numCameras > 0)
    {
      osg::CameraNode *currCam;
      DistanceAccumulator::DistancePair currPair;

      for(i = 0; i < _numCameras; i++)
      {
        // Create the camera, and clamp it's projection matrix
        currPair = camPairs[i];  // (near,far) pair for current camera
        currCam = createOrReuseCamera(projection, currPair.first, 
                                      currPair.second, i);

        // Set the modelview matrix and viewport of the camera
        currCam->setViewMatrix(modelview);
        currCam->setViewport(viewport);

        // Redirect the CullVisitor to the current camera
        currCam->accept(nv);
      }

      // Set the clear color for the first camera
      _cameraList[0]->setClearColor(cv->getRenderStage()->getClearColor());
    }
}

bool CURRENT_CLASS::addChild(osg::Node *child)
{
    return insertChild(_children.size(), child);
}

bool CURRENT_CLASS::insertChild(unsigned int index, osg::Node *child)
{
    if(!Group::insertChild(index, child)) return false; // Insert child

    // Insert child into each CameraNode
    unsigned int totalCameras = _cameraList.size();
    for(unsigned int i = 0; i < totalCameras; i++)
    {
      _cameraList[i]->insertChild(index, child);
    }
    return true;
}


bool CURRENT_CLASS::removeChildren(unsigned int pos, unsigned int numRemove)
{
    if(!Group::removeChildren(pos, numRemove)) return false; // Remove child

    // Remove child from each CameraNode
    unsigned int totalCameras = _cameraList.size();
    for(unsigned int i = 0; i < totalCameras; i++)
    {
      _cameraList[i]->removeChildren(pos, numRemove);
    }
    return true;
}

bool CURRENT_CLASS::setChild(unsigned int i, osg::Node *node)
{
    if(!Group::setChild(i, node)) return false; // Set child

    // Set child for each CameraNode
    unsigned int totalCameras = _cameraList.size();
    for(unsigned int j = 0; j < totalCameras; j++)
    {
      _cameraList[j]->setChild(i, node);
    }
    return true;
}

osg::CameraNode* CURRENT_CLASS::createOrReuseCamera(const osg::Matrix& proj, 
                            double znear, double zfar, 
                            const unsigned int &camNum)
{
    if(_cameraList.size() <= camNum) _cameraList.resize(camNum+1);
    osg::CameraNode *camera = _cameraList[camNum].get();
    
    if(!camera) // Create a new CameraNode
    {
      camera = new osg::CameraNode;
      camera->setCullingActive(false);
      camera->setRenderOrder(_renderOrder);
      camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);

      // We will compute the near/far planes ourselves
      camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);
      camera->setCullingMode(osg::CullSettings::ENABLE_ALL_CULLING);

      if(camNum == 0 && _clearColorBuffer)
        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      else
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);

      // Add our children to the new CameraNode's children
      unsigned int numChildren = _children.size();
      for(unsigned int i = 0; i < numChildren; i++)
      {
        camera->addChild(_children[i].get());
      }

      _cameraList[camNum] = camera;
    }

    osg::Matrixd &projection = camera->getProjectionMatrix();
    projection = proj;

    // Slightly inflate the near & far planes to avoid objects at the
    // extremes being clipped out.
    znear *= 0.999;
    zfar *= 1.001;

    // Clamp the projection matrix z values to the range (near, far)
    double epsilon = 1.0e-6;
    if(fabs(projection(0,3)) < epsilon &&
       fabs(projection(1,3)) < epsilon &&
       fabs(projection(2,3)) < epsilon ) // Projection is Orthographic
    {
      epsilon = -1.0/(zfar - znear); // Used as a temp variable
      projection(2,2) = 2.0*epsilon;
      projection(3,2) = (zfar + znear)*epsilon;
    }
    else // Projection is Perspective
    {
      double trans_near = (-znear*projection(2,2) + projection(3,2)) /
                          (-znear*projection(2,3) + projection(3,3));
      double trans_far = (-zfar*projection(2,2) + projection(3,2)) /
                         (-zfar*projection(2,3) + projection(3,3));
      double ratio = fabs(2.0/(trans_near - trans_far));
      double center = -0.5*(trans_near + trans_far);

      projection.postMult(osg::Matrixd(1.0, 0.0, 0.0, 0.0,
                                       0.0, 1.0, 0.0, 0.0,
                            0.0, 0.0, ratio, 0.0,
                       0.0, 0.0, center*ratio, 1.0));
    }

    return camera;
}
#undef CURRENT_CLASS
