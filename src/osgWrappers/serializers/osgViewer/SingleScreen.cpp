#include <osgViewer/config/SingleScreen>

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgViewer_SingleScreen,
                         new osgViewer::SingleScreen,
                         osgViewer::SingleScreen,
                         "osg::Object osgViewer::ViewConfig osgViewer::SingleScreen" )
{
    ADD_UINT_SERIALIZER( ScreenNum, 0u);
}
