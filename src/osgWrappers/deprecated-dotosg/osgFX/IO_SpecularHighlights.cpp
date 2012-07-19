#include <osgFX/SpecularHighlights>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool SpecularHighlights_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool SpecularHighlights_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(SpecularHighlights_Proxy)
(
    new osgFX::SpecularHighlights,
    "osgFX::SpecularHighlights",
    "Object Node Group osgFX::Effect osgFX::SpecularHighlights",
    SpecularHighlights_readLocalData,
    SpecularHighlights_writeLocalData
);

bool SpecularHighlights_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::SpecularHighlights &myobj = static_cast<osgFX::SpecularHighlights &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("lightNumber")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setLightNumber(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("textureUnit")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setTextureUnit(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("specularColor")) {
        osg::Vec4 w;
        if (fr[1].getFloat(w.x()) && fr[2].getFloat(w.y()) &&
            fr[3].getFloat(w.z()) && fr[4].getFloat(w.w())) {
            myobj.setSpecularColor(w);
            fr += 5;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("specularExponent")) {
        float f;
        if (fr[1].getFloat(f)) {
            myobj.setSpecularExponent(f);
            fr += 2;
            itAdvanced = true;
        }
    }


    return itAdvanced;
}

bool SpecularHighlights_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::SpecularHighlights &myobj = static_cast<const osgFX::SpecularHighlights &>(obj);

    fw.indent() << "lightNumber " << myobj.getLightNumber() << "\n";
    fw.indent() << "textureUnit " << myobj.getTextureUnit() << "\n";
    fw.indent() << "specularColor " << myobj.getSpecularColor() << "\n";
    fw.indent() << "specularExponent " << myobj.getSpecularExponent() << "\n";

    return true;
}
