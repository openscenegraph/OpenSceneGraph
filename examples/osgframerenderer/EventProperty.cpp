#include "EventProperty.h"

namespace gsc
{

void EventProperty::update(osgViewer::View* view)
{
    if (view && view->getEventQueue() && _event.valid())
    {
        view->getEventQueue()->addEvent(_event.get());
    }
}

/////////////////////////////////////////////////////////////////////////////////////////
//
// Serialization support
//
REGISTER_OBJECT_WRAPPER( gsc_EventProperty,
                         new gsc::EventProperty,
                         gsc::EventProperty,
                         "osg::Object gsc::EventProperty" )
{
    ADD_OBJECT_SERIALIZER( Event, osgGA::GUIEventAdapter, NULL );
}


}

namespace osgGA
{

    

    
namespace B
{


}
 
}
