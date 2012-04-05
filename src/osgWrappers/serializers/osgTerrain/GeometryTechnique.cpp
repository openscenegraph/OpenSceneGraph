#include <osgTerrain/GeometryTechnique>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkFilterMatrix( const osgTerrain::GeometryTechnique& tech )
{
    return true;
}

static bool readFilterMatrix( osgDB::InputStream& is, osgTerrain::GeometryTechnique& tech )
{
    osg::Matrix3 matrix;
    is >> is.BEGIN_BRACKET;
    for ( int r=0; r<3; ++r )
    {
        is >> matrix(r, 0) >> matrix(r, 1) >> matrix(r, 2);
    }
    is >> is.END_BRACKET;
    tech.setFilterMatrix( matrix );
    return true;
}

static bool writeFilterMatrix( osgDB::OutputStream& os, const osgTerrain::GeometryTechnique& tech )
{
    const osg::Matrix3& matrix = tech.getFilterMatrix();
    os << os.BEGIN_BRACKET << std::endl;
    for ( int r=0; r<3; ++r )
    {
        os << matrix(r, 0) << matrix(r, 1) << matrix(r, 2) << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgTerrain_GeometryTechnique,
                         new osgTerrain::GeometryTechnique,
                         osgTerrain::GeometryTechnique,
                         "osg::Object osgTerrain::TerrainTechnique osgTerrain::GeometryTechnique" )
{
    ADD_FLOAT_SERIALIZER( FilterBias, 0.0f );  // _filterBias
    ADD_FLOAT_SERIALIZER( FilterWidth, 0.1f );  // _filterWidth
    ADD_USER_SERIALIZER( FilterMatrix );  // _filterMatrix
}
