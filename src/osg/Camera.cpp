#include "osg/GL"
#include <osg/Camera>

using namespace osg;

Camera::Camera()
{
    _fovy = 60.0;
    _aspectRatio = 1.0;
    home();
}

Camera::~Camera()
{
}

void Camera::home()
{
    _eyePoint.set(0.0f,0.0f,0.0f);
    _lookPoint.set(0.0f,0.0f,-1.0f);
    _upVector.set(0.0f,1.0f,0.0f);
    _nearPlane = 1.0;
    _farPlane = 1000.0;
}

void Camera::setView(osg::Vec3 eyePoint, osg::Vec3 lookPoint, osg::Vec3 upVector)
{
    // Should do some checking here!
    _eyePoint = eyePoint;
    _lookPoint = lookPoint;
    _upVector = upVector;
}

void Camera::draw_PROJECTION() const
{
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    gluPerspective(_fovy, _aspectRatio, static_cast<GLdouble>(_nearPlane), static_cast<GLdouble>(_farPlane));

    glMatrixMode( GL_MODELVIEW );
}

void Camera::draw_MODELVIEW() const
{
    glMatrixMode( GL_MODELVIEW );
    gluLookAt( _eyePoint.x(),  _eyePoint.y(),  _eyePoint.z(),
	       _lookPoint.x(), _lookPoint.y(), _lookPoint.z(), 
	       _upVector.x(),  _upVector.y(),  _upVector.z());

}

void Camera::ensureOrthogonalUpVector()
{
    Vec3 lv = _lookPoint-_eyePoint;
    Vec3 sv = lv^_upVector;
    _upVector = sv^lv;
    _upVector.normalize();
}

void Camera::mult(const Camera& camera,const Matrix& m)
{
    // transform camera.
    _upVector = (camera._lookPoint+camera._upVector)*m;
    _eyePoint = camera._eyePoint*m;
    _lookPoint = camera._lookPoint*m;
    _upVector -= _lookPoint;

    // now reset up vector so it remains at 90 degrees to look vector,
    // as it may drift during transformation.
    ensureOrthogonalUpVector();
}

void Camera::mult(const Matrix& m,const Camera& camera)
{
    // transform camera.
    _upVector = m*(camera._lookPoint+camera._upVector);
    _eyePoint = m*camera._eyePoint;
    _lookPoint = m*camera._lookPoint;
    _upVector -= _lookPoint;

    // now reset up vector so it remains at 90 degrees to look vector,
    // as it may drift during transformation.
    ensureOrthogonalUpVector();
}

