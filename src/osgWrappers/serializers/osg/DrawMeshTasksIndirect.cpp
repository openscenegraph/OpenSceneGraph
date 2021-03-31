#include <osg/DrawMeshTasksIndirect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( DrawMeshTasksIndirect,
                         new osg::DrawMeshTasksIndirect,
                         osg::DrawMeshTasksIndirect,
                         "osg::Object osg::Node osg::Drawable osg::DrawMeshTasksIndirect" )
{
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLintptr >( \
        "Offset", 0, &MyClass::getOffset, &MyClass::setOffset), osgDB::BaseSerializer::RW_INT );
}
