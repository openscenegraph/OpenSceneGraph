#include <osgViewer/config/PanoramicSphericalDisplay>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_PanoramicSphericalDisplay,
                         new osgViewer::PanoramicSphericalDisplay,
                         osgViewer::PanoramicSphericalDisplay,
                         "osg::Object osgViewer::ViewConfig osgViewer::PanoramicSphericalDisplay" )
{
    ADD_DOUBLE_SERIALIZER(Radius, 1.0);
    ADD_DOUBLE_SERIALIZER(Collar, 0.45);
    ADD_UINT_SERIALIZER(ScreenNum, 0u);
    ADD_IMAGE_SERIALIZER(IntensityMap, osg::Image, NULL);
    ADD_MATRIXD_SERIALIZER(ProjectionMatrix, osg::Matrixd() );
}
