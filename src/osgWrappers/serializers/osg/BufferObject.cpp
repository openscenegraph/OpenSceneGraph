#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>




namespace BufferDataWrapper
{

REGISTER_OBJECT_WRAPPER( BufferData,
                         0,
                         osg::BufferData,
                         "osg::Object osg::BufferData" )
{

    ADD_OBJECT_SERIALIZER(BufferObject,osg::BufferObject,NULL);
}
}
/*
static bool checkArrays( const osg::VertexBufferObject& node )
{
    return node.getNumBufferData()//Arrays();
           >0;
}

static bool readArrays( osgDB::InputStream& is, osg::VertexBufferObject& node )
{
    unsigned int size = 0;
    is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> obj = is.readObject();
        osg::Array* child = dynamic_cast<osg::Array*>( obj.get() );
        if ( child ) node.addArray( child );
    }
    is >> is.END_BRACKET;
    node.dirty();
    return true;
}

static bool writeArrays( osgDB::OutputStream& os, const osg::VertexBufferObject& node )
{
    unsigned int size = node.getNumBufferData();//Arrays();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getArray(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}
*/
namespace VertexBufferObjectWrapper
{
REGISTER_OBJECT_WRAPPER( VertexBufferObject,
                         new osg::VertexBufferObject,
                         osg::VertexBufferObject,
                         "osg::Object osg::BufferObject osg::VertexBufferObject" )
{

    // ADD_USER_SERIALIZER( Arrays );  // _BufferData
}

}




static bool checkBufferData( const osg::BufferObject& node )
{
    return node.getNumBufferData()>0;
}

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


namespace BufferObjectWrapper
{

REGISTER_OBJECT_WRAPPER( BufferObject,
                         /*new osg::BufferObject*/NULL,
                         osg::BufferObject,
                         "osg::Object osg::BufferObject" )
{

    //ADD_OBJECT_SERIALIZER( BufferObject, osg::BufferObject, NULL );  // _bufferObject
    //  ADD_UINT_SERIALIZER( Index, 0);  // _index
    ADD_GLENUM_SERIALIZER( Target,GLenum, GL_ARRAY_BUFFER_ARB);  // _type
    ADD_GLENUM_SERIALIZER( Usage,GLenum, GL_STATIC_DRAW_ARB);  // _type   setTarget(GL_ARRAY_BUFFER_ARB);
    //setUsage(GL_STATIC_DRAW_ARB);
    ADD_BOOL_SERIALIZER(CopyDataAndReleaseGLBufferObject,false);
    ADD_USER_SERIALIZER( BufferData );  // _BufferData
}

}

