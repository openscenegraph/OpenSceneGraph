//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
//

#include <stdexcept>
#include <osg/Notify>
#include <osg/ProxyNode>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/ReentrantMutex>

#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

#define SERIALIZER() OpenThreads::ScopedLock<osgDB::ReentrantMutex> lock(_serializerMutex)  

using namespace flt;
using namespace osg;
using namespace osgDB;


class ReadExternalsVisitor : public osg::NodeVisitor
{
    osg::ref_ptr<ReaderWriter::Options> _options;

public:

    ReadExternalsVisitor(ReaderWriter::Options* options) :
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _options(options)
    {
    }
        
    virtual ~ReadExternalsVisitor() {}

    virtual void apply(ProxyNode& node)
    {
        for (unsigned int pos=0; pos<node.getNumFileNames(); pos++)
        {
            std::string filename = node.getFileName(pos);

            // read external
            osg::Node* external = osgDB::readNodeFile(filename,_options.get());
            if (external)
                node.addChild(external);
        }
    }
};


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

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // in local cache?
            {
                osg::Node* node = flt::Registry::instance()->getFromLocalCache(fileName);
                if (node)
                    return ReadResult(node, ReaderWriter::ReadResult::FILE_LOADED_FROM_CACHE);
            }

            // setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->setDatabasePath(osgDB::getFilePath(fileName));

            ReadResult rr;

            // read file
            {
                std::ifstream istream;
                istream.imbue(std::locale::classic());
                istream.open(fileName.c_str(), std::ios::in | std::ios::binary);

                if (istream)
                {
                    rr = readNode(istream,local_opt.get());
                }
            }

            static int nestedExternalsLevel = 0;
            if (rr.success())
            {
                // add to local cache.
                flt::Registry::instance()->addToLocalCache(fileName,rr.getNode());

                // read externals.
                if (rr.getNode())
                {
                    nestedExternalsLevel++;
                    ReadExternalsVisitor visitor(local_opt.get());
                    rr.getNode()->accept(visitor);
                    nestedExternalsLevel--;
                }
            }

            // clear local cache.
            if (nestedExternalsLevel==0)
                flt::Registry::instance()->clearLocalCache();

            return rr;
        }
        
        virtual ReadResult readObject(std::istream& fin, const Options* options) const
        {
            return readNode(fin, options);
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options* options) const
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

            if (!document.getHeaderNode())
                return ReadResult::ERROR_IN_READING_FILE;

            return document.getHeaderNode();
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

        mutable osgDB::ReentrantMutex _serializerMutex;
};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<FLTReaderWriter> g_FLTReaderWriterProxy;















