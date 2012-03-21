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

#include <osgTerrain/TerrainTile>

class ReaderWriterTerrain : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTerrain()
        {
            supportsExtension("osgTerrain","OpenSceneGraph terrain extension to .osg ascii format");
            supportsExtension("terrain","OpenSceneGraph terrain ascii format");
        }

        virtual const char* className() const { return "Terrain ReaderWriter"; }

        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* opt) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);

            if (osgDB::equalCaseInsensitive(ext,"terrain"))
            {
#if 0
                KeywordValueMap keywordValueMap;
                parseTerrainString(osgDB::getNameLessExtension(file), keywordValueMap);

                for(KeywordValueMap::iterator itr = keywordValueMap.begin();
                    itr != keywordValueMap.end();
                    ++itr)
                {
                    OSG_NOTICE<<"["<<itr->first<<"] = "<<"["<<itr->second<<"]"<<std::endl;
                }
#else
                std::istringstream fin(osgDB::getNameLessExtension(file));
                if (fin) return readNode(fin,opt);
#endif
                return 0;
            }

            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, opt );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths.
            osg::ref_ptr<Options> local_opt = opt ? static_cast<Options*>(opt->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            osgDB::ifstream fin(fileName.c_str());
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
                    if (fr[0].getStr()) { OSG_NOTICE<<"Terrain file - unreconised token : "<<fr[0].getStr() <<""<< std::endl; }
                    ++fr;
                }
            }

            if (group->getNumChildren()>0) return group.release();
            else return 0;
        }

        typedef std::map<std::string, std::string> KeywordValueMap;
        bool parseTerrainString(const std::string& str, KeywordValueMap& keywordValueMap) const
        {
            bool success = false;

            std::string::size_type pos = 0;
            while(pos != std::string::npos)
            {
                pos = str.find_first_not_of(' ',pos);
                if (pos == std::string::npos) break;

                std::string::size_type semicolon = str.find_first_of(';', pos);
                std::string::size_type startstatement = pos;
                std::string::size_type endstatement = std::string::npos;
                if (semicolon!=std::string::npos)
                {
                    endstatement = semicolon-1;
                    pos = semicolon+1;
                }
                else
                {
                    endstatement = str.length()-1;
                    pos = std::string::npos;
                }

                if (startstatement<endstatement) endstatement = str.find_last_not_of(' ',endstatement);

                if (startstatement<=endstatement)
                {
                    // OSG_NOTICE<<"statement = ["<<str.substr(startstatement, endstatement-startstatement+1)<<"]"<<std::endl;
                    std::string::size_type assignment = str.find_first_of('=', startstatement);
                    if (assignment!=std::string::npos && assignment>endstatement)
                    {
                        assignment = std::string::npos;
                    }

                    std::string::size_type startvariable = startstatement;
                    std::string::size_type endvariable = startstatement;
                    std::string::size_type startvalue = startstatement;
                    std::string::size_type endvalue = endstatement;

                    if (assignment!=std::string::npos)
                    {
                        endvariable = assignment-1;
                        startvalue = assignment+1;

                        if (startvariable<=endvariable)
                        {
                            endvariable = str.find_last_not_of(' ',endvariable);
                        }

                        if (startvariable<=endvariable)
                        {
                            ++endvariable;
                        }
                    }


                    if (startvalue<=endvalue)
                    {
                        startvalue = str.find_first_not_of(' ',startvalue);
                    }

                    if (startvalue<=endvalue)
                    {
                        if (startvariable<endvariable)
                        {
                            keywordValueMap[str.substr(startvariable, endvariable-startvariable)] = str.substr(startvalue, endvalue-startvalue+1);
                            success = true;
                        }
                        else
                        {
                            keywordValueMap[""] = str.substr(startvalue, endvalue-startvalue+1);
                            success = true;
                        }
                    }

                }
            }

            return success;
        }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(terrain, ReaderWriterTerrain)

