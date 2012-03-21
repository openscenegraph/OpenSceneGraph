#include <osgSim/ScalarBar>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _stc
static bool checkScalarsToColors( const osgSim::ScalarBar& bar )
{ return bar.getScalarsToColors()!=NULL; }

static bool readScalarsToColors( osgDB::InputStream& is, osgSim::ScalarBar& bar )
{
    float min, max;
    is >> osgDB::BEGIN_BRACKET;
    is >> osgDB::PROPERTY("Range") >> min >> max;

    bool hasColorRange = false;
    unsigned int colorSize = 0;
    is >> osgDB::PROPERTY("Colors") >> hasColorRange >> colorSize;
    if ( !hasColorRange )
    {
        osgSim::ScalarsToColors* stc = new osgSim::ScalarsToColors(min, max);
        bar.setScalarsToColors( stc );
    }
    else
    {
        is >> osgDB::BEGIN_BRACKET;
        std::vector<osg::Vec4> colors;
        for ( unsigned int i=0; i<colorSize; ++i )
        {
            osg::Vec4 color; is >> color;
            colors.push_back( color );
        }
        is >> osgDB::END_BRACKET;

        osgSim::ColorRange* cr = new osgSim::ColorRange(min, max, colors);
        bar.setScalarsToColors( cr );
    }
    return true;
}

static bool writeScalarsToColors( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    const osgSim::ScalarsToColors* stc = bar.getScalarsToColors();
    os << osgDB::BEGIN_BRACKET << std::endl;
    os << osgDB::PROPERTY("Range") << stc->getMin() << stc->getMax() << std::endl;
    os << osgDB::PROPERTY("Colors");

    unsigned int colorSize = 0;
    const osgSim::ColorRange* cr = dynamic_cast<const osgSim::ColorRange*>(stc);
    if ( cr )
    {
        const std::vector<osg::Vec4>& colors = cr->getColors();
        colorSize = colors.size();

        os << true << colorSize << osgDB::BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<colorSize; ++i )
        {
            os << colors[i] << std::endl;
        }
        os << osgDB::END_BRACKET << std::endl;
    }
    else
        os << false << colorSize << std::endl;
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

// _sp
static bool checkScalarPrinter( const osgSim::ScalarBar& bar )
{
    return bar.getScalarPrinter()!=NULL &&
           dynamic_cast<const osg::Object*>(bar.getScalarPrinter());
}

static bool readScalarPrinter( osgDB::InputStream& is, osgSim::ScalarBar& bar )
{
    is >> osgDB::BEGIN_BRACKET;
    osgSim::ScalarBar::ScalarPrinter* sp =
        dynamic_cast<osgSim::ScalarBar::ScalarPrinter*>( is.readObject() );
    if ( sp ) bar.setScalarPrinter( sp );
    is >> osgDB::END_BRACKET;
    return true;
}

static bool writeScalarPrinter( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    os << osgDB::BEGIN_BRACKET << std::endl;
    os.writeObject( dynamic_cast<const osg::Object*>(bar.getScalarPrinter()) );
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

// _textProperties
static bool checkTextProperties( const osgSim::ScalarBar& bar )
{ return true; }

static bool readTextProperties( osgDB::InputStream& is, osgSim::ScalarBar& bar )
{
    osgSim::ScalarBar::TextProperties prop;
    int resX, resY;
    is >> osgDB::BEGIN_BRACKET;
    is >> osgDB::PROPERTY("Font") >> prop._fontFile;
    is >> osgDB::PROPERTY("Resolution") >> resX >> resY;
    is >> osgDB::PROPERTY("CharacterSize") >> prop._characterSize;
    is >> osgDB::PROPERTY("Color") >> prop._fontFile;
    is >> osgDB::END_BRACKET;

    prop._fontResolution = std::pair<int, int>(resX, resY);
    bar.setTextProperties( prop );
    return true;
}

static bool writeTextProperties( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    const osgSim::ScalarBar::TextProperties& prop = bar.getTextProperties();
    os << osgDB::BEGIN_BRACKET << std::endl;
    os << osgDB::PROPERTY("Font") << prop._fontFile << std::endl;
    os << osgDB::PROPERTY("Resolution") << prop._fontResolution.first
                                        << prop._fontResolution.second << std::endl;
    os << osgDB::PROPERTY("CharacterSize") << prop._characterSize << std::endl;
    os << osgDB::PROPERTY("Color") << prop._color << std::endl;
    os << osgDB::END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ScalarBar,
                         new osgSim::ScalarBar,
                         osgSim::ScalarBar,
                         "osg::Object osg::Node osg::Geode osgSim::ScalarBar" )
{
    ADD_INT_SERIALIZER( NumColors, 256 );  // _numColors
    ADD_INT_SERIALIZER( NumLabels, 0 );  // _numLabels
    ADD_USER_SERIALIZER( ScalarsToColors );  // _stc
    ADD_STRING_SERIALIZER( Title, "" );  // _title
    ADD_VEC3_SERIALIZER( Position, osg::Vec3() );  // _position
    ADD_FLOAT_SERIALIZER( Width, 0.0f );  // _width
    ADD_FLOAT_SERIALIZER( AspectRatio, 0.0f );  // _aspectRatio

    BEGIN_ENUM_SERIALIZER( Orientation, HORIZONTAL );
        ADD_ENUM_VALUE( HORIZONTAL );
        ADD_ENUM_VALUE( VERTICAL );
    END_ENUM_SERIALIZER();  // _orientation

    ADD_USER_SERIALIZER( ScalarPrinter );  // _sp
    ADD_USER_SERIALIZER( TextProperties );  // _textProperties
}
