#include <osg/CoordinateSystemNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( EllipsoidModel,
                         new osg::EllipsoidModel,
                         osg::EllipsoidModel,
                         "osg::Object osg::EllipsoidModel" )
{
    ADD_DOUBLE_SERIALIZER( RadiusEquator, 0.0 );  // _radiusEquator
    ADD_DOUBLE_SERIALIZER( RadiusPolar, 0.0 );  // _radiusPolar
}
