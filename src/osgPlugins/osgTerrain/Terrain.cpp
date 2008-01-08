#include <osgTerrain/Terrain>

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

bool Terrain_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Terrain_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

osgDB::RegisterDotOsgWrapperProxy Terrain_Proxy
(
    new osgTerrain::Terrain,
    "Terrain",
    "Object Terrain Group",
    Terrain_readLocalData,
    Terrain_writeLocalData
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


bool Terrain_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::Terrain& terrain = static_cast<osgTerrain::Terrain&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
    if (readObject.valid()) itrAdvanced = true;

    osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readObject.get());
    if (locator) terrain.setLocator(locator);

    if (fr.matchSequence("ElevationLayer {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool localAdvanced = false;

            osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
            osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readObject.get());
            if (readObject.valid()) localAdvanced = true;

            unsigned int minLevel=0;
            if (fr.read("MinLevel",minLevel))
            {
                itrAdvanced = true;
            }

            unsigned int maxLevel = MAXIMUM_NUMBER_OF_LEVELS;
            if (fr.read("MaxLevel",maxLevel))
            {
                itrAdvanced = true;
            }

            if (fr.matchSequence("ProxyLayer %s") || fr.matchSequence("ProxyLayer %w") )
            {
                osg::ref_ptr<osg::Object> image = osgDB::readObjectFile(std::string(fr[1].getStr())+".gdal");
                osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(image.get());
                if (proxyLayer)
                {
                    if (locator) proxyLayer->setLocator(locator);
                    if (minLevel!=0) proxyLayer->setMinLevel(minLevel);
                    if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) proxyLayer->setMaxLevel(maxLevel);

                    terrain.setElevationLayer(proxyLayer);
                }
            
                fr += 2;

                localAdvanced = true;
            }
            else
            {
                osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Layer>());
                osgTerrain::Layer* readLayer = dynamic_cast<osgTerrain::Layer*>(readObject.get());
                if (readLayer)
                {
                    if (locator) readLayer->setLocator(locator);
                    if (minLevel!=0) readLayer->setMinLevel(minLevel);
                    if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) readLayer->setMaxLevel(maxLevel);

                    terrain.setElevationLayer(readLayer);
                }

                if (readObject.valid()) localAdvanced = true;
            }

            if (!localAdvanced) ++fr;
        }

        itrAdvanced = true;
    }

    bool firstMatched = false;
    if ((firstMatched = fr.matchSequence("ColorLayer %i {")) || fr.matchSequence("ColorLayer {") )
    {
        unsigned int layerNum = 0;
        if (firstMatched)
        {
            fr[1].getUInt(layerNum);        
            ++fr;
        }

        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool localAdvanced = false;

            osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
            osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readObject.get());
            if (readObject.valid()) localAdvanced = true;

            unsigned int minLevel=0;
            if (fr.read("MinLevel",minLevel))
            {
                itrAdvanced = true;
            }

            unsigned int maxLevel = MAXIMUM_NUMBER_OF_LEVELS;
            if (fr.read("MaxLevel",maxLevel))
            {
                itrAdvanced = true;
            }

            if (fr.matchSequence("ProxyFile %s") || fr.matchSequence("ProxyFile %w") )
            {
                osg::ref_ptr<osg::Object> image = osgDB::readObjectFile(std::string(fr[1].getStr())+".gdal");
                osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(image.get());
                if (proxyLayer)
                {
                    if (locator) proxyLayer->setLocator(locator);
                    if (minLevel!=0) proxyLayer->setMinLevel(minLevel);
                    if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) proxyLayer->setMaxLevel(maxLevel);

                    terrain.setColorLayer(layerNum, proxyLayer);
                }                
            
                fr += 2;

                localAdvanced = true;
            }
            else
            {
                osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Layer>());
                osgTerrain::Layer* readLayer = dynamic_cast<osgTerrain::Layer*>(readObject.get());
                if (readLayer)
                {
                    if (locator) readLayer->setLocator(locator);
                    if (minLevel!=0) readLayer->setMinLevel(minLevel);
                    if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) readLayer->setMaxLevel(maxLevel);

                    terrain.setColorLayer(layerNum, readLayer);
                }

                if (readObject.valid()) localAdvanced = true;
            }

            if (!localAdvanced) ++fr;
        }

        itrAdvanced = true;
    }

    if ((firstMatched = fr.matchSequence("ColorTransferFunction %i {")) || fr.matchSequence("ColorTransferFunction {") )
    {
        unsigned int layerNum = 0;
        if (firstMatched)
        {
             fr[1].getUInt(layerNum);        
            ++fr;
        }

        osg::TransferFunction* tf = readTransferFunction(fr);
        if (tf) terrain.setColorTransferFunction(layerNum, tf);

        itrAdvanced = true;
    }

    if (fr[0].matchWord("ColorFilter"))
    {
        unsigned int layerNum = 0;
        if (fr.matchSequence("ColorFilter %i"))
        {
            fr[1].getUInt(layerNum);
            fr += 2;
        }
        else
        {
            ++fr;
        }

        if (fr[0].matchWord("NEAREST")) terrain.setColorFilter(layerNum, osgTerrain::Terrain::NEAREST);
        else if (fr[0].matchWord("LINEAR")) terrain.setColorFilter(layerNum, osgTerrain::Terrain::LINEAR);

        ++fr;
        itrAdvanced = true;
    }

    readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::TerrainTechnique>());
    if (readObject.valid())
    {
        terrain.setTerrainTechnique(dynamic_cast<osgTerrain::TerrainTechnique*>(readObject.get()));
        itrAdvanced = true;
    }


    return itrAdvanced;
}

bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Terrain& terrain = static_cast<const osgTerrain::Terrain&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    if (terrain.getLocator())
    {
        fw.writeObject(*terrain.getLocator());
    }

    if (terrain.getElevationLayer())
    {
        fw.indent()<<"ElevationLayer {"<<std::endl;

        fw.moveIn();
        
        const osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<const osgTerrain::ProxyLayer*>(terrain.getElevationLayer());
        if (proxyLayer)
        {
            if (!proxyLayer->getFileName().empty())
            {
                const osgTerrain::Locator* locator = proxyLayer->getLocator();
                if (locator && !locator->getDefinedInFile())
                {
                    fw.writeObject(*locator);
                }

                if (proxyLayer->getMinLevel()!=0)
                {
                    fw.indent()<<"MinLevel "<<proxyLayer->getMinLevel()<<std::endl;
                } 

                if (proxyLayer->getMaxLevel()!=MAXIMUM_NUMBER_OF_LEVELS)
                {
                    fw.indent()<<"MaxLevel "<<proxyLayer->getMaxLevel()<<std::endl;
                } 

                fw.indent()<<"ProxyLayer "<<proxyLayer->getFileName()<<std::endl;
            }
        }
        else if (terrain.getElevationLayer())
        {
            fw.writeObject(*terrain.getElevationLayer());
        }

        fw.moveOut();

        fw.indent()<<"}"<<std::endl;
    }

    for(unsigned int i=0; i<terrain.getNumColorLayers(); ++i)
    {
        const osgTerrain::Layer* layer = terrain.getColorLayer(i);
        if (layer)
        {
            if (i>0)
            {
                fw.indent()<<"ColorLayer "<<i<<" {"<<std::endl;
            }
            else
            {
                fw.indent()<<"ColorLayer {"<<std::endl;
            }

            fw.moveIn();
            
            const osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<const osgTerrain::ProxyLayer*>(layer);
            if (proxyLayer)
            {
                const osgTerrain::Locator* locator = proxyLayer->getLocator();
                if (locator && !locator->getDefinedInFile())
                {
                    fw.writeObject(*locator);
                }

                if (proxyLayer->getMinLevel()!=0)
                {
                    fw.indent()<<"MinLevel "<<proxyLayer->getMinLevel()<<std::endl;
                } 

                if (proxyLayer->getMaxLevel()!=MAXIMUM_NUMBER_OF_LEVELS)
                {
                    fw.indent()<<"MaxLevel "<<proxyLayer->getMaxLevel()<<std::endl;
                } 

                if (!proxyLayer->getFileName().empty()) fw.indent()<<"ProxyLayer "<<proxyLayer->getFileName()<<std::endl;
            }
            else if (layer)
            {
                fw.writeObject(*terrain.getColorLayer(i));
            }

            fw.moveOut();

            fw.indent()<<"}"<<std::endl;
        }
    }

    if (terrain.getTerrainTechnique())
    {
        fw.writeObject(*terrain.getTerrainTechnique());
    }    

    fw.precision(prec);
    
    return true;
}
