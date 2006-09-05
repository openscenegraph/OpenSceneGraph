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
#include <osgUtil/Optimizer>

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
        // Transfer ownership of pools.
        _options->setUserData( node.getUserData() );
        node.setUserData(NULL);

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
        
                bool keepExternalReferences = false;
                if (options)
                    keepExternalReferences = (options->getOptionString().find("keepExternalReferences")!=std::string::npos);


                if ( !keepExternalReferences )
                {
                    osg::notify(osg::DEBUG_INFO) << "keepExternalReferences not found, so externals will be re-readed"<<std::endl;
                    // read externals.
                    if (rr.getNode())
                    {
                        nestedExternalsLevel++;
                        ReadExternalsVisitor visitor(local_opt.get());
                        rr.getNode()->accept(visitor);
                        nestedExternalsLevel--;
                    }
                }
                else
                {
                    osg::notify(osg::DEBUG_INFO) << "keepExternalReferences found, so externals will be left as ProxyNodes"<<std::endl;    
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

            // option string and parent pools
            if (options)
            {
                const char readerMsg[] = "flt reader option: ";
                
                document.setKeepExternalReferences((options->getOptionString().find("keepExternalReferences")!=std::string::npos));
                osg::notify(osg::DEBUG_INFO) << readerMsg << "keepExternalReferences=" << document.getKeepExternalReferences() << std::endl;

                document.setPreserveFace((options->getOptionString().find("preserveFace")!=std::string::npos));
                osg::notify(osg::DEBUG_INFO) << readerMsg << "preserveFace=" << document.getPreserveFace() << std::endl;

                document.setPreserveObject((options->getOptionString().find("preserveObject")!=std::string::npos));
                osg::notify(osg::DEBUG_INFO) << readerMsg << "preserveObject=" << document.getPreserveObject() << std::endl;

                document.setDefaultDOFAnimationState((options->getOptionString().find("dofAnimation")!=std::string::npos));
                osg::notify(osg::DEBUG_INFO) << readerMsg << "dofAnimation=" << document.getDefaultDOFAnimationState() << std::endl;

                document.setUseTextureAlphaForTransparancyBinning(options->getOptionString().find("noTextureAlphaForTransparancyBinning")==std::string::npos);
                osg::notify(osg::DEBUG_INFO) << readerMsg << "noTextureAlphaForTransparancyBinning=" << document.getUseTextureAlphaForTransparancyBinning() << std::endl;

                document.setDoUnitsConversion((options->getOptionString().find("noUnitsConversion")==std::string::npos)); // default to true, unless noUnitsConversion is specified.
                osg::notify(osg::DEBUG_INFO) << readerMsg << "noUnitsConversion=" << document.getDoUnitsConversion() << std::endl;

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

                const ParentPools* pools = dynamic_cast<const ParentPools*>( options->getUserData() );
                if (pools)
                {
                    // This file is an external reference. The individual pools will
                    // be non-NULL if the parent is overriding the ext ref model's pools.
                    if (pools->getColorPool())
                        document.setColorPool( pools->getColorPool(), true );
                    if (pools->getTexturePool())
                        document.setTexturePool( pools->getTexturePool(), true );
                    if (pools->getMaterialPool())
                        document.setMaterialPool( pools->getMaterialPool(), true );
                    if (pools->getLPAppearancePool())
                        document.setLightPointAppearancePool( pools->getLPAppearancePool(), true );
                    if (pools->getShaderPool())
                        document.setShaderPool( pools->getShaderPool(), true );
                }
            }

            {
                // read records
                flt::RecordInputStream recordStream(fin.rdbuf());
                while (recordStream.good() && !document.done())
                {
                    recordStream.readRecord(document);
                }
            }

            if (!document.getHeaderNode())
                return ReadResult::ERROR_IN_READING_FILE;

            if (!document.getPreserveFace())
            {
                osgUtil::Optimizer optimizer;
                optimizer.optimize(document.getHeaderNode(),  osgUtil::Optimizer::SHARE_DUPLICATE_STATE | osgUtil::Optimizer::MERGE_GEOMETRY | osgUtil::Optimizer::MERGE_GEODES | osgUtil::Optimizer::TESSELATE_GEOMETRY );
            }

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

        virtual WriteResult writeNode(const Node& /*node*/,std::ostream& /*fout*/, const osgDB::ReaderWriter::Options* /*options*/) const
        {
            return WriteResult::FILE_NOT_HANDLED;

        }

    protected:

        mutable osgDB::ReentrantMutex _serializerMutex;
};

// now register with Registry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<FLTReaderWriter> g_FLTReaderWriterProxy;















