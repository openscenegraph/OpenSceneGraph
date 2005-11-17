#include <osgFX/Cartoon>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool Cartoon_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Cartoon_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Cartoon_Proxy
(
    new osgFX::Cartoon,
    "osgFX::Cartoon",
    "Object Node Group osgFX::Effect osgFX::Cartoon",
    Cartoon_readLocalData,
    Cartoon_writeLocalData
);

bool Cartoon_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::Cartoon &myobj = static_cast<osgFX::Cartoon &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("lightNumber")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setLightNumber(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("outlineColor")) {
        osg::Vec4 w;
        if (fr[1].getFloat(w.x()) && fr[2].getFloat(w.y()) && 
            fr[3].getFloat(w.z()) && fr[4].getFloat(w.w())) {
            myobj.setOutlineColor(w);
            fr += 5;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("outlineLineWidth")) {
        float f;
        if (fr[1].getFloat(f)) {
            myobj.setOutlineLineWidth(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool Cartoon_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::Cartoon &myobj = static_cast<const osgFX::Cartoon &>(obj);

    fw.indent() << "lightNumber " << myobj.getLightNumber() << "\n";
    fw.indent() << "outlineColor " << myobj.getOutlineColor() << "\n";
    fw.indent() << "outlineLineWidth " << myobj.getOutlineLineWidth() << "\n";

    return true;
}
