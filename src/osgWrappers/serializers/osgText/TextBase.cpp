#include <osgText/TextBase>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _font
static bool checkFont( const osgText::TextBase& text )
{
    return text.getFont()!=NULL;
}

static bool readFont( osgDB::InputStream& is, osgText::TextBase& text )
{
    std::string fontName; is.readWrappedString( fontName );
    text.setFont( osgText::readFontFile(fontName) );
    return true;
}

static bool writeFont( osgDB::OutputStream& os, const osgText::TextBase& text )
{
    os.writeWrappedString( text.getFont()->getFileName() );
    os << std::endl;
    return true;
}

// _fontSize
static bool checkFontSize( const osgText::TextBase& text )
{
    return true;
}

static bool readFontSize( osgDB::InputStream& is, osgText::TextBase& text )
{
    unsigned int width, height; is >> width >> height;
    text.setFontResolution( width, height );
    return true;
}

static bool writeFontSize( osgDB::OutputStream& os, const osgText::TextBase& text )
{
    os << text.getFontWidth() << text.getFontHeight() << std::endl;
    return true;
}

// _characterHeight, _characterAspectRatio
static bool checkCharacterSize( const osgText::TextBase& text )
{
    return true;
}

static bool readCharacterSize( osgDB::InputStream& is, osgText::TextBase& text )
{
    float height, aspectRatio; is >> height >> aspectRatio;
    text.setCharacterSize( height, aspectRatio );
    return true;
}

static bool writeCharacterSize( osgDB::OutputStream& os, const osgText::TextBase& text )
{
    os << text.getCharacterHeight() << text.getCharacterAspectRatio() << std::endl;
    return true;
}

// _text
static bool checkText( const osgText::TextBase& text )
{
    return text.getText().size()>0;
}

static bool readText( osgDB::InputStream& is, osgText::TextBase& text )
{
    bool isACString; is >> isACString;
    if ( isACString )
    {
        std::string acString; is.readWrappedString( acString );
        text.setText( acString );
    }
    else
    {
        osg::UIntArray* array = dynamic_cast<osg::UIntArray*>( is.readArray() );
        if ( array )
        {
            osgText::String string;
            for ( osg::UIntArray::iterator itr=array->begin(); itr!=array->end(); ++itr )
            {
                string.push_back( *itr );
            }
            text.setText( string );
        }
    }
    return true;
}

static bool writeText( osgDB::OutputStream& os, const osgText::TextBase& text )
{
    bool isACString = true;
    const osgText::String& string = text.getText();
    for ( osgText::String::const_iterator itr=string.begin(); itr!=string.end(); ++itr )
    {
        if ( *itr==0 || *itr>256 )
        {
            isACString = false;
            break;
        }
    }

    os << isACString;
    if ( isACString )
    {
        std::string acString;
        for ( osgText::String::const_iterator itr=string.begin(); itr!=string.end(); ++itr )
        {
            acString += (char)(*itr);
        }
        os.writeWrappedString( acString );
        os << std::endl;
    }
    else
    {
        osg::ref_ptr<osg::UIntArray> array = new osg::UIntArray( string.begin(), string.end() );
        os << array.get();
    }
    return true;
}

// _drawMode
static bool checkDrawMode( const osgText::TextBase& text )
{
    return text.getDrawMode()!=osgText::TextBase::TEXT;
}

static bool readDrawMode( osgDB::InputStream& is, osgText::TextBase& text )
{
    unsigned int mask = osgText::TextBase::TEXT;
    if ( is.isBinary() )
        is >> mask;
    else
    {
        std::string maskSetString; is >> maskSetString;
        osgDB::StringList maskList; osgDB::split( maskSetString, maskList, '|' );
        for ( unsigned int i=0; i<maskList.size(); ++i )
        {
            const std::string& maskValue = maskList[i];
            if ( maskValue=="TEXT" ) mask |= osgText::TextBase::TEXT;
            else if ( maskValue=="BOUND" ) mask |= osgText::TextBase::BOUNDINGBOX;
            else if ( maskValue=="FILLED" ) mask |= osgText::TextBase::FILLEDBOUNDINGBOX;
            else if ( maskValue=="ALIGNMENT" ) mask |= osgText::TextBase::ALIGNMENT;
        }
    }
    text.setDrawMode( mask );
    return true;
}

static bool writeDrawMode( osgDB::OutputStream& os, const osgText::TextBase& text )
{
    unsigned int mask = text.getDrawMode();
    if ( os.isBinary() )
        os << mask;
    else
    {
        std::string maskString;
        if ( mask==osgText::TextBase::TEXT ) maskString += std::string("TEXT|");
        if ( mask==osgText::TextBase::BOUNDINGBOX ) maskString += std::string("BOUND|");
        if ( mask==osgText::TextBase::FILLEDBOUNDINGBOX ) maskString += std::string("FILLED|");
        if ( mask==osgText::TextBase::ALIGNMENT ) maskString += std::string("ALIGNMENT|");
        if ( !maskString.size() ) maskString = std::string("NONE|");
        os << maskString.substr(0, maskString.size()-1) << std::endl;
    }
    return true;
}

REGISTER_OBJECT_WRAPPER( osgText_TextBase,
                         /*new osgText::TextBase*/NULL,
                         osgText::TextBase,
                         "osg::Object osg::Drawable osgText::TextBase" )
{
    ADD_USER_SERIALIZER( Font );  // _font
    ADD_USER_SERIALIZER( FontSize );  // _fontSize
    ADD_USER_SERIALIZER( CharacterSize );  // _characterHeight, _characterAspectRatio

    BEGIN_ENUM_SERIALIZER( CharacterSizeMode, OBJECT_COORDS );
        ADD_ENUM_VALUE( OBJECT_COORDS );
        ADD_ENUM_VALUE( SCREEN_COORDS );
        ADD_ENUM_VALUE( OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT );
    END_ENUM_SERIALIZER();  // _characterSizeMode

    ADD_FLOAT_SERIALIZER( MaximumWidth, 0.0f );  // _maximumWidth
    ADD_FLOAT_SERIALIZER( MaximumHeight, 0.0f );  // _maximumHeight
    ADD_FLOAT_SERIALIZER( LineSpacing, 0.0f );  // _lineSpacing
    ADD_USER_SERIALIZER( Text );  // _text
    ADD_VEC3_SERIALIZER( Position, osg::Vec3() );  // _position

    BEGIN_ENUM_SERIALIZER2( Alignment, osgText::TextBase::AlignmentType, LEFT_BASE_LINE );
        ADD_ENUM_VALUE( LEFT_TOP );
        ADD_ENUM_VALUE( LEFT_CENTER );
        ADD_ENUM_VALUE( LEFT_BOTTOM );
        ADD_ENUM_VALUE( CENTER_TOP );
        ADD_ENUM_VALUE( CENTER_CENTER );
        ADD_ENUM_VALUE( CENTER_BOTTOM );
        ADD_ENUM_VALUE( RIGHT_TOP );
        ADD_ENUM_VALUE( RIGHT_CENTER );
        ADD_ENUM_VALUE( RIGHT_BOTTOM );
        ADD_ENUM_VALUE( LEFT_BASE_LINE );
        ADD_ENUM_VALUE( CENTER_BASE_LINE );
        ADD_ENUM_VALUE( RIGHT_BASE_LINE );
        ADD_ENUM_VALUE( LEFT_BOTTOM_BASE_LINE );
        ADD_ENUM_VALUE( CENTER_BOTTOM_BASE_LINE );
        ADD_ENUM_VALUE( RIGHT_BOTTOM_BASE_LINE );
    END_ENUM_SERIALIZER();  // _alignment

    BEGIN_ENUM_SERIALIZER( AxisAlignment, XY_PLANE );
        ADD_ENUM_VALUE( XY_PLANE );
        ADD_ENUM_VALUE( REVERSED_XY_PLANE );
        ADD_ENUM_VALUE( XZ_PLANE );
        ADD_ENUM_VALUE( REVERSED_XZ_PLANE );
        ADD_ENUM_VALUE( YZ_PLANE );
        ADD_ENUM_VALUE( REVERSED_YZ_PLANE );
        ADD_ENUM_VALUE( SCREEN );
        ADD_ENUM_VALUE( USER_DEFINED_ROTATION );
    END_ENUM_SERIALIZER();  // _axisAlignment

    ADD_QUAT_SERIALIZER( Rotation, osg::Quat() );  // _rotation
    ADD_BOOL_SERIALIZER( AutoRotateToScreen, false );  // _autoRotateToScreen

    BEGIN_ENUM_SERIALIZER( Layout, LEFT_TO_RIGHT );
        ADD_ENUM_VALUE( LEFT_TO_RIGHT );
        ADD_ENUM_VALUE( RIGHT_TO_LEFT );
        ADD_ENUM_VALUE( VERTICAL );
    END_ENUM_SERIALIZER();  // _layout

    ADD_USER_SERIALIZER( DrawMode );  // _drawMode
    ADD_FLOAT_SERIALIZER( BoundingBoxMargin, 0.0f );  // _textBBMargin
    ADD_VEC4_SERIALIZER( BoundingBoxColor, osg::Vec4() );  // _textBBColor
}
