#include <osg/PatchParameter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( PatchParameter,
                         new osg::PatchParameter,
                         osg::PatchParameter,
                         "osg::Object osg::StateAttribute osg::PatchParameter" )
{
    ADD_INT_SERIALIZER( Vertices, 3);
    ADD_VEC2_SERIALIZER( PatchDefaultInnerLevel, osg::Vec2(1.0f,1.0f));
    ADD_VEC4_SERIALIZER( PatchDefaultOuterLevel, osg::Vec4(1.0f,1.0f,1.0f,1.0f));
}
