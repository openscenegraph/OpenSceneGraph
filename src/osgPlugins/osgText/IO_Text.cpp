#include <osgText/Text>
#include <osgText/Font>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool Text_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Text_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Text_Proxy
(
    new osgText::Text,
    "Text",
    "Object Drawable Text",
    Text_readLocalData,
    Text_writeLocalData
);

bool Text_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::Text &text = static_cast<osgText::Text &>(obj);
    bool itAdvanced = false;

    // position
    if (fr[0].matchWord("position")) {
        osg::Vec3 p;
        if (fr[1].getFloat(p.x()) && fr[2].getFloat(p.y()) && fr[3].getFloat(p.z())) {
            text.setPosition(p);
            fr += 4;
            itAdvanced = true;
        }
    }

    // color
    if (fr[0].matchWord("color")) {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w())) {
            text.setColor(c);
            fr += 4;
            itAdvanced = true;
        }
    }

    // draw mode
    if (fr[0].matchWord("drawMode")) {
        int i;
        if (fr[1].getInt(i)) {
            text.setDrawMode(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // alignment
    if (fr[0].matchWord("alignment")) {
        int i;
        if (fr[1].getInt(i)) {
            text.setAlignment((osgText::Text::AlignmentType)i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // text
    if (fr.matchSequence("text %s")) {
        text.setText(std::string(fr[1].getStr()));
        fr += 2;
        itAdvanced = true;
    }

    return itAdvanced;
}

bool Text_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Text &text = static_cast<const osgText::Text &>(obj);

    if (text.getFont())
    {
        fw.indent() << "font " << text.getFont()->getFileName() << std::endl;
    }

    fw.indent() << "fontSize " << text.getFontWidth() << " " << text.getFontHeight() << std::endl;

    // position
    osg::Vec3 p = text.getPosition();
    fw.indent() << "position " << p.x() << " " << p.y() << " " << p.z() << std::endl;

    // color
    osg::Vec4 c = text.getColor();
    fw.indent() << "color " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // draw mode
    fw.indent() << "drawMode " << text.getDrawMode() << std::endl;

    // alignment
    fw.indent() << "alignment " << text.getAlignment() << std::endl;

    // text
    const osgText::Text::TextString& textstring = text.getText();
    bool isACString = true;
    for(osgText::Text::TextString::const_iterator itr=textstring.begin();
        itr!=textstring.end() && isACString;
        ++itr)
    {
        if (*itr==0 || *itr>256) isACString=false;
    }
    if (isACString)
    {
        std::string str;
        std::copy(textstring.begin(),textstring.end(),std::back_inserter(str));
        fw.indent() << "text " << fw.wrapString(str) << std::endl;
    }
    else
    {
        // do it the hardway...
    }

    return true;
}


