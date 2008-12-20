#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include "ReaderWriterMDL.h"
#include "MDLReader.h"

using namespace mdl;
using namespace osg;
using namespace osgDB;


const char* ReaderWriterMDL::className() const
{
    // Return a description of this class
    return "Valve/Source Engine MDL Reader";
}


bool ReaderWriterMDL::acceptsExtension(const std::string& extension) const
{
    // If the extension is empty or "mdl", we accept it
    return osgDB::equalCaseInsensitive(extension, "mdl") || extension.empty();
}


ReaderWriter::ReadResult ReaderWriterMDL::readNode(
                                  const std::string& file,
                                  const ReaderWriter::Options* options) const
{
    MDLReader *      mdlReader;
    ref_ptr<Node>    result;

    // See if we handle this kind of file
    if (!acceptsExtension(osgDB::getFileExtension(file)))
        return ReadResult::FILE_NOT_HANDLED;

    // See if we can find the requested file
    std::string fileName = osgDB::findDataFile(file, options, CASE_INSENSITIVE);
    if (fileName.empty()) 
        return ReadResult::FILE_NOT_FOUND;
   
    // Read the file (pass the base name and not the file that was found, this
    // allows us to also find the .vvd and .vtx files without the leading
    // path confusing things)
    mdlReader = new MDLReader();
    if (mdlReader->readFile(file))
    {
        // Get the results of our read
        result = mdlReader->getRootNode();

        // Clean up the reader
        delete mdlReader;

        // Return the results
        return ReadResult(result.get());
    }
    else
    {
        // Clean up the reader
        delete mdlReader;

        // Return the error
        return ReadResult::ERROR_IN_READING_FILE;
    }
}


REGISTER_OSGPLUGIN(mdl, ReaderWriterMDL)

