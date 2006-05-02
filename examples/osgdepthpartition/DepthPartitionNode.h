#ifndef _OF_DEPTHPARTITIONNODE_
#define _OF_DEPTHPARTITIONNODE_

#include "DistanceAccumulator.h"
#include <osg/CameraNode>

#define CURRENT_CLASS DepthPartitionNode
/**********************************************************
 * Ravi Mathur
 * OpenFrames API, class DepthPartitionNode
 * A type of osg::Group that analyzes a scene, then partitions it into
 * several segments that can be rendered separately. Each segment
 * is small enough in the z-direction to avoid depth buffer problems
 * for very large scenes.
**********************************************************/
class CURRENT_CLASS : public osg::Group
{
  public:
	CURRENT_CLASS();
	CURRENT_CLASS(const CURRENT_CLASS& dpn, 
	              const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

	META_Node( OpenFrames, CURRENT_CLASS ); // Common Node functions

	/** Set the active state. If not active, this node will simply add the
	    specified scene as it's child, without analyzing it at all. */
	void setActive(bool active);
	inline bool getActive() const { return _active; }

	/** Specify whether the color buffer should be cleared before the first
	    CameraNode draws it's scene. */
	void setClearColorBuffer(bool clear);
	inline bool getClearColorBuffer() const { return _clearColorBuffer; }

	/** Specify the render order for each CameraNode */
	void setRenderOrder(osg::CameraNode::RenderOrder order);
	inline osg::CameraNode::RenderOrder getRenderOrder() const
	{ return _renderOrder; }

	/** Set/get the maximum depth that the scene will be traversed to.
	    Defaults to UINT_MAX. */
	void setMaxTraversalDepth(unsigned int depth) 
	{ _distAccumulator->setMaxDepth(depth); }

	inline unsigned int getMaxTraversalDepth() const
	{ return _distAccumulator->getMaxDepth(); }

	/** Override update and cull traversals */
	virtual void traverse(osg::NodeVisitor &nv);

	/** Catch child management functions so the CameraNodes can be informed
	    of added or removed children. */
	virtual bool addChild(osg::Node *child);
	virtual bool insertChild(unsigned int index, osg::Node *child);
	virtual bool removeChildren(unsigned int pos, unsigned int numRemove = 1);
	virtual bool setChild(unsigned int i, osg::Node *node);

  protected:
	typedef std::vector< osg::ref_ptr<osg::CameraNode> > CameraList;

	~CURRENT_CLASS();

	void init();

	// Creates a new CameraNode object with default settings
	osg::CameraNode* createOrReuseCamera(const osg::Matrix& proj, 
                                double znear, double zfar, 
                                const unsigned int &camNum);

	bool _active; // Whether partitioning is active on the scene

	// The NodeVisitor that computes cameras for the scene
	osg::ref_ptr<DistanceAccumulator> _distAccumulator;

	osg::CameraNode::RenderOrder _renderOrder;
	bool _clearColorBuffer;

	// Cameras that should be used to draw the scene.  These cameras
	// will be reused on every frame in order to save time and memory.
	CameraList _cameraList;
	unsigned int _numCameras; // Number of CameraNodes actually being used
};
#undef CURRENT_CLASS

#endif
