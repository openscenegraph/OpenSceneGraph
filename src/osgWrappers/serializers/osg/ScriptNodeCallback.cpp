#undef OBJECT_CAST
#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#include <osg/ScriptEngine>


REGISTER_OBJECT_WRAPPER( ScriptNodeCallback,
                         new osg::ScriptNodeCallback,
                         osg::ScriptNodeCallback,
                         "osg::Object osg::Callback osg::CallbackObject osg::ScriptNodeCallback" )
{
    ADD_OBJECT_SERIALIZER( Script, osg::Script, NULL );  // _script
    ADD_STRING_SERIALIZER( EntryPoint, "" );  // _entrypoint
}


#undef OBJECT_CAST
#define OBJECT_CAST static_cast

