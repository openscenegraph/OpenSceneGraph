#include <osgText/Font>
#include <osgText/Font>

#include <iostream>
#include <string>

#include <osg/Vec3>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

/////////////////////////////////////////////////////////////////////////////
// class Font
/////////////////////////////////////////////////////////////////////////////

bool Font_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Font_Proxy
(
    0,
    "Font",
    "Object Font",
    0,
    Font_writeLocalData
);

bool Font_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Font &myobj = static_cast<const osgText::Font &>(obj);

    fw.indent() << "parameters ";
    fw << myobj.getPointSize() << " " << myobj.getTextureSize() << " ";
    fw << "\"" << myobj.getFontName() << "\"" << std::endl;

    return true;
}


/////////////////////////////////////////////////////////////////////////////
// class BitmapFont
/////////////////////////////////////////////////////////////////////////////

bool BitmapFont_readLocalData(osg::Object &obj, osgDB::Input &fr);

osgDB::RegisterDotOsgWrapperProxy BitmapFont_Proxy
(
    osgNew osgText::BitmapFont,
    "BitmapFont",
    "Object Font RasterFont BitmapFont",
    BitmapFont_readLocalData,
    0
);

bool BitmapFont_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::BitmapFont &myobj = static_cast<osgText::BitmapFont &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("parameters")) {
        int psize;
        if (fr[1].getInt(psize) && fr[2].isInt() && fr[3].isString()) {
            osgText::BitmapFont *temp = new osgText::BitmapFont(std::string(fr[3].getStr()), psize);
            temp->copyAndInvalidate(myobj);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}


/////////////////////////////////////////////////////////////////////////////
// class PixmapFont
/////////////////////////////////////////////////////////////////////////////

bool PixmapFont_readLocalData(osg::Object &obj, osgDB::Input &fr);

osgDB::RegisterDotOsgWrapperProxy PixmapFont_Proxy
(
    osgNew osgText::PixmapFont,
    "PixmapFont",
    "Object Font RasterFont PixmapFont",
    PixmapFont_readLocalData,
    0
);

bool PixmapFont_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::PixmapFont &myobj = static_cast<osgText::PixmapFont &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("parameters")) {
        int psize;
        if (fr[1].getInt(psize) && fr[2].isInt() && fr[3].isString()) {
            osgText::PixmapFont *temp = new osgText::PixmapFont(std::string(fr[3].getStr()), psize);
            temp->copyAndInvalidate(myobj);            
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

/////////////////////////////////////////////////////////////////////////////
// class TextureFont
/////////////////////////////////////////////////////////////////////////////

bool TextureFont_readLocalData(osg::Object &obj, osgDB::Input &fr);

osgDB::RegisterDotOsgWrapperProxy TextureFont_Proxy
(
    osgNew osgText::TextureFont,
    "TextureFont",
    "Object Font RasterFont TextureFont",
    TextureFont_readLocalData,
    0
);

bool TextureFont_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::TextureFont &myobj = static_cast<osgText::TextureFont &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("parameters")) {
        int psize, txsize;
        if (fr[1].getInt(psize) && fr[2].getInt(txsize) && fr[3].isString()) {
            osgText::TextureFont *temp = new osgText::TextureFont(std::string(fr[3].getStr()), psize, txsize);
            temp->copyAndInvalidate(myobj);            
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}


/////////////////////////////////////////////////////////////////////////////
// class OutlineFont
/////////////////////////////////////////////////////////////////////////////

bool OutlineFont_readLocalData(osg::Object &obj, osgDB::Input &fr);

osgDB::RegisterDotOsgWrapperProxy OutlineFont_Proxy
(
    osgNew osgText::OutlineFont,
    "OutlineFont",
    "Object Font VectorFont OutlineFont",
    OutlineFont_readLocalData,
    0
);

bool OutlineFont_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::OutlineFont &myobj = static_cast<osgText::OutlineFont &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("parameters")) {
        int psize;
        if (fr[1].getInt(psize) && fr[2].isInt() && fr[3].isString()) {
            osgText::OutlineFont *temp = new osgText::OutlineFont(std::string(fr[3].getStr()), psize, 1);
            temp->copyAndInvalidate(myobj);            
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

/////////////////////////////////////////////////////////////////////////////
// class PolygonFont
/////////////////////////////////////////////////////////////////////////////

bool PolygonFont_readLocalData(osg::Object &obj, osgDB::Input &fr);

osgDB::RegisterDotOsgWrapperProxy PolygonFont_Proxy
(
    osgNew osgText::PolygonFont,
    "PolygonFont",
    "Object Font VectorFont PolygonFont",
    PolygonFont_readLocalData,
    0
);

bool PolygonFont_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::PolygonFont &myobj = static_cast<osgText::PolygonFont &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("parameters")) {
        int psize;
        if (fr[1].getInt(psize) && fr[2].isInt() && fr[3].isString()) {
            osgText::PolygonFont *temp = new osgText::PolygonFont(std::string(fr[3].getStr()), psize, 1);
            temp->copyAndInvalidate(myobj);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}
