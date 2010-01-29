#include <osgText/FadeText>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgText_FadeText,
                         new osgText::FadeText,
                         osgText::FadeText,
                         "osg::Object osg::Drawable osgText::TextBase osgText::Text osgText::FadeText" )
{
    ADD_FLOAT_SERIALIZER( FadeSpeed, 0.0f );  // _fadeSpeed
}
