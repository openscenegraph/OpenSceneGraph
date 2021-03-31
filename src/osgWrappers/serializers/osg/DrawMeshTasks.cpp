#include <osg/DrawMeshTasks>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( DrawMeshTasks,
                         new osg::DrawMeshTasks,
                         osg::DrawMeshTasks,
                         "osg::Object osg::Node osg::Drawable osg::DrawMeshTasks" )
{
    ADD_UINT_SERIALIZER( First, 0 );
    ADD_UINT_SERIALIZER( Count, 0 );
}
