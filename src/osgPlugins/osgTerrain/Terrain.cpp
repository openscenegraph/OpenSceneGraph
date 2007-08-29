#include <osgTerrain/Terrain>
#include <osgTerrain/GeometryTechnique>

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
    "Object Terrain Group ",
    Terrain_readLocalData,
    Terrain_writeLocalData
);

osgTerrain::Layer* readLayer(osgDB::Input& fr)
{
    osg::ref_ptr<osgTerrain::Layer> layer;
    osg::ref_ptr<osgTerrain::Locator> locator;

    int entry = fr[0].getNoNestedBrackets();

    fr += 2;

    while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool itrAdvanced = false;
        
        if (fr.matchSequence("CompositeLayer {"))
        {
            
            osg::ref_ptr<osgTerrain::CompositeLayer> cl = new osgTerrain::CompositeLayer;

            int local_entry = fr[0].getNoNestedBrackets();

            fr += 2;
            while (!fr.eof() && fr[0].getNoNestedBrackets()>local_entry)
            {
                osgTerrain::Layer* layer = readLayer(fr);
                if (layer) cl->addLayer(layer);                

                ++fr;
            }
            
            layer = cl.get();
        
            itrAdvanced = true;
            
            ++fr;

        }

        if (fr.matchSequence("ProxyLayer %w") || fr.matchSequence("ProxyLayer %w"))
        {
            osg::ref_ptr<osg::Object> image = osgDB::readObjectFile(std::string(fr[1].getStr())+".gdal");
            osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(image.get());
            if (proxyLayer)
            {
                layer = proxyLayer;
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Warning: Failed to create ProxyLayer "<<fr[1].getStr()<<std::endl;
            }
                        
            fr += 2;
            itrAdvanced = true;
        }

        if (fr.matchSequence("Images {") || fr.matchSequence("images {"))
        {
            osg::ref_ptr<osgTerrain::CompositeLayer> cl = new osgTerrain::CompositeLayer;
            
            int local_entry = fr[0].getNoNestedBrackets();

            fr += 2;
            while (!fr.eof() && fr[0].getNoNestedBrackets()>local_entry)
            {
                cl->addLayer(fr[0].getStr());
                ++fr;
            }
            
            layer = cl.get();
        
            itrAdvanced = true;
            
            ++fr;
        }

        if (fr.matchSequence("Image %w") || fr.matchSequence("image %w") || 
            fr.matchSequence("Image %s") || fr.matchSequence("image %s"))
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(fr[1].getStr());
            if (image.valid())
            {
                osg::ref_ptr<osgTerrain::ImageLayer> imagelayer = new osgTerrain::ImageLayer;
                imagelayer->setImage(image.get());

                layer = imagelayer.get();
            }
                        
            fr += 2;
            itrAdvanced = true;
        }

        if (fr.matchSequence("HeightField %w") || fr.matchSequence("HeightField  %s"))
        {
            osg::ref_ptr<osg::HeightField> hf = osgDB::readHeightFieldFile(fr[1].getStr());
            if (hf.valid())
            {
                osg::ref_ptr<osgTerrain::HeightFieldLayer> hflayer = new osgTerrain::HeightFieldLayer;
                hflayer->setHeightField(hf.get());

                layer = hflayer.get();
            }
                        
            fr += 2;
            itrAdvanced = true;
        }

        if (fr.matchSequence("Locator %w %f %f %f"))
        {
            
            double x,y,w,h;
            fr[2].getFloat(x);
            fr[3].getFloat(y);
            fr[4].getFloat(w);
            fr[5].getFloat(h);
            
            locator = new osgTerrain::Locator;
            
            if (fr[1].matchWord("GEOCENTRIC")) locator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
            else if (fr[1].matchWord("GEOGRAPHIC")) locator->setCoordinateSystemType(osgTerrain::Locator::GEOGRAPHIC);
            else locator->setCoordinateSystemType(osgTerrain::Locator::PROJECTED);
            locator->setExtents(x,y,x+w,y+h);

            fr += 6;
            itrAdvanced = true;
        }

        if (fr.matchSequence("Locator {"))
        {
            locator = new osgTerrain::Locator;

            int local_entry = fr[0].getNoNestedBrackets();

            fr += 2;
            while (!fr.eof() && fr[0].getNoNestedBrackets()>local_entry)
            {
                bool localAdvanced = false;

                if (fr.matchSequence("Format %w") || fr.matchSequence("Format %s") )
                {
                    locator->setFormat(fr[1].getStr());

                    fr += 2;
                    localAdvanced = true;
                }

                if (fr.matchSequence("CoordinateSystemType %w"))
                {
                    if (fr[1].matchWord("GEOCENTRIC")) locator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
                    else if (fr[1].matchWord("GEOGRAPHIC")) locator->setCoordinateSystemType(osgTerrain::Locator::GEOGRAPHIC);
                    else locator->setCoordinateSystemType(osgTerrain::Locator::PROJECTED);

                    fr += 2;
                    localAdvanced = true;
                }


                if (fr.matchSequence("CoordinateSystem %w") || fr.matchSequence("CoordinateSystem %s") )
                {
                    locator->setCoordinateSystem(fr[1].getStr());

                    fr += 2;
                    localAdvanced = true;
                }

                if (fr.matchSequence("Extents %f %f %f %f"))
                {
                    double minX,minY,maxX,maxY;
                    fr[1].getFloat(minX);
                    fr[2].getFloat(minY);
                    fr[3].getFloat(maxX);
                    fr[4].getFloat(maxY);
                    
                    locator->setExtents(minX, minY, maxX, maxY);
                    
                    fr += 5;
                    localAdvanced = true;
                }
                
                if (!localAdvanced) ++fr;
            }
            
            itrAdvanced = true;
            
            ++fr;
        }

        if (fr.matchSequence("EllipsoidLocator %f %f %f %f"))
        {
            double x,y,w,h;
            fr[1].getFloat(x);
            fr[2].getFloat(y);
            fr[3].getFloat(w);
            fr[4].getFloat(h);
        
            locator = new osgTerrain::Locator;
            locator->setCoordinateSystemType(osgTerrain::Locator::GEOCENTRIC);
            locator->setExtents(x,y,x+w,y+h);
            
            fr += 5;
            itrAdvanced = true;
        }

        if (fr.matchSequence("CartesianLocator %f %f %f %f"))
        {
            double x,y,w,h;
            fr[1].getFloat(x);
            fr[2].getFloat(y);
            fr[3].getFloat(w);
            fr[4].getFloat(h);
        
            locator = new osgTerrain::Locator;
            locator->setCoordinateSystemType(osgTerrain::Locator::PROJECTED);
            locator->setExtents(x,y,x+w,y+h);
         
            fr += 5;
            itrAdvanced = true;
        }

        if (!itrAdvanced)
        {
            if (fr[0].getStr()) osg::notify(osg::NOTICE)<<"Layer - unreconised token : "<<fr[0].getStr() << std::endl;
            ++fr;
        }
    }

    // step over trailing }
    ++fr;
    
    if (layer.valid() && locator.valid())
    {
        layer->setLocator(locator.get());
    }
    
    return layer.release();
}

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

    if (fr.matchSequence("ElevationLayer {"))
    {
        osgTerrain::Layer* layer = readLayer(fr);

        if (layer) terrain.setElevationLayer(layer);

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

        osgTerrain::Layer* layer = readLayer(fr);
        if (layer) terrain.setColorLayer(layerNum, layer);

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

#if 1
    if (!(terrain.getTerrainTechnique()))
    {
        terrain.setTerrainTechnique(new osgTerrain::GeometryTechnique);
    }
#endif
    return itrAdvanced;
}

bool writeLocator(const osgTerrain::Locator& locator, osgDB::Output& fw)
{
    
    if (locator.getCoordinateSystem().empty())
    {    
        fw.indent()<<"Locator ";
        switch(locator.getCoordinateSystemType())
        {
            case(osgTerrain::Locator::GEOCENTRIC):
            {        
                fw<<"GEOCENTRIC";
                break;
            }
            case(osgTerrain::Locator::GEOGRAPHIC):
            {        
                fw<<"GEOGRPAHIC";
                break;
            }
            case(osgTerrain::Locator::PROJECTED):
            {        
                fw<<"PROJECTED";
                break;
            }
        }    

        fw<<" "<<locator.getMinX()<<" "<<locator.getMinY()<<" "<<locator.getMaxX()<<" "<<locator.getMaxY()<<std::endl;
    }
    else
    {
        fw.indent()<<"Locator {"<<std::endl;
        fw.moveIn();

        if (!locator.getFormat().empty()) fw.indent()<<"Format \""<<locator.getFormat()<<"\""<<std::endl;
        if (!locator.getCoordinateSystem().empty()) fw.indent()<<"CoordinateSystem \""<<locator.getCoordinateSystem()<<"\""<<std::endl;

        fw.indent()<<"CoordinateSystemType ";
        switch(locator.getCoordinateSystemType())
        {
            case(osgTerrain::Locator::GEOCENTRIC):
            {        
                fw<<"GEOCENTRIC"<<std::endl;
                break;
            }
            case(osgTerrain::Locator::GEOGRAPHIC):
            {        
                fw<<"GEOGRAPHIC"<<std::endl;
                break;
            }
            case(osgTerrain::Locator::PROJECTED):
            {        
                fw<<"PROJECTED"<<std::endl;;
                break;
            }
        }    

        fw.indent()<<"Extents "<<locator.getMinX()<<" "<<locator.getMinY()<<" "<<locator.getMaxX()<<" "<<locator.getMaxY()<<std::endl;

        fw.moveOut();
        fw.indent()<<"}"<<std::endl;

    }
    return false;
}

bool writeLayer(const osgTerrain::Layer& layer, osgDB::Output& fw)
{
    if (layer.getLocator()) 
    {
        writeLocator(*layer.getLocator(),fw);
    }

    const osgTerrain::ImageLayer* imageLayer = dynamic_cast<const osgTerrain::ImageLayer*>(&layer);
    if (imageLayer)
    {
        fw.indent()<<"Image "<<imageLayer->getFileName()<<std::endl;
        return true;
    }
    
    const osgTerrain::HeightFieldLayer* hfLayer = dynamic_cast<const osgTerrain::HeightFieldLayer*>(&layer);
    if (hfLayer)
    {
        fw.indent()<<"HeightField "<<hfLayer->getFileName()<<std::endl;
        return true;
    }
    
    
    const osgTerrain::CompositeLayer* compositeLayer = dynamic_cast<const osgTerrain::CompositeLayer*>(&layer);
    if (compositeLayer)
    {
        fw.indent()<<"CompositeLayer {"<<std::endl;
        fw.moveIn();
        for(unsigned int i=0; i<compositeLayer->getNumLayers();++i)
        {
            if (compositeLayer->getLayer(i))
            {
                writeLayer(*(compositeLayer->getLayer(i)), fw);
            }
            else if (!compositeLayer->getFileName(i).empty())
            {
                fw.indent()<<"image "<<compositeLayer->getFileName(i)<<std::endl;
            }
        }
        fw.moveOut();
        fw.indent()<<"}"<<std::endl;
        
        return true;
        
    }
    
    return false;
}

bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Terrain& terrain = static_cast<const osgTerrain::Terrain&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    if (terrain.getLocator()) 
    {
        writeLocator(*terrain.getLocator(),fw);
    }

    if (terrain.getElevationLayer())
    {
        fw.indent()<<"ElevationLayer {"<<std::endl;

        fw.moveIn();
        
        writeLayer(*terrain.getElevationLayer(), fw);
        
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

            writeLayer(*layer, fw);

            fw.moveOut();

            fw.indent()<<"}"<<std::endl;
        }
    }
    
    fw.precision(prec);
    
    return true;
}
