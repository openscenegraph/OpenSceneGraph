#include <osgFX/MultiTextureControl>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool MultiTextureControl_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool MultiTextureControl_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(MultiTextureControl_Proxy)
(
    new osgFX::MultiTextureControl,
    "osgFX::MultiTextureControl",
    "Object Node osgFX::MultiTextureControl Group",
    MultiTextureControl_readLocalData,
    MultiTextureControl_writeLocalData
);

bool MultiTextureControl_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgFX::MultiTextureControl &mtc = static_cast<osgFX::MultiTextureControl &>(obj);
    bool iteratorAdvanced = false;

    bool matchFirst = false;
    if ((matchFirst=fr.matchSequence("TextureWeights {")) || fr.matchSequence("TextureWeights %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();
        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            fr += 3;
        }

        float weight=0.0f;
        unsigned int i=0;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(weight))
            {
                mtc.setTextureWeight(i,weight);
                ++fr;
                ++i;
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }

    return iteratorAdvanced;
}

bool MultiTextureControl_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgFX::MultiTextureControl &mtc = static_cast<const osgFX::MultiTextureControl &>(obj);

    fw.indent() << "TextureWeights "<<mtc.getNumTextureWeights()<<" {"<< std::endl;
    fw.moveIn();

    for(unsigned int i=0; i<mtc.getNumTextureWeights();++i)
    {
        fw.indent() << mtc.getTextureWeight(i)<<std::endl;
    }
    fw.moveOut();
    fw.indent() << "}"<< std::endl;


    return true;
}
