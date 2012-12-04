#ifndef CAMERAPROPERTY_H
#define CAMERAPROPERTY_H

#include <osg/AnimationPath>

#include "UpdateProperty.h"

namespace gsc
{

class CameraProperty : public gsc::UpdateProperty
{
public:

    CameraProperty():
        _center(0.0,0.0,0.0),
        _eye(0.0,-1.0,0.0),
        _up(0.0,0.0,1.0),
        _rotationCenter(0.0,0.0,0.0),
        _rotationAxis(0.0,0.0,1.0),
        _rotationSpeed(0.0) {}
        
    CameraProperty(const CameraProperty& cp, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        _center(cp._center),
        _eye(cp._eye),
        _up(cp._up),
        _rotationCenter(cp._rotationCenter),
        _rotationAxis(cp._rotationAxis),
        _rotationSpeed(cp._rotationSpeed)
        {}
 
    META_Object(gsc, CameraProperty);

    void setToModel(const osg::Node* node);

    void setCenter(const osg::Vec3d& center) { _center = center; }
    const osg::Vec3d& getCenter() const { return _center; }

    void setEyePoint(const osg::Vec3d& eye) { _eye = eye; }
    const osg::Vec3d& getEyePoint() const { return _eye; }
    
    void setUpVector(const osg::Vec3d& up) { _up = up; }
    const osg::Vec3d& getUpVector() const { return _up; }

    void setRotationCenter(const osg::Vec3d& center) { _rotationCenter = center; }
    const osg::Vec3d& getRotationCenter() const { return _rotationCenter; }

    void setRotationAxis(const osg::Vec3d& axis) { _rotationAxis = axis; }
    const osg::Vec3d& getRotationAxis() const { return _rotationAxis; }

    void setRotationSpeed(double speed) { _rotationSpeed = speed; }
    double getRotationSpeed() const { return _rotationSpeed; }

    virtual void update(osgViewer::View* view);

protected:

    virtual ~CameraProperty() {}


    osg::Vec3d  _center;
    osg::Vec3d  _eye;
    osg::Vec3d  _up;
    osg::Vec3d  _rotationCenter;
    osg::Vec3d  _rotationAxis;
    double      _rotationSpeed;

};

}

#endif