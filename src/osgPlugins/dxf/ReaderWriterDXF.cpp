/* dxfReader for OpenSceneGraph  Copyright (C) 2005 by GraphArchitecture ( grapharchitecture.com )
 * Programmed by Paul de Repentigny <pdr@grapharchitecture.com>
 * 
 * OpenSceneGraph is (C) 2004 Robert Osfield
 * 
 * This library is provided as-is, without support of any kind.
 *
 * Read DXF docs or OSG docs for any related questions.
 * 
 * You may contact the author if you have suggestions/corrections/enhancements.
 */
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <map>
#include <iostream>
#include <utility>

#include <string>
#include <sstream>
#include <string.h>

#include "dxfFile.h"

using namespace osg;
using namespace osgDB;
using namespace std;


class ReaderWriterdxf : public osgDB::ReaderWriter
{
public:
    ReaderWriterdxf()
    {
        supportsExtension("dxf","Autodesk DXF format");
    }
    
    virtual const char* className() { return "Autodesk DXF Reader"; }
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
protected:
};

// register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(dxf, ReaderWriterdxf)


// read file and convert to OSG.
osgDB::ReaderWriter::ReadResult 
ReaderWriterdxf::readNode(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    // extract accuracy options if available
    if (options) {
        bool useAccuracy=false;  // if we specify accuracy of curve rendering or not
        double maxError=0.0;     // if useAccuracy - the accuracy (max deviation) from the arc
        bool improveAccuracyOnly=false; // if true only use the given accuracy if it would improve the curve compared to the previous implementation
                                        // Thus you can ensure that large curves get rendered better but small ones don't get worse
        
        std::string optionsstring=options->getOptionString();
        
        size_t accstart=optionsstring.find("Accuracy(");
        if (accstart>=0) {
            const char* start=optionsstring.c_str() + accstart + strlen("Accuracy(");
            if (sscanf(start,"%lf",&maxError)==1) useAccuracy=true;
        }
        if (useAccuracy) {
            // Option to only use the new accuracy code when it would improve on the accuracy of the old method
            if (optionsstring.find("ImproveAccuracyOnly") != std::string::npos) {
                improveAccuracyOnly=true;
            } 
            // Pull out the initial dxfArc copy from the registry and set accuracy there. 
            // When actual dxfArcs/Circles are created they will inherit these parameters from the exemplar
            dxfEntity::getRegistryEntity("ARC")->setAccuracy(true,maxError,improveAccuracyOnly);
            dxfEntity::getRegistryEntity("CIRCLE")->setAccuracy(true,maxError,improveAccuracyOnly); 
        } // accuracy options exists
    } // options exist


    // Open
    dxfFile df(filename);
    if (df.parseFile()) {
        // convert to OSG
        osg::Group* osg_top = df.dxf2osg();
        return (osg_top);
    }
    return ReadResult::FILE_NOT_HANDLED;
}

