#ifndef DOTOSGSIM_LIGHTPOINT
#define DOTOSGSIM_LIGHTPOINT

#include <osgSim/LightPoint>
#include <osgDB/Input>
#include <osgDB/Output>

extern bool readLightPoint(osgSim::LightPoint & lp, osgDB::Input &fr);
extern bool writeLightPoint(const osgSim::LightPoint & lp, osgDB::Output &fw);

#endif
