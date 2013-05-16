#include <osgViewer/config/WoWVxDisplay>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_WoWVxDisplay,
                         new osgViewer::WoWVxDisplay,
                         osgViewer::WoWVxDisplay,
                         "osg::Object osgViewer::Config osgViewer::WoWVxDisplay" )
{
    ADD_UINT_SERIALIZER(ScreenNum, 0u);
    
    ADD_UCHAR_SERIALIZER(Content, 0);
    ADD_UCHAR_SERIALIZER(Factor, 0);
    ADD_UCHAR_SERIALIZER(Offset, 0);
    
    ADD_FLOAT_SERIALIZER(DisparityZD, 0);
    ADD_FLOAT_SERIALIZER(DisparityVZ, 0);
    ADD_FLOAT_SERIALIZER(DisparityM, 0);
    ADD_FLOAT_SERIALIZER(DisparityC, 0);
}
