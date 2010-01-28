#include <osg/TransferFunction>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TransferFunction,
                         new osg::TransferFunction,
                         osg::TransferFunction,
                         "osg::Object osg::TransferFunction" )
{
}
