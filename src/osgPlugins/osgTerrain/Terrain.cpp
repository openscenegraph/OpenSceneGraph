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

        if (fr.matchSequence("EllipsoidLocator %f %f %f %f"))
        {
            double x,y,w,h;
            fr[1].getFloat(x);
            fr[2].getFloat(y);
            fr[3].getFloat(w);
            fr[4].getFloat(h);
        
            locator = new osgTerrain::EllipsoidLocator(x,y,w,h,0);
            
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
        
            locator = new osgTerrain::CartesianLocator(x,y,w,h,0);
            
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

    if (!(terrain.getTerrainTechnique()))
    {
        terrain.setTerrainTechnique(new osgTerrain::GeometryTechnique);
    }

    return itrAdvanced;
}

bool Terrain_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::Terrain& terrain = static_cast<const osgTerrain::Terrain&>(obj);
    
    return true;
}
