#include <osgText/Paragraph>
#include <osgText/Font>

#include <iostream>
#include <string>

#include <osg/Vec3>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool Paragraph_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Paragraph_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

// osgDB::RegisterDotOsgWrapperProxy Paragraph_Proxy
// (
//     osgNew osgText::Paragraph,
//     "Paragraph",
//     "Object Node Geode Paragraph",
//     Paragraph_readLocalData,
//     Paragraph_writeLocalData
// );
// 
bool Paragraph_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::Paragraph &myobj = static_cast<osgText::Paragraph &>(obj);
    bool itAdvanced = false;

    // font
    osgText::Font *font = dynamic_cast<osgText::Font *>(fr.readObject());
    if (font) {
        myobj.setFont(font);
        itAdvanced = true;
    }

    // maximum chars
    if (fr[0].matchWord("maximumNoCharactersPerLine")) {
        int i;
        if (fr[1].getInt(i)) {
            myobj.setMaximumNoCharactersPerLine(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // text
    if (fr[0].matchWord("text") && fr[1].isString()) {
        myobj.setText(std::string(fr[1].getStr()));
        fr += 2;
        itAdvanced = true;
    }    

    // position
    if (fr[0].matchWord("position")) {
        osg::Vec3 p;
        if (fr[1].getFloat(p.x()) && fr[2].getFloat(p.y()) && fr[3].getFloat(p.z())) {
            myobj.setPosition(p);
            fr += 4;
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

    return itAdvanced;
}

bool Paragraph_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Paragraph &myobj = static_cast<const osgText::Paragraph &>(obj);

    // font
    fw.writeObject(*myobj.getFont());

    // maximum chars
    fw.indent() << "maximumNoCharactersPerLine " << myobj.getMaximumNoCharactersPerLine() << std::endl;

    // text
    fw.indent() << "text " << myobj.getText() << std::endl;

    // position
    osg::Vec3 p = myobj.getPosition();
    fw.indent() << "position " << p.x() << " " << p.y() << " " << p.z() << std::endl;

    // alignment
    fw.indent() << "alignment " << myobj.getAlignment() << std::endl;

    return true;
}


