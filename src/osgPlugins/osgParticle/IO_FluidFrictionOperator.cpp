
#include <osgParticle/FluidFrictionOperator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <iostream>

bool  FluidFrictionOperator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  FluidFrictionOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  FluidFrictionOperator_Proxy
(
    new osgParticle::FluidFrictionOperator,
    "FluidFrictionOperator",
    "Object Operator FluidFrictionOperator",
    FluidFrictionOperator_readLocalData,
    FluidFrictionOperator_writeLocalData
);

bool FluidFrictionOperator_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::FluidFrictionOperator &aop = static_cast<osgParticle::FluidFrictionOperator &>(obj);
    bool itAdvanced = false;

    float f;

    if (fr[0].matchWord("fluidDensity")) {
        if (fr[1].getFloat(f)) {
            aop.setFluidDensity(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("fluidViscosity")) {
        if (fr[1].getFloat(f)) {
            aop.setFluidViscosity(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("overrideRadius")) {
        if (fr[1].getFloat(f)) {
            aop.setOverrideRadius(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool FluidFrictionOperator_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::FluidFrictionOperator &aop = static_cast<const osgParticle::FluidFrictionOperator &>(obj);
    fw.indent() << "fluidDensity " << aop.getFluidDensity() << std::endl;
    fw.indent() << "fluidViscosity " << aop.getFluidViscosity() << std::endl;
    fw.indent() << "overrideRadius " << aop.getOverrideRadius();
    return true;
}
