#include <osg/BufferObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


class BufferData_serializer_BufferData:public osg::BufferData
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

///don't add BufferData to BufferObject (let BufferObject Serializer::readBufferData do it)
static bool readBufferObject( osgDB::InputStream& is, osg::BufferData& bd )
{
    BufferData_serializer_BufferData &localbd  =static_cast<BufferData_serializer_BufferData&>(bd);
    osg::ref_ptr<osg::Object> obj = is.readObject();
    osg::BufferObject* bo = dynamic_cast<osg::BufferObject*>( obj.get() );
    if ( bo ) localbd.setBufferObjectWithoutAddingBD2BO(bo);
    return true;
}

static bool writeBufferObject( osgDB::OutputStream& os, const osg::BufferData& bd )
{
    if (os.getWriteBufferObjectConfiguration())
        os << bd.getBufferObject();
    else os << (osg::BufferObject*)NULL;
    return true;
}
static bool checkBufferIndex( const osg::BufferData& bd )
{
    return true;
}

static bool readBufferIndex( osgDB::InputStream& is, osg::BufferData& bd )
{
    unsigned int index;
    is >> index;
    bd.setBufferIndex(index);
    return true;
}

static bool writeBufferIndex( osgDB::OutputStream& os, const osg::BufferData& bd )
{
    if (os.getWriteBufferObjectConfiguration())
        os << bd.getBufferIndex();
    else os << 0;
    return true;
}

REGISTER_OBJECT_WRAPPER( BufferData,
                         0,
                         osg::BufferData,
                         "osg::Object osg::BufferData" )
{
    {
        UPDATE_TO_VERSION_SCOPED( 147 )
        ADD_USER_SERIALIZER(BufferObject);
        ADD_USER_SERIALIZER(BufferIndex);
    }
}
