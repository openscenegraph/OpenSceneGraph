#include <osgFX/BumpMapping>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool BumpMapping_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool BumpMapping_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(BumpMapping_Proxy)
(
    new osgFX::BumpMapping,
    "osgFX::BumpMapping",
    "Object Node Group osgFX::Effect osgFX::BumpMapping",
    BumpMapping_readLocalData,
    BumpMapping_writeLocalData
);

bool BumpMapping_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::BumpMapping &myobj = static_cast<osgFX::BumpMapping &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("lightNumber")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setLightNumber(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("diffuseUnit")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setDiffuseTextureUnit(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("normalMapUnit")) {
        int n;
        if (fr[1].getInt(n)) {
            myobj.setNormalMapTextureUnit(n);
            fr += 2;
            itAdvanced = true;
        }
    }

    osg::ref_ptr<osg::Texture2D> diffuse_tex = static_cast<osg::Texture2D *>(fr.readObjectOfType(osgDB::type_wrapper<osg::Texture2D>()));
    if (diffuse_tex.valid()) {
        myobj.setOverrideDiffuseTexture(diffuse_tex.get());
        itAdvanced = true;
    }

    osg::ref_ptr<osg::Texture2D> normal_tex = static_cast<osg::Texture2D *>(fr.readObjectOfType(osgDB::type_wrapper<osg::Texture2D>()));
    if (normal_tex.valid()) {
        myobj.setOverrideNormalMapTexture(normal_tex.get());
        itAdvanced = true;
    }

    return itAdvanced;
}

bool BumpMapping_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::BumpMapping &myobj = static_cast<const osgFX::BumpMapping &>(obj);

    fw.indent() << "lightNumber " << myobj.getLightNumber() << "\n";
    fw.indent() << "diffuseUnit " << myobj.getDiffuseTextureUnit() << "\n";
    fw.indent() << "normalMapUnit " << myobj.getNormalMapTextureUnit() << "\n";

    if (myobj.getOverrideDiffuseTexture()) {
        fw.writeObject(*myobj.getOverrideDiffuseTexture());
    }

    if (myobj.getOverrideNormalMapTexture()) {
        fw.writeObject(*myobj.getOverrideNormalMapTexture());
    }

    return true;
}
