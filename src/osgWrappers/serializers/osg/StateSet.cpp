#include <osg/StateSet>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static int readValue( osgDB::InputStream& is )
{
    int value = osg::StateAttribute::INHERIT;
    if ( is.isBinary() )
        is >> value;
    else
    {
        std::string enumValue;
        is >> enumValue;
        if ( enumValue=="OFF" ) value = osg::StateAttribute::OFF;
        else if ( enumValue=="ON" ) value = osg::StateAttribute::ON;
        else if ( enumValue=="INHERIT" ) value = osg::StateAttribute::INHERIT;
        else if ( enumValue=="OFF|OVERRIDE" )
            value = osg::StateAttribute::OFF|osg::StateAttribute::OVERRIDE;
        else if ( enumValue=="OFF|PROTECTED" )
            value = osg::StateAttribute::OFF|osg::StateAttribute::PROTECTED;
        else if ( enumValue=="ON|OVERRIDE" )
            value = osg::StateAttribute::ON|osg::StateAttribute::OVERRIDE;
        else if ( enumValue=="ON|PROTECTED" )
            value = osg::StateAttribute::ON|osg::StateAttribute::PROTECTED;
    }
    return value;
}

static void readModes( osgDB::InputStream& is, osg::StateSet::ModeList& modes )
{
    unsigned int size = is.readSize();
    if ( size>0 )
    {
        is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            DEF_GLENUM(mode); is >> mode;
            int value = readValue( is );
            modes[mode.get()] = (osg::StateAttribute::Values)value;
        }
        is >> is.END_BRACKET;
    }
}

static void readAttributes( osgDB::InputStream& is, osg::StateSet::AttributeList& attrs )
{
    unsigned int size = is.readSize();
    if ( size>0 )
    {
        is >> is.BEGIN_BRACKET;
        for ( unsigned int i=0; i<size; ++i )
        {
            osg::StateAttribute* sa = dynamic_cast<osg::StateAttribute*>( is.readObject() );
            is >> is.PROPERTY("Value");
            int value = readValue( is );
            if ( sa )
                attrs[sa->getTypeMemberPair()] = osg::StateSet::RefAttributePair(sa,value);
        }
            is >> is.END_BRACKET;
        }
}

static void writeValue( osgDB::OutputStream& os, int value )
{
    if ( os.isBinary() )
        os << value;
    else
    {
        if ( value==osg::StateAttribute::OFF ) os << std::string("OFF");
        else if ( value==osg::StateAttribute::ON ) os << std::string("ON");
        else if ( value==osg::StateAttribute::INHERIT ) os << std::string("INHERIT");
        else if ( value==osg::StateAttribute::OFF+osg::StateAttribute::OVERRIDE )
            os << std::string("OFF|OVERRIDE");
        else if ( value==osg::StateAttribute::OFF+osg::StateAttribute::PROTECTED )
            os << std::string("OFF|PROTECTED");
        else if ( value==osg::StateAttribute::ON+osg::StateAttribute::OVERRIDE )
            os << std::string("ON|OVERRIDE");
        else if ( value==osg::StateAttribute::ON+osg::StateAttribute::PROTECTED )
            os << std::string("ON|PROTECTED");
        else os << std::string("INHERIT");
    }
}

static void writeModes( osgDB::OutputStream& os, const osg::StateSet::ModeList& modes )
{
    os.writeSize(modes.size());
    if ( modes.size()>0 )
    {
        os << os.BEGIN_BRACKET << std::endl;
        for ( osg::StateSet::ModeList::const_iterator itr=modes.begin();
              itr!=modes.end(); ++itr )
        {
            os << GLENUM(itr->first);
            writeValue(os, itr->second);
            os << std::endl;
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
}

static void writeAttributes( osgDB::OutputStream& os, const osg::StateSet::AttributeList& attrs )
{
    os.writeSize(attrs.size());
    if ( attrs.size()>0 )
    {
        os << os.BEGIN_BRACKET << std::endl;
        for ( osg::StateSet::AttributeList::const_iterator itr=attrs.begin();
              itr!=attrs.end(); ++itr )
        {
            os << itr->second.first.get();
            os << os.PROPERTY("Value");
            writeValue(os, itr->second.second);
            os << std::endl;
        }
        os << os.END_BRACKET;
    }
    os << std::endl;
}

// _modeList
static bool checkModeList( const osg::StateSet& ss )
{
    return ss.getModeList().size()>0;
}

static bool readModeList( osgDB::InputStream& is, osg::StateSet& ss )
{
    osg::StateSet::ModeList modes; readModes( is, modes );
    for ( osg::StateSet::ModeList::iterator itr=modes.begin();
          itr!=modes.end(); ++itr )
    {
        ss.setMode( itr->first, itr->second );
    }
    return true;
}

static bool writeModeList( osgDB::OutputStream& os, const osg::StateSet& ss )
{
    writeModes( os, ss.getModeList() );
    return true;
}

// _attributeList
static bool checkAttributeList( const osg::StateSet& ss )
{
    return ss.getAttributeList().size()>0;
}

static bool readAttributeList( osgDB::InputStream& is, osg::StateSet& ss )
{
    osg::StateSet::AttributeList attrs; readAttributes( is, attrs );
    for ( osg::StateSet::AttributeList::iterator itr=attrs.begin();
          itr!=attrs.end(); ++itr )
    {
        ss.setAttribute( itr->second.first.get(), itr->second.second );
    }
    return true;
}

static bool writeAttributeList( osgDB::OutputStream& os, const osg::StateSet& ss )
{
    writeAttributes( os, ss.getAttributeList() );
    return true;
}

// _textureModeList
static bool checkTextureModeList( const osg::StateSet& ss )
{
    return ss.getTextureModeList().size()>0;
}

static bool readTextureModeList( osgDB::InputStream& is, osg::StateSet& ss )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    osg::StateSet::ModeList modes;
    for ( unsigned int i=0; i<size; ++i )
    {
        is >> is.PROPERTY("Data");
        readModes( is, modes );
        for ( osg::StateSet::ModeList::iterator itr=modes.begin();
              itr!=modes.end(); ++itr )
        {
            ss.setTextureMode( i, itr->first, itr->second );
        }
        modes.clear();
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeTextureModeList( osgDB::OutputStream& os, const osg::StateSet& ss )
{
    const osg::StateSet::TextureModeList& tml = ss.getTextureModeList();
    os.writeSize(tml.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::StateSet::TextureModeList::const_iterator itr=tml.begin();
          itr!=tml.end(); ++itr )
    {
        os << os.PROPERTY("Data");
        writeModes( os, *itr );
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _textureAttributeList
static bool checkTextureAttributeList( const osg::StateSet& ss )
{
    return ss.getTextureAttributeList().size()>0;
}

static bool readTextureAttributeList( osgDB::InputStream& is, osg::StateSet& ss )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    osg::StateSet::AttributeList attrs;
    for ( unsigned int i=0; i<size; ++i )
    {
        is >> is.PROPERTY("Data");
        readAttributes( is, attrs );
        for ( osg::StateSet::AttributeList::iterator itr=attrs.begin();
              itr!=attrs.end(); ++itr )
        {
            ss.setTextureAttribute( i, itr->second.first.get(), itr->second.second );
        }
        attrs.clear();
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeTextureAttributeList( osgDB::OutputStream& os, const osg::StateSet& ss )
{
    const osg::StateSet::TextureAttributeList& tal = ss.getTextureAttributeList();
    os.writeSize(tal.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::StateSet::TextureAttributeList::const_iterator itr=tal.begin();
          itr!=tal.end(); ++itr )
    {
        os << os.PROPERTY("Data");
        writeAttributes( os, *itr );
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _uniformList
static bool checkUniformList( const osg::StateSet& ss )
{
    return ss.getUniformList().size()>0;
}

static bool readUniformList( osgDB::InputStream& is, osg::StateSet& ss )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Uniform* uniform = dynamic_cast<osg::Uniform*>( is.readObject() );
        is >> is.PROPERTY("Value");
        int value = readValue( is );
        if ( uniform )
            ss.addUniform( uniform, (osg::StateAttribute::Values)value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeUniformList( osgDB::OutputStream& os, const osg::StateSet& ss )
{
    const osg::StateSet::UniformList& ul = ss.getUniformList();
    os.writeSize(ul.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::StateSet::UniformList::const_iterator itr=ul.begin();
          itr!=ul.end(); ++itr )
    {
        os << itr->second.first.get();
        os << os.PROPERTY("Value");
        writeValue(os, itr->second.second);
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( StateSet,
                         new osg::StateSet,
                         osg::StateSet,
                         "osg::Object osg::StateSet" )
{
    ADD_USER_SERIALIZER( ModeList );  // _modeList
    ADD_USER_SERIALIZER( AttributeList );  // _attributeList
    ADD_USER_SERIALIZER( TextureModeList );  // _textureModeList
    ADD_USER_SERIALIZER( TextureAttributeList );  // _textureAttributeList
    ADD_USER_SERIALIZER( UniformList );  // _uniformList
    ADD_INT_SERIALIZER( RenderingHint, osg::StateSet::DEFAULT_BIN );  // _renderingHint

    BEGIN_ENUM_SERIALIZER( RenderBinMode, INHERIT_RENDERBIN_DETAILS );
        ADD_ENUM_VALUE( INHERIT_RENDERBIN_DETAILS );
        ADD_ENUM_VALUE( USE_RENDERBIN_DETAILS );
        ADD_ENUM_VALUE( OVERRIDE_RENDERBIN_DETAILS );
    END_ENUM_SERIALIZER();  // _binMode

    ADD_INT_SERIALIZER( BinNumber, 0 );  // _binNum
    ADD_STRING_SERIALIZER( BinName, "" );  // _binName
    ADD_BOOL_SERIALIZER( NestRenderBins, true );  // _nestRenderBins
    ADD_OBJECT_SERIALIZER( UpdateCallback, osg::StateSet::Callback, NULL );  // _updateCallback
    ADD_OBJECT_SERIALIZER( EventCallback, osg::StateSet::Callback, NULL );  // _eventCallback
}
