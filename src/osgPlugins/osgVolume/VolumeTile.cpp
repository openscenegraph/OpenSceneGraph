#include <osgVolume/VolumeTile>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool VolumeTile_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool VolumeTile_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy VolumeTile_Proxy
(
    new osgVolume::VolumeTile,
    "VolumeTile",
    "Object Node VolumeTile Group",
    VolumeTile_readLocalData,
    VolumeTile_writeLocalData
);

osg::TransferFunction* readTransferFunction(osgDB::Input& fr)
{
    osg::ref_ptr<osg::TransferFunction1D> tf = new osg::TransferFunction1D;
    
    int entry = fr[0].getNoNestedBrackets();

    fr += 2;

    std::vector<osg::Vec4> colours;

    while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool itrAdvanced = false;
        if (fr.matchSequence("range %f %f"))
        {
            float minValue,maxValue;
            fr[1].getFloat(minValue);
            fr[2].getFloat(maxValue);
        
            tf->setInputRange(minValue,maxValue);
            
            fr += 3;
            itrAdvanced = true;
        }

        if (fr.matchSequence("color %f %f %f %f"))
        {
            float r,g,b,a;
            fr[1].getFloat(r);
            fr[2].getFloat(g);
            fr[3].getFloat(b);
            fr[4].getFloat(a);
        
            colours.push_back(osg::Vec4(r,g,b,a));
            
            fr += 5;
            itrAdvanced = true;
        }

        if (fr.matchSequence("color %f %f %f"))
        {
            float r,g,b;
            fr[1].getFloat(r);
            fr[2].getFloat(g);
            fr[3].getFloat(b);
        
            colours.push_back(osg::Vec4(r,g,b,1.0f));
            
            fr += 5;
            itrAdvanced = true;
        }

        if (!itrAdvanced)
        {
            if (fr[0].getStr()) osg::notify(osg::NOTICE)<<"TransferFunction - unreconised token : "<<fr[0].getStr() << std::endl;
            ++fr;
        }
    }
    

    // step over trailing }
    ++fr;
    
    if (!colours.empty())
    {
        tf->allocate(colours.size());
        for(unsigned int i=0; i<colours.size(); ++i)
        {
            tf->setValue(i, colours[i]);
        }
    }

    if (tf->getNumberCellsX()==0)
    {
        tf->allocate(6);
        tf->setValue(0, osg::Vec4(1.0,1.0,1.0,1.0));
        tf->setValue(1, osg::Vec4(1.0,0.0,1.0,1.0));
        tf->setValue(2, osg::Vec4(1.0,0.0,0.0,1.0));
        tf->setValue(3, osg::Vec4(1.0,1.0,0.0,1.0));
        tf->setValue(4, osg::Vec4(0.0,1.0,1.0,1.0));
        tf->setValue(5, osg::Vec4(0.0,1.0,0.0,1.0));
    }

    return tf.release();
}


bool VolumeTile_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgVolume::VolumeTile& volumeTile = static_cast<osgVolume::VolumeTile&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Locator>());
    if (readObject.valid()) itrAdvanced = true;

    osgVolume::Locator* locator = dynamic_cast<osgVolume::Locator*>(readObject.get());
    if (locator) volumeTile.setLocator(locator);


    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::Layer>());
    if (readObject.valid()) itrAdvanced = true;

    osgVolume::Layer* readLayer = dynamic_cast<osgVolume::Layer*>(readObject.get());
    if (readLayer) volumeTile.setLayer(readLayer);


    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgVolume::VolumeTechnique>());
    if (readObject.valid())
    {
        volumeTile.setVolumeTechnique(dynamic_cast<osgVolume::VolumeTechnique*>(readObject.get()));
        itrAdvanced = true;
    }

    return itrAdvanced;
}

bool VolumeTile_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgVolume::VolumeTile& volumeTile = static_cast<const osgVolume::VolumeTile&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    if (volumeTile.getLocator())
    {
        fw.writeObject(*volumeTile.getLocator());
    }

    if (volumeTile.getLayer())
    {
        fw.writeObject(*volumeTile.getLayer());
    }

    if (volumeTile.getVolumeTechnique())
    {
        fw.writeObject(*volumeTile.getVolumeTechnique());
    }    

    fw.precision(prec);
    
    return true;
}
