#include <osg/FragmentProgram>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _programLocalParameters
static bool checkLocalParameters( const osg::FragmentProgram& fp )
{
    return fp.getLocalParameters().size()>0;
}

static bool readLocalParameters( osgDB::InputStream& is, osg::FragmentProgram& fp )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        GLuint key; osg::Vec4d value;
        is >> key >> value;
        fp.setProgramLocalParameter( key, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLocalParameters( osgDB::OutputStream& os, const osg::FragmentProgram& fp )
{
    const osg::FragmentProgram::LocalParamList& params = fp.getLocalParameters();
    os.writeSize(params.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::FragmentProgram::LocalParamList::const_iterator itr=params.begin();
          itr!=params.end(); ++itr )
    {
        os << itr->first << osg::Vec4d(itr->second) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _matrixList
static bool checkMatrices( const osg::FragmentProgram& fp )
{
    return fp.getMatrices().size()>0;
}

static bool readMatrices( osgDB::InputStream& is, osg::FragmentProgram& fp )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        unsigned int key; osg::Matrixd value;
        is >> key >> value;
        fp.setMatrix( key, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeMatrices( osgDB::OutputStream& os, const osg::FragmentProgram& fp )
{
    const osg::FragmentProgram::MatrixList& matrices = fp.getMatrices();
    os.writeSize(matrices.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::FragmentProgram::MatrixList::const_iterator itr=matrices.begin();
          itr!=matrices.end(); ++itr )
    {
        os << (unsigned int)itr->first << osg::Matrixd(itr->second) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( FragmentProgram,
                         new osg::FragmentProgram,
                         osg::FragmentProgram,
                         "osg::Object osg::StateAttribute osg::FragmentProgram" )
{
    ADD_STRING_SERIALIZER( FragmentProgram, "" );  // _fragmentProgram
    ADD_USER_SERIALIZER( LocalParameters );  // _programLocalParameters
    ADD_USER_SERIALIZER( Matrices );  // _matrixList
}
