#include <osgGA/AnimationPathManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_AnimationPathManipulator,
                         new osgGA::AnimationPathManipulator,
                         osgGA::AnimationPathManipulator,
                         "osg::Object osgGA::AnimationPathManipulator" )
{
}
