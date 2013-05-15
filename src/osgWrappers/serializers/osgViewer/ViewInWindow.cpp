#include <osgViewer/Config>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_ViewInWindow,
                         new osgViewer::ViewInWindow,
                         osgViewer::ViewInWindow,
                         "osg::Object osgViewer::Config osgViewer::ViewInWindow" )
{
    ADD_INT_SERIALIZER( X, 0);
    ADD_INT_SERIALIZER( Y, 0);
    ADD_INT_SERIALIZER( Width, -1);
    ADD_INT_SERIALIZER( Height, -1);
    ADD_UINT_SERIALIZER( ScreenNum, 0u);
}
