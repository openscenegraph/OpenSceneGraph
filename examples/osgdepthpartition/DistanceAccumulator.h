#ifndef _OF_DISTANCEACCUMULATOR_
#define _OF_DISTANCEACCUMULATOR_

#include <osg/Group>
#include <osg/NodeVisitor>
#include <osg/Polytope>
#include <osg/fast_back_stack>

#define CURRENT_CLASS DistanceAccumulator
/**********************************************************
 * Ravi Mathur
 * OpenFrames API, class DistanceAccumulator
 * Class that traverses the scene and computes the distance to each
 * visible drawable, and splits up the scene if the drawables are
 * too far away (in the z direction) from each other. 
**********************************************************/
class CURRENT_CLASS : public osg::NodeVisitor
{
  public:
	typedef std::pair<double, double> DistancePair;
	typedef std::vector<DistancePair> PairList;

	CURRENT_CLASS();

	virtual void apply(osg::Node &node);
	virtual void apply(osg::Projection &proj);
	virtual void apply(osg::Transform &transform);
	virtual void apply(osg::Geode &geode);

	// Specify the modelview & projection matrices
	void setMatrices(const osg::Matrix &modelview,
	                 const osg::Matrix &projection);

	// Reset visitor before a new traversal
	virtual void reset();

	// Create a (near,far) distance pair for each camera of the specified
	// distance pair list and distance limits.
	void computeCameraPairs();

	// Get info on the cameras that should be used for scene rendering
	PairList& getCameraPairs() { return _cameraPairs; }

	// Get info on the computed distance pairs
	PairList& getDistancePairs() { return _distancePairs; }

	// Get info on the computed nearest/farthest distances
	DistancePair& getLimits() { return _limits; }

	// Set/get the desired near/far ratio
	void setNearFarRatio(double ratio);
	inline double getNearFarRatio() const { return _nearFarRatio; }

	inline void setMaxDepth(unsigned int depth) { _maxDepth = depth; }
	inline unsigned int getMaxDepth() const { return _maxDepth; }

  protected:
	virtual ~CURRENT_CLASS();

	void pushLocalFrustum();
	void pushDistancePair(double zNear, double zFar);
	bool shouldContinueTraversal(osg::Node &node);

	// Stack of matrices accumulated during traversal
	osg::fast_back_stack<osg::Matrix> _viewMatrices;
	osg::fast_back_stack<osg::Matrix> _projectionMatrices;

	// Main modelview/projection matrices
	osg::Matrix _modelview, _projection;

	// The view frusta in local coordinate space
	osg::fast_back_stack<osg::Polytope> _localFrusta;

	// Bounding box corners that should be used for cull computation
	typedef std::pair<unsigned int, unsigned int> bbCornerPair;
	osg::fast_back_stack<bbCornerPair> _bbCorners;

	// Nar/far planes that should be used for each camera
	PairList _cameraPairs;

	// Accumulated pairs of min/max distances
	PairList _distancePairs;

	// The closest & farthest distances found while traversing
	DistancePair _limits;

	// Ratio of nearest/farthest clip plane for each section of the scene
	double _nearFarRatio;

	// Maximum depth to traverse to
	unsigned int _maxDepth, _currentDepth;
};
#undef CURRENT_CLASS

#endif
