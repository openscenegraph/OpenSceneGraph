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
    osgText::Text &myobj = static_cast<osgText::Text &>(obj);
    bool itAdvanced = false;

    // position
    if (fr[0].matchWord("position")) {
        osg::Vec3 p;
        if (fr[1].getFloat(p.x()) && fr[2].getFloat(p.y()) && fr[3].getFloat(p.z())) {
            myobj.setPosition(p);
            fr += 4;
            itAdvanced = true;
        }
    }

    // color
    if (fr[0].matchWord("color")) {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w())) {
            myobj.setColor(c);
            fr += 4;
            itAdvanced = true;
        }
    }

    // draw mode
    if (fr[0].matchWord("drawMode")) {
        int i;
        if (fr[1].getInt(i)) {
            myobj.setDrawMode(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // bounding box
    if (fr[0].matchWord("boundingBox")) {
        int i;
        if (fr[1].getInt(i)) {
            myobj.setBoundingBox(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // alignment
    if (fr[0].matchWord("alignment")) {
        int i;
        if (fr[1].getInt(i)) {
            myobj.setAlignment(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // font
    osgText::Font *font = dynamic_cast<osgText::Font *>(fr.readObject());
    if (font) {
        myobj.setFont(font);
        itAdvanced = true;
    }

    // text
    if (fr.matchSequence("text %s")) {
        myobj.setText(std::string(fr[1].getStr()));
        fr += 2;
        itAdvanced = true;
    }

    return itAdvanced;
}

bool Text_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Text &myobj = static_cast<const osgText::Text &>(obj);

    // position
    osg::Vec3 p = myobj.getPosition();
    fw.indent() << "position " << p.x() << " " << p.y() << " " << p.z() << std::endl;

    // color
    osg::Vec4 c = myobj.getColor();
    fw.indent() << "color " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // draw mode
    fw.indent() << "drawMode " << myobj.getDrawMode() << std::endl;

    // bounding box
    fw.indent() << "boundingBox " << myobj.getBoundingBox() << std::endl;

    // alignment
    fw.indent() << "alignment " << myobj.getAlignment() << std::endl;

    // font
    fw.writeObject(*myobj.getFont());

    // text
    fw.indent() << "text " << fw.wrapString(myobj.getText()) << std::endl;

    return true;
}


