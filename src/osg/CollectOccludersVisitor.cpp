#include <osg/CollectOccludersVisitor>
#include <osg/Transform>
#include <osg/Switch>
#include <osg/LOD>
#include <osg/OccluderNode>
#include <osg/Projection>

using namespace osg;

CollectOccludersVisitor::CollectOccludersVisitor()
{
    // overide the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    setCullingMode(VIEW_FRUSTUM_CULLING|
                   NEAR_PLANE_CULLING|
                   FAR_PLANE_CULLING|
                   SMALL_FEATURE_CULLING);
    
    _minimumShadowOccluderVolume = 0.01f;
    _createDrawables = false;
    
}

CollectOccludersVisitor::~CollectOccludersVisitor()
{
}

void CollectOccludersVisitor::reset()
{
    CullStack::reset();
}

void CollectOccludersVisitor::apply(osg::Node& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();
    
    handle_cull_callbacks_and_traverse(node);
    
    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Transform& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::Matrix> matrix = createOrReuseMatrix(getModelViewMatrix());
    node.getLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix.get());
    
    handle_cull_callbacks_and_traverse(node);

    popModelViewMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Projection& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    ref_ptr<osg::Matrix> matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix.get());
    
    handle_cull_callbacks_and_traverse(node);

    popProjectionMatrix();

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::Switch& node)
{
    apply((Group&)node);
}

void CollectOccludersVisitor::apply(osg::LOD& node)
{
    if (isCulled(node)) return;

    int eval = node.evaluate(getEyeLocal(),_LODBias);
    if (eval<0) return;

    // push the culling mode.
    pushCurrentMask();

    //notify(INFO) << "selecting child "<<eval<< std::endl;
    handle_cull_callbacks_and_accept(node,node.getChild(eval));

    // pop the culling mode.
    popCurrentMask();
}

void CollectOccludersVisitor::apply(osg::OccluderNode& node)
{
    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableOccluder(_nodePath);
    

    if (isCulled(node)) return;

//    std::cout<<"CollectOccludersVisitor:: We have found an Occlusion node in frustum"<<&node<<std::endl;

    // push the culling mode.
    pushCurrentMask();


    if (node.getOccluder())
    {
        // computeOccluder will check if the occluder is the view frustum,
        // if it ins't then the it will return false, when in it will
        // clip the occluder's polygons in clip space, then create occluder
        // planes, all with their normals facing inward towards the volume,
        // and then transform them back into projection space.
        ShadowVolumeOccluder svo;
        if (svo.computeOccluder(_nodePath, *node.getOccluder(), *this,_createDrawables))
        {
        
            if (svo.getVolume()>_minimumShadowOccluderVolume)
            {
                // need to test occluder against view frustum.
                std::cout << "    adding in Occluder"<<std::endl;
                _occluderList.push_back(svo);
            }
            else
            {
                std::cout << "    rejecting Occluder as its volume is too small "<<svo.getVolume()<<std::endl;
            }
        }
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the culling mode.
    popCurrentMask();
}


