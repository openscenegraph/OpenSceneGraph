
#include <osgParticle/FluidProgram>
#include <osgParticle/Operator>

#include <iostream>

#include <osg/Vec3>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  FluidProgram_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  FluidProgram_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  FluidProgram_Proxy
(
    new osgParticle::FluidProgram,
    "FluidProgram",
    "Object Node ParticleProcessor osgParticle::Program FluidProgram",
    FluidProgram_readLocalData,
    FluidProgram_writeLocalData
);

bool FluidProgram_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgParticle::FluidProgram &myobj = static_cast<osgParticle::FluidProgram &>(obj);
    bool itAdvanced = false;

    osg::Vec3 vec;
    float f;

    if (fr[0].matchWord("acceleration")) {
        if (fr[1].getFloat(vec.x()) && fr[2].getFloat(vec.y()) && fr[3].getFloat(vec.z())) {
            myobj.setAcceleration(vec);
            fr += 4;
            itAdvanced = true;
        }
    }
    
    if (fr[0].matchWord("viscosity")) {
        if (fr[1].getFloat(f)) {
            myobj.setFluidViscosity(f);
            fr += 2;
            itAdvanced = true;
        }
    }
    
    if (fr[0].matchWord("density")) {
        if (fr[1].getFloat(f)) {
            myobj.setFluidDensity(f);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("wind")) {
        if (fr[1].getFloat(vec.x()) && fr[2].getFloat(vec.y()) && fr[3].getFloat(vec.z())) {
            myobj.setWind(vec);
            fr += 4;
            itAdvanced = true;
        }
    }

    return itAdvanced;
}

bool FluidProgram_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgParticle::FluidProgram &myobj = static_cast<const osgParticle::FluidProgram &>(obj);

    osg::Vec3 vec;
    float f;
    
    vec = myobj.getAcceleration();
    fw.indent() << "acceleration " << vec << std::endl;
    
    f = myobj.getFluidViscosity();
    fw.indent() << "viscosity " << f << std::endl;
    
    f = myobj.getFluidDensity();
    fw.indent() << "density " << f << std::endl;
        
    vec = myobj.getWind();
    fw.indent() << "wind " << vec << std::endl;

    return true;
}
