#include <osgViewer/config/SingleWindow>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_SingleWindow,
                         new osgViewer::SingleWindow,
                         osgViewer::SingleWindow,
                         "osg::Object osgViewer::ViewConfig osgViewer::SingleWindow" )
{
    ADD_INT_SERIALIZER( X, 0);
    ADD_INT_SERIALIZER( Y, 0);
    ADD_INT_SERIALIZER( Width, -1);
    ADD_INT_SERIALIZER( Height, -1);
    ADD_UINT_SERIALIZER( ScreenNum, 0u);
    ADD_BOOL_SERIALIZER( WindowDecoration, true);
    ADD_BOOL_SERIALIZER( OverrideRedirect, false);
}
