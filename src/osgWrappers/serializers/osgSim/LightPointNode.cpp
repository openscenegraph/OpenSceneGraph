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
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgSim::LightPoint pt;
        is >> is.PROPERTY("LightPoint") >> is.BEGIN_BRACKET;
        is >> is.PROPERTY("Position") >> pt._position;
        is >> is.PROPERTY("Color") >> pt._color;

        int blendingMode = 0;
        is >> is.PROPERTY("Attributes") >> pt._on >> blendingMode >> pt._intensity >> pt._radius;
        pt._blendingMode = (osgSim::LightPoint::BlendingMode)blendingMode;

        bool hasObject = false; is >> is.PROPERTY("Sector") >> hasObject;
        if ( hasObject )
        {
            is >> is.BEGIN_BRACKET;
            pt._sector = is.readObjectOfType<osgSim::Sector>();
            is >> is.END_BRACKET;
        }
        hasObject = false; is >> is.PROPERTY("BlinkSequence") >> hasObject;
        if ( hasObject )
        {
            is >> is.BEGIN_BRACKET;
            pt._blinkSequence = is.readObjectOfType<osgSim::BlinkSequence>();
            is >> is.END_BRACKET;
        }
        is >> is.END_BRACKET;
        node.addLightPoint( pt );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeLightPointList( osgDB::OutputStream& os, const osgSim::LightPointNode& node )
{
    unsigned int size = node.getNumLightPoints();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgSim::LightPoint& pt = node.getLightPoint(i);
        os << os.PROPERTY("LightPoint") << os.BEGIN_BRACKET << std::endl;
        os << os.PROPERTY("Position") << pt._position << std::endl;
        os << os.PROPERTY("Color") << pt._color << std::endl;
        os << os.PROPERTY("Attributes") << pt._on << (int)pt._blendingMode
                                            << pt._intensity << pt._radius << std::endl;
        os << os.PROPERTY("Sector") << (pt._sector!=NULL);
        if ( pt._sector!=NULL )
        {
            os << os.BEGIN_BRACKET << std::endl;
            os.writeObject( pt._sector.get() );
            os << os.END_BRACKET << std::endl;
        }
        os << os.PROPERTY("BlinkSequence") << (pt._blinkSequence!=NULL);
        if ( pt._blinkSequence!=NULL )
        {
            os << os.BEGIN_BRACKET << std::endl;
            os.writeObject( pt._blinkSequence.get() );
            os << os.END_BRACKET << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
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
