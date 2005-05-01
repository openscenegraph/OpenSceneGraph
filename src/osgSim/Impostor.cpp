/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osgSim/Impostor>

#include <algorithm>

using namespace osgSim;

Impostor::Impostor()
{
    _impostorThreshold = -1.0f;
}


ImpostorSprite* Impostor::findBestImpostorSprite(unsigned int contextID, const osg::Vec3& currLocalEyePoint) const
{
    ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

    float min_distance2 = FLT_MAX;
    ImpostorSprite* impostorSprite = NULL;
    for(ImpostorSpriteList::iterator itr=impostorSpriteList.begin();
        itr!=impostorSpriteList.end();
        ++itr)
    {
        float distance2 = (currLocalEyePoint-(*itr)->getStoredLocalEyePoint()).length2();
        if (distance2<min_distance2)
        {
            min_distance2 = distance2;
            impostorSprite = itr->get();
        }
    }
    return impostorSprite;
}

void Impostor::addImpostorSprite(unsigned int contextID, ImpostorSprite* is)
{
    if (is && is->getParent()!=this)
    {
        ImpostorSpriteList& impostorSpriteList = _impostorSpriteListBuffer[contextID];

        // add it to my impostor list first, so it remains referenced
        // when its reference in the previous_owner is removed.
        impostorSpriteList.push_back(is);

        if (is->getParent())
        {
            Impostor* previous_owner = is->getParent();
            ImpostorSpriteList& isl = previous_owner->_impostorSpriteListBuffer[contextID];
            
            // find and erase reference to is.
            for(ImpostorSpriteList::iterator itr=isl.begin();
                itr!=isl.end();
                ++itr)
            {
                if ((*itr)==is)
                {
                    isl.erase(itr);
                    break;
                }
            }
        }
        is->setParent(this);
        
    }
}

bool Impostor::computeBound() const
{
    return LOD::computeBound();
}

void Impostor::traverse(osg::NodeVisitor& nv)
{
    LOD::traverse(nv);
}


#if 0

// From CullVisitor header

        /** Create an impostor sprite by setting up a pre-rendering stage
          * to generate the impostor texture. */
        osg::ImpostorSprite* createImpostorSprite(osg::Impostor& node);

        osg::ref_ptr<osg::ImpostorSpriteManager>    _impostorSpriteManager;

// From CullVisitor constructor

    _impostorSpriteManager = new ImpostorSpriteManager;

// From CullVisitor reset
    if (_impostorSpriteManager.valid()) _impostorSpriteManager->reset();
    
    
// Form CullVisitor.cpp

void CullVisitor::apply(Impostor& node)
{

    if (isCulled(node)) return;

    osg::Vec3 eyeLocal = getEyeLocal();

    // push the culling mode.
    pushCurrentMask();

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    const BoundingSphere& bs = node.getBound();
    
    unsigned int contextID = 0;
    if (_state.valid()) contextID = _state->getContextID();

    float distance2 = (eyeLocal-bs.center()).length2();
    if (!_impostorActive ||
        distance2*_LODScale*_LODScale<osg::square(node.getImpostorThreshold()) ||
        distance2<bs.radius2()*2.0f)
    {
        // outwith the impostor distance threshold therefore simple
        // traverse the appropriate child of the LOD.
        handle_cull_callbacks_and_traverse(node);
    }
    else if (_viewportStack.empty())
    {
        // need to use impostor but no valid viewport is defined to simply
        // default to using the LOD child as above.
        handle_cull_callbacks_and_traverse(node);
    }
    else
    {    

        // within the impostor distance threshold therefore attempt
        // to use impostor instead.
        
        RefMatrix& matrix = getModelViewMatrix();

        // search for the best fit ImpostorSprite;
        ImpostorSprite* impostorSprite = node.findBestImpostorSprite(contextID,eyeLocal);
        
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

            //if (impostorSprite) impostorSprite->_color.set(0.0f,0.0f,1.0f,1.0f);

        }
        //else impostorSprite->_color.set(1.0f,1.0f,1.0f,1.0f);
        
        if (impostorSprite)
        {
            
            // update frame number to show that impostor is in action.
            impostorSprite->setLastFrameUsed(getTraversalNumber());

            if (_computeNearFar) updateCalculatedNearFar(matrix,*impostorSprite, false);

            StateSet* stateset = impostorSprite->getStateSet();
            
            if (stateset) pushStateSet(stateset);
            
            addDrawableAndDepth(impostorSprite,&matrix,distance(node.getCenter(),matrix));

            if (stateset) popStateSet();
            
            
        }
        else
        {
           // no impostor has been selected or created so default to 
           // traversing the usual LOD selected child.
            handle_cull_callbacks_and_traverse(node);
        }
                
    }

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    popCurrentMask();
}

ImpostorSprite* CullVisitor::createImpostorSprite(Impostor& node)
{
   
    unsigned int contextID = 0;
    if (_state.valid()) contextID = _state->getContextID();

    // default to true right now, will dertermine if perspective from the
    // projection matrix...
    bool isPerspectiveProjection = true;

    const Matrix& matrix = getModelViewMatrix();
    const BoundingSphere& bs = node.getBound();
    osg::Vec3 eye_local = getEyeLocal();

    if (!bs.valid())
    {
        osg::notify(osg::WARN) << "bb invalid"<<&node<<std::endl;
        return NULL;
    }


    Vec3 eye_world(0.0,0.0,0.0);
    Vec3 center_world = bs.center()*matrix;

    // no appropriate sprite has been found therefore need to create
    // one for use.

    // create the render to texture stage.
    ref_ptr<RenderToTextureStage> rtts = new RenderToTextureStage;

    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    RenderStage* previous_stage = _currentRenderBin->getStage();

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
    osg::RefMatrix* projection = new osg::RefMatrix;
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

    osg::RefMatrix* rotate_matrix = new osg::RefMatrix(
        osg::Matrix::translate(-eye_local)*        
        osg::Matrix::rotate(rotate_from,rotate_to)*
        osg::Matrix::translate(eye_local)*
        getModelViewMatrix());

    // pushing the cull view state will update it so it takes
    // into account the new camera orientation.
    pushModelViewMatrix(rotate_matrix);

    StateSet* localPreRenderState = _impostorSpriteManager->createOrReuseStateSet();

    pushStateSet(localPreRenderState);

    {

        // traversing the usual LOD selected child.
        handle_cull_callbacks_and_traverse(node);

    }

    popStateSet();

    // restore the previous model view matrix.
    popModelViewMatrix();

    // restore the previous model view matrix.
    popProjectionMatrix();

    // restore the previous renderbin.
    _currentRenderBin = previousRenderBin;



    if (rtts->getRenderGraphList().size()==0 && rtts->getRenderBinList().size()==0)
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

    Viewport* new_viewport = new Viewport;
    new_viewport->setViewport(center_x-new_s/2,center_y-new_t/2,new_s,new_t);
    rtts->setViewport(new_viewport);
    
    localPreRenderState->setAttribute(new_viewport);

    // create the impostor sprite.
    ImpostorSprite* impostorSprite = 
        _impostorSpriteManager->createOrReuseImpostorSprite(new_s,new_t,getTraversalNumber()-_numFramesToKeepImpostorSprites);

    if (impostorSprite==NULL)
    {
        osg::notify(osg::WARN)<<"Warning: unable to create required impostor sprite."<<std::endl;
        return NULL;
    }

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed(getTraversalNumber());


    // have successfully created an impostor sprite so now need to
    // add it into the impostor.
    node.addImpostorSprite(contextID,impostorSprite);

    if (_depthSortImpostorSprites)
    {
        // the depth sort bin should probably be user definable,
        // will look into this later. RO July 2001.
        StateSet* stateset = impostorSprite->getStateSet();
        stateset->setRenderBinDetails(10,"DepthSortedBin");
    }
    
    Texture2D* texture = impostorSprite->getTexture();

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
    _currentRenderBin->getStage()->addToDependencyList(rtts.get());

    // attach texture to the RenderToTextureStage.
    rtts->setTexture(texture);

    // must sort the RenderToTextureStage so that all leaves are
    // accounted correctly in all renderbins i.e depth sorted bins.    
    rtts->sort();

    return impostorSprite;

}

#endif
    
