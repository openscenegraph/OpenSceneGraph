#include <osg/StateAttribute>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( StateAttribute,
                         /*new osg::StateAttribute*/NULL,
                         osg::StateAttribute,
                         "osg::Object osg::StateAttribute" )
{
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::StateAttributeCallback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::StateAttributeCallback, NULL );  // _eventCallback
}
