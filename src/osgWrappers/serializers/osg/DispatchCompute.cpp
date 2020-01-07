#include <osg/DispatchCompute>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _numGroupsX/Y/Z
static bool checkComputeGroups( const osg::DispatchCompute& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    return numX>0 && numY>0 && numZ>0;
}

static bool readComputeGroups( osgDB::InputStream& is, osg::DispatchCompute& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    is >> numX >> numY >> numZ;
    attr.setComputeGroups( numX, numY, numZ );
    return true;
}

static bool writeComputeGroups( osgDB::OutputStream& os, const osg::DispatchCompute& attr )
{
    GLint numX = 0, numY = 0, numZ = 0;
    attr.getComputeGroups( numX, numY, numZ );
    os << numX << numY << numZ << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( DispatchCompute,
                         new osg::DispatchCompute,
                         osg::DispatchCompute,
                         "osg::Object osg::Node osg::Drawable osg::DispatchCompute" )
{
        ADD_USER_SERIALIZER( ComputeGroups );  // _numGroupsX/Y/Z
}
