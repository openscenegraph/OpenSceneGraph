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

};

osgDB::ReaderWriter::ReadResult ReaderWriterTerrain::readNode(std::istream& fin, const osgDB::ReaderWriter::Options* options) const
{
    fin.imbue(std::locale::classic());

    osgDB::Input fr;
    fr.attach(&fin);
    fr.setOptions(options);

    osg::notify(osg::NOTICE)<<"Reading terrain"<<std::endl;
    
    osg::ref_ptr<osg::Group> group = new osg::Group;
    
    while(!fr.eof())
    {
        osg::notify(osg::NOTICE)<<"Read : "<<fr[0].getStr() << std::endl;
        
        bool itrAdvanced = false;
        
        if (fr.matchSequence("file %s"))
        {
            osg::Node* node = osgDB::readNodeFile(fr[1].getStr());
            
            if (node) group->addChild(node);
            
            fr += 2;
            itrAdvanced = true;
        }
        
        if (fr.matchSequence("TerrainNode {") || fr.matchSequence("Terrain {") )
        {
            osg::Node* node = readTerrainNode(fr);
            
            itrAdvanced = true;
        }
        
        
        if (!itrAdvanced) ++fr;
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
        osg::notify(osg::NOTICE)<<"Terrain : "<<fr[0].getStr() << std::endl;

        bool itrAdvanced = false;
        if (fr.matchSequence("Name %s"))
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

        if (fr.matchSequence("ColorLayer {") || fr.matchSequence("ColourLayer {") )
        {
            osgTerrain::Layer* layer = readLayer(fr);
            
            if (layer) terrain->setColorLayer(0, layer);
            
            itrAdvanced = true;
        }

        if (fr.matchSequence("ColorLayer %i {") || fr.matchSequence("ColourLayer %i {") )
        {
            unsigned int layerNum;
             fr[1].getUInt(layerNum);
            
            ++fr;
        
            osgTerrain::Layer* layer = readLayer(fr);
            
            if (layer) terrain->setColorLayer(layerNum, layer);
            
            itrAdvanced = true;
        }

        if (!itrAdvanced)
        {
            ++fr;
        }
    }
    
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
        osg::notify(osg::NOTICE)<<"Layer : "<<fr[0].getStr() << std::endl;

        bool itrAdvanced = false;
        if (fr.matchSequence("Image %s") || fr.matchSequence("image %s"))
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
            ++fr;
        }
    }
    
    if (layer.valid() && locator.valid())
    {
        layer->setLocator(locator.get());
    }
    
    return layer.release();
}


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(terrain, ReaderWriterTerrain)

