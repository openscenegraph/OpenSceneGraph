#include <osg/SubroutineUniform>


#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#include <osg/ScriptEngine>
#if 0
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "../../../osgPlugins/lua/LuaScriptEngine.h"
namespace wrapperluaosgEngineScript
{
REGISTER_OBJECT_WRAPPER( LuaScriptengine,
                         new lua::LuaScriptEngine,
                         lua::LuaScriptEngine,
                         "osg::Object osg::ScriptEngine lua::LuaScriptEngine" )
{}
}

REGISTER_OBJECT_WRAPPER( ScriptNodeCallback,
                         new osg::ScriptNodeCallback,
                         osg::ScriptNodeCallback,
                         "osg::Object osg::Callback osg::ScriptNodeCallback" )
{
//osg::Callback osg::NodeCallback
    ADD_OBJECT_SERIALIZER( Script, osg::Script, NULL );  // _script
    ADD_STRING_SERIALIZER( EntryPoint, "" );  // _entrypoint
}



#undef OBJECT_CAST
#define OBJECT_CAST static_cast
namespace wrapperosgEngineScript
{
REGISTER_OBJECT_WRAPPER( Scriptengine,
                         NULL,
                         osg::ScriptEngine,
                         "osg::Object osg::ScriptEngine" )
{}
}

namespace wrapperosgScript
{
REGISTER_OBJECT_WRAPPER( Script,
                         new osg::Script,
                         osg::Script,
                         "osg::Object osg::Script" )
{
ADD_STRING_SERIALIZER( Script, "" );  // _script
ADD_STRING_SERIALIZER( Language, "" );  // _script
}


}
#endif
namespace wrapperosgSubroutineUniform
{
static bool checkSubroutineName( const osg::SubroutineUniform& attr )
{
    return true;
}
static bool readSubroutineName( osgDB::InputStream& is, osg::SubroutineUniform& attr )
{
    unsigned int size = is.readSize();
    is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string str;
        is>> str;
        attr.addSubroutineName(str);
    }
    is >> is.END_BRACKET;
    return true;
}
static bool writeSubroutineName( osgDB::OutputStream& os, const osg::SubroutineUniform& attr )
{
    unsigned int size = attr.getNumSubroutineNames();
    os.writeSize(size);
    os << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << attr.getSubroutineName(i)<< std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( SubroutineUniform,
                         new osg::SubroutineUniform,
                         osg::SubroutineUniform,
                         "osg::Object osg::StateAttribute osg::SubroutineUniform" )
{

    ADD_USER_SERIALIZER( SubroutineName );

    {
        typedef osgDB::EnumSerializer<osg::SubroutineUniform, osg::Shader::Type, bool> ShaderTypeSerializer;
        osg::ref_ptr<ShaderTypeSerializer> serializer = new ShaderTypeSerializer("ShaderType",osg::Shader::VERTEX,
                &osg::SubroutineUniform::getShaderType,
                &osg::SubroutineUniform::setShaderType);
        serializer->add("VERTEX", osg::Shader::VERTEX);
        serializer->add("FRAGMENT", osg::Shader::FRAGMENT);
        serializer->add("GEOMETRY", osg::Shader::GEOMETRY);
        serializer->add("COMPUTE", osg::Shader::COMPUTE);
        serializer->add("TESSCONTROL", osg::Shader::TESSCONTROL);
        serializer->add("TESSEVALUATION", osg::Shader::TESSEVALUATION);
        wrapper->addSerializer(serializer.get(), osgDB::BaseSerializer::RW_ENUM);
    }


}
}
