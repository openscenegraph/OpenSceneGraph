#include <osgViewer/Viewer>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool Viewer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Viewer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Viewer_Proxy
(
    new osgViewer::Viewer,
    "Viewer",
    "Object Viewer View",
    Viewer_readLocalData,
    Viewer_writeLocalData
);

bool Viewer_readLocalData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
    // osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&obj);
    bool iteratorAdvanced = false;

    return iteratorAdvanced;
}

bool Viewer_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    // const osgViewer::Viewer* viewer = dynamic_cast<const osgViewer::Viewer*>(&obj);
    return true;
}
