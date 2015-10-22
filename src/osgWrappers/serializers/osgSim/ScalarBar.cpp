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
    is >> is.BEGIN_BRACKET;
    is >> is.PROPERTY("Range") >> min >> max;

    bool hasColorRange = false;
    unsigned int colorSize = 0;
    is >> is.PROPERTY("Colors") >> hasColorRange >> colorSize;
    if ( !hasColorRange )
    {
        osgSim::ScalarsToColors* stc = new osgSim::ScalarsToColors(min, max);
        bar.setScalarsToColors( stc );
    }
    else
    {
        is >> is.BEGIN_BRACKET;
        std::vector<osg::Vec4> colors;
        for ( unsigned int i=0; i<colorSize; ++i )
        {
            osg::Vec4 color; is >> color;
            colors.push_back( color );
        }
        is >> is.END_BRACKET;

        osgSim::ColorRange* cr = new osgSim::ColorRange(min, max, colors);
        bar.setScalarsToColors( cr );
    }
    return true;
}

static bool writeScalarsToColors( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    const osgSim::ScalarsToColors* stc = bar.getScalarsToColors();
    os << os.BEGIN_BRACKET << std::endl;
    os << os.PROPERTY("Range") << stc->getMin() << stc->getMax() << std::endl;
    os << os.PROPERTY("Colors");

    unsigned int colorSize = 0;
    const osgSim::ColorRange* cr = dynamic_cast<const osgSim::ColorRange*>(stc);
    if ( cr )
    {
        const std::vector<osg::Vec4>& colors = cr->getColors();
        colorSize = colors.size();

        os << true << colorSize << os.BEGIN_BRACKET << std::endl;
        for ( unsigned int i=0; i<colorSize; ++i )
        {
            os << colors[i] << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    else
        os << false << colorSize << std::endl;
    os << os.END_BRACKET << std::endl;
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
    is >> is.BEGIN_BRACKET;
    osg::ref_ptr<osgSim::ScalarBar::ScalarPrinter> sp = is.readObjectOfType<osgSim::ScalarBar::ScalarPrinter>();
    if ( sp ) bar.setScalarPrinter( sp.get() );
    is >> is.END_BRACKET;
    return true;
}

static bool writeScalarPrinter( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    os << os.BEGIN_BRACKET << std::endl;
    os.writeObject( dynamic_cast<const osg::Object*>(bar.getScalarPrinter()) );
    os << os.END_BRACKET << std::endl;
    return true;
}

// _textProperties
static bool checkTextProperties( const osgSim::ScalarBar& bar )
{ return true; }

static bool readTextProperties( osgDB::InputStream& is, osgSim::ScalarBar& bar )
{
    osgSim::ScalarBar::TextProperties prop;
    int resX, resY;
    is >> is.BEGIN_BRACKET;
    is >> is.PROPERTY("Font") >> prop._fontFile;
    is >> is.PROPERTY("Resolution") >> resX >> resY;
    is >> is.PROPERTY("CharacterSize") >> prop._characterSize;
    is >> is.PROPERTY("Color") >> prop._fontFile;
    is >> is.END_BRACKET;

    prop._fontResolution = std::pair<int, int>(resX, resY);
    bar.setTextProperties( prop );
    return true;
}

static bool writeTextProperties( osgDB::OutputStream& os, const osgSim::ScalarBar& bar )
{
    const osgSim::ScalarBar::TextProperties& prop = bar.getTextProperties();
    os << os.BEGIN_BRACKET << std::endl;
    os << os.PROPERTY("Font") << prop._fontFile << std::endl;
    os << os.PROPERTY("Resolution") << prop._fontResolution.first
                                        << prop._fontResolution.second << std::endl;
    os << os.PROPERTY("CharacterSize") << prop._characterSize << std::endl;
    os << os.PROPERTY("Color") << prop._color << std::endl;
    os << os.END_BRACKET << std::endl;
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
