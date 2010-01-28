#include <osg/Hint>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Hint,
                         new osg::Hint,
                         osg::Hint,
                         "osg::Object osg::StateAttribute osg::Hint" )
{
    ADD_GLENUM_SERIALIZER( Target, GLenum, GL_NONE );  // _target
    ADD_GLENUM_SERIALIZER( Mode, GLenum, GL_DONT_CARE );  // _mode
}
