#include <osg/MultiDrawMeshTasksIndirect>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( MultiDrawMeshTasksIndirect,
                         new osg::MultiDrawMeshTasksIndirect,
                         osg::MultiDrawMeshTasksIndirect,
                         "osg::Object osg::Node osg::Drawable osg::MultiDrawMeshTasksIndirect" )
{
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLintptr >( \
        "Offset", 0, &MyClass::getOffset, &MyClass::setOffset), osgDB::BaseSerializer::RW_INT );

    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLsizei >( \
        "DrawCount", 0, &MyClass::getDrawCount, &MyClass::setDrawCount), osgDB::BaseSerializer::RW_INT );

    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLsizei>( \
        "Stride", 0, &MyClass::getStride, &MyClass::setStride), osgDB::BaseSerializer::RW_INT );
}
