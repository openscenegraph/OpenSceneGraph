#include "osgUtil/RenderVisitor"
#include "osg/DCS"
#include "osg/Geode"
#include "osg/LOD"
#include "osg/Billboard"
#include "osg/LightSource"
#include "osg/Notify"

#include <float.h>
#include <algorithm>

#ifndef OSG_USE_IO_DOT_H
#include <iostream>
using namespace std;
#endif

using namespace osg;
using namespace osgUtil;

static bool g_debugging = false;

ViewState::ViewState()
{
    _matrix = NULL;
    _inverse = NULL;
    _ratio = 0.002f;
    _viewFrustumCullingActive=true;
    _smallFeatureCullingActive=true;
}

ViewState::~ViewState()
{
    if (_matrix) _matrix->unref();
    if (_inverse) _inverse->unref();
}


bool ViewState::isCulled(const BoundingSphere& sp)
{
    if (!sp.isValid()) return true;


    Vec3 delta = sp._center-_eyePoint;
    if (_smallFeatureCullingActive)
    {
        if (sp._radius<delta.length()*_ratio)
        {
            return true;
        }
    }
        
    if (_viewFrustumCullingActive)
    {
        if (delta*_frustumTopNormal>sp._radius) 
        {
            return true;
        }
        if (delta*_frustumBottomNormal>sp._radius) 
        {
            return true;
        }
        if (delta*_frustumLeftNormal>sp._radius) 
        {
            return true;
        }
        if (delta*_frustumRightNormal>sp._radius) 
        {
            return true;
        }
    }
    return false;
}

bool ViewState::isCulled(const BoundingBox& bb)
{
    if (!bb.isValid()) return true;

    if (_viewFrustumCullingActive)
    {
        unsigned int c;
        for(c=0;c<8;++c)
        {
            if ((bb.corner(c)-_eyePoint)*_frustumLeftNormal<=0.0f) break;
        }
        // if all corners have been checked, therefore all points are
        // and the far side of the left clipping plane and hence should be culled.
        if (c==8) return true;

        for(c=0;c<8;++c)
        {
            if ((bb.corner(c)-_eyePoint)*_frustumRightNormal<=0.0f) break;
        }
        // if all corners have been checked, therefore all points are
        // and the far side of the right clipping plane and hence should be culled.
        if (c==8) return true;

        for(c=0;c<8;++c)
        {
            if ((bb.corner(c)-_eyePoint)*_frustumTopNormal<=0.0f) break;
        }
        // if all corners have been checked, therefore all points are
        // and the far side of the top clipping plane and hence should be culled.
        if (c==8) return true;


        for(c=0;c<8;++c)
        {
            if ((bb.corner(c)-_eyePoint)*_frustumBottomNormal<=0.0f) break;
        }
        // if all corners have been checked, therefore all points are
        // and the far side of the bottom clipping plane and hence should be culled.
        if (c==8) return true;
    }

    return false;
}

RenderVisitor::RenderVisitor()
{
    // overide the default node visitor mode.
    setTraverseMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    _globalState=NULL;
    _LODBias = 1.0f;

    _fovy=60.0f;
    _aspect=1.0f;
    _znear=1.0f;
    _zfar=1000.0f;

    _tvs = new ViewState;
    _tvs->_eyePoint.set(0.0f,0.0f,1.0f);
    _tvs->_centerPoint.set(0.0f,0.0f,0.0f);
    _tvs->_lookVector.set(0.0f,0.0f,-1.0f);
    _tvs->_upVector.set(0.0f,1.0f,0.0f);
    _tvs->ref();

    _cvs = _tvs;
    _cvs->ref();

    _tsm = LOOK_VECTOR_DISTANCE;
    _tsm = OBJECT_EYE_POINT_DISTANCE;

    _viewFrustumCullingActive=true;
    _smallFeatureCullingActive=true;

    calculateClippingPlanes();
}

RenderVisitor::~RenderVisitor()
{
    reset();
    // if a global geostate is attached simply unref it.
    if (_globalState) 
    {
        _globalState->unref();
        _globalState = NULL;
    }
    if (_tvs) _tvs->unref();
    if (_cvs) _cvs->unref();
}

void RenderVisitor::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    std::for_each(_viewStateStack.begin(),_viewStateStack.end(),UnrefOp<ViewState>());
    _viewStateStack.erase(_viewStateStack.begin(),_viewStateStack.end());
    if (_cvs!=_tvs)
    {
        if (_cvs) _cvs->unref();
        _cvs = _tvs;
        _cvs->ref();
    }
    
    for(std::multimap<float,MatrixGeoSet>::iterator titr= _transparentGeoSets.begin();
        titr!= _transparentGeoSets.end();
        ++titr)
    {
        if ((*titr).second.first) (*titr).second.first->unref();
    }
    _transparentGeoSets.erase(_transparentGeoSets.begin(),_transparentGeoSets.end());

    for(std::multimap<GeoState*,MatrixGeoSet>::iterator oitr= _opaqueGeoSets.begin();
        oitr!= _opaqueGeoSets.end();
        ++oitr)
    {
        if ((*oitr).second.first) (*oitr).second.first->unref();
    }
    _opaqueGeoSets.erase(_opaqueGeoSets.begin(),_opaqueGeoSets.end());

    _lights.erase(_lights.begin(),_lights.end());

}

void RenderVisitor::setGlobalState(GeoState* global)
{
    if (global==_globalState) return;
    if (_globalState) _globalState->unref();
    _globalState = global;
    _globalState->ref();
}


void RenderVisitor::setPerspective(const osg::Camera& camera)
{
    _fovy=camera.getFieldOfViewY();
    _aspect=camera.getAspectRatio();
    _znear=camera.getNearPlane();
    _zfar=camera.getFarPlane();

    calculateClippingPlanes();
}

void RenderVisitor::setPerspective(float fovy,float aspect,float znear,float zfar)
{
    _fovy=fovy;
    _aspect=aspect;
    _znear=znear;
    _zfar=zfar;

    calculateClippingPlanes();
}

void RenderVisitor::setLookAt(const Camera& camera)
{
    setLookAt(camera.getEyePoint(),camera.getLookPoint(),camera.getUpVector());
}

void RenderVisitor::setLookAt(const Vec3& eye,const Vec3& center,const Vec3& upVector)
{
    _tvs->_eyePoint = eye;

    _tvs->_centerPoint = center;

    _tvs->_lookVector = center-eye;
    _tvs->_lookVector.normalize();

    _tvs->_upVector = upVector;

    calculateClippingPlanes();
}


void RenderVisitor::setLookAt(double eyeX,double eyeY,double eyeZ,
                              double centerX,double centerY,double centerZ,
                              double upX,double upY,double upZ)
{
    _tvs->_eyePoint[0] = eyeX;
    _tvs->_eyePoint[1] = eyeY;
    _tvs->_eyePoint[2] = eyeZ;

    _tvs->_centerPoint[0] = centerX;
    _tvs->_centerPoint[1] = centerY;
    _tvs->_centerPoint[2] = centerZ;

    _tvs->_lookVector[0] = centerX-eyeX;
    _tvs->_lookVector[1] = centerY-eyeY;
    _tvs->_lookVector[2] = centerZ-eyeZ;
    _tvs->_lookVector.normalize();

    _tvs->_upVector[0] = upX;
    _tvs->_upVector[1] = upY;
    _tvs->_upVector[2] = upZ;

    calculateClippingPlanes();
}

void RenderVisitor::setCullingActive(CullingType ct,bool active)
{
    switch(ct)
    {
    case(VIEW_FRUSTUM_CULLING):_viewFrustumCullingActive=active;break;
    case(SMALL_FEATURE_CULLING):_smallFeatureCullingActive=active;break;
    }
}

bool RenderVisitor::getCullingActive(CullingType ct)
{
    switch(ct)
    {
    case(VIEW_FRUSTUM_CULLING):return _viewFrustumCullingActive;
    case(SMALL_FEATURE_CULLING):return _smallFeatureCullingActive;
    }
    return false;
}

void RenderVisitor::calculateClippingPlanes()
{
    float half_fovy = _fovy*0.5f;
    float s = sinf(half_fovy*M_PI/180.0f);
    float c = cosf(half_fovy*M_PI/180.0f);

    Vec3 lv = _tvs->_lookVector;
    Vec3 up = _tvs->_upVector;
    Vec3 sv = lv ^ up;
    sv.normalize();

    _tvs->_frustumTopNormal = -lv*s + up*c;
    _tvs->_frustumTopNormal.normalize();

    _tvs->_frustumBottomNormal = -lv*s - up*c;
    _tvs->_frustumBottomNormal.normalize();

    _tvs->_frustumLeftNormal = -sv*c - lv*(s*_aspect);
    _tvs->_frustumLeftNormal.normalize();

    _tvs->_frustumRightNormal = sv*c - lv*(s*_aspect);
    _tvs->_frustumRightNormal.normalize();

//     notify(INFO) << "_frustumTopNormal = "<<_tvs->_frustumTopNormal[0]<<"\t"<<_tvs->_frustumTopNormal[1]<<"\t"<<_tvs->_frustumTopNormal[2]<<endl;
//     notify(INFO) << "_frustumBottomNormal = "<<_tvs->_frustumBottomNormal[0]<<"\t"<<_tvs->_frustumBottomNormal[1]<<"\t"<<_tvs->_frustumBottomNormal[2]<<endl;
//     notify(INFO) << "_frustumLeftNormal = "<<_tvs->_frustumLeftNormal[0]<<"\t"<<_tvs->_frustumLeftNormal[1]<<"\t"<<_tvs->_frustumLeftNormal[2]<<endl;
//     notify(INFO) << "_frustumRightNormal = "<<_tvs->_frustumRightNormal[0]<<"\t"<<_tvs->_frustumRightNormal[1]<<"\t"<<_tvs->_frustumRightNormal[2]<<endl;
    
}


void RenderVisitor::pushMatrix(const Matrix& matrix)
{

    ViewState* nvs = new ViewState;
    nvs->ref();
    nvs->_viewFrustumCullingActive=_viewFrustumCullingActive;
    nvs->_smallFeatureCullingActive=_smallFeatureCullingActive;

    if (_cvs && _cvs->_matrix)
    {
        nvs->_matrix = new Matrix;
        nvs->_matrix->mult(matrix,*(_cvs->_matrix));
    }
    else
    {
        nvs->_matrix = new Matrix(matrix);
    }
    nvs->_matrix->ref();

    Matrix* inverse_world = new Matrix;
    inverse_world->ref();
    inverse_world->invert(*(nvs->_matrix));
    nvs->_inverse = inverse_world;

    nvs->_eyePoint = _tvs->_eyePoint*(*inverse_world);
    nvs->_centerPoint = _tvs->_centerPoint*(*inverse_world);

    nvs->_lookVector = nvs->_centerPoint - nvs->_eyePoint;
    nvs->_lookVector.normalize();

    nvs->_frustumTopNormal = (_tvs->_eyePoint + _tvs->_frustumTopNormal)*(*inverse_world)-nvs->_eyePoint;
    nvs->_frustumTopNormal.normalize();

    nvs->_frustumBottomNormal = (_tvs->_eyePoint + _tvs->_frustumBottomNormal)*(*inverse_world)-nvs->_eyePoint;
    nvs->_frustumBottomNormal.normalize();

    nvs->_frustumLeftNormal = (_tvs->_eyePoint + _tvs->_frustumLeftNormal)*(*inverse_world)-nvs->_eyePoint;
    nvs->_frustumLeftNormal.normalize();

    nvs->_frustumRightNormal = (_tvs->_eyePoint + _tvs->_frustumRightNormal)*(*inverse_world)-nvs->_eyePoint;
    nvs->_frustumRightNormal.normalize();

    if (_cvs) _cvs->unref();

    _cvs = nvs;
    
    if (_cvs) _cvs->ref();
    _viewStateStack.push_back(nvs);
}

void RenderVisitor::popMatrix()
{
    // pop the top of the view stack and unref it.
    ViewState* pvs = _viewStateStack.back();
    _viewStateStack.pop_back();
    pvs->unref();

    // unref previous cvs
    if (_cvs) _cvs->unref();

    // to new cvs and ref it.
    if (_viewStateStack.empty())
    {
        _cvs = _tvs;
        if (_cvs) _cvs->ref();
    }
    else
    {
        _cvs = _viewStateStack.back();
        if (_cvs) _cvs->ref();
    }
    
}

Matrix* RenderVisitor::getCurrentMatrix()
{
    return _cvs->_matrix;
}

Matrix* RenderVisitor::getInverseCurrentMatrix()
{
    return _cvs->_inverse;
}

const Vec3& RenderVisitor::getEyeLocal()
{
    return _cvs->_eyePoint;
}

const Vec3& RenderVisitor::getCenterLocal()
{
    return _cvs->_centerPoint;
}

const Vec3& RenderVisitor::getLookVectorLocal()
{
    return _cvs->_lookVector;
}

bool RenderVisitor::isCulled(const BoundingSphere& sp)
{
    return _cvs->isCulled(sp);
}

bool RenderVisitor::isCulled(const BoundingBox& bb)
{
    return _cvs->isCulled(bb);
}

void RenderVisitor::apply(Node& node)
{
    if (isCulled(node.getBound())) return;

    traverse(node);
}

void RenderVisitor::apply(Geode& node)
{
    if (isCulled(node.getBound())) return;

    Matrix* matrix = getCurrentMatrix();
    for(int i=0;i<node.getNumGeosets();++i)
    {
        GeoSet* gset = node.getGeoSet(i);
        if (isCulled(gset->getBound())) continue;

        GeoState* gstate = gset->getGeoState();
        bool isTransparent = gstate && gstate->isTransparent();
        if (isTransparent)
        {

            Vec3 center;
            if (matrix)
            {
                center = (gset->getBound().center())*(*matrix);
            }
            else
            {
                center = gset->getBound().center();
            }
            Vec3 delta_center = center-_tvs->_eyePoint;

            if (g_debugging)
            {
                notify(INFO) << "center ["<<center.x()<<","<<center.y()<<","<<center.z()<<"]"<<endl;
                notify(INFO) << "delta_center ["<<delta_center.x()<<","<<delta_center.y()<<","<<delta_center.z()<<"]"<<endl;
                notify(INFO) << "_lookVector ["<<_tvs->_lookVector.x()<<","<<_tvs->_lookVector.y()<<","<<_tvs->_lookVector.z()<<"]"<<endl;
            }

            float depth;
            switch(_tsm)
            {
            case(LOOK_VECTOR_DISTANCE):depth = _tvs->_lookVector*delta_center;break;
            case(OBJECT_EYE_POINT_DISTANCE):
            default: depth = delta_center.length2();break;
            }

            if (matrix) matrix->ref();
            _transparentGeoSets.insert(
                std::pair<float,MatrixGeoSet>(depth,MatrixGeoSet(matrix,gset))
                );
        }
        else
        {
            if (matrix) matrix->ref();
            _opaqueGeoSets.insert(
                std::pair<GeoState*,MatrixGeoSet>(gstate,MatrixGeoSet(matrix,gset))
                );
        }
    }
}

void RenderVisitor::apply(Billboard& node)
{
    if (isCulled(node.getBound())) return;

    Vec3 eye_local = getEyeLocal();
    
    for(int i=0;i<node.getNumGeosets();++i)
    {
        Vec3 pos;
        node.getPos(i,pos);
        
        GeoSet* gset = node.getGeoSet(i);
        // need to modify isCulled to handle the billboard offset.
        // if (isCulled(gset->getBound())) continue;

        Matrix local_mat;
        node.calcTransform(eye_local,pos,local_mat);
        
        Matrix* matrix = NULL;
        if (_cvs->_matrix)
        {
            matrix = new Matrix();
            matrix->mult(local_mat,*(_cvs->_matrix));
        }
        else 
        {
            matrix = new Matrix(local_mat);
        }

        GeoState* gstate = gset->getGeoState();
        bool isTransparent = gstate && gstate->isTransparent();
        if (isTransparent)
        {

            Vec3 center;
            if (matrix)
            {
                center = (gset->getBound().center())*(*matrix);
            }
            else
            {
                center = gset->getBound().center();
            }
            Vec3 delta_center = center-_tvs->_eyePoint;

            if (g_debugging)
            {
                notify(INFO) << "center ["<<center.x()<<","<<center.y()<<","<<center.z()<<"]"<<endl;
                notify(INFO) << "delta_center ["<<delta_center.x()<<","<<delta_center.y()<<","<<delta_center.z()<<"]"<<endl;
                notify(INFO) << "_lookVector ["<<_tvs->_lookVector.x()<<","<<_tvs->_lookVector.y()<<","<<_tvs->_lookVector.z()<<"]"<<endl;
            }

            float depth;
            switch(_tsm)
            {
            case(LOOK_VECTOR_DISTANCE):depth = _tvs->_lookVector*delta_center;break;
            case(OBJECT_EYE_POINT_DISTANCE):
            default: depth = delta_center.length2();break;
            }

            if (matrix) matrix->ref();
            _transparentGeoSets.insert(
                std::pair<float,MatrixGeoSet>(depth,MatrixGeoSet(matrix,gset))
                );
        }
        else
        {
            if (matrix) matrix->ref();
            _opaqueGeoSets.insert(
                std::pair<GeoState*,MatrixGeoSet>(gstate,MatrixGeoSet(matrix,gset))
                );
        }
        
    }
}

void RenderVisitor::apply(LightSource& node)
{
    Matrix* matrix = getCurrentMatrix();
    Light* light = node.getLight();
    if (light)
    {
        if (matrix) matrix->ref();
        _lights.insert(std::pair<Matrix*,Light*>(matrix,light));
    }
}

void RenderVisitor::apply(Group& node)
{
    if (isCulled(node.getBound())) return;

    traverse(node);
}


void RenderVisitor::apply(DCS& node)
{
    if (isCulled(node.getBound())) return;

    pushMatrix(*node.getMatrix());

    traverse(node);

    popMatrix();
}


void RenderVisitor::apply(Switch& node)
{
    apply((Group&)node);
}


void RenderVisitor::apply(LOD& node)
{
    if (isCulled(node.getBound())) return;

    int eval = node.evaluate(getEyeLocal(),_LODBias);
    if (eval<0)
    {
        //notify(INFO) << "culled LOD"<<endl;
        return;
    }
    else
    {
        //notify(INFO) << "selecting child "<<eval<<endl;
        node.getChild(eval)->accept(*this);
    }
    
}


void RenderVisitor::apply(Scene& node)
{
    apply((Group&)node);
}


bool RenderVisitor::calcNearFar(double& near_plane,double& far_plane)
{
    if (_opaqueGeoSets.empty() && _transparentGeoSets.empty()) return false;

    near_plane = FLT_MAX;
    far_plane = -FLT_MAX;

    Vec3 eyePoint = getEyeLocal();
    Vec3 lookVector = getLookVectorLocal();

    for(OpaqueList::iterator oitr = _opaqueGeoSets.begin();
                             oitr != _opaqueGeoSets.end();
                             ++oitr)
    {
        Matrix* matrix = (oitr->second).first;
        GeoSet* gset = (oitr->second).second;
        const BoundingBox& bb = gset->getBound();
        for(int c=0;c<8;++c)
        {
            float d;
            if (matrix) d = ((bb.corner(c)*(*matrix))-eyePoint)*lookVector;
            else d = (bb.corner(c)-eyePoint)*lookVector;
            if (d<near_plane) near_plane = d;
            if (d>far_plane) far_plane = d;
        }
    }

    for(TransparentList::iterator titr = _transparentGeoSets.begin();
                                  titr != _transparentGeoSets.end();
                                  ++titr)
    {
        Matrix* matrix = (titr->second).first;
        GeoSet* gset = (titr->second).second;
        const BoundingBox& bb = gset->getBound();
        for(int c=0;c<8;++c)
        {
            float d;
            if (matrix) d = ((bb.corner(c)*(*matrix))-eyePoint)*lookVector;
            else d = (bb.corner(c)-eyePoint)*lookVector;
            if (d<near_plane) near_plane = d;
            if (d>far_plane) far_plane = d;
        }
    }

    return true;
}

void RenderVisitor::render()
{

    Matrix* prevMatrix = NULL;
    Matrix* currMatrix = NULL;

    GeoState* currGeoState = NULL;
    GeoState* prevGeoState = NULL;

    if (g_debugging) notify(INFO) << "start of render"<<endl;

    if (_globalState)
    {
        _globalState->apply();
        prevGeoState=_globalState;
    }


    // apply the lists.
    for(LightList::iterator litr=_lights.begin();
                            litr!=_lights.end();
                            ++litr)
    {
        currMatrix = litr->first;
        if (currMatrix!=prevMatrix)
        {
            if (prevMatrix) glPopMatrix();
            if (currMatrix)
            {
                glPushMatrix();
                glMultMatrixf( (GLfloat *)(currMatrix->_mat) );
            }
            prevMatrix = currMatrix;
        }
        (litr->second)->apply();
    }

    // draw the opaque bin.
    for(std::multimap<GeoState*,MatrixGeoSet>::iterator oitr= _opaqueGeoSets.begin();
        oitr!= _opaqueGeoSets.end();
        ++oitr)
    {
        if (g_debugging)
        {
            notify(INFO) << "  drawing opaque matrix["<<(*oitr).second.first<<"]"
                << "  geoset["<<(*oitr).second.second<<"]"
                << "  geostate["<<(*oitr).second.second->getGeoState()<<"]"<<endl;
        }

        currMatrix = (*oitr).second.first;
        if (currMatrix!=prevMatrix)
        {
            if (prevMatrix) glPopMatrix();
            if (currMatrix)
            {
                glPushMatrix();
                glMultMatrixf( (GLfloat *)(currMatrix->_mat) );
            }
            prevMatrix = currMatrix;
        }

        currGeoState = (*oitr).first;
        if (currGeoState!=prevGeoState)
        {
            if (currGeoState) currGeoState->apply(_globalState,prevGeoState);
            prevGeoState = currGeoState;
        }

        (*oitr).second.second->draw();
    }


//    glPushAttrib( GL_DEPTH_TEST );
//    glDisable( GL_DEPTH_TEST );

    // render the transparent geoset in reverse order, so to render the
    // deepest transparent objects, relative to the look vector, first.
    for(std::multimap<float,MatrixGeoSet>::reverse_iterator titr= _transparentGeoSets.rbegin();
        titr!= _transparentGeoSets.rend();
        ++titr)
    {
        if (g_debugging)
        {
            notify(INFO) << "  drawing transparent matrix["<<(*titr).second.first<<"]"
                << "  geoset["<<(*titr).second.second<<"]"
                << "  geostate["<<(*titr).second.second->getGeoState()<<"]"
                << "  depth["<<(*titr).first<<"]"<<endl;
        }

        currMatrix = (*titr).second.first;
        if (currMatrix!=prevMatrix)
        {
            if (prevMatrix) glPopMatrix();
            if (currMatrix)
            {
                glPushMatrix();
                glMultMatrixf( (GLfloat *)(currMatrix->_mat) );
            }
            prevMatrix = currMatrix;
        }

        currGeoState = (*titr).second.second->getGeoState();
        if (currGeoState!=prevGeoState)
        {
            if (currGeoState) currGeoState->apply(_globalState,prevGeoState);
            prevGeoState = currGeoState;
        }

        (*titr).second.second->draw();
    }

//    glPopAttrib();


    if (currMatrix) glPopMatrix();

    if (g_debugging)
    {
        notify(INFO) << "end of render"<<endl<<endl;
        notify(INFO).flush();
    }
}
