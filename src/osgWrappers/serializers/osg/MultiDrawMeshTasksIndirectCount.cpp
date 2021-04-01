#include <osg/MultiDrawMeshTasksIndirectCount>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( MultiDrawMeshTasksIndirectCount,
                         new osg::MultiDrawMeshTasksIndirectCount,
                         osg::MultiDrawMeshTasksIndirectCount,
                         "osg::Object osg::Node osg::Drawable osg::MultiDrawMeshTasksIndirectCount" )
{
    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLintptr >( \
        "Offset", 0, &MyClass::getOffset, &MyClass::setOffset), osgDB::BaseSerializer::RW_INT );

    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLintptr  >( \
        "DrawCount", 0, &MyClass::getDrawCount, &MyClass::setDrawCount), osgDB::BaseSerializer::RW_INT );

    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLsizei >( \
        "MaxDrawCount", 0, &MyClass::getMaxDrawCount, &MyClass::setMaxDrawCount), osgDB::BaseSerializer::RW_INT );

    wrapper->addSerializer( new osgDB::PropByValSerializer< MyClass, GLsizei>( \
        "Stride", 0, &MyClass::getStride, &MyClass::setStride), osgDB::BaseSerializer::RW_INT );
}
