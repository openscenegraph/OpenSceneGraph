// ReaderWriterFLT.cpp

#if defined(_MSC_VER)
#pragma warning( disable : 4786 )
#endif

#include "ReaderWriterFLT.h"
#include "FltFile.h"
#include "Registry.h"

#include <osg/Object>
#include <osg/Node>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/FileUtils>

using namespace flt;

osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* opt) const
{
    return readNode(fileName,opt);
}


osgDB::ReaderWriter::ReadResult ReaderWriterFLT::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    // serialize all access to the OpenFlight plugin as its not thread safe by itself.
    OpenThreads::ScopedLock<osgDB::ReentrantMutex> lock(_serializerMutex);

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    osg::ref_ptr<FltFile> read = new FltFile;

    if (options)
    {
        read->setUseTextureAlphaForTransparancyBinning(options->getOptionString().find("noTextureAlphaForTransparancyBinning")==std::string::npos);
        osg::notify(osg::DEBUG_INFO) << "FltFile.getUseTextureAlphaForTransparancyBinning()=" << read->getUseTextureAlphaForTransparancyBinning() << std::endl;
        read->setDoUnitsConversion((options->getOptionString().find("noUnitsConversion")==std::string::npos)); // default to true, unless noUnitsConversion is specified.o
        osg::notify(osg::DEBUG_INFO) << "FltFile.getDoUnitsConversion()=" << read->getDoUnitsConversion() << std::endl;

        if (read->getDoUnitsConversion())
        {
            if (options->getOptionString().find("convertToFeet")!=std::string::npos)
                read->setDesiredUnits( FltFile::ConvertToFeet );
            else if (options->getOptionString().find("convertToInches")!=std::string::npos)
                read->setDesiredUnits( FltFile::ConvertToInches );
            else if (options->getOptionString().find("convertToMeters")!=std::string::npos)
                read->setDesiredUnits( FltFile::ConvertToMeters );
            else if (options->getOptionString().find("convertToKilometers")!=std::string::npos)
                read->setDesiredUnits( FltFile::ConvertToKilometers );
            else if (options->getOptionString().find("convertToNauticalMiles")!=std::string::npos)
                read->setDesiredUnits( FltFile::ConvertToNauticalMiles );
            osg::notify(osg::DEBUG_INFO) << "FltFile.getDesiredUnits()=" << read->getDesiredUnitsString() << std::endl;
        }
        
    }

    osg::ref_ptr<osgDB::ReaderWriter::Options> local_options = options ? 
                static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp(osg::CopyOp::SHALLOW_COPY))) : 
                new osgDB::ReaderWriter::Options;
    local_options->setDatabasePath(osgDB::getFilePath(fileName));
    read->setOptions(local_options.get());

    osg::Node* node = read->readNode(fileName);

    flt::Registry::instance()->clearObjectCache();

    if (node) return node;
    else return ReadResult::FILE_NOT_HANDLED;
}


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFLT> g_fltReaderWriterProxy;
