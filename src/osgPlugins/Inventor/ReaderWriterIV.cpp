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
# include <Inventor/VRMLnodes/SoVRMLImageTexture.h>
#endif

#include "ReaderWriterIV.h"
#include "ConvertFromInventor.h"
#include "ConvertToInventor.h"

// forward declarations of static functions
static void addSearchPaths(const osgDB::FilePathList *searchPaths);
static void removeSearchPaths(const osgDB::FilePathList *searchPaths);


// Register with Registry to instantiate the inventor reader.
REGISTER_OSGPLUGIN(Inventor, ReaderWriterIV)


/**
 * Constructor.
 * Initializes the ReaderWriterIV.
 */
ReaderWriterIV::ReaderWriterIV()
{
    // Set supported extensions and options
    supportsExtension("iv","Inventor format");
    supportsExtension("wrl","VRML world file");

    // Initialize Inventor
    initInventor();
}


/**
 * Initializes Open Inventor.
 */
void ReaderWriterIV::initInventor() const
{
    // Initialize Inventor
    SoDB::init();
    SoNodeKit::init();
    SoInteraction::init();

#ifdef __COIN__
    // Disable delayed loading of VRML textures
    SoVRMLImageTexture::setDelayFetchURL(FALSE);
#endif
}


/**
 * Read from SoInput and convert to OSG.
 * This is a method used by readNode(string,options) and readNode(istream,options).
 */
osgDB::ReaderWriter::ReadResult
ReaderWriterIV::readNodeFromSoInput(SoInput &input,
          std::string &fileName, const osgDB::ReaderWriter::Options *options) const
{
    // Parse options and add search paths to SoInput
    const osgDB::FilePathList *searchPaths = options ? &options->getDatabasePathList() : NULL;
    if (options)
        addSearchPaths(searchPaths);

    // Create the inventor scenegraph by reading from SoInput
    SoSeparator* rootIVNode = SoDB::readAll(&input);

    // Remove recently appened search paths
    if (options)
        removeSearchPaths(searchPaths);

    // Close the file
    input.closeFile();

    // Perform conversion
    ReadResult result;
    if (rootIVNode)
    {
        rootIVNode->ref();
        // Convert the inventor scenegraph to an osg scenegraph
        ConvertFromInventor convertIV;
        convertIV.preprocess(rootIVNode);
        result = convertIV.convert(rootIVNode);
        rootIVNode->unref();
    } else
        result = ReadResult::FILE_NOT_HANDLED;

    // Notify
    if (result.success()) {
        if (fileName.length())
            osg::notify(osg::NOTICE) << "osgDB::ReaderWriterIV::readNode() "
                      << "File " << fileName.data()
                      << " loaded successfully." << std::endl;
        else
            osg::notify(osg::NOTICE) << "osgDB::ReaderWriterIV::readNode() "
                      << "Stream loaded successfully." << std::endl;
    } else {
        if (fileName.length())
            osg::notify(osg::WARN) << "osgDB::ReaderWriterIV::readNode() "
                      << "Failed to load file " << fileName.data()
                      << "." << std::endl;
        else
            osg::notify(osg::WARN) << "osgDB::ReaderWriterIV::readNode() "
                  << "Failed to load stream." << std::endl;
    }

    return result;
}


// Read file and convert to OSG
osgDB::ReaderWriter::ReadResult
ReaderWriterIV::readNode(const std::string& file,
                         const osgDB::ReaderWriter::Options* options) const
{
    // Accept extension
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // Find file
    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // Notify
    osg::notify(osg::NOTICE) << "osgDB::ReaderWriterIV::readNode() Reading file "
                             << fileName.data() << std::endl;
    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::readNode() Inventor version: "
                           << SoDB::getVersion() << std::endl;

    // Open the file
    SoInput input;
    if (!input.openFile(fileName.data()))
    {
        osg::notify(osg::WARN) << "osgDB::ReaderWriterIV::readIVFile() "
                               << "Cannot open file " << fileName << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    // Perform reading from SoInput
    return readNodeFromSoInput(input, fileName, options);
}


osgDB::ReaderWriter::ReadResult
ReaderWriterIV::readNode(std::istream& fin,
                         const osgDB::ReaderWriter::Options* options) const
{
    // Notify
    osg::notify(osg::NOTICE) << "osgDB::ReaderWriterIV::readNode() "
              "Reading from stream." << std::endl;
    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::readNode() "
              "Inventor version: " << SoDB::getVersion() << std::endl;

    // Open the file
    SoInput input;

    // Assign istream to SoInput
    // note: It seems there is no straightforward way to do that.
    // SoInput accepts only FILE by setFilePointer or memory buffer
    // by setBuffer. The FILE is dangerous on Windows, since it forces
    // the plugin and Inventor DLL to use the same runtime library
    // (otherwise there are app crashes).
    // The memory buffer seems much better option here, even although
    // there will not be a real streaming. However, the model data
    // are usually much smaller than textures, so we should not worry
    // about it and think how to stream textures instead.

    // Get the data to the buffer
    size_t bufSize = 126*1024; // let's make it something bellow 128KB
    char *buf = (char*)malloc(bufSize);
    size_t dataSize = 0;
    while (!fin.eof() && fin.good()) {
        fin.read(buf+dataSize, bufSize-dataSize);
        dataSize += fin.gcount();
        if (bufSize == dataSize) {
           bufSize *= 2;
           buf = (char*)realloc(buf, bufSize);
        }
    }
    input.setBuffer(buf, dataSize);
    osg::notify(osg::INFO) << "osgDB::ReaderWriterIV::readNode() "
              "Stream size: " << dataSize << std::endl;

    // Perform reading from SoInput
    osgDB::ReaderWriter::ReadResult r;
    std::string fileName("");
    r = readNodeFromSoInput(input, fileName, options);

    // clean up and return
    free(buf);
    return r;
}


osgDB::ReaderWriter::WriteResult
ReaderWriterIV::writeNode(const osg::Node& node, const std::string& fileName,
                          const osgDB::ReaderWriter::Options* options) const
{
    // accept extension
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;
    bool useVRML1 = !isInventorExtension(osgDB::getFileExtension(fileName));

    osg::notify(osg::NOTICE) << "osgDB::ReaderWriterIV::writeNode() Writing file "
                             << fileName.data() << std::endl;

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


static void addSearchPaths(const osgDB::FilePathList *searchPaths)
{
    for (int i=searchPaths->size()-1; i>=0; i--)
        SoInput::addDirectoryFirst(searchPaths->operator[](i).c_str());
}


static void removeSearchPaths(const osgDB::FilePathList *searchPaths)
{
    for (int i=0, c=searchPaths->size(); i<c; i++)
        SoInput::addDirectoryFirst(searchPaths->operator[](i).c_str());
}

