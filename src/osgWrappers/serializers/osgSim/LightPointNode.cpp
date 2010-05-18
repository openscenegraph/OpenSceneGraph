#include <osgSim/LightPointNode>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkLightPointList( const osgSim::LightPointNode& node )
{
    return node.getNumLightPoints()>0;
}

static bool readLightPointList( osgDB::InputStream& is, osgSim::LightPointNode& node )
{
    unsigned int size = 0; is >> size >> osgDB::BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgSim::LightPoint pt;
        is >> osgDB::PROPERTY("LightPoint") >> osgDB::BEGIN_BRACKET;
        is >> osgDB::PROPERTY("Position") >> pt._position;
        is >> osgDB::PROPERTY("Color") >> pt._color;
        
        int blendingMode = 0;
        is >> osgDB::PROPERTY("Attributes") >> pt._on >> blendingMode >> pt._intensity >> pt._radius;
        pt._blendingMode = (osgSim::LightPoint::BlendingMode)blendingMode;
        
        is >> osgDB::PROPERTY("Sector");
        pt._sector = dynamic_cast<osgSim::Sector*>( is.readObject() );
        is >> osgDB::PROPERTY("BlinkSequence");
        pt._blinkSequence = dynamic_cast<osgSim::BlinkSequence*>( is.readObject() );
        is >> osgDB::END_BRACKET;
        node.addLightPoint( pt );
    }
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeLightPointList( osgDB::OutputStream& os, const osgSim::LightPointNode& node )
{
    unsigned int size = node.getNumLightPoints();
    os << size << osgDB::BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgSim::LightPoint& pt = node.getLightPoint(i);
        os << osgDB::PROPERTY("LightPoint") << osgDB::BEGIN_BRACKET << std::endl;
        os << osgDB::PROPERTY("Position") << pt._position << std::endl;
        os << osgDB::PROPERTY("Color") << pt._color << std::endl;
        os << osgDB::PROPERTY("Attributes") << pt._on << (int)pt._blendingMode
                                            << pt._intensity << pt._radius << std::endl;
        os << osgDB::PROPERTY("Sector"); os.writeObject( pt._sector.get() );
        os << osgDB::PROPERTY("BlinkSequence"); os.writeObject( pt._blinkSequence.get() );
        os << osgDB::END_BRACKET << std::endl;
    }
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_LightPointNode,
                         new osgSim::LightPointNode,
                         osgSim::LightPointNode,
                         "osg::Object osg::Node osgSim::LightPointNode" )
{
    ADD_USER_SERIALIZER( LightPointList );  // _lightPointList
    ADD_FLOAT_SERIALIZER( MinPixelSize, 0.0f );  // _minPixelSize
    ADD_FLOAT_SERIALIZER( MaxPixelSize, 30.0f );  // _maxPixelSize
    ADD_FLOAT_SERIALIZER( MaxVisibleDistance2, FLT_MAX );  // _maxVisibleDistance2
    ADD_OBJECT_SERIALIZER( LightPointSystem, osgSim::LightPointSystem, NULL );  // _lightSystem
    ADD_BOOL_SERIALIZER( PointSprite, false );  // _pointSprites
}
