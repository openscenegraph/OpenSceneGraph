#include <osg/Texture2DMultisample>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Texture2DMultisample,
                         new osg::Texture2DMultisample,
                         osg::Texture2DMultisample,
                         "osg::Object osg::StateAttribute osg::Texture osg::Texture2DMultisample" )
{
    ADD_INT_SERIALIZER( TextureWidth, 0 );                  // _textureWidth
    ADD_INT_SERIALIZER( TextureHeight, 0 );                 // _textureHeight
    ADD_INT_SERIALIZER( NumSamples, 1 );                    // _numSamples
    ADD_UCHAR_SERIALIZER( FixedSampleLocations, GL_FALSE ); // _fixedsamplelocations
}
