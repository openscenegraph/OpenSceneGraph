#include "osg/CoordinateSystemNode"
#include "osg/Notify"

#include "osgDB/Registry"
#include "osgDB/Input"
#include "osgDB/Output"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool EllipsoidModel_readLocalData(Object& obj, Input& fr);
bool EllipsoidModel_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(EllipsoidModel)
(
    new osg::EllipsoidModel,
    "EllipsoidModel",
    "Object EllipsoidModel",
    &EllipsoidModel_readLocalData,
    &EllipsoidModel_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool EllipsoidModel_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    EllipsoidModel& em = static_cast<EllipsoidModel&>(obj);

    if (fr.matchSequence("RadiusEquator %f"))
    {
        double radius;
        fr[1].getFloat(radius);
        em.setRadiusEquator(radius);
        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("RadiusPolar %f"))
    {
        double radius;
        fr[1].getFloat(radius);
        em.setRadiusPolar(radius);
        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool EllipsoidModel_writeLocalData(const Object& obj, Output& fw)
{
    const EllipsoidModel& em = static_cast<const EllipsoidModel&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    fw.indent()<<"RadiusEquator "<<em.getRadiusEquator()<<std::endl;
    fw.indent()<<"RadiusPolar "<<em.getRadiusPolar()<<std::endl;

    fw.precision(prec);

    return true;
}
