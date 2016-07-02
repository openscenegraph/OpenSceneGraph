#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgAnimation/MorphGeometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgAnimation_UpdateMorph,
                         new osgAnimation::UpdateMorph,
                         osgAnimation::UpdateMorph,
                         "osg::Object osg::Callback osg::NodeCallback osgAnimation::UpdateMorph" )
{
    ADD_VECTOR_SERIALIZER( TargetNames, osgAnimation::UpdateMorph::TargetNames, osgDB::BaseSerializer::RW_STRING, 1 );
}

#undef OBJECT_CAST
#define OBJECT_CAST static_cast
