#include <osg/Shader>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkShaderSource( const osg::Shader& shader )
{
    return !shader.getShaderSource().empty();
}

static bool readShaderSource( osgDB::InputStream& is, osg::Shader& shader )
{
    std::string code;
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string line;
        is.readWrappedString( line );
        code.append( line ); code.append( 1, '\n' );
    }
    is >> is.END_BRACKET;
    shader.setShaderSource( code );
    return true;
}

static bool writeShaderSource( osgDB::OutputStream& os, const osg::Shader& shader )
{
    std::vector<std::string> lines;
    std::istringstream iss( shader.getShaderSource() );
    std::string line;
    while ( std::getline(iss, line) )
    {
        lines.push_back( line );
    }

    os.writeSize(lines.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( std::vector<std::string>::const_iterator itr=lines.begin();
          itr!=lines.end(); ++itr )
    {
        os.writeWrappedString( *itr );
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Shader,
                         new osg::Shader,
                         osg::Shader,
                         "osg::Object osg::Shader" )
{
    BEGIN_ENUM_SERIALIZER3( Type, UNDEFINED );
        ADD_ENUM_VALUE( VERTEX );
        ADD_ENUM_VALUE( TESSCONTROL );
        ADD_ENUM_VALUE( TESSEVALUATION );
        ADD_ENUM_VALUE( FRAGMENT );
        ADD_ENUM_VALUE( GEOMETRY );
        ADD_ENUM_VALUE( COMPUTE );
        ADD_ENUM_VALUE( UNDEFINED );
    END_ENUM_SERIALIZER();  // _type

    ADD_USER_SERIALIZER( ShaderSource );  // _shaderSource
    ADD_OBJECT_SERIALIZER( ShaderBinary, osg::ShaderBinary, NULL );  // _shaderBinary
}
