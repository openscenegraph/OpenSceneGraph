#include <osg/GL>
#include <osg/Camera>
#include <osg/Types>
#include <osg/Notify>

using namespace osg;

#define DEG2RAD(x)	((x)*M_PI/180.0)
#define RAD2DEG(x)	((x)*180.0/M_PI)

Camera::Camera()
{

    _adjustAspectRatioMode = ADJUST_HORIZONTAL;

    // projection details.
    setPerspective(60,1.0,1.0,1000.0);
        
    // look at details.
    _lookAtType =USE_HOME_POSITON;

    _eye.set(0.0f,0.0f,0.0f);
    _center.set(0.0f,0.0f,-1.0f);
    _up.set(0.0f,1.0f,0.0f);

    _focalLength = 1.0f;

    _useNearClippingPlane = false;
    _useFarClippingPlane = false;

}


Camera::~Camera()
{
}



/** Set a orthographics projection. See glOrtho for further details.*/
void Camera::setOrtho(const double left, const double right,
                      const double bottom, const double top,
                      const double zNear, const double zFar)
{
    _projectionType = ORTHO;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _zNear = zNear;
    _zFar = zFar;

    _dirty = true;
}


/** Set a 2D orthographics projection. See gluOrtho2D for further details.*/
void Camera::setOrtho2D(const double left, const double right,
                        const double bottom, const double top)
{
    _projectionType = ORTHO2D;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _zNear = -1.0;
    _zFar = 1.0;

    _dirty = true;
}


/** Set a perspective projection. See glFrustum for further details.*/
void Camera::setFrustum(const double left, const double right,
                        const double bottom, const double top,
                        const double zNear, const double zFar)
{
    _projectionType = FRUSTUM;
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.
    _left = left/zNear;
    _right = right/zNear;
    _bottom = bottom/zNear;
    _top = top/zNear;
    _zNear = zNear;
    _zFar = zFar;

    _dirty = true;
}



/** Set a sysmetical perspective projection, See gluPerspective for further details.*/
void Camera::setPerspective(const double fovy,const double aspectRatio,
                            const double zNear, const double zFar)
{
    _projectionType = PERSPECTIVE;
    
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.

    // calculate the appropriate left, right etc.
    double tan_fovy = tan(DEG2RAD(fovy*0.5));
    _right  =  tan_fovy * aspectRatio;
    _left   = -_right;
    _top    =  tan_fovy;
    _bottom =  -_top;

    _zNear = zNear;
    _zFar = zFar;

    notify(INFO)<<"osg::Camera::setPerspective(fovy="<<fovy<<",aspectRatio="<<aspectRatio<<","<<endl;
    notify(INFO)<<"                            zNear="<<zNear<<", zFar="<<zFar<<")"<<endl;    
    notify(INFO)<<"    osg::Camera::calc_fovx()="<<calc_fovx()<<endl;
    notify(INFO)<<"    osg::Camera::calc_fovy()="<<calc_fovy()<<endl;
    
    _dirty = true;
}

/** Set a sysmetical perspective projection using field of view.*/
void Camera::setFOV(const double fovx,const double fovy,
                    const double zNear, const double zFar)
{
    _projectionType = PERSPECTIVE;
    
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.

    // calculate the appropriate left, right etc.
    double tan_fovx = tan(DEG2RAD(fovx*0.5));
    double tan_fovy = tan(DEG2RAD(fovy*0.5));
    _right  =  tan_fovx;
    _left   = -_right;
    _top    =  tan_fovy;
    _bottom =  -_top;

    _zNear = zNear;
    _zFar = zFar;

    notify(INFO)<<"osg::Camera::setFOV(fovx="<<fovx<<",fovy="<<fovy<<","<<endl;
    notify(INFO)<<"                    zNear="<<zNear<<", zFar="<<zFar<<")"<<endl;    
    notify(INFO)<<"    osg::Camera::calc_fovx()="<<calc_fovx()<<endl;
    notify(INFO)<<"    osg::Camera::calc_fovy()="<<calc_fovy()<<endl;
    
    _dirty = true;
}

/** Set the near and far clipping planes.*/
void Camera::setNearFar(const double zNear, const double zFar)
{
    _zNear = zNear;
    _zFar = zFar;
    _dirty = true;
    if (_projectionType==ORTHO2D)
    {
        if (_zNear!=-1.0 || _zFar!=1.0) _projectionType = ORTHO;
    }

    _dirty = true;
}

/** Adjust the clipping planes to account for a new window aspcect ratio.
  * Typicall used after resizeing a window.*/
void Camera::adjustAspectRatio(const double newAspectRatio, const AdjustAspectRatioMode aa)
{
    double previousAspectRatio = (_right-_left)/(_top-_bottom);
    double deltaRatio = newAspectRatio/previousAspectRatio;
    if (aa == ADJUST_HORIZONTAL)
    {
        _left *= deltaRatio;
        _right *= deltaRatio;
    }
    else // aa == ADJUST_VERTICAL
    {
        _bottom /= deltaRatio;
        _top /= deltaRatio;
    }

    notify(INFO)<<"osg::Camera::adjustAspectRatio(newAspectRatio="<<newAspectRatio<<", AdjustAxis="<<aa<<")"<<endl;
    notify(INFO)<<"    osg::Camera::calc_fovx()="<<calc_fovx()<<endl;
    notify(INFO)<<"    osg::Camera::calc_fovy()="<<calc_fovy()<<endl;

    _dirty = true;
}

const double Camera::left() const
{
    switch(_projectionType)
    {
        case(FRUSTUM):
        case(PERSPECTIVE): return _left * _zNear;
    }
    return _left;
}
const double Camera::right() const
{
    switch(_projectionType)
    {
        case(FRUSTUM):
        case(PERSPECTIVE): return _right * _zNear;
    }
    return _right;
}

const double Camera::top() const
{
    switch(_projectionType)
    {
        case(FRUSTUM):
        case(PERSPECTIVE): return _top * _zNear;
    }
    return _top;
}

const double Camera::bottom() const
{
    switch(_projectionType)
    {
        case(FRUSTUM):
        case(PERSPECTIVE): return _bottom * _zNear;
    }
    return _bottom;
}

const double Camera::zNear() const
{
    return _zNear;
}

const double Camera::zFar() const
{
    return _zFar;
}

/** Calculate and return the equivilant fovx for the current project setting.
  * This value is only valid for when a symetric persepctive projection exists.
  * i.e. getProjectionType()==PERSPECTIVE.*/
const double Camera::calc_fovy() const
{
     // note, _right & _left are prescaled by znear so 
     // no need to account for it.
    return RAD2DEG(atan(_top)-atan(_bottom));
}


/** Calculate and return the equivilant fovy for the current project setting.
  * This value is only valid for when a symetric persepctive projection exists.
  * i.e. getProjectionType()==PERSPECTIVE.*/
const double Camera::calc_fovx() const
{
     // note, _right & _left are prescaled by znear so 
     // no need to account for it.
    return RAD2DEG(atan(_right)-atan(_left));
}


/** Calculate and return the projection aspect ratio.*/
const double Camera::calc_aspectRatio() const
{
    double delta_x = _right-_left;
    double delta_y = _top-_bottom;
    return delta_x/delta_y;
}

const Matrix& Camera::getProjectionMatrix() const
{
    if (_dirty) calculateMatricesAndClippingVolume();
    return *_projectionMatrix;
}

void Camera::home()
{
    // OpenGL default position.
    _lookAtType = USE_HOME_POSITON;
    _eye.set(0.0f,0.0f,0.0f);
    _center.set(0.0f,0.0f,-1.0f);
    _up.set(0.0f,1.0f,0.0f);
    
    // need to set to appropriate values..
    _focalLength = 1.0f;
    
    _dirty = true;
}

void Camera::setView(const Vec3& eyePoint, const Vec3& lookPoint, const Vec3& upVector)
{
    setLookAt(eyePoint,lookPoint,upVector);
}



void Camera::setLookAt(const Vec3& eye,
                       const Vec3& center,
                       const Vec3& up)
{
    _lookAtType = USE_EYE_CENTER_AND_UP;
    _eye = eye;
    _center = center;
    _up = up;
    
    ensureOrthogonalUpVector();

    // need to set to appropriate values..
    _focalLength = (center-eye).length();
    
    _dirty = true;
}


void Camera::setLookAt(const double eyeX, const double eyeY, const double eyeZ,
               const double centerX, const double centerY, const double centerZ,
               const double upX, const double upY, const double upZ)
{
    _lookAtType = USE_EYE_CENTER_AND_UP;
    _eye.set(eyeX,eyeY,eyeZ);
    _center.set(centerX,centerY,centerZ);
    _up.set(upX,upY,upZ);
    
    ensureOrthogonalUpVector();

    // need to set to appropriate values..
    _focalLength = (_center-_eye).length();
    
    _dirty = true;
}


/** post multiple the existing eye point and orientation by matrix.
  * note, does not affect any ModelTransforms that are applied.*/
void Camera::transformLookAt(const Matrix& matrix)
{
//    cout << "transformLookAt"<<matrix<<endl;
    _up = (_up+_eye)*matrix;
    _eye = _eye*matrix;
    _center = _center*matrix;
    _up -= _eye;
    _up.normalize();
}

const Vec3 Camera::getLookVector() const
{
    osg::Vec3 lv(_center-_eye);
    lv.normalize();
    return lv;
}

const Vec3 Camera::getSideVector() const
{
    osg::Vec3 lv(_center-_eye);
    lv.normalize();
    osg::Vec3 sv(lv^_up);
    sv.normalize();
    return sv;
}


void Camera::attachTransform(const TransformMode mode, Matrix* matrix)
{
    switch(mode)
    {
    case(EYE_TO_MODEL):
        {
            _eyeToModelTransform = matrix;
            if (_eyeToModelTransform.valid())
            {
                _attachedTransformMode = mode;
                if (!_modelToEyeTransform.valid()) _modelToEyeTransform = new Matrix;
                _modelToEyeTransform->invert(*_eyeToModelTransform);
            }
            else
            {
                _attachedTransformMode = NO_ATTACHED_TRANSFORM;
                _modelToEyeTransform = NULL;
            }
        }
        break;
    case(MODEL_TO_EYE):
        {
            _modelToEyeTransform = matrix;
            if (_modelToEyeTransform.valid())
            {
                _attachedTransformMode = mode;
                if (!_eyeToModelTransform.valid()) _eyeToModelTransform = new Matrix;
                _eyeToModelTransform->invert(*_modelToEyeTransform);
            }
            else
            {
                _attachedTransformMode = NO_ATTACHED_TRANSFORM;
                _eyeToModelTransform = NULL;
            }
        }
        break;
    case(NO_ATTACHED_TRANSFORM):
        _attachedTransformMode = NO_ATTACHED_TRANSFORM;
        _eyeToModelTransform =  NULL;
        _modelToEyeTransform = NULL;
        break;
    default: 
        _attachedTransformMode = NO_ATTACHED_TRANSFORM;
        notify(WARN)<<"Warning: invalid TransformMode pass to osg::Camera::attachTransform(..)"<<endl;
        notify(WARN)<<"         setting Camera to NO_ATTACHED_TRANSFORM."<<endl;
        break;
    }

    _dirty = true;
}

void Camera::dirtyTransform()
{
    _dirty = true;

    switch(_attachedTransformMode)
    {
    case(EYE_TO_MODEL):
        // should be safe to assume that these matrices are valid
        // as attachTransform will ensure it.
        _modelToEyeTransform->invert(*_eyeToModelTransform);
        break;
    case(MODEL_TO_EYE):
        // should be safe to assume that these matrices are valid
        // as attachTransform will ensure it.
        _eyeToModelTransform->invert(*_modelToEyeTransform);
        break;
    }

}

Matrix* Camera::getTransform(const TransformMode mode)
{
    switch(mode)
    {
    case(EYE_TO_MODEL): return _eyeToModelTransform.get();
    case(MODEL_TO_EYE): return _modelToEyeTransform.get();
    default: return NULL;
    }
}

const Matrix* Camera::getTransform(const TransformMode mode) const
{
    switch(mode)
    {
    case(EYE_TO_MODEL): return _eyeToModelTransform.get();
    case(MODEL_TO_EYE): return _modelToEyeTransform.get();
    default: return NULL;
    }
}


const Vec3 Camera::getEyePoint_Model() const
{
    if (_eyeToModelTransform.valid()) return _eye*(*_eyeToModelTransform);
    else return _eye;
}

const Vec3 Camera::getCenterPoint_Model() const
{
    if (_eyeToModelTransform.valid()) return _center*(*_eyeToModelTransform);
    else return _center;
}


const Vec3 Camera::getLookVector_Model() const
{
    if (_eyeToModelTransform.valid())
    {
        Vec3 zero_transformed = Vec3(0.0f,0.0f,0.0f)*(*_eyeToModelTransform);
        Vec3 look_transformed = getLookVector()*(*_eyeToModelTransform);
        look_transformed -= zero_transformed;
        look_transformed.normalize();
        return look_transformed;
    }
    else return getLookVector();
}

const Vec3 Camera::getUpVector_Model() const
{
    if (_eyeToModelTransform.valid())
    {
        Vec3 zero_transformed = Vec3(0.0f,0.0f,0.0f)*(*_eyeToModelTransform);
        Vec3 up_transformed = getUpVector()*(*_eyeToModelTransform);
        up_transformed -= zero_transformed;
        up_transformed.normalize();
        return up_transformed;
    }
    else return getUpVector();
}

const Vec3 Camera::getSideVector_Model() const
{
    if (_eyeToModelTransform.valid())
    {
        Vec3 zero_transformed = Vec3(0.0f,0.0f,0.0f)*(*_eyeToModelTransform);
        Vec3 side_transformed = getSideVector()*(*_eyeToModelTransform);
        side_transformed -= zero_transformed;
        side_transformed.normalize();
        return side_transformed;
    }
    else return getSideVector();
}

const Matrix& Camera::getModelViewMatrix() const
{
    if (_dirty) calculateMatricesAndClippingVolume();
    return *_modelViewMatrix;
}

void Camera::setUseNearClippingPlane(const bool use)
{
    if (_useNearClippingPlane != use)
    {
        _useNearClippingPlane = use;
        _dirty = true;
    }
}

void Camera::setUseFarClippingPlane(const bool use)
{
    if (_useFarClippingPlane != use)
    {
        _useFarClippingPlane = use; 
        _dirty = true;
    }
}

const ClippingVolume& Camera::getClippingVolume() const
{
    if (_dirty) calculateMatricesAndClippingVolume();
    return _clippingVolume;
}

void Camera::calculateMatricesAndClippingVolume() const
{

    // set up the projection matrix.
    switch(_projectionType)
    {
        case(ORTHO):
        case(ORTHO2D):
            {
                float A = 2.0/(_right-_left);
                float B = 2.0/(_top-_bottom);
                float C = -2.0 / (_zFar-_zNear);
                float tx = -(_right+_left)/(_right-_left);
                float ty = -(_top+_bottom)/(_top-_bottom);
                float tz = -(_zFar+_zNear)/(_zFar-_zNear);

                _projectionMatrix = new Matrix(
                    A,    0.0f, 0.0f, 0.0f,
                    0.0f, B,    0.0f, 0.0f,
                    0.0f, 0.0f, C,    0.0f,
                    tx,   ty,   tz,    1.0f );
            }
            break;
        case(FRUSTUM):
        case(PERSPECTIVE):
            {
                
                // note, in Frustum/Perspective mode these values are scaled
                // by the zNear from when they were initialised to ensure that
                // subsequent changes in zNear do not affect them.

                float A = (2.0)/(_right-_left);
                float B = (2.0)/(_top-_bottom);
                float C = (_right+_left) / (_right-_left);
                float D = (_top+_bottom) / (_top-_bottom);
                float E = -(_zFar+_zNear) / (_zFar-_zNear);
                float F = -(2.0*_zFar*_zNear) / (_zFar-_zNear);

                _projectionMatrix = new Matrix(
                    A,    0.0f, 0.0f, 0.0f,
                    0.0f, B,    0.0f, 0.0f,
                    C,    D,    E,    -1.0f,
                    0.0f, 0.0f, F,    0.0f );

            }
            break;

    }


    // set up the model view matrix.
    switch(_lookAtType)
    {
    case(USE_HOME_POSITON):
        if (_modelToEyeTransform.valid())
        {
            _modelViewMatrix = _modelToEyeTransform;
        }
        else
        {
            _modelViewMatrix = new Matrix;
            _modelViewMatrix->makeIdent();
        }
        break;
    case(USE_EYE_AND_QUATERNION): // not implemented yet, default to eye,center,up.
    case(USE_EYE_CENTER_AND_UP):
    default:
        {
        
            Vec3 f(_center-_eye);
            f.normalize();
            Vec3 s(f^_up);
            s.normalize();
            Vec3 u(s^f);
            u.normalize();
            
            ref_ptr<Matrix> matrix = new Matrix(
                s[0],     u[0],     -f[0],     0.0f,
                s[1],     u[1],     -f[1],     0.0f,
                s[2],     u[2],     -f[2],     0.0f,
                0.0f,     0.0f,     0.0f,      1.0f);

            (*matrix) = Matrix::trans(-_eye[0], -_eye[1], -_eye[2]) * (*matrix);
                        
            if (_modelToEyeTransform.valid())
            {
                _modelViewMatrix = new Matrix;
                (*_modelViewMatrix) = (*matrix) * (*_modelToEyeTransform);
            }
            else
            {
                _modelViewMatrix = matrix;
            }
            
        }
        break;
    }

    _clippingVolume.clear();

    // set the clipping volume.
    switch(_projectionType)
    {
        case(ORTHO):
        case(ORTHO2D):
            {
            }
            break;
        case(FRUSTUM):
        case(PERSPECTIVE):
            {
                // calculate the frustum normals, postive pointing inwards.
                // left clipping plane
                // note, _left,_right,_top and _bottom are already devided
                // by _zNear so no need to take into account for normal
                // calculations.
                Vec3 leftNormal  (1.0f,0.0f,_left);
                leftNormal.normalize();
                _clippingVolume.add(Plane(leftNormal,0.0f));
                
                
                Vec3 rightNormal (-1.0f,0.0f,-_right);
                rightNormal.normalize();
                _clippingVolume.add(Plane(rightNormal,0.0f));
                
                Vec3 bottomNormal(0.0f,1.0f,_bottom); 
                bottomNormal.normalize();
                _clippingVolume.add(Plane(bottomNormal,0.0f));
                
                Vec3 topNormal(0.0f,-1.0f,-_top);
                topNormal.normalize();
                _clippingVolume.add(Plane(topNormal,0.0f));
                
                if (_useNearClippingPlane)
                {
                    _clippingVolume.add(Plane(0.0f,0.0f,-1.0f,-_zNear));
                }
            
                if (_useFarClippingPlane)
                {
                    _clippingVolume.add(Plane(0.0f,0.0f,1.0f,_zFar));
                }

            }
            break;

    }

    _clippingVolume.transformProvidingInverse(*_modelViewMatrix);

    if (!_MP.valid()) _MP = new Matrix;
    _MP->mult(*_modelViewMatrix,*_projectionMatrix);
    
    if (!_inverseMP.valid()) _inverseMP = new Matrix;
    _inverseMP->invert(*_MP);

    _dirty = false;
}

void Camera::ensureOrthogonalUpVector()
{
    Vec3 lv = _center-_eye;
    Vec3 sv = lv^_up;
    _up = sv^lv;
    _up.normalize();
}

const bool Camera::project(const Vec3& obj,const Viewport& viewport,Vec3& win) const
{
    if (_MP.valid())
    {
        Vec3 v = obj * (*_MP);
        
        win.set(
                (float)viewport.x() + (float)viewport.width()*(v[0]+1.0f)*0.5f,
                (float)viewport.y() + (float)viewport.height()*(v[1]+1.0f)*0.5f,
                (v[2]+1.0f)*0.5f
               );
        
        return true;
    }
    else
        return false;
}

const bool Camera::unproject(const Vec3& win,const Viewport& viewport,Vec3& obj) const
{
    if (_inverseMP.valid())
    {
        Vec3 v(
                2.0f*(win[0]-(float)viewport.x())/viewport.width() - 1.0f,
                2.0f*(win[1]-(float)viewport.y())/viewport.height() - 1.0f,
                2.0f*(win[2]) - 1.0f
               );
               
        obj = v * (*_inverseMP);
        
        return true;
    }
    else
        return false;
}
