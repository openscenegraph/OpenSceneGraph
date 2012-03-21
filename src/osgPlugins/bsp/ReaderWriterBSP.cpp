#include <osg/Node>
#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include "ReaderWriterBSP.h"
#include "VBSPReader.h"
#include "Q3BSPReader.h"

using namespace bsp;
using namespace osg;
using namespace osgDB;


// "VBSP" for Valve BSP files
const int VBSP_MAGIC_NUMBER  = (('P'<<24)+('S'<<16)+('B'<<8)+'V');

// "IBSP" for id (Quake 3) BSP files
const int IBSP_MAGIC_NUMBER = (('P'<<24)+('S'<<16)+('B'<<8)+'I');



const char* ReaderWriterBSP::className() const
{
    // Return a description of this class
    return "BSP File Reader";
}


bool ReaderWriterBSP::acceptsExtension(const std::string& extension) const
{
    // If the extension is empty or "bsp", we accept it
    return osgDB::equalCaseInsensitive(extension, "bsp") || extension.empty();
}


ReaderWriter::ReadResult ReaderWriterBSP::readNode(
                                  const std::string& file,
                                  const ReaderWriter::Options* options) const
{
    VBSPReader *               vbspReader;
    Q3BSPReader *              q3bspReader;
    ref_ptr<Node>              result;
    osgDB::ifstream            stream;
    int                        magicNumber;
    int                        version;

    // See if we handle this kind of file
    if (!acceptsExtension(osgDB::getFileExtension(file)))
        return ReadResult::FILE_NOT_HANDLED;

    // See if we can find the requested file
    std::string fileName = osgDB::findDataFile(file, options);
    if (fileName.empty())
        return ReadResult::FILE_NOT_FOUND;

    // Open the file and read the magic number and version
    stream.open(fileName.c_str(), std::ios::binary);
    stream.read((char *) &magicNumber, sizeof(int));
    stream.read((char *) &version, sizeof(int));
    stream.close();

    // See which kind of BSP file this is
    if ((magicNumber == VBSP_MAGIC_NUMBER) &&
        (version >= 19) && (version <= 20))
    {
        // Read the Valve file
        vbspReader = new VBSPReader();
        if (vbspReader->readFile(fileName))
        {
            // Get the results of our read
            result = vbspReader->getRootNode();

            // Clean up the reader
            delete vbspReader;

            // Return the results
            return ReadResult(result.get());
        }
        else
        {
            // Clean up the reader
            delete vbspReader;

            // Return the error
            return ReadResult::ERROR_IN_READING_FILE;
        }
    }
    else if ((magicNumber == IBSP_MAGIC_NUMBER) && (version == 0x2E))
    {
        // Read the Quake 3 file
        q3bspReader = new Q3BSPReader();
        if (q3bspReader->readFile(file, options))
        {
            // Get the results of our read
            result = q3bspReader->getRootNode();

            // Clean up the reader
            delete q3bspReader;

            // Return the results
            return ReadResult(result.get());
        }
        else
        {
            // Clean up the reader
            delete q3bspReader;

            // Return the error
            return ReadResult::ERROR_IN_READING_FILE;
        }
    }

    return ReadResult::FILE_NOT_HANDLED;
}


REGISTER_OSGPLUGIN(bsp, ReaderWriterBSP)

