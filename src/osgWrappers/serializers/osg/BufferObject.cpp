#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>



////////////////////////////////////////
///         BufferData
////////////////////////////////////////
namespace BufferDataWrapper
{
class HackedBufferData:public osg::BufferData
{

public:
    void setBufferObjectWithoutAddingBD2BO( osg::BufferObject*bufferObject)
    {
        if (_bufferObject==bufferObject) return;

        _bufferObject = bufferObject;
    }
};
static bool checkBufferObject( const osg::BufferData& node )
{
    return true;
}

static bool readBufferObject( osgDB::InputStream& is, osg::BufferData& node1 )
{
    unsigned int size = 0;
    HackedBufferData&node  =static_cast<HackedBufferData&>(node1);

    osg::ref_ptr<osg::Object> obj = is.readObject();
    osg::BufferObject* bo = dynamic_cast<osg::BufferObject*>( obj.get() );
    if ( bo ) node.setBufferObjectWithoutAddingBD2BO(bo);///don't add BufferData to BufferObject (let Serializer do it)
    return true;
}

static bool writeBufferObject( osgDB::OutputStream& os, const osg::BufferData& node )
{
    os << node.getBufferObject();
    return true;
}

REGISTER_OBJECT_WRAPPER( BufferData,
                         0,
                         osg::BufferData,
                         "osg::Object osg::BufferData" )
{
    ADD_USER_SERIALIZER(BufferObject);
    ADD_UINT_SERIALIZER(BufferIndex,0);
}
}

////////////////////////////////////////
///         BufferObject
////////////////////////////////////////

namespace BufferObjectWrapper
{
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


REGISTER_OBJECT_WRAPPER( BufferObject,
                         /*new osg::BufferObject*/NULL,
                         osg::BufferObject,
                         "osg::Object osg::BufferObject" )
{
    ADD_GLENUM_SERIALIZER( Target,GLenum, GL_ARRAY_BUFFER_ARB);  // _type
    ADD_GLENUM_SERIALIZER( Usage,GLenum, GL_STATIC_DRAW_ARB);  // _type   setTarget(GL_ARRAY_BUFFER_ARB);
    ADD_BOOL_SERIALIZER(CopyDataAndReleaseGLBufferObject,false);
    ADD_USER_SERIALIZER( BufferData );  // _BufferData
}
}

namespace VertexBufferObjectWrapper
{
REGISTER_OBJECT_WRAPPER( VertexBufferObject,
                         new osg::VertexBufferObject,
                         osg::VertexBufferObject,
                         "osg::Object osg::BufferObject osg::VertexBufferObject" ) {    }
}
namespace ElementBufferObjectWrapper
{
REGISTER_OBJECT_WRAPPER( ElementBufferObject,
                         new osg::ElementBufferObject,
                         osg::ElementBufferObject,
                         "osg::Object osg::BufferObject osg::ElementBufferObject" ) {    }
}
