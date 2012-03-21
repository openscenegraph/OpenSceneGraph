#include "osgSim/DOFTransform"
#include <osg/io_utils>

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgSim;
using namespace osgDB;
using namespace std;

// forward declare functions to use later.
bool DOFTransform_readLocalData(Object& obj, Input& fr);
bool DOFTransform_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(g_DOFTransformProxy)
(
    new osgSim::DOFTransform,
    "DOFTransform",
    "Object Node Transform DOFTransform Group",
    &DOFTransform_readLocalData,
    &DOFTransform_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool DOFTransform_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    DOFTransform& dof = static_cast<DOFTransform&>(obj);

    if (fr.matchSequence("PutMatrix {"))
    {
        fr += 2; // skip over "putMatrix {"
        iteratorAdvanced = true;

        bool matched = true;
        for(int k=0;k<16 && matched;++k)
        {
            matched = fr[k].isFloat();
        }
        if (matched)
        {
            osg::Matrix matrix;
            int k=0;
            double v;
            for(int i=0;i<4;++i)
            {
                for(int j=0;j<4;++j)
                {
                    fr[k].getFloat(v);
                    matrix(i,j)=v;
                    k++;
                }
            }

            dof.setPutMatrix(matrix);
            dof.setInversePutMatrix(Matrix::inverse(matrix));
        }

        fr.advanceToEndOfCurrentBlock();
    }


#define ReadVec3(A,B) {  \
    if (fr[0].matchWord(B) && \
        fr[1].getFloat(vec3[0]) && \
        fr[2].getFloat(vec3[1]) && \
        fr[3].getFloat(vec3[2])) \
    { \
        dof.A(vec3); \
        fr+=4; \
        iteratorAdvanced = true; \
    } \
}

    Vec3 vec3;

    ReadVec3(setMinHPR,"minHPR")
    ReadVec3(setMaxHPR,"maxHPR")
    ReadVec3(setIncrementHPR,"incrementHPR")
    ReadVec3(setCurrentHPR,"currentHPR")

    ReadVec3(setMinTranslate,"minTranslate")
    ReadVec3(setMaxTranslate,"maxTranslate")
    ReadVec3(setIncrementTranslate,"incrementTranslate")
    ReadVec3(setCurrentTranslate,"currentTranslate")

    ReadVec3(setMinScale,"minScale")
    ReadVec3(setMaxScale,"maxScale")
    ReadVec3(setIncrementScale,"incrementScale")
    ReadVec3(setCurrentScale,"currentScale")

    if (fr[0].matchWord("multOrder"))
    {
        if (fr[1].matchWord("PRH")) dof.setHPRMultOrder(DOFTransform::PRH);
        else if(fr[1].matchWord("PHR")) dof.setHPRMultOrder(DOFTransform::PHR);
        else if(fr[1].matchWord("HPR")) dof.setHPRMultOrder(DOFTransform::HPR);
        else if(fr[1].matchWord("HRP")) dof.setHPRMultOrder(DOFTransform::HRP);
        else if(fr[1].matchWord("RHP")) dof.setHPRMultOrder(DOFTransform::RHP);
        else if(fr[1].matchWord("RPH")) dof.setHPRMultOrder(DOFTransform::RPH);
    }


    if (fr.matchSequence("limitationFlags %i"))
    {
        unsigned int flags;
        fr[1].getUInt(flags);
        dof.setLimitationFlags(flags);

        fr += 2;
        iteratorAdvanced = true;

    }

    if (fr[0].matchWord("animationOn"))
    {

        if (fr[1].matchWord("TRUE")) dof.setAnimationOn(true);
        else if (fr[1].matchWord("FALSE")) dof.setAnimationOn(false);

        fr += 2;
        iteratorAdvanced = true;

    }

#undef ReadVec3

    return iteratorAdvanced;
}


bool DOFTransform_writeLocalData(const Object& obj, Output& fw)
{
    const DOFTransform& transform = static_cast<const DOFTransform&>(obj);

    const Matrix& matrix = transform.getPutMatrix();
    fw.indent()<<"PutMatrix {"<<std::endl;
    fw.moveIn();
    fw.indent() << matrix(0,0) << " " << matrix(0,1) << " " << matrix(0,2) << " " << matrix(0,3) << std::endl;
    fw.indent() << matrix(1,0) << " " << matrix(1,1) << " " << matrix(1,2) << " " << matrix(1,3) << std::endl;
    fw.indent() << matrix(2,0) << " " << matrix(2,1) << " " << matrix(2,2) << " " << matrix(2,3) << std::endl;
    fw.indent() << matrix(3,0) << " " << matrix(3,1) << " " << matrix(3,2) << " " << matrix(3,3) << std::endl;
    fw.indent() << "}" << std::endl;
    fw.moveOut();


    fw.indent()<<"minHPR             "<<transform.getMinHPR()<<std::endl;
    fw.indent()<<"maxHPR             "<<transform.getMaxHPR()<<std::endl;
    fw.indent()<<"incrementHPR       "<<transform.getIncrementHPR()<<std::endl;
    fw.indent()<<"currentHPR         "<<transform.getCurrentHPR()<<std::endl;

    fw.indent()<<"minTranslate       "<<transform.getMinTranslate()<<std::endl;
    fw.indent()<<"maxTranslate       "<<transform.getMaxTranslate()<<std::endl;
    fw.indent()<<"incrementTranslate "<<transform.getIncrementTranslate()<<std::endl;
    fw.indent()<<"currentTranslate   "<<transform.getCurrentTranslate()<<std::endl;

    fw.indent()<<"minScale           "<<transform.getMinScale()<<std::endl;
    fw.indent()<<"maxScale           "<<transform.getMaxScale()<<std::endl;
    fw.indent()<<"incrementScale     "<<transform.getIncrementScale()<<std::endl;
    fw.indent()<<"currentScale       "<<transform.getCurrentScale()<<std::endl;


    const char* mOrderStr[] = {"PRH", "PHR", "HPR", "HRP", "RPH", "RHP"};
    fw.indent()<<"multOrder          "<<mOrderStr[transform.getHPRMultOrder()]<<std::endl;

    fw.indent()<<"limitationFlags    0x"<<hex<<transform.getLimitationFlags()<<dec<<std::endl;

    fw.indent()<<"animationOn        ";
    if (transform.getAnimationOn()) fw<<"TRUE"<<std::endl;
    else fw<<"FALSE"<<std::endl;

    return true;
}
