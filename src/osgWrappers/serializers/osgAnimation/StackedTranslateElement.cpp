#include <osgAnimation/StackedTranslateElement>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_StackedTranslateElement,
                         new osgAnimation::StackedTranslateElement,
                         osgAnimation::StackedTranslateElement,
                         "osg::Object osgAnimation::StackedTransformElement osgAnimation::StackedTranslateElement" )
{
    ADD_VEC3_SERIALIZER( Translate, osg::Vec3() );  // _translate
}
