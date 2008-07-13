#include "ReaderWriterIV.h"

// OSG headers
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

// Inventor headers
#include <Inventor/SoDB.h>
#include <Inventor/SoInteraction.h>
#include <Inventor/nodekits/SoNodeKit.h>
#include <Inventor/nodes/SoSeparator.h>
#include <Inventor/actions/SoWriteAction.h>
#include <Inventor/actions/SoCallbackAction.h>

#ifdef __COIN__
#include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#endif

#include "ConvertFromInventor.h"
#include "GroupSoLOD.h"
#include "ConvertToInventor.h"


// Register with Registry to instantiate the inventor reader.
REGISTER_OSGPLUGIN(Inventor, ReaderWriterIV)

ReaderWriterIV::ReaderWriterIV()
{
    supportsExtension("iv","Inventor format");
    supportsExtension("wrl","VRML world file");
}

// Read file and convert to OSG
osgDB::ReaderWriter::ReadResult 
ReaderWriterIV::readNode(const std::string& file,
                         const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::readNode() Reading file " 
                           << fileName.data() << std::endl;
    
    // Initialize Inventor
    SoDB::init();
    SoNodeKit::init();
    SoInteraction::init();
    

    // Initial GroupSoLOD node
    GroupSoLOD::initClass();

#ifdef __COIN__
    // Disable delayed loading of VRML textures
    SoVRMLImageTexture::setDelayFetchURL(FALSE);
#endif

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
        rootIVNode->ref();
        // Convert the inventor scenegraph to an osg scenegraph and return it
        ConvertFromInventor convertIV;
        ReadResult result = convertIV.convert(rootIVNode);
        rootIVNode->unref();
        return result;
    }

    return ReadResult::FILE_NOT_HANDLED;
}


osgDB::ReaderWriter::WriteResult
ReaderWriterIV::writeNode(const osg::Node& node, const std::string& fileName,
                          const osgDB::ReaderWriter::Options* options) const
{
    // accept extension
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
    bool useVRML1 = !isInventorExtension(osgDB::getFileExtension(fileName));

    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::writeNode() Writing file " 
                           << fileName.data() << std::endl;
    
    // Initialize Inventor
    SoInteraction::init();

    // Convert OSG graph to Inventor graph
    ConvertToInventor osg2iv;
    osg2iv.setVRML1Conversion(useVRML1);
    (const_cast<osg::Node*>(&node))->accept(osg2iv);
    SoNode *ivRoot = osg2iv.getIvSceneGraph();
    if (ivRoot == NULL)
        return WriteResult::ERROR_IN_WRITING_FILE;
    ivRoot->ref();

    // Change prefix according to VRML spec:
    // Node names must not begin with a digit, and must not contain spaces or
    // control characters, single or double quote characters, backslashes, curly braces,
    // the sharp (#) character, the plus (+) character or the period character.
    if (useVRML1)
      SoBase::setInstancePrefix("_");

    // Write Inventor graph to file
    SoOutput out;
    out.setHeaderString((useVRML1) ? "#VRML V1.0 ascii" : "#Inventor V2.1 ascii");
    if (!out.openFile(fileName.c_str()))
        return WriteResult::ERROR_IN_WRITING_FILE;
    SoWriteAction wa(&out);
    wa.apply(ivRoot);
    ivRoot->unref();

    return WriteResult::FILE_SAVED;
}
