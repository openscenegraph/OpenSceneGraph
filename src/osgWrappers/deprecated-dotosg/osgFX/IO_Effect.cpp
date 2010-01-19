#include <osgFX/Effect>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool Effect_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Effect_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Effect_Proxy)
(
    0,
    "osgFX::Effect",
    "Object Node Group osgFX::Effect",
    Effect_readLocalData,
    Effect_writeLocalData
);

bool Effect_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::Effect &myobj = static_cast<osgFX::Effect &>(obj);
    bool itAdvanced = false;

    if (fr[0].matchWord("enabled")) {
        if (fr[1].matchWord("FALSE")) {
            myobj.setEnabled(false);
        } else {
            myobj.setEnabled(true);
        }
        fr += 2;
        itAdvanced = true;
    }

    if (fr[0].matchWord("selectedTechnique")) {
        if (fr[1].matchWord("AUTO_DETECT")) {
            myobj.selectTechnique(osgFX::Effect::AUTO_DETECT);
            fr += 2;
            itAdvanced = true;
        } else {
            int i;
            if (fr[1].getInt(i)) {
                myobj.selectTechnique(i);
                fr += 2;
                itAdvanced = true;
            }
        }
    }

    return itAdvanced;
}

bool Effect_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::Effect &myobj = static_cast<const osgFX::Effect &>(obj);

    fw.indent() << "enabled " << (myobj.getEnabled() ? "TRUE" : "FALSE") << "\n";
    fw.indent() << "selectedTechnique ";
    if (myobj.getSelectedTechnique() == osgFX::Effect::AUTO_DETECT) {
        fw << "AUTO_DETECT\n";
    } else {
        fw << myobj.getSelectedTechnique() << "\n";
    }

    return true;
}
