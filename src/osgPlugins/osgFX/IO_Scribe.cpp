#include <osgFX/Scribe>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool Scribe_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Scribe_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Scribe_Proxy
(
    new osgFX::Scribe,
    "osgFX::Scribe",
    "Object Node Group osgFX::Effect osgFX::Scribe",
    Scribe_readLocalData,
    Scribe_writeLocalData
);

bool Scribe_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::Scribe &myobj = static_cast<osgFX::Scribe &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("wireframeColor")) {
        osg::Vec4 w;
        if (fr[1].getFloat(w.x()) && fr[2].getFloat(w.y()) && 
            fr[3].getFloat(w.z()) && fr[4].getFloat(w.w())) {
            myobj.setWireframeColor(w);
            fr += 5;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("wireframeLineWidth")) {
        float f;
        if (fr[1].getFloat(f)) {
            myobj.setWireframeLineWidth(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool Scribe_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::Scribe &myobj = static_cast<const osgFX::Scribe &>(obj);

    fw.indent() << "wireframeColor " << myobj.getWireframeColor() << "\n";
    fw.indent() << "wireframeLineWidth " << myobj.getWireframeLineWidth() << "\n";

    return true;
}
