#ifndef EVENTPROPERTY_H
#define EVENTPROPERTY_H

#include <osgGA/GUIEventAdapter>
#include "UpdateProperty.h"

namespace gsc
{

class EventProperty : public gsc::UpdateProperty
{
public:

    EventProperty() {}
    EventProperty(osgGA::GUIEventAdapter* event):_event(event) {}
    EventProperty(const EventProperty& cpp, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}

    META_Object(gsc, EventProperty);

    void setEvent(osgGA::GUIEventAdapter* ea) { _event = ea; }
    osgGA::GUIEventAdapter* getEvent() { return _event.get(); }
    const osgGA::GUIEventAdapter* getEvent() const { return _event.get(); }
    
    virtual void update(osgViewer::View* view);

protected:

    virtual ~EventProperty() {}

    double                               _previousFrameTime;
    osg::ref_ptr<osgGA::GUIEventAdapter> _event;
};



}

#endif