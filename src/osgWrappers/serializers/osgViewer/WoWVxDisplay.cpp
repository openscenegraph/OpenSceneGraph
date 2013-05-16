#include <osgViewer/config/WoWVxDisplay>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_WoWVxDisplay,
                         new osgViewer::WoWVxDisplay,
                         osgViewer::WoWVxDisplay,
                         "osg::Object osgViewer::ViewConfig osgViewer::WoWVxDisplay" )
{
    ADD_UINT_SERIALIZER(ScreenNum, 0u);
#if 0
    ADD_UCHAR_SERIALIZER(Content, 0x02);
    ADD_UCHAR_SERIALIZER(Factor, 0x40);
    ADD_UCHAR_SERIALIZER(Offset, 0x80);
#endif

    ADD_FLOAT_SERIALIZER(DisparityZD, 0.459813f);
    ADD_FLOAT_SERIALIZER(DisparityVZ, 6.180772f);
    ADD_FLOAT_SERIALIZER(DisparityM, -1586.34f);
    ADD_FLOAT_SERIALIZER(DisparityC, 127.5f);
}
