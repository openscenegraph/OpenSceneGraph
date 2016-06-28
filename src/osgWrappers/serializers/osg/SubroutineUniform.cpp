#include <osg/SubroutineUniform>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

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
