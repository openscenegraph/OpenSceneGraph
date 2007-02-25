#include <osgViewer/View>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool View_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool View_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy View_Proxy
(
    new osgViewer::View,
    "View",
    "Object View",
    View_readLocalData,
    View_writeLocalData
);

bool View_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgViewer::View& view = static_cast<osgViewer::View&>(obj);
    bool itAdvanced = false;

    return itAdvanced;
}

bool View_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgViewer::View& view = static_cast<const osgViewer::View&>(obj);

    if (view.getCamera())
    {
        fw.writeObject(*view.getCamera());
    }
    
    if (view.getNumSlaves() != 0)
    {
        fw.indent()<<"Slaves {"<<std::endl;
        fw.moveIn();
    
        for(unsigned int i=0; i<view.getNumSlaves(); ++i)
        {
            const osg::Camera* camera = view.getSlave(i)._camera.get();
            if (camera)
            {
                fw.writeObject(*camera);
            }
        }
        
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
    }
    
    return true;
}
