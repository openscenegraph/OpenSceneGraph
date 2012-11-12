#ifndef UPDATEPROPERTY_H
#define UPDATEPROPERTY_H

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osg/AnimationPath>

namespace gsc
{

class UpdateProperty : public osg::Object
{
public:

    UpdateProperty() {}
    UpdateProperty(const UpdateProperty& up, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}

    META_Object(gsc, UpdateProperty);

    virtual void update(osgViewer::View* view) {}

protected:

    virtual ~UpdateProperty() {}
};


}

#endif