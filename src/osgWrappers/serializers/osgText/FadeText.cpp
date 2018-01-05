#include <osgText/FadeText>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgText_FadeText,
                         new osgText::FadeText,
                         osgText::FadeText,
                         "osg::Object osg::Node osg::Drawable osgText::TextBase osgText::Text osgText::FadeText" )
{
    {
         UPDATE_TO_VERSION_SCOPED( 154 )
         ADDED_ASSOCIATE("osg::Node")
    }

    ADD_FLOAT_SERIALIZER( FadeSpeed, 0.0f );  // _fadeSpeed
}
