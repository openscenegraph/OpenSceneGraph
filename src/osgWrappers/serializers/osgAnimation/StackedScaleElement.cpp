#include <osgAnimation/StackedScaleElement>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_StackedScaleElement,
                         new osgAnimation::StackedScaleElement,
                         osgAnimation::StackedScaleElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedScaleElement" )
{
    ADD_VEC3_SERIALIZER( Scale, osg::Vec3() );  // _scale
}
