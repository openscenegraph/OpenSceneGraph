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
        osg::ref_ptr<osg::Shader> shader = is.readObjectOfType<osg::Shader>();
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
// feedBackVaryings
static bool checkFeedBackVaryingsName( const osg::Program& attr )
{
	return true;
}
static bool readFeedBackVaryingsName( osgDB::InputStream& is, osg::Program& attr )
{
	unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
	for ( unsigned int i=0; i<size; ++i )
	{
		std::string str;
		is>> str;
		attr.addTransformFeedBackVarying(str);
	}
	is >> is.END_BRACKET;
	return true;
}
static bool writeFeedBackVaryingsName( osgDB::OutputStream& os, const osg::Program& attr )
{
	unsigned int size = attr.getNumTransformFeedBackVaryings();
	os.writeSize(size); os << os.BEGIN_BRACKET << std::endl;
	for ( unsigned int i=0; i<size; ++i )
	{
		os << attr.getTransformFeedBackVarying(i)<< std::endl;
	}
	os << os.END_BRACKET << std::endl;
	return true;
}
// feedBack mode
static bool checkFeedBackMode( const osg::Program& attr )
{
	return true;
}
static bool readFeedBackMode( osgDB::InputStream& is, osg::Program& attr )
{
	unsigned int size ;
	is>>size;
	attr.setTransformFeedBackMode(size);
	return true;
}
static bool writeFeedBackMode( osgDB::OutputStream& os, const osg::Program& attr )
{
	os << attr.getTransformFeedBackMode()<< std::endl;
	return true;
}
// _numGroupsX/Y/Z
static bool checkComputeGroups( const osg::Program& attr )
{
    return false;
}

static bool readComputeGroups( osgDB::InputStream& is, osg::Program& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    is >> numX >> numY >> numZ;
    return true;
}

static bool writeComputeGroups( osgDB::OutputStream& os, const osg::Program& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    os << numX << numY << numZ << std::endl;
    return true;
}

static bool checkBindUniformBlock( const osg::Program& node )
{
    return true;
}

static bool readBindUniformBlock( osgDB::InputStream& is, osg::Program& p )
{
    unsigned int  size = 0; is >> size >> is.BEGIN_BRACKET;
    std::string name; unsigned int index;
    for ( unsigned int i=0; i<size; ++i )
    {
        is >>name;        is >>index;    
        p.addBindUniformBlock(name, index);
    }
    is >> is.END_BRACKET;
    return true;
}


static bool writeBindUniformBlock( osgDB::OutputStream& os, const osg::Program& p )
{
    unsigned int size = p.getUniformBlockBindingList().size();
    os << size << os.BEGIN_BRACKET << std::endl;
    for(osg::Program::UniformBlockBindingList::const_iterator bbit = p.getUniformBlockBindingList().begin();
        bbit != p.getUniformBlockBindingList().end(); ++bbit)
    {
        os << bbit->first;
        os << bbit->second;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}



struct ProgramGetNumShaders : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        outputParameters.push_back(new osg::UIntValueObject("return", program->getNumShaders()));
        return true;
    }
};

struct ProgramGetShader : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }
        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        outputParameters.push_back(program->getShader(index));

        return true;
    }
};

struct ProgramAddShader : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Shader* shader = dynamic_cast<osg::Shader*>(inputParameters[0].get());
        if (!shader) return false;

        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        program->addShader(shader);

        return true;
    }
};


struct ProgramRemoveShader : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Shader* shader = dynamic_cast<osg::Shader*>(inputParameters[0].get());
        if (!shader) return false;

        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        program->removeShader(shader);

        return true;
    }
};

struct ProgramAddBindAttribLocation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.size()<2) return false;

        // decode name
        std::string name;
        osg::Object* nameObject = inputParameters[0].get();
        osg::StringValueObject* sno = dynamic_cast<osg::StringValueObject*>(nameObject);
        if (sno) name = sno->getValue();

        if (name.empty()) return false;

        // decode index
        GLuint index = 0;
        osg::ValueObject* indexObject = inputParameters[1]->asValueObject();
        if (indexObject) indexObject->getScalarValue(index);

        // assign the name and index to the program
        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        program->addBindAttribLocation(name, index);

        return true;
    }
};

/** Remove an attribute location binding. */
// void removeBindAttribLocation( const std::string& name );
struct ProgramRemoveBindAttribLocation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        // decode name
        std::string name;
        osg::Object* nameObject = inputParameters[0].get();
        osg::StringValueObject* sno = dynamic_cast<osg::StringValueObject*>(nameObject);
        if (sno) name = sno->getValue();

        if (name.empty()) return false;

        // assign the name and index to the program
        osg::Program* program = reinterpret_cast<osg::Program*>(objectPtr);
        program->removeBindAttribLocation(name);

        return true;
    }
};


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

    {
        UPDATE_TO_VERSION_SCOPED( 153 )
        REMOVE_SERIALIZER( ComputeGroups );
    }

    {
        UPDATE_TO_VERSION_SCOPED( 116 )
        ADD_USER_SERIALIZER( FeedBackVaryingsName );
        ADD_USER_SERIALIZER( FeedBackMode );
    }
    {
        UPDATE_TO_VERSION_SCOPED( 150 )
        ADD_USER_SERIALIZER( BindUniformBlock );
    }


    ADD_METHOD_OBJECT( "getNumShaders", ProgramGetNumShaders );
    ADD_METHOD_OBJECT( "getShader", ProgramGetShader );
    ADD_METHOD_OBJECT( "addShader", ProgramAddShader );
    ADD_METHOD_OBJECT( "removeShader", ProgramRemoveShader );

    ADD_METHOD_OBJECT( "addBindAttribLocation", ProgramAddBindAttribLocation );
    ADD_METHOD_OBJECT( "removeBindAttribLocation", ProgramRemoveBindAttribLocation );
}
