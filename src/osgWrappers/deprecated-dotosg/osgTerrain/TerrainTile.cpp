#include <osgTerrain/TerrainTile>

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

bool TerrainTile_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TerrainTile_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(TerrainTile_Proxy)
(
    new osgTerrain::TerrainTile,
    "TerrainTile",
    "Object Node TerrainTile Group",
    TerrainTile_readLocalData,
    TerrainTile_writeLocalData
);

bool TerrainTile_readLocalData(osg::Object& obj, osgDB::Input &fr)
{
    osgTerrain::TerrainTile& terrainTile = static_cast<osgTerrain::TerrainTile&>(obj);

    bool itrAdvanced = false;

    osg::ref_ptr<osg::Object> readLocatorObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
    if (readLocatorObject.valid()) itrAdvanced = true;

    std::string blendingPolicy;
    if (fr.read("BlendingPolicy",blendingPolicy))
    {
        if (blendingPolicy == "INHERIT") terrainTile.setBlendingPolicy(osgTerrain::TerrainTile::INHERIT);
        else if (blendingPolicy == "DO_NOT_SET_BLENDING") terrainTile.setBlendingPolicy(osgTerrain::TerrainTile::DO_NOT_SET_BLENDING);
        else if (blendingPolicy == "ENABLE_BLENDING") terrainTile.setBlendingPolicy(osgTerrain::TerrainTile::ENABLE_BLENDING);
        else if (blendingPolicy == "ENABLE_BLENDING_WHEN_ALPHA_PRESENT") terrainTile.setBlendingPolicy(osgTerrain::TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT);
    }

    osgTerrain::Locator* tileLocator = dynamic_cast<osgTerrain::Locator*>(readLocatorObject.get());
    if (tileLocator) terrainTile.setLocator(tileLocator);

    if (fr.matchSequence("ElevationLayer {"))
    {
        int entry = fr[0].getNoNestedBrackets();
        fr += 2;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool localAdvanced = false;

            osg::ref_ptr<osg::Object> readElevationLocatorObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
            osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readElevationLocatorObject.get());
            if (readElevationLocatorObject.valid()) localAdvanced = true;

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
                osgTerrain::ProxyLayer* proxyLayer = new osgTerrain::ProxyLayer;
                proxyLayer->setFileName(fr[1].getStr());

                if (locator) proxyLayer->setLocator(locator);
                if (minLevel!=0) proxyLayer->setMinLevel(minLevel);
                if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) proxyLayer->setMaxLevel(maxLevel);

                terrainTile.setElevationLayer(proxyLayer);

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

                    terrainTile.setElevationLayer(readLayer);
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

            osg::ref_ptr<osg::Object> readElevationLocatorObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::Locator>());
            osgTerrain::Locator* locator = dynamic_cast<osgTerrain::Locator*>(readElevationLocatorObject.get());
            if (readElevationLocatorObject.valid()) localAdvanced = true;

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
                osg::ref_ptr<osg::Object> image = osgDB::readRefObjectFile(std::string(fr[1].getStr())+".gdal");
                osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<osgTerrain::ProxyLayer*>(image.get());
                if (proxyLayer)
                {
                    if (locator) proxyLayer->setLocator(locator);
                    if (minLevel!=0) proxyLayer->setMinLevel(minLevel);
                    if (maxLevel!=MAXIMUM_NUMBER_OF_LEVELS) proxyLayer->setMaxLevel(maxLevel);

                    terrainTile.setColorLayer(layerNum, proxyLayer);
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

                    terrainTile.setColorLayer(layerNum, readLayer);
                }

                if (readObject.valid()) localAdvanced = true;
            }

            if (!localAdvanced) ++fr;
        }

        itrAdvanced = true;
    }


    osg::ref_ptr<osg::Object> readObject = fr.readObjectOfType(osgDB::type_wrapper<osgTerrain::TerrainTechnique>());
    if (readObject.valid())
    {
        terrainTile.setTerrainTechnique(dynamic_cast<osgTerrain::TerrainTechnique*>(readObject.get()));
        itrAdvanced = true;
    }

    if (fr.getOptions())
    {
        osg::ref_ptr<osg::Node> node;
        if (fr.getOptions()->getTerrain().lock(node))
        {
            terrainTile.setTerrain(node->asTerrain());
        }
    }

    if (osgTerrain::TerrainTile::getTileLoadedCallback().valid())
        osgTerrain::TerrainTile::getTileLoadedCallback()->loaded(&terrainTile, fr.getOptions());

    return itrAdvanced;
}

bool TerrainTile_writeLocalData(const osg::Object& obj, osgDB::Output& fw)
{
    const osgTerrain::TerrainTile& terrainTile = static_cast<const osgTerrain::TerrainTile&>(obj);

    int prec = fw.precision();
    fw.precision(15);

    switch(terrainTile.getBlendingPolicy())
    {
        case(osgTerrain::TerrainTile::INHERIT): fw.indent()<<"BlendingPolicy INHERIT"<<std::endl; break;
        case(osgTerrain::TerrainTile::DO_NOT_SET_BLENDING): fw.indent()<<"BlendingPolicy DO_NOT_SET_BLENDING"<<std::endl; break;
        case(osgTerrain::TerrainTile::ENABLE_BLENDING): fw.indent()<<"BlendingPolicy ENABLE_BLENDING"<<std::endl; break;
        case(osgTerrain::TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT): fw.indent()<<"BlendingPolicy ENABLE_BLENDING_WHEN_ALPHA_PRESENT"<<std::endl; break;
    }

    if (terrainTile.getLocator())
    {
        fw.writeObject(*terrainTile.getLocator());
    }

    if (terrainTile.getElevationLayer())
    {
        fw.indent()<<"ElevationLayer {"<<std::endl;

        fw.moveIn();

        const osgTerrain::ProxyLayer* proxyLayer = dynamic_cast<const osgTerrain::ProxyLayer*>(terrainTile.getElevationLayer());
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
        else if (terrainTile.getElevationLayer())
        {
            fw.writeObject(*terrainTile.getElevationLayer());
        }

        fw.moveOut();

        fw.indent()<<"}"<<std::endl;
    }

    for(unsigned int i=0; i<terrainTile.getNumColorLayers(); ++i)
    {
        const osgTerrain::Layer* layer = terrainTile.getColorLayer(i);
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
                fw.writeObject(*terrainTile.getColorLayer(i));
            }

            fw.moveOut();

            fw.indent()<<"}"<<std::endl;
        }
    }

    if (terrainTile.getTerrainTechnique())
    {
        fw.writeObject(*terrainTile.getTerrainTechnique());
    }

    fw.precision(prec);

    return true;
}
