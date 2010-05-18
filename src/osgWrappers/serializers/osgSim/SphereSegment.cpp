#include <osgSim/SphereSegment>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkArea( const osgSim::SphereSegment& sphere )
{ return true; }

static bool readArea( osgDB::InputStream& is, osgSim::SphereSegment& sphere )
{
    float azMin, azMax, elevMin, elevMax;
    is >> azMin >> azMax >> elevMin >> elevMax;
    sphere.setArea( azMin, azMax, elevMin, elevMax );
    return true;
}

static bool writeArea( osgDB::OutputStream& os, const osgSim::SphereSegment& sphere )
{
    float azMin, azMax, elevMin, elevMax;
    sphere.getArea( azMin, azMax, elevMin, elevMax );
    os << azMin << azMax << elevMin << elevMax << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_SphereSegment,
                         new osgSim::SphereSegment,
                         osgSim::SphereSegment,
                         "osg::Object osg::Node osg::Geode osgSim::SphereSegment" )
{
    ADD_VEC3_SERIALIZER( Centre, osg::Vec3() );  // _centre
    ADD_FLOAT_SERIALIZER( Radius, 1.0f );  // _radius
    ADD_USER_SERIALIZER( Area );  // _azMin, _azMax, _elevMin, _elevMax
    ADD_INT_SERIALIZER( Density, 10 );  // _density
    ADD_INT_SERIALIZER( DrawMask, osgSim::SphereSegment::ALL );  // _drawMask
    ADD_VEC4_SERIALIZER( SurfaceColor, osg::Vec4() );  // _surfaceColor
    ADD_VEC4_SERIALIZER( SpokeColor, osg::Vec4() );  // _spokeColor
    ADD_VEC4_SERIALIZER( EdgeLineColor, osg::Vec4() );  // _edgeLineColor
    ADD_VEC4_SERIALIZER( SideColor, osg::Vec4() );  // _planeColor
}
