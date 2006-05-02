//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <stdexcept>
#include <osg/Notify>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/ReentrantMutex>

#include <osgSim/OpenFlightOptimizer>

#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

#define SERIALIZER() OpenThreads::ScopedLock<osgDB::ReentrantMutex> lock(_serializerMutex)  

using namespace flt;
using namespace osg;
using namespace osgDB;


class FLTReaderWriter : public ReaderWriter
{
    public:
        virtual const char* className() const { return "FLT Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return equalCaseInsensitive(extension,"flt");
        }

        virtual ReadResult readObject(const std::string& file, const Options* options) const
        {
            return readNode(file, options);
        }
        
        virtual ReadResult readNode(const std::string& file, const Options* options) const
        {
            SERIALIZER();

            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            std::ifstream istream;
            istream.imbue(std::locale::classic());
            istream.open(fileName.c_str(), std::ios::in | std::ios::binary);

            return readNode(istream,local_opt.get());
        }
        
        virtual ReadResult readObject(std::istream& fin, const Options* options) const
        {
            return readNode(fin, options);
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options* options) const
        {
            try
            {
                Document document;
                document.setOptions(options);

                // option string
                if (options)
                {
                    document.setUseTextureAlphaForTransparancyBinning(options->getOptionString().find("noTextureAlphaForTransparancyBinning")==std::string::npos);
                    osg::notify(osg::DEBUG_INFO) << "FltFile.getUseTextureAlphaForTransparancyBinning()=" << document.getUseTextureAlphaForTransparancyBinning() << std::endl;
                    document.setDoUnitsConversion((options->getOptionString().find("noUnitsConversion")==std::string::npos)); // default to true, unless noUnitsConversion is specified.o
                    osg::notify(osg::DEBUG_INFO) << "FltFile.getDoUnitsConversion()=" << document.getDoUnitsConversion() << std::endl;

                    if (document.getDoUnitsConversion())
                    {
                        if (options->getOptionString().find("convertToFeet")!=std::string::npos)
                            document.setDesiredUnits(FEET);
                        else if (options->getOptionString().find("convertToInches")!=std::string::npos)
                            document.setDesiredUnits(INCHES);
                        else if (options->getOptionString().find("convertToMeters")!=std::string::npos)
                            document.setDesiredUnits(METERS);
                        else if (options->getOptionString().find("convertToKilometers")!=std::string::npos)
                            document.setDesiredUnits(KILOMETERS);
                        else if (options->getOptionString().find("convertToNauticalMiles")!=std::string::npos)
                            document.setDesiredUnits(NAUTICAL_MILES);
                    }
                }

                {
                    // read records
                    flt::RecordInputStream recordStream(&fin);
                    while (recordStream().good() && !document.done())
                    {
                        recordStream.readRecord(document);
                    }
                }

                {
                    // optimize
                    osgFlightUtil::Optimizer optimizer;
                    optimizer.optimize(document.getHeaderNode());
                }

                readExternals(options);

                return document.getHeaderNode();
            }
            catch (std::exception &e) 
            {
                osg::notify(osg::NOTICE) << "Error reading file: " << e.what() << std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }
            catch (...)
            {
                return ReadResult::FILE_NOT_HANDLED;
            }
        }

        virtual WriteResult writeObject(const Object& object,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            const Node* node = dynamic_cast<const Node*>(&object);
            if (node) return writeNode( *node, fileName, options );
            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeNode(const Node& node,const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            if(local_opt->getDatabasePathList().empty())
                local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            std::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            WriteResult result = writeNode(node, fout, local_opt.get());
            fout.close();
            return result;
        }
        
        virtual WriteResult writeObject(const Object& object,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            const Node* node = dynamic_cast<const Node*>(&object);
            if (node) return writeNode( *node, fout, options );
            return WriteResult::FILE_NOT_HANDLED;
        }

        virtual WriteResult writeNode(const Node& node,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {
            return WriteResult::FILE_NOT_HANDLED;

        }

    protected:

        void readExternals(const Options* options) const
        {
            flt::Registry::ExternalQueue& eq = flt::Registry::instance()->getExternalQueue();
            while (!eq.empty())
            {
                std::string extfilename = eq.front().first;
                osg::ref_ptr<osg::Group> external = eq.front().second;
                eq.pop();

                if (external.valid())
                {
                    osg::Node* extmodel = osgDB::readNodeFile(extfilename,options);
                    external->addChild(extmodel);
                }
            }
        }

        mutable osgDB::ReentrantMutex _serializerMutex;
};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<FLTReaderWriter> g_FLTReaderWriterProxy;












