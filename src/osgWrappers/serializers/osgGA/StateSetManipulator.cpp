#include <osgGA/StateSetManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_StateSetManipulator,
                         new osgGA::StateSetManipulator,
                         osgGA::StateSetManipulator,
                         "osg::Object osgGA::StateSetManipulator" )
{
        ADD_INT_SERIALIZER(KeyEventToggleBackfaceCulling, 'b');
        ADD_INT_SERIALIZER(KeyEventToggleLighting, 'l');
        ADD_INT_SERIALIZER(KeyEventToggleTexturing, 't');
        ADD_INT_SERIALIZER(KeyEventCyclePolygonMode, 'w');
}
