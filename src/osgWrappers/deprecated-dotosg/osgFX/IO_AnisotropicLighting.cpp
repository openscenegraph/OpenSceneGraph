#include <osgFX/AnisotropicLighting>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool AnisotropicLighting_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool AnisotropicLighting_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(AnisotropicLighting_Proxy)
(
    new osgFX::AnisotropicLighting,
    "osgFX::AnisotropicLighting",
    "Object Node Group osgFX::Effect osgFX::AnisotropicLighting",
    AnisotropicLighting_readLocalData,
    AnisotropicLighting_writeLocalData
);

bool AnisotropicLighting_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::AnisotropicLighting &myobj = static_cast<osgFX::AnisotropicLighting &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("lightNumber")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setLightNumber(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("lightingMapFileName") && fr[1].isString()) {
        osg::ref_ptr<osg::Image> lmap = fr.readImage(fr[1].getStr());
        if (lmap) {
            myobj.setLightingMap(lmap.get());
        }
        fr += 2;
        itAdvanced = true;
    }

    return itAdvanced;
}

bool AnisotropicLighting_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::AnisotropicLighting &myobj = static_cast<const osgFX::AnisotropicLighting &>(obj);

    fw.indent() << "lightNumber " << myobj.getLightNumber() << "\n";

    const osg::Image *lmap = myobj.getLightingMap();
    if (lmap) {
        if (!lmap->getFileName().empty()) {
            fw.indent() << "lightingMapFileName \"" << lmap->getFileName() << "\"\n";
        }
    }

    return true;
}
