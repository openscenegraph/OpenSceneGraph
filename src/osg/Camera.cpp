#include <osg/GL>
#include <osg/Camera>
#include <osg/Types>
#include <osg/Notify>
#include <osg/State>

using namespace osg;

Camera::Camera(DisplaySettings* ds)
{

    _adjustAspectRatioMode = ADJUST_HORIZONTAL;

    // projection details.
    
    float fovy = 45.0f;
    if (ds)
    {
        fovy = 2.0f*RadiansToDegrees(atan(ds->getScreenHeight()*0.5/ds->getScreenDistance()));
    }
        
    setPerspective(fovy,1.0,1.0,1000.0);
    
    // look at details.
    _lookAtType =USE_HOME_POSITON;

    _eye.set(0.0f,0.0f,0.0f);
    _center.set(0.0f,0.0f,-1.0f);
    _up.set(0.0f,1.0f,0.0f);

    _attachedTransformMode = NO_ATTACHED_TRANSFORM;

    if (ds) _screenDistance = ds->getScreenDistance();
    else _screenDistance = 0.33f;

    _fusionDistanceMode = PROPORTIONAL_TO_LOOK_DISTANCE;
    _fusionDistanceRatio = 1.0f;
}

Camera::Camera(const Camera& camera):Referenced()
{
    copy(camera);
}

Camera& Camera::operator=(const Camera& camera)
{
    if (&camera==this) return *this;

    copy(camera);

    return *this;
}

void Camera::copy(const Camera& camera)
{
    _projectionType = camera._projectionType;

    // how the window dimensions should be altered during a window resize.
    _adjustAspectRatioMode = camera._adjustAspectRatioMode;

    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.
    _left = camera._left;
    _right = camera._right;
    _bottom = camera._bottom;
    _top = camera._top;

    _zNear = camera._zNear;
    _zFar = camera._zFar;


    // look at details.
    _lookAtType = camera._lookAtType;

    _eye = camera._eye;
    _center = camera._center;
    _up = camera._up;

    _attachedTransformMode = camera._attachedTransformMode;
    _eyeToModelTransform = camera._eyeToModelTransform;
    _modelToEyeTransform = camera._modelToEyeTransform;

    _screenDistance = camera._screenDistance;
    _fusionDistanceMode = camera._fusionDistanceMode;
    _fusionDistanceRatio = camera._fusionDistanceRatio;

}

Camera::~Camera()
{
}



/** Set a orthographics projection. See glOrtho for further details.*/
void Camera::setOrtho(double left, double right,
                      double bottom, double top,
                      double zNear, double zFar)
{
    _projectionType = ORTHO;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _zNear = zNear;
    _zFar = zFar;
}


/** Set a 2D orthographics projection. See gluOrtho2D for further details.*/
void Camera::setOrtho2D(double left, double right,
                        double bottom, double top)
{
    _projectionType = ORTHO2D;
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _zNear = -1.0;
    _zFar = 1.0;
}


/** Set a perspective projection. See glFrustum for further details.*/
void Camera::setFrustum(double left, double right,
                        double bottom, double top,
                        double zNear, double zFar)
{
    _projectionType = FRUSTUM;
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.
    _left = left;
    _right = right;
    _bottom = bottom;
    _top = top;
    _zNear = zNear;
    _zFar = zFar;
}



/** Set a sysmetical perspective projection, See gluPerspective for further details.*/
void Camera::setPerspective(double fovy,double aspectRatio,
                            double zNear, double zFar)
{
    _projectionType = PERSPECTIVE;
    
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.

    // calculate the appropriate left, right etc.
    double tan_fovy = tan(DegreesToRadians(fovy*0.5));
    _right  =  tan_fovy * aspectRatio * zNear;
    _left   = -_right;
    _top    =  tan_fovy * zNear;
    _bottom =  -_top;

    _zNear = zNear;
    _zFar = zFar;
}

/** Set a sysmetical perspective projection using field of view.*/
void Camera::setFOV(double fovx,double fovy,
                    double zNear, double zFar)
{
    _projectionType = PERSPECTIVE;
    
    // note, in Frustum/Perspective mode these values are scaled
    // by the zNear from when they were initialised to ensure that
    // subsequent changes in zNear do not affect them.

    // calculate the appropriate left, right etc.
    double tan_fovx = tan(DegreesToRadians(fovx*0.5));
    double tan_fovy = tan(DegreesToRadians(fovy*0.5));
    _right  =  tan_fovx * zNear;
    _left   = -_right;
    _top    =  tan_fovy * zNear;
    _bottom =  -_top;

    _zNear = zNear;
    _zFar = zFar;
}

/** Set the near and far clipping planes.*/
void Camera::setNearFar(double zNear, double zFar)
{
    if (_projectionType==FRUSTUM || _projectionType==PERSPECTIVE)
    {
        double adjustRatio = zNear/_zNear;

        _left *= adjustRatio;
        _right *= adjustRatio;
        _bottom *= adjustRatio;
        _top *= adjustRatio;
    }
        
    _zNear = zNear;
    _zFar = zFar;

    if (_projectionType==ORTHO2D)
    {
        if (_zNear!=-1.0 || _zFar!=1.0) _projectionType = ORTHO;
    }
}

/** Adjust the clipping planes to account for a new window aspcect ratio.
  * Typicall used after resizeing a window.*/
void Camera::adjustAspectRatio(double newAspectRatio, AdjustAspectRatioMode aa)
{
    if (newAspectRatio<0.01f || newAspectRatio>100.0f)
    {
        notify(NOTICE)<<"Warning: aspect ratio out of range (0.01..100) in Camera::adjustAspectRatio("<<newAspectRatio<<","<<aa<<")"<<std::endl;
        return;
    }

    if(aa != ADJUST_NONE)  // If adjustment todo
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
    }
}

/** Calculate and return the equivilant fovx for the current project setting.
  * This value is only valid for when a symetric persepctive projection exists.
  * i.e. getProjectionType()==PERSPECTIVE.*/
double Camera::calc_fovy() const
{
     // note, _right & _left are prescaled by znear so 
     // no need to account for it.
    return RadiansToDegrees(atan(_top/_zNear)-atan(_bottom/_zNear));
}


/** Calculate and return the equivilant fovy for the current project setting.
  * This value is only valid for when a symetric persepctive projection exists.
  * i.e. getProjectionType()==PERSPECTIVE.*/
double Camera::calc_fovx() const
{
     // note, _right & _left are prescaled by znear so 
     // no need to account for it.
    return RadiansToDegrees(atan(_right/_zNear)-atan(_left/_zNear));
}


/** Calculate and return the projection aspect ratio.*/
double Camera::calc_aspectRatio() const
{
    double delta_x = _right-_left;
    double delta_y = _top-_bottom;
    return delta_x/delta_y;
}

Matrix Camera::getProjectionMatrix() const
{
    // set up the projection matrix.
    switch(_projectionType)
    {
        case(ORTHO):
        case(ORTHO2D):
            {
                return Matrix::ortho(_left,_right,_bottom,_top,_zNear,_zFar);
            }
            break;
        case(FRUSTUM):
        case(PERSPECTIVE):
            {
                return Matrix::frustum(_left,_right,_bottom,_top,_zNear,_zFar);
            }
            break;

    }

    // shouldn't get here if camera is set up properly.
    // return identity.
    return Matrix();
}

void Camera::home()
{
    // OpenGL default position.
    _lookAtType = USE_HOME_POSITON;
    _eye.set(0.0f,0.0f,0.0f);
    _center.set(0.0f,0.0f,-1.0f);
    _up.set(0.0f,1.0f,0.0f);
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
}


void Camera::setLookAt(double eyeX, double eyeY, double eyeZ,
               double centerX, double centerY, double centerZ,
               double upX, double upY, double upZ)
{
    _lookAtType = USE_EYE_CENTER_AND_UP;
    _eye.set(eyeX,eyeY,eyeZ);
    _center.set(centerX,centerY,centerZ);
    _up.set(upX,upY,upZ);
    
    ensureOrthogonalUpVector();
}


/** post multiple the existing eye point and orientation by matrix.
  * note, does not affect any ModelTransforms that are applied.*/
void Camera::transformLookAt(const Matrix& matrix)
{
    _up = (_up+_eye)*matrix;
    _eye = _eye*matrix;
    _center = _center*matrix;
    _up -= _eye;
    _up.normalize();

    _lookAtType=USE_EYE_CENTER_AND_UP;
}

Vec3 Camera::getLookVector() const
{
    osg::Vec3 lv(_center-_eye);
    lv.normalize();
    return lv;
}

Vec3 Camera::getSideVector() const
{
    osg::Vec3 lv(_center-_eye);
    lv.normalize();
    osg::Vec3 sv(lv^_up);
    sv.normalize();
    return sv;
}


void Camera::attachTransform(TransformMode mode, Matrix* matrix)
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
                if (!_modelToEyeTransform->invert(*_eyeToModelTransform))
                {
                    notify(WARN)<<"Warning: Camera::attachTransform() failed to invert _modelToEyeTransform"<<std::endl;
                }
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
                if (!_eyeToModelTransform->invert(*_modelToEyeTransform))
                {
                    notify(WARN)<<"Warning: Camera::attachTransform() failed to invert _modelToEyeTransform"<<std::endl;
                }
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
        notify(WARN)<<"Warning: invalid TransformMode pass to osg::Camera::attachTransform(..)"<<std::endl;
        notify(WARN)<<"         setting Camera to NO_ATTACHED_TRANSFORM."<<std::endl;
        break;
    }
}

Matrix* Camera::getTransform(TransformMode mode)
{
    switch(mode)
    {
    case(EYE_TO_MODEL): return _eyeToModelTransform.get();
    case(MODEL_TO_EYE): return _modelToEyeTransform.get();
    default: return NULL;
    }
}

const Matrix* Camera::getTransform(TransformMode mode) const
{
    switch(mode)
    {
    case(EYE_TO_MODEL): return _eyeToModelTransform.get();
    case(MODEL_TO_EYE): return _modelToEyeTransform.get();
    default: return NULL;
    }
}


Matrix Camera::getModelViewMatrix() const
{
    Matrix modelViewMatrix;

    // set up the model view matrix.
    switch(_lookAtType)
    {
    case(USE_HOME_POSITON):
        {
            if (_eyeToModelTransform.valid()) 
                modelViewMatrix.invert(*_eyeToModelTransform);
            else if (_modelToEyeTransform.valid())
                modelViewMatrix = *_modelToEyeTransform;
            else
                modelViewMatrix.makeIdentity();
        }
        break;
    case(USE_EYE_AND_QUATERNION): // not implemented yet, default to eye,center,up.
    case(USE_EYE_CENTER_AND_UP):
    default:
        {
            if (_eyeToModelTransform.valid()) 
            {
                modelViewMatrix.invert(*_eyeToModelTransform);
                modelViewMatrix.postMult(Matrix::lookAt(_eye,_center,_up));
            }
            else if (_modelToEyeTransform.valid())
            {
                modelViewMatrix.makeLookAt(_eye,_center,_up);
                modelViewMatrix.preMult(*_modelToEyeTransform);
            }
            else
                modelViewMatrix.makeLookAt(_eye,_center,_up);
            
        }
        break;
    }
    return modelViewMatrix;
}

float Camera::getFusionDistance() const
{
    switch(_fusionDistanceMode)
    {
        case(PROPORTIONAL_TO_SCREEN_DISTANCE): return _screenDistance*_fusionDistanceRatio;
        case(PROPORTIONAL_TO_LOOK_DISTANCE): 
        default: return getLookDistance()*_fusionDistanceRatio;
    }        
}

void Camera::ensureOrthogonalUpVector()
{
    Vec3 lv = _center-_eye;
    Vec3 sv = lv^_up;
    _up = sv^lv;
    _up.normalize();
}
