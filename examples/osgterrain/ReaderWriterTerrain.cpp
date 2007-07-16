#include <sstream>

#include <osg/Image>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include <osgTerrain/TerrainNode>
#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/Layer>

class ReaderWriterTerrain : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTerrain()
        {
        }
        
        virtual const char* className() const { return "Terrain Reader"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"terrain");
        }

        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* opt) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, opt );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = opt ? static_cast<Options*>(opt->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            std::ifstream fin(fileName.c_str());
            if (fin)
            {
                return readNode(fin, local_opt.get());
            }
            return 0L;
                        
        }
        
        virtual osgDB::ReaderWriter::ReadResult readNode(std::istream& fin, const Options* options) const;

        osg::Node* readTerrainNode(osgDB::Input& fr) const;
        osgTerrain::Layer* readLayer(osgDB::Input& fr) const;
        osg::TransferFunction* readTransferFunction(osgDB::Input& fr) const;

};

osgDB::ReaderWriter::ReadResult ReaderWriterTerrain::readNode(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
{
    fin.imbue(std::locale::classic());

    osgDB::Input fr;
    fr.attach(&fin);
    fr.setOptions(options);

    osg::ref_ptr<osg::Group> group = new osg::Group;
    
    while(!fr.eof())
    {
        
        bool itrAdvanced = false;
        
        if (fr.matchSequence("file %s") || fr.matchSequence("file %w") )
        {
            osg::Node* node = osgDB::readNodeFile(fr[1].getStr());
            
            if (node) group->addChild(node);
            
            fr += 2;
            itrAdvanced = true;
        }
        
        if (fr.matchSequence("TerrainNode {") || fr.matchSequence("Terrain {") )
        {
            osg::Node* node = readTerrainNode(fr);
            
            if (node) group->addChild(node);
            
            itrAdvanced = true;
        }
        
        
        if (!itrAdvanced)
        {
            if (fr[0].getStr()) osg::notify(osg::NOTICE)<<"Terrain file - unreconised token : "<<fr[0].getStr() <<""<< std::endl;
            ++fr;
        }
    }

    if (group->getNumChildren()>0) return group.release();
    else return 0;
}

osg::Node* ReaderWriterTerrain::readTerrainNode(osgDB::Input& fr) const
{
    osg::ref_ptr<osgTerrain::TerrainNode> terrain = new osgTerrain::TerrainNode;

    int entry = fr[0].getNoNestedBrackets();

    fr += 2;

    while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool itrAdvanced = false;
        if (fr.matchSequence("Name %s") || fr.matchSequence("Name %w") ||
            fr.matchSequence("name %s") || fr.matchSequence("name %w") )
        {
            terrain->setName(fr[1].getStr());
        
            fr += 2;
            itrAdvanced = true;
        }
        
        if (fr.matchSequence("ElevationLayer {"))
        {
            osgTerrain::Layer* layer = readLayer(fr);
            
            if (layer) terrain->setElevationLayer(layer);
            
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
            if (layer) terrain->setColorLayer(layerNum, layer);
            
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
            if (tf) terrain->setColorTransferFunction(layerNum, tf);
            
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
           
            if (fr[0].matchWord("NEAREST")) terrain->setColorFilter(layerNum, osgTerrain::TerrainNode::NEAREST);
            else if (fr[0].matchWord("LINEAR")) terrain->setColorFilter(layerNum, osgTerrain::TerrainNode::LINEAR);
            
            ++fr;
            itrAdvanced = true;
        }

        if (!itrAdvanced)
        {
            if (fr[0].getStr()) osg::notify(osg::NOTICE)<<"Terrain - unreconised token : ["<<fr[0].getStr() <<"]"<< std::endl;
            ++fr;
        }
    }

    // step over trailing }
    ++fr;
    
    terrain->setTerrainTechnique(new osgTerrain::GeometryTechnique);
    
    return terrain.release();

}

osgTerrain::Layer* ReaderWriterTerrain::readLayer(osgDB::Input& fr) const
{
    osg::ref_ptr<osgTerrain::Layer> layer;
    osg::ref_ptr<osgTerrain::Locator> locator;

    int entry = fr[0].getNoNestedBrackets();

    fr += 2;

    while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool itrAdvanced = false;
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

osg::TransferFunction* ReaderWriterTerrain::readTransferFunction(osgDB::Input& fr) const
{
    osg::ref_ptr<osg::TransferFunction1D> tf = new osg::TransferFunction1D;
    
    tf->allocate(6);
    tf->setValue(0, osg::Vec4(1.0,1.0,1.0,1.0));
    tf->setValue(1, osg::Vec4(1.0,0.0,1.0,1.0));
    tf->setValue(2, osg::Vec4(1.0,0.0,0.0,1.0));
    tf->setValue(3, osg::Vec4(1.0,1.0,0.0,1.0));
    tf->setValue(4, osg::Vec4(0.0,1.0,1.0,1.0));
    tf->setValue(5, osg::Vec4(0.0,1.0,0.0,1.0));

    int entry = fr[0].getNoNestedBrackets();

    fr += 2;

    while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
    {
        bool itrAdvanced = false;
        if (fr.matchSequence("range %f %f"))
        {
            double minValue,maxValue;
            fr[1].getFloat(minValue);
            fr[2].getFloat(maxValue);
        
            tf->setInputRange(minValue,maxValue);
            
            fr += 3;
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

    return tf.release();
}

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(terrain, ReaderWriterTerrain)

