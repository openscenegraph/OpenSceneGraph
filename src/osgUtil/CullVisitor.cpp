#include <osg/Transform>
#include <osg/Projection>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/LightSource>
#include <osg/ClipNode>
#include <osg/OccluderNode>
#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/AlphaFunc>

#include <osg/LineSegment>

#include <osg/GeoSet>

#include <osgUtil/CullVisitor>
#include <osgUtil/RenderToTextureStage>

#include <osgDB/ReadFile>

#include <float.h>
#include <algorithm>

#include <osg/Timer>

using namespace osg;
using namespace osgUtil;

inline float MAX_F(float a, float b)
    { return a>b?a:b; }
inline int EQUAL_F(float a, float b)
    { return a == b || fabsf(a-b) <= MAX_F(fabsf(a),fabsf(b))*1e-3f; }


class PrintVisitor : public NodeVisitor
{

   public:
   
        PrintVisitor():NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
            _indent = 0;
            _step = 4;
        }
        
        inline void moveIn() { _indent += _step; }
        inline void moveOut() { _indent -= _step; }
        inline void writeIndent() 
        {
            for(int i=0;i<_indent;++i) std::cout << " ";
        }
                
        virtual void apply(Node& node)
        {
            moveIn();
            writeIndent(); std::cout << node.className() <<std::endl;
            traverse(node);
            moveOut();
        }

        virtual void apply(Geode& node)         { apply((Node&)node); }
        virtual void apply(Billboard& node)     { apply((Geode&)node); }
        virtual void apply(LightSource& node)   { apply((Node&)node); }
        virtual void apply(ClipNode& node)      { apply((Node&)node); }
        
        virtual void apply(Group& node)         { apply((Node&)node); }
        virtual void apply(Transform& node)     { apply((Group&)node); }
        virtual void apply(Projection& node)    { apply((Group&)node); }
        virtual void apply(Switch& node)        { apply((Group&)node); }
        virtual void apply(LOD& node)           { apply((Group&)node); }
        virtual void apply(Impostor& node)      { apply((LOD&)node); }

   protected:
    
        int _indent;
        int _step;
};

CullVisitor::CullVisitor()
{
    // overide the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    _LODBias = 1.0f;

    //_tsm = LOOK_VECTOR_DISTANCE;
    _tsm = OBJECT_EYE_POINT_DISTANCE;


    _computeNearFar = COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES;
    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;
    
    _impostorActive = true;
    _depthSortImpostorSprites = false;
    _impostorPixelErrorThreshold = 4.0f;
    _numFramesToKeepImpostorSprites = 10;
    _impostorSpriteManager = osgNew ImpostorSpriteManager;

}


CullVisitor::~CullVisitor()
{
    reset();
}


void CullVisitor::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    
    CullStack::reset();

    // reset the calculated near far planes.
    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;
    
    
    osg::Vec3 lookVector(0.0,0.0,-1.0);
    
    _bbCornerFar = (lookVector.x()>=0?1:0) |
                   (lookVector.y()>=0?2:0) |
                   (lookVector.z()>=0?4:0);

    _bbCornerNear = (~_bbCornerFar)&7;
    
    // reset the resuse lists.
    _currentReuseMatrixIndex = 0;
    _currentReuseRenderLeafIndex = 0;
    
    for(RenderLeafList::iterator itr=_reuseRenderLeafList.begin();
        itr!=_reuseRenderLeafList.end();
        ++itr)
    {
        (*itr)->reset();
    }
    
}


void CullVisitor::popProjectionMatrix()
{

    if (_computeNearFar && _computed_zfar>0.0f)
    {

        // adjust the projection matrix so that it encompases the local coords.
        // so it doesn't cull them out.
        osg::Matrix& projection = *_projectionStack.back();

        double desired_znear = _computed_znear*0.98f;
        double desired_zfar = _computed_zfar*1.02f;

        double min_near_plane = _computed_zfar*0.0005f;
        if (desired_znear<min_near_plane) desired_znear=min_near_plane;

        double trans_near_plane = (-desired_znear*projection(2,2)+projection(3,2))/(-desired_znear*projection(2,3)+projection(3,3));
        double trans_far_plane = (-desired_zfar*projection(2,2)+projection(3,2))/(-desired_zfar*projection(2,3)+projection(3,3));

        double ratio = fabs(2.0f/(trans_near_plane-trans_far_plane));
        double center = -(trans_near_plane+trans_far_plane)/2.0f;

        projection.postMult(osg::Matrix(1.0f,0.0f,0.0f,0.0f,
                                        0.0f,1.0f,0.0f,0.0f,
                                        0.0f,0.0f,ratio,0.0f,
                                        0.0f,0.0f,center*ratio,1.0f));
    }

    CullStack::popProjectionMatrix();
}

inline float distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{
    return -(coord[0]*matrix(0,2)+coord[1]*matrix(1,2)+coord[2]*matrix(2,2)+matrix(3,2));
}


void CullVisitor::updateCalculatedNearFar(const osg::Matrix& matrix,const osg::BoundingBox& bb)
{
    float d_near = distance(bb.corner(_bbCornerNear),matrix);
    float d_far = distance(bb.corner(_bbCornerFar),matrix);

    if (d_near<=d_far)
    {
        if (d_near<_computed_znear) _computed_znear = d_near;
        if (d_far>_computed_zfar) _computed_zfar = d_far;
    }
    else
    {
        if ( !EQUAL_F(d_near, d_far) ) 
        {
            osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            osg::notify(osg::WARN)<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
        // note, need to reverse the d_near/d_far association because they are
        // the wrong way around...
        if (d_far<_computed_znear) _computed_znear = d_far;
        if (d_near>_computed_zfar) _computed_zfar = d_near;
    }

}
void CullVisitor::updateCalculatedNearFar(const osg::Vec3& pos)
{
    float d;
    if (!_modelviewStack.empty())
    {
        const osg::Matrix& matrix = *(_modelviewStack.back());
        d = distance(pos,matrix);
    }
    else
    {
        d = -pos.z();
    }

    if (d<_computed_znear) _computed_znear = d;
    if (d>_computed_zfar) _computed_zfar = d;
}   

void CullVisitor::apply(Node& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();
    
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();
    
    // pop the culling mode.
    popCurrentMask();
}


void CullVisitor::apply(Geode& node)
{
    if (isCulled(node)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    Matrix& matrix = getModelViewMatrix();
    for(int i=0;i<node.getNumDrawables();++i)
    {
        Drawable* drawable = node.getDrawable(i);
        const BoundingBox &bb =drawable->getBound();

        if( drawable->getCullCallback() )
        {
            if( drawable->getCullCallback()->cull( this, drawable ) == true )
            continue;
        }
        else
        {
            if (isCulled(bb)) continue;
        }


        if (_computeNearFar) updateCalculatedNearFar(matrix,*drawable);

        // push the geoset's state on the geostate stack.    
        StateSet* stateset = drawable->getStateSet();

        bool isTransparent = stateset && stateset->getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        if (isTransparent)
        {

            Vec3 center = (drawable->getBound().center())*matrix;

            float depth;
            switch(_tsm)
            {
                case(LOOK_VECTOR_DISTANCE):depth = -center.z();break;
                case(OBJECT_EYE_POINT_DISTANCE):
                default: depth = center.length2();break;
            }

            if (stateset) pushStateSet(stateset);
            addDrawableAndDepth(drawable,&matrix,depth);
            if (stateset) popStateSet();

        }
        else
        {
            if (stateset) pushStateSet(stateset);
            addDrawable(drawable,&matrix);
            if (stateset) popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(Billboard& node)
{
    if (isCulled(node)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    const Vec3& eye_local = getEyeLocal();
    const Matrix& modelview = getModelViewMatrix();

    for(int i=0;i<node.getNumDrawables();++i)
    {
        const Vec3& pos = node.getPos(i);

        Drawable* drawable = node.getDrawable(i);
        // need to modify isCulled to handle the billboard offset.
        // if (isCulled(drawable->getBound())) continue;

        Matrix* billboard_matrix = createOrReuseMatrix(modelview);

        node.getMatrix(*billboard_matrix,eye_local,pos);

        Vec3 center;
        center = pos*modelview;

        if (_computeNearFar)
        {
            float d =  center.z();
            if (d<_computed_znear) _computed_znear = d;
            if (d>_computed_zfar) _computed_zfar = d;
        }

        StateSet* stateset = drawable->getStateSet();
        
        bool isTransparent = stateset && stateset->getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        if (isTransparent)
        {


            float depth;
            switch(_tsm)
            {
                case(LOOK_VECTOR_DISTANCE):depth = -center.z();break;
                case(OBJECT_EYE_POINT_DISTANCE):
                default: depth = center.length2();break;
            }

            if (stateset) pushStateSet(stateset);
            addDrawableAndDepth(drawable,billboard_matrix,depth);
            if (stateset) popStateSet();

        }
        else
        {
            if (stateset) pushStateSet(stateset);
            addDrawable(drawable,billboard_matrix);
            if (stateset) popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(LightSource& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    Matrix& matrix = getModelViewMatrix();
    Light* light = node.getLight();
    if (light)
    {
        addPositionedAttribute(&matrix,light);
    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();
}

void CullVisitor::apply(ClipNode& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    Matrix& matrix = getModelViewMatrix();

    const ClipNode::ClipPlaneList& planes = node.getClipPlaneList();
    for(ClipNode::ClipPlaneList::const_iterator itr=planes.begin();
        itr!=planes.end();
        ++itr)
    {
        addPositionedAttribute(&matrix,itr->get());
    }

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();
}

void CullVisitor::apply(Group& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Transform& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    ref_ptr<osg::Matrix> matrix = createOrReuseMatrix(getModelViewMatrix());
    node.getLocalToWorldMatrix(*matrix,this);
    pushModelViewMatrix(matrix.get());
    
    handle_cull_callbacks_and_traverse(node);

    popModelViewMatrix();

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Projection& node)
{
    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);


    // record previous near and far values.
    float previous_znear = _computed_znear;
    float previous_zfar = _computed_zfar;
    
    _computed_znear = FLT_MAX;
    _computed_zfar = -FLT_MAX;

    ref_ptr<osg::Matrix> matrix = createOrReuseMatrix(node.getMatrix());
    pushProjectionMatrix(matrix.get());
    
    handle_cull_callbacks_and_traverse(node);

    popProjectionMatrix();

    _computed_znear = previous_znear;
    _computed_zfar = previous_zfar;


    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(Switch& node)
{
    apply((Group&)node);
}


void CullVisitor::apply(LOD& node)
{
    if (isCulled(node)) return;

    int eval = node.evaluate(getEyeLocal(),_LODBias);
    if (eval<0) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    //notify(INFO) << "selecting child "<<eval<< std::endl;
    handle_cull_callbacks_and_accept(node,node.getChild(eval));

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

void CullVisitor::apply(osg::EarthSky& node)
{
    // simply override the current earth sky.
    setEarthSky(&node);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

}

void CullVisitor::apply(osg::OccluderNode& node)
{
    // need to check if occlusion node is in the occluder
    // list, if so disable the appropriate ShadowOccluderVolume
    disableOccluder(_nodePath);
    
    std::cout<<"We are in an Occlusion node"<<&node<<std::endl;


    if (isCulled(node)) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);



    handle_cull_callbacks_and_traverse(node);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}



void CullVisitor::apply(Impostor& node)
{

    if (isCulled(node)) return;

    osg::Vec3 eyeLocal = getEyeLocal();

    int eval = node.evaluate(eyeLocal,_LODBias);
    if (eval<0) return;

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    const BoundingSphere& bs = node.getBound();

    float distance2 = (eyeLocal-bs.center()).length2();
    if (!_impostorActive ||
        distance2*_LODBias*_LODBias<node.getImpostorThreshold2() ||
        distance2<bs.radius2()*2.0f)
    {
        // outwith the impostor distance threshold therefore simple
        // traverse the appropriate child of the LOD.
        handle_cull_callbacks_and_accept(node,node.getChild(eval));
    }
    else if (_viewportStack.empty())
    {
        // need to use impostor but no valid viewport is defined to simply
        // default to using the LOD child as above.
        handle_cull_callbacks_and_accept(node,node.getChild(eval));
    }
    else
    {    

        // within the impostor distance threshold therefore attempt
        // to use impostor instead.
        
        Matrix& matrix = getModelViewMatrix();

        // search for the best fit ImpostorSprite;
        ImpostorSprite* impostorSprite = node.findBestImpostorSprite(eyeLocal);
        
        if (impostorSprite)
        {
            // impostor found, now check to see if it is good enough to use
            float error = impostorSprite->calcPixelError(getMVPW());

            if (error>_impostorPixelErrorThreshold)
            {
                // chosen impostor sprite pixel error is too great to use
                // from this eye point, therefore invalidate it.
                impostorSprite=NULL;
            }
        }
        

// need to think about sprite reuse and support for multiple context's.

        if (impostorSprite==NULL)
        {
            // no appropriate sprite has been found therefore need to create
            // one for use.
            
            // create the impostor sprite.
            impostorSprite = createImpostorSprite(node);

        }
        
        if (impostorSprite)
        {
            
            if (_computeNearFar) updateCalculatedNearFar(matrix,*impostorSprite);

            StateSet* stateset = impostorSprite->getStateSet();
            
            if (stateset) pushStateSet(stateset);
            
            if (_depthSortImpostorSprites)
            {
                Vec3 center = node.getCenter()*matrix;

                float depth;
                switch(_tsm)
                {
                    case(LOOK_VECTOR_DISTANCE):depth = -center.z();break;
                    case(OBJECT_EYE_POINT_DISTANCE):
                    default: depth = center.length2();break;
                }

                addDrawableAndDepth(impostorSprite,&matrix,depth);
            }
            else
            {
                addDrawable(impostorSprite,&matrix);
            }

            if (stateset) popStateSet();
            
            // update frame number to show that impostor is in action.
            impostorSprite->setLastFrameUsed(getTraversalNumber());
            
        }
        else
        {
           // no impostor has been selected or created so default to 
           // traversing the usual LOD selected child.
            handle_cull_callbacks_and_accept(node,node.getChild(eval));
        }
                
    }

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

ImpostorSprite* CullVisitor::createImpostorSprite(Impostor& node)
{
   
    // default to true right now, will dertermine if perspective from the
    // projection matrix...
    bool isPerspectiveProjection = true;

    const Matrix& matrix = getModelViewMatrix();
    const BoundingSphere& bs = node.getBound();
    osg::Vec3 eye_local = getEyeLocal();
    int eval = node.evaluate(eye_local,_LODBias);

    if (!bs.isValid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<&node<<std::endl;
        return NULL;
    }


    Vec3 eye_world(0.0,0.0,0.0);
    Vec3 center_world = bs.center()*matrix;

    // no appropriate sprite has been found therefore need to create
    // one for use.

    // create the render to texture stage.
    ref_ptr<RenderToTextureStage> rtts = osgNew RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    RenderStage* previous_stage = _currentRenderBin->_stage;

    // set up the background color and clear mask.
    osg::Vec4 clear_color = previous_stage->getClearColor();
    clear_color[3] = 0.0f; // set the alpha to zero.
    rtts->setClearColor(clear_color);
    rtts->setClearMask(previous_stage->getClearMask());

    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());


    // record the render bin, to be restored after creation
    // of the render to text
    RenderBin* previousRenderBin = _currentRenderBin;

    // set the current renderbin to be the newly created stage.
    _currentRenderBin = rtts.get();

    // create quad coords (in local coords)

    Vec3 center_local = bs.center();
    Vec3 camera_up_local = getUpLocal();
    Vec3 lv_local = center_local-eye_local;

    float distance_local = lv_local.length();
    lv_local /= distance_local;
   
    Vec3 sv_local = lv_local^camera_up_local;
    sv_local.normalize();
    
    Vec3 up_local = sv_local^lv_local;
    

    
    float width = bs.radius();
    if (isPerspectiveProjection)
    {
        // expand the width to account for projection onto sprite.
        width *= (distance_local/sqrtf(distance_local*distance_local-bs.radius2()));
    }
    
    // scale up and side vectors to sprite width.
    up_local *= width;
    sv_local *= width;
    
    // create the corners of the sprite.
    Vec3 c00(center_local - sv_local - up_local);
    Vec3 c10(center_local + sv_local - up_local);
    Vec3 c01(center_local - sv_local + up_local);
    Vec3 c11(center_local + sv_local + up_local);
    
// adjust camera left,right,up,down to fit (in world coords)

    Vec3 near_local  ( center_local-lv_local*width );
    Vec3 far_local   ( center_local+lv_local*width );
    Vec3 top_local   ( center_local+up_local);
    Vec3 right_local ( center_local+sv_local);
    
    Vec3 near_world = near_local * matrix;
    Vec3 far_world = far_local * matrix;
    Vec3 top_world = top_local * matrix;
    Vec3 right_world = right_local * matrix;
    
    float znear = (near_world-eye_world).length();
    float zfar  = (far_world-eye_world).length();
        
    float top   = (top_world-center_world).length();
    float right = (right_world-center_world).length();

    znear *= 0.9f;
    zfar *= 1.1f;

    // set up projection.
    osg::Matrix* projection = osgNew osg::Matrix;
    if (isPerspectiveProjection)
    {
        // deal with projection issue move the top and right points
        // onto the near plane.
        float ratio = znear/(center_world-eye_world).length();
        top *= ratio;
        right *= ratio;
        projection->makeFrustum(-right,right,-top,top,znear,zfar);
    }
    else
    {
        projection->makeOrtho(-right,right,-top,top,znear,zfar);
    }

    pushProjectionMatrix(projection);

    Vec3 rotate_from = bs.center()-eye_local;
    Vec3 rotate_to   = getLookVectorLocal();

    osg::Matrix* rotate_matrix = osgNew osg::Matrix(
        osg::Matrix::translate(-eye_local)*        
        osg::Matrix::rotate(rotate_from,rotate_to)*
        osg::Matrix::translate(eye_local)*
        getModelViewMatrix());

    // pushing the cull view state will update it so it takes
    // into account the new camera orientation.
    pushModelViewMatrix(rotate_matrix);

    ref_ptr<StateSet> dummyState = osgNew StateSet;

    pushStateSet(dummyState.get());

    {

        // traversing the usual LOD selected child.
        handle_cull_callbacks_and_accept(node,node.getChild(eval));

    }

    popStateSet();

    // restore the previous model view matrix.
    popModelViewMatrix();

    // restore the previous model view matrix.
    popProjectionMatrix();

    // restore the previous renderbin.
    _currentRenderBin = previousRenderBin;



    if (rtts->_renderGraphList.size()==0 && rtts->_bins.size()==0)
    {
        // getting to this point means that all the subgraph has been
        // culled by small feature culling or is beyond LOD ranges.
        return NULL;
    }




    const osg::Viewport& viewport = *getViewport();
    

    // calc texture size for eye, bs.

    // convert the corners of the sprite (in world coords) into their
    // equivilant window coordinates by using the camera's project method.
    const osg::Matrix& MVPW = getMVPW();
    Vec3 c00_win = c00 * MVPW;
    Vec3 c11_win = c11 * MVPW;

// adjust texture size to be nearest power of 2.

    float s  = c11_win.x()-c00_win.x();
    float t  = c11_win.y()-c00_win.y();

    // may need to reverse sign of width or height if a matrix has
    // been applied which flips the orientation of this subgraph.
    if (s<0.0f) s = -s;
    if (t<0.0f) t = -t;

    // bias value used to assist the rounding up or down of
    // the texture dimensions to the nearest power of two.
    // bias near 0.0 will almost always round down.
    // bias near 1.0 will almost always round up. 
    float bias = 0.7f;

    float sp2 = logf((float)s)/logf(2.0f);
    float rounded_sp2 = floorf(sp2+bias);
    int new_s = (int)(powf(2.0f,rounded_sp2));

    float tp2 = logf((float)t)/logf(2.0f);
    float rounded_tp2 = floorf(tp2+bias);
    int new_t = (int)(powf(2.0f,rounded_tp2));

    // if dimension is bigger than window divide it down.    
    while (new_s>viewport.width()) new_s /= 2;

    // if dimension is bigger than window divide it down.    
    while (new_t>viewport.height()) new_t /= 2;


    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = viewport.x()+viewport.width()/2;
    int center_y = viewport.y()+viewport.height()/2;

    Viewport* new_viewport = osgNew Viewport;
    new_viewport->setViewport(center_x-new_s/2,center_y-new_t/2,new_s,new_t);
    rtts->setViewport(new_viewport);
    
    dummyState->setAttribute(new_viewport);

// create the impostor sprite.

    ImpostorSprite* impostorSprite = 
        _impostorSpriteManager->createOrReuseImpostorSprite(new_s,new_t,getTraversalNumber()-_numFramesToKeepImpostorSprites);

    if (impostorSprite==NULL) return NULL;

    // have successfully created an impostor sprite so now need to
    // add it into the impostor.
    node.addImpostorSprite(impostorSprite);

    if (_depthSortImpostorSprites)
    {
        // the depth sort bin should probably be user definable,
        // will look into this later. RO July 2001.
        StateSet* stateset = impostorSprite->getStateSet();
        stateset->setRenderBinDetails(1,"DepthSortedBin");
    }
    
    Texture* texture = impostorSprite->getTexture();

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed(getTraversalNumber());

    Vec3* coords = impostorSprite->getCoords();
    Vec2* texcoords = impostorSprite->getTexCoords();
    
    coords[0] = c01;
    texcoords[0].set(0.0f,1.0f);

    coords[1] = c00;
    texcoords[1].set(0.0f,0.0f);

    coords[2] = c10;
    texcoords[2].set(1.0f,0.0f);

    coords[3] = c11;
    texcoords[3].set(1.0f,1.0f);
    
    impostorSprite->dirtyBound();
    
    Vec3* controlcoords = impostorSprite->getControlCoords();
    
    if (isPerspectiveProjection)
    {
        // deal with projection issue by moving the coorners of the quad
        // towards the eye point.
        float ratio = width/(center_local-eye_local).length();
        float one_minus_ratio = 1.0f-ratio;
        Vec3 eye_local_ratio = eye_local*ratio;
        
        controlcoords[0] = coords[0]*one_minus_ratio + eye_local_ratio;
        controlcoords[1] = coords[1]*one_minus_ratio + eye_local_ratio;
        controlcoords[2] = coords[2]*one_minus_ratio + eye_local_ratio;
        controlcoords[3] = coords[3]*one_minus_ratio + eye_local_ratio;
    }
    else
    {
        // project the control points forward towards the eyepoint,
        // but since this an othographics projection this projection is
        // parallel.
        Vec3 dv = lv_local*width;

        controlcoords[0] = coords[0]-dv;
        controlcoords[1] = coords[1]-dv;
        controlcoords[2] = coords[2]-dv;
        controlcoords[3] = coords[3]-dv;
    }

    impostorSprite->setStoredLocalEyePoint(eye_local);


    // and the render to texture stage to the current stages
    // dependancy list.
    _currentRenderBin->_stage->addToDependencyList(rtts.get());

    // attach texture to the RenderToTextureStage.
    rtts->setTexture(texture);

    // must sort the RenderToTextureStage so that all leaves are
    // accounted correctly in all renderbins i.e depth sorted bins.    
    rtts->sort();

    return impostorSprite;

}
