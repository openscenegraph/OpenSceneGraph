
#include <osgParticle/LinearInterpolator>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

bool  LinearInterpolator_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool  LinearInterpolator_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy  LinearInterpolator_Proxy
(
    new osgParticle::LinearInterpolator,
    "LinearInterpolator",
    "Object Interpolator LinearInterpolator",
    LinearInterpolator_readLocalData,
    LinearInterpolator_writeLocalData
);

bool LinearInterpolator_readLocalData(osg::Object &, osgDB::Input &)
{
    return false;
}

bool LinearInterpolator_writeLocalData(const osg::Object &, osgDB::Output &)
{
    return false;
}
