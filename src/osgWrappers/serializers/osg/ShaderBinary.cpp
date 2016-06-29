#include <osg/Shader>
#include <osgDB/ObjectWrapper>
#include <osgDB/Serializer>

static bool checkData( const osg::ShaderBinary& sb )
{
    return sb.getSize()>0;
}

static bool readData( osgDB::InputStream& is, osg::ShaderBinary& sb )
{
    unsigned int size; is >> size;
    char* data = new char[size]();
    if ( is.isBinary() )
    {
        is.readCharArray( data, size );
    }
    else
    {
        is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            is >> std::hex >> data[i] >> std::dec;
        }
        is >> is.END_BRACKET;
    }

    if (size>0)
    {
        sb.assign( size, (unsigned char*)data );
    }

    delete [] data;
    return true;
}

static bool writeData( osgDB::OutputStream& os, const osg::ShaderBinary& sb )
{
    if ( os.isBinary() )
    {
        os << (unsigned int)sb.getSize();
        os.writeCharArray( (char*)sb.getData(), sb.getSize() );
    }
    else
    {
        const unsigned char* data = sb.getData();
        os << (unsigned int)sb.getSize();
        os << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<sb.getSize(); ++i )
        {
            os << std::hex << data[i] << std::dec << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    return true;
}

REGISTER_OBJECT_WRAPPER( ShaderBinary,
                         new osg::ShaderBinary,
                         osg::ShaderBinary,
                         "osg::Object osg::ShaderBinary" )
{
    ADD_USER_SERIALIZER( Data );  // _data
}
