#include <osg/Program>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#define PROGRAM_LIST_FUNC( PROP, TYPE, DATA ) \
    static bool check##PROP(const osg::Program& attr) \
    { return attr.get##TYPE().size()>0; } \
    static bool read##PROP(osgDB::InputStream& is, osg::Program& attr) { \
        unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET; \
        for ( unsigned int i=0; i<size; ++i ) { \
            std::string key; unsigned int value; \
            is >> key >> value; attr.add##DATA(key, value); \
        } \
        is >> is.END_BRACKET; \
        return true; \
    } \
    static bool write##PROP( osgDB::OutputStream& os, const osg::Program& attr ) \
    { \
        const osg::Program::TYPE& plist = attr.get##TYPE(); \
        os.writeSize(plist.size()); os << os.BEGIN_BRACKET << std::endl; \
        for ( osg::Program::TYPE::const_iterator itr=plist.begin(); \
              itr!=plist.end(); ++itr ) { \
            os << itr->first << itr->second << std::endl; \
        } \
        os << os.END_BRACKET << std::endl; \
        return true; \
    }

PROGRAM_LIST_FUNC( AttribBinding, AttribBindingList, BindAttribLocation )
PROGRAM_LIST_FUNC( FragDataBinding, FragDataBindingList, BindFragDataLocation )

#define PROGRAM_PARAMETER_FUNC( PROP, NAME ) \
    static bool check##PROP(const osg::Program& attr) \
    { return true; } \
    static bool read##PROP(osgDB::InputStream& is, osg::Program& attr) { \
        int value; is >> is.PROPERTY(#NAME) >> value; \
        attr.setParameter(NAME, value); \
        return true; \
    } \
    static bool write##PROP(osgDB::OutputStream& os, const osg::Program& attr) { \
        os << os.PROPERTY(#NAME) << (int)attr.getParameter(NAME) << std::endl; \
        return true; \
    }

PROGRAM_PARAMETER_FUNC( GeometryVerticesOut, GL_GEOMETRY_VERTICES_OUT_EXT )
PROGRAM_PARAMETER_FUNC( GeometryInputType, GL_GEOMETRY_INPUT_TYPE_EXT )
PROGRAM_PARAMETER_FUNC( GeometryOutputType, GL_GEOMETRY_OUTPUT_TYPE_EXT )

// _shaderList
static bool checkShaders( const osg::Program& attr )
{
    return attr.getNumShaders()>0;
}

static bool readShaders( osgDB::InputStream& is, osg::Program& attr )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Shader* shader = dynamic_cast<osg::Shader*>( is.readObject() );
        if ( shader ) attr.addShader( shader );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeShaders( osgDB::OutputStream& os, const osg::Program& attr )
{
    unsigned int size = attr.getNumShaders();
    os.writeSize(size); os << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << attr.getShader(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _numGroupsX/Y/Z
static bool checkComputeGroups( const osg::Program& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    return numX>0 && numY>0 && numZ>0;
}

static bool readComputeGroups( osgDB::InputStream& is, osg::Program& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    is >> numX >> numY >> numZ;
    attr.setComputeGroups( numX, numY, numZ );
    return true;
}

static bool writeComputeGroups( osgDB::OutputStream& os, const osg::Program& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    os << numX << numY << numZ << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Program,
                         new osg::Program,
                         osg::Program,
                         "osg::Object osg::StateAttribute osg::Program" )
{
    ADD_USER_SERIALIZER( AttribBinding );  // _attribBindingList
    ADD_USER_SERIALIZER( FragDataBinding );  // _fragDataBindingList
    ADD_USER_SERIALIZER( Shaders );  // _shaderList
    ADD_USER_SERIALIZER( GeometryVerticesOut );  // _geometryVerticesOut
    ADD_USER_SERIALIZER( GeometryInputType );  // _geometryInputType
    ADD_USER_SERIALIZER( GeometryOutputType );  // _geometryOutputType
    {
        UPDATE_TO_VERSION_SCOPED( 95 )
        ADD_USER_SERIALIZER( ComputeGroups );  // _numGroupsX/Y/Z
    }
}
