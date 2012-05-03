#include <osgViewer/CompositeViewer>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool CompositeViewer_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool CompositeViewer_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(CompositeViewer_Proxy)
(
    new osgViewer::CompositeViewer,
    "CompositeViewer",
    "Object CompositeViewer",
    CompositeViewer_readLocalData,
    CompositeViewer_writeLocalData
);

bool CompositeViewer_readLocalData(osg::Object& /*obj*/, osgDB::Input& /*fr*/)
{
    // osgViewer::CompositeViewer* compositeViewer = dynamic_cast<osgViewer::CompositeViewer*>(&obj);
    bool iteratorAdvanced = false;

    osg::notify(osg::NOTICE)<<"CompositeViewer_readLocalData"<<std::endl;

    return iteratorAdvanced;
}

bool CompositeViewer_writeLocalData(const osg::Object& /*obj*/, osgDB::Output& /*fw*/)
{
    // const osgViewer::CompositeViewer* compositeViewer = dynamic_cast<const osgViewer::CompositeViewer*>(&obj);

    osg::notify(osg::NOTICE)<<"CompositeViewer_writeLocalData"<<std::endl;

    return true;
}
