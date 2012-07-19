 /* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

using namespace osg;
using namespace osgSim;

// use this cull callback to allow the camera to traverse the Impostor's children without
// actuall having them assigned as children to the camea itself.  This make the camera a
// decorator without ever directly being assigned to it.
class ImpostorTraverseNodeCallback : public osg::NodeCallback
{
public:

    ImpostorTraverseNodeCallback(osgSim::Impostor* node):_node(node) {}

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        _node->LOD::traverse(*nv);
    }

    osgSim::Impostor* _node;
};

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

osg::BoundingSphere Impostor::computeBound() const
{
    return LOD::computeBound();
}

inline osgUtil::CullVisitor::value_type distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{

    //std::cout << "distance("<<coord<<", "<<matrix<<")"<<std::endl;

    return -((osgUtil::CullVisitor::value_type)coord[0]*(osgUtil::CullVisitor::value_type)matrix(0,2)+(osgUtil::CullVisitor::value_type)coord[1]*(osgUtil::CullVisitor::value_type)matrix(1,2)+(osgUtil::CullVisitor::value_type)coord[2]*(osgUtil::CullVisitor::value_type)matrix(2,2)+matrix(3,2));
}

void Impostor::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR)
    {
        LOD::traverse(nv);
        return;
    }

    osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
    if (!cv)
    {
        LOD::traverse(nv);
        return;
    }


    osg::Vec3 eyeLocal = nv.getEyePoint();
    const BoundingSphere& bs = getBound();

    unsigned int contextID = cv->getState() ? cv->getState()->getContextID() : 0;

    float distance2 = (eyeLocal-bs.center()).length2();
    float LODScale = cv->getLODScale();
    if (!cv->getImpostorsActive() ||
        distance2*LODScale*LODScale<osg::square(getImpostorThreshold()) ||
        distance2<bs.radius2()*2.0f)
    {
        // outwith the impostor distance threshold therefore simple
        // traverse the appropriate child of the LOD.
        LOD::traverse(nv);
    }
    else
    {

        // within the impostor distance threshold therefore attempt
        // to use impostor instead.

        RefMatrix& matrix = *cv->getModelViewMatrix();

        // search for the best fit ImpostorSprite;
        ImpostorSprite* impostorSprite = findBestImpostorSprite(contextID,eyeLocal);

        if (impostorSprite)
        {
            // impostor found, now check to see if it is good enough to use
            float error = impostorSprite->calcPixelError(*(cv->getMVPW()));

            if (error>cv->getImpostorPixelErrorThreshold())
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
            // one for use

            // create the impostor sprite.
            impostorSprite = createImpostorSprite(cv);

            //if (impostorSprite) impostorSprite->_color.set(0.0f,0.0f,1.0f,1.0f);

        }
        //else impostorSprite->_color.set(1.0f,1.0f,1.0f,1.0f);

        if (impostorSprite)
        {

            // update frame number to show that impostor is in action.
            impostorSprite->setLastFrameUsed(cv->getTraversalNumber());

            if (cv->getComputeNearFarMode()) cv->updateCalculatedNearFar(matrix,*impostorSprite, false);

            StateSet* stateset = impostorSprite->getStateSet();

            if (stateset) cv->pushStateSet(stateset);

            cv->addDrawableAndDepth(impostorSprite, &matrix, distance(getCenter(),matrix));

            if (stateset) cv->popStateSet();


        }
        else
        {
           // no impostor has been selected or created so default to
           // traversing the usual LOD selected child.
            LOD::traverse(nv);
        }

    }
}

ImpostorSprite* Impostor::createImpostorSprite(osgUtil::CullVisitor* cv)
{
    unsigned int contextID = cv->getState() ? cv->getState()->getContextID() : 0;

     osgSim::ImpostorSpriteManager* impostorSpriteManager = dynamic_cast<osgSim::ImpostorSpriteManager*>(cv->getUserData());
     if (!impostorSpriteManager)
     {
          impostorSpriteManager = new osgSim::ImpostorSpriteManager;
          cv->setUserData(impostorSpriteManager);
     }


    // default to true right now, will dertermine if perspective from the
    // projection matrix...
    bool isPerspectiveProjection = true;

    const Matrix& matrix = *(cv->getModelViewMatrix());
    const BoundingSphere& bs = getBound();
    osg::Vec3 eye_local = cv->getEyeLocal();

    if (!bs.valid())
    {
        OSG_WARN << "bb invalid"<<std::endl;
        return NULL;
    }

    Vec3 center_local = bs.center();
    Vec3 camera_up_local = cv->getUpLocal();
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

    // calc texture size for eye, bs.

    // convert the corners of the sprite (in world coords) into their
    // equivalent window coordinates by using the camera's project method.
    const osg::Matrix& MVPW = *(cv->getMVPW());
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

    const osg::Viewport& viewport = *(cv->getViewport());

    // if dimension is bigger than window divide it down.
    while (new_s>viewport.width()) new_s /= 2;

    // if dimension is bigger than window divide it down.
    while (new_t>viewport.height()) new_t /= 2;

    // create the impostor sprite.
    ImpostorSprite* impostorSprite =
        impostorSpriteManager->createOrReuseImpostorSprite(new_s,new_t,cv->getTraversalNumber()-cv->getNumberOfFrameToKeepImpostorSprites());

    if (impostorSprite==NULL)
    {
        OSG_WARN<<"Warning: unable to create required impostor sprite."<<std::endl;
        return NULL;
    }

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed(cv->getTraversalNumber());


    // have successfully created an impostor sprite so now need to
    // add it into the impostor.
    addImpostorSprite(contextID,impostorSprite);

    if (cv->getDepthSortImpostorSprites())
    {
        // the depth sort bin should probably be user definable,
        // will look into this later. RO July 2001.
        StateSet* stateset = impostorSprite->getStateSet();
        stateset->setRenderBinDetails(10,"DepthSortedBin");
    }

    osg::Texture2D* texture = impostorSprite->getTexture();

    texture->setTextureSize(new_s, new_t);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed(cv->getTraversalNumber());

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

    Vec3 eye_world(0.0,0.0,0.0);
    Vec3 center_world = bs.center()*matrix;


    osg::Camera* camera = impostorSprite->getCamera();
    if (!camera)
    {
        camera = new osg::Camera;
        impostorSprite->setCamera(camera);
    }

    camera->setCullCallback(new ImpostorTraverseNodeCallback(this));

    osgUtil::RenderStage* previous_stage = cv->getRenderStage();

    // set up the background color and clear mask.
    osg::Vec4 clear_color = previous_stage->getClearColor();
    clear_color[3] = 0.0f; // set thae alpha to zero.
    camera->setClearColor(clear_color);
    camera->setClearMask(previous_stage->getClearMask());


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
    if (isPerspectiveProjection)
    {
        // deal with projection issue move the top and right points
        // onto the near plane.
        float ratio = znear/(center_world-eye_world).length();
        top *= ratio;
        right *= ratio;
        camera->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
    }
    else
    {
        camera->setProjectionMatrixAsOrtho(-right,right,-top,top,znear,zfar);
    }

    Vec3 rotate_from = bs.center()-eye_local;
    Vec3 rotate_to   = cv-> getLookVectorLocal();

    osg::Matrix rotate_matrix =
        osg::Matrix::translate(-eye_local)*
        osg::Matrix::rotate(rotate_from,rotate_to)*
        osg::Matrix::translate(eye_local)*
        *cv->getModelViewMatrix();

    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(rotate_matrix);

    camera->setViewport(0,0,new_s,new_t);

    // tell the camera to use OpenGL frame buffer object where supported.
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT, osg::Camera::FRAME_BUFFER);

    // set the camera to render before the main camera.
    camera->setRenderOrder(osg::Camera::PRE_RENDER);

    // attach the texture and use it as the color buffer.
    camera->attach(osg::Camera::COLOR_BUFFER, texture);

    // do the cull traversal on the subgraph
    camera->accept(*cv);

    return impostorSprite;

}
