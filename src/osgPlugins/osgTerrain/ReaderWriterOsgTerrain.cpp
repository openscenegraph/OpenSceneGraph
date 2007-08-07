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
        
        virtual const char* className() const { return "Terrain ReaderWriter"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive( extension, "osgTerrain" ) || osgDB::equalCaseInsensitive(extension,"terrain");
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

        virtual osgDB::ReaderWriter::ReadResult readNode(std::istream& fin, const Options* options) const
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

                osg::ref_ptr<osg::Node> node = fr.readNode();
                if (node.valid())
                {
                    group->addChild(node.get());
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

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(terrain, ReaderWriterTerrain)

