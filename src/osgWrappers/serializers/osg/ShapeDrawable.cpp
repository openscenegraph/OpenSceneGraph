#include <osg/ShapeDrawable>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( ShapeDrawable,
                         new osg::ShapeDrawable,
                         osg::ShapeDrawable,
                         "osg::Object osg::Drawable osg::ShapeDrawable" )
{
    ADD_VEC4_SERIALIZER( Color, osg::Vec4() );  // _color
    ADD_OBJECT_SERIALIZER( TessellationHints, osg::TessellationHints, NULL );  // _tessellationHints
}
