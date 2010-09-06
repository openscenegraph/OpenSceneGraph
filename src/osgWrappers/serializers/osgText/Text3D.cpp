#include <osgText/Text3D>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkFont( const osgText::Text3D& text )
{
    return text.getFont()!=NULL;
}

static bool readFont( osgDB::InputStream& is, osgText::Text3D& text )
{
    std::string fontName; is.readWrappedString( fontName );
    text.setFont( osgText::readFontFile(fontName) );
    return true;
}

static bool writeFont( osgDB::OutputStream& os, const osgText::Text3D& text )
{
    os.writeWrappedString( text.getFont()->getFileName() );
    os << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgText_Text3D,
                         new osgText::Text3D,
                         osgText::Text3D,
                         "osg::Object osg::Drawable osgText::TextBase osgText::Text3D" )
{
    ADD_FLOAT_SERIALIZER( CharacterDepth, 1.0f );  // _characterDepth
    ADD_USER_SERIALIZER( Font );  // _font
    
    BEGIN_ENUM_SERIALIZER( RenderMode, PER_GLYPH );
        ADD_ENUM_VALUE( PER_FACE );
        ADD_ENUM_VALUE( PER_GLYPH );
    END_ENUM_SERIALIZER();  // _renderMode
}
