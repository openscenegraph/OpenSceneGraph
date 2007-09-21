#include <osgViewer/View>

#include <iostream>
#include <string>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ReadFile>
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
    bool iteratorAdvanced = false;

    bool matchedFirst = false;
    if ((matchedFirst = fr.matchSequence("setUpViewFor3DSphericalDisplay {")) ||
        fr.matchSequence("setUpViewForPanoramicSphericalDisplay {"))
    {
        double radius=1.0;
        double collar=0.45;
        unsigned int screenNum=0;
        std::string filename;
        osg::Image* intensityMap=0;
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool local_itrAdvanced = false;
            if (fr.read("radius",radius)) local_itrAdvanced = true;
            if (fr.read("collar",collar)) local_itrAdvanced = true;
            if (fr.read("screenNum",screenNum)) local_itrAdvanced = true;
            if (fr.read("intensityMap",filename)) local_itrAdvanced = true;
            
            if (!local_itrAdvanced) ++fr;
        }
        
        // skip trainling '}'
        ++fr;
        
        iteratorAdvanced = true;
        
        if (!filename.empty())
        {
            intensityMap = osgDB::readImageFile(filename);
        }

        if (matchedFirst) view.setUpViewFor3DSphericalDisplay(radius, collar, screenNum, intensityMap);
        else view.setUpViewForPanoramicSphericalDisplay(radius, collar, screenNum, intensityMap);
    }

    int x = 0;
    int y = 0;
    int width = 128;
    int height = 1024;
    unsigned int screenNum = 0;
    
    if (fr.read("setUpViewOnSingleScreen",screenNum))
    {
        view.setUpViewOnSingleScreen(screenNum);
        iteratorAdvanced = true;
    }
    
    if (fr.read("setUpViewAcrossAllScreens"))
    {
        view.setUpViewAcrossAllScreens();
        iteratorAdvanced = true;
    }
    
    if (fr.read("setUpViewInWindow",x,y,width,height,screenNum))
    {
        view.setUpViewInWindow(x, y, width, height, screenNum);
    }
    
    if (fr.read("setUpViewInWindow",x,y,width,height))
    {
        view.setUpViewInWindow(x, y, width, height);
    }


    osg::ref_ptr<osg::Object> readObject;
    while((readObject=fr.readObjectOfType(osgDB::type_wrapper<osg::Camera>())).valid())
    {
        view.setCamera(static_cast<osg::Camera*>(readObject.get()));
        iteratorAdvanced = true;
    }
    
    if (fr.matchSequence("Slaves {"))
    {
        int entry = fr[0].getNoNestedBrackets();

        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            readObject = fr.readObjectOfType(osgDB::type_wrapper<osg::Camera>());
            if (readObject.valid()) view.addSlave(static_cast<osg::Camera*>(readObject.get()));
            else ++fr;
        }
        
        // skip trainling '}'
        ++fr;
        
        iteratorAdvanced = true;

    }
    
    return iteratorAdvanced;
}

bool View_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgViewer::View& view = static_cast<const osgViewer::View&>(obj);

    osg::notify(osg::NOTICE)<<"View_writeLocalData"<<std::endl;

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
