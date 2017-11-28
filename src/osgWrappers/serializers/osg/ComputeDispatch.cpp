#include <osg/ComputeDispatch>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _numGroupsX/Y/Z
static bool checkComputeGroups( const osg::ComputeDispatch& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    return numX>0 && numY>0 && numZ>0;
}

static bool readComputeGroups( osgDB::InputStream& is, osg::ComputeDispatch& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    is >> numX >> numY >> numZ;
    attr.setComputeGroups( numX, numY, numZ );
    return true;
}

static bool writeComputeGroups( osgDB::OutputStream& os, const osg::ComputeDispatch& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    os << numX << numY << numZ << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( ComputeDispatch,
                         new osg::ComputeDispatch,
                         osg::ComputeDispatch,
                         "osg::Object osg::Node osg::Drawable osg::ComputeDispatch" )
{
        ADD_USER_SERIALIZER( ComputeGroups );  // _numGroupsX/Y/Z
}
