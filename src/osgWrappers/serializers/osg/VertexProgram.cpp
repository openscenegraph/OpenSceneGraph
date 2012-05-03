#include <osg/VertexProgram>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _programLocalParameters
static bool checkLocalParameters( const osg::VertexProgram& vp )
{
    return vp.getLocalParameters().size()>0;
}

static bool readLocalParameters( osgDB::InputStream& is, osg::VertexProgram& vp )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        GLuint key; osg::Vec4d value;
        is >> key >> value;
        vp.setProgramLocalParameter( key, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLocalParameters( osgDB::OutputStream& os, const osg::VertexProgram& vp )
{
    const osg::VertexProgram::LocalParamList& params = vp.getLocalParameters();
    os.writeSize(params.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::VertexProgram::LocalParamList::const_iterator itr=params.begin();
          itr!=params.end(); ++itr )
    {
        os << itr->first << osg::Vec4d(itr->second) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _matrixList
static bool checkMatrices( const osg::VertexProgram& vp )
{
    return vp.getMatrices().size()>0;
}

static bool readMatrices( osgDB::InputStream& is, osg::VertexProgram& vp )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        unsigned int key; osg::Matrixd value;
        is >> key >> value;
        vp.setMatrix( key, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeMatrices( osgDB::OutputStream& os, const osg::VertexProgram& vp )
{
    const osg::VertexProgram::MatrixList& matrices = vp.getMatrices();
    os.writeSize(matrices.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::VertexProgram::MatrixList::const_iterator itr=matrices.begin();
          itr!=matrices.end(); ++itr )
    {
        os << (unsigned int)itr->first << osg::Matrixd(itr->second) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( VertexProgram,
                         new osg::VertexProgram,
                         osg::VertexProgram,
                         "osg::Object osg::StateAttribute osg::VertexProgram" )
{
    ADD_STRING_SERIALIZER( VertexProgram, "" );  // _fragmentProgram
    ADD_USER_SERIALIZER( LocalParameters );  // _programLocalParameters
    ADD_USER_SERIALIZER( Matrices );  // _matrixList
}
