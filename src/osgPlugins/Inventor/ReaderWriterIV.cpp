#include "ReaderWriterIV.h"

// OSG headers
#include <osg/Notify>
#include <osgDB/FileNameUtils>

// Inventor headers
#include <Inventor/SoInteraction.h>
#include <Inventor/nodes/SoSeparator.h>

#include "ConvertFromInventor.h"
#include "GroupSoLOD.h"

// Register with Registry to instantiate the inventor reader.
osgDB::RegisterReaderWriterProxy<ReaderWriterIV> g_ivReaderWriterProxy;

ReaderWriterIV::ReaderWriterIV()
{
}

// Read file and convert to OSG
osgDB::ReaderWriter::ReadResult 
ReaderWriterIV::readNode(const std::string& fileName,
                         const osgDB::ReaderWriter::Options*)
{
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext)) 
        return ReadResult::FILE_NOT_HANDLED;

    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::readNode() Reading file " 
                           << fileName.data() << std::endl;
    
    // Initialize Inventor
    SoInteraction::init();

    // Initial GroupSoLOD node
    GroupSoLOD::initClass();

    // Open the file
    SoInput input;
    if (!input.openFile(fileName.data()))
    {
        osg::notify(osg::WARN) << "osgDB::ReaderWriterIV::readIVFile() "
                               << "Cannot open file " << fileName << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    // Create the inventor scenegraph from the file
    SoSeparator* rootIVNode = SoDB::readAll(&input);

    // Close the file
    input.closeFile();

    if (rootIVNode)
    {
        // Convert the inventor scenegraph to an osg scenegraph and return it
        ConvertFromInventor convertIV;
        return convertIV.convert(rootIVNode);
    }

    return ReadResult::FILE_NOT_HANDLED;
}

