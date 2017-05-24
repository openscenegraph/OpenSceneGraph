#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkBufferData( const osg::BufferObject& node )
{
    return node.getNumBufferData()>0;
}

/// add BufferData to BufferObject (let BufferData Serializer::readBufferObject not in charge of it)
static bool readBufferData( osgDB::InputStream& is, osg::BufferObject& node )
{
    unsigned int size = 0;
    is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> obj = is.readObject();
        osg::BufferData* child = dynamic_cast<osg::BufferData*>( obj.get() );
        if ( child ) node.addBufferData( child );
    }
    is >> is.END_BRACKET;
    node.dirty();
    return true;
}

static bool writeBufferData( osgDB::OutputStream& os, const osg::BufferObject& node )
{
    unsigned int size = node.getNumBufferData();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getBufferData(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( BufferObject,
                         /*new osg::BufferObject*/NULL,
                         osg::BufferObject,
                         "osg::Object osg::BufferObject" )
{
    ADD_GLENUM_SERIALIZER( Target,GLenum, GL_ARRAY_BUFFER_ARB );  // _type
    ADD_GLENUM_SERIALIZER( Usage,GLenum, GL_STATIC_DRAW_ARB );  // _usage
    ADD_BOOL_SERIALIZER( CopyDataAndReleaseGLBufferObject,false );
    ADD_USER_SERIALIZER( BufferData );  // _BufferData
}

