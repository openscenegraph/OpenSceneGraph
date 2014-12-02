#include <osg/BlendFunci>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( BlendFunci,
                         new osg::BlendFunci,
                         osg::BlendFunci,
                         "osg::Object osg::StateAttribute osg::BlendFunc osg::BlendFunci" )
{
    ADD_UINT_SERIALIZER( Index, 0 );
}
