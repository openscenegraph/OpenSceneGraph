#ifndef CAMERAPATHPROPERTY_H
#define CAMERAPATHPROPERTY_H

#include <osg/AnimationPath>

#include "UpdateProperty.h"

namespace gsc
{

class CameraPathProperty : public gsc::UpdateProperty
{
public:

    CameraPathProperty() {}
    CameraPathProperty(const std::string& filename) { setAnimationPathFileName(filename); }
    CameraPathProperty(const CameraPathProperty& cpp, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}
 
    META_Object(gsc, CameraPathProperty);

    void setAnimationPathFileName(const std::string& filename) { _filename = filename; loadAnimationPath(); }
    const std::string& getAnimationPathFileName() const { return _filename; }

    void setAnimationPath(osg::AnimationPath* ap) { _animationPath = ap; }
    osg::AnimationPath* getAnimationPath() { return _animationPath.get(); }
    const osg::AnimationPath* getAnimationPath() const { return _animationPath.get(); }

    bool getTimeRange(double& startTime, double& endTime) const;

    void resetTimeRange(double startTime, double endTime);

    virtual void update(osgViewer::View* view);

protected:

    virtual ~CameraPathProperty() {}

    void loadAnimationPath();

    std::string                         _filename;
    osg::ref_ptr<osg::AnimationPath>    _animationPath;
};

}

#endif