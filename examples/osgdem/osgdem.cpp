/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/State>
#include <osg/ShapeDrawable>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ImageOptions>
#include <osgDB/FileNameUtils>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TangentSpaceGenerator>

#include <osgFX/BumpMapping>

#include <osgProducer/Viewer>

#include "DataSet.h"

#include <osg/Switch>

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-d <filename>","Specify the digital elevation map input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-t <filename>","Specify the texture map input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-m <filename>","Specify the 3D database model input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-o <outputfile>","Specify the output master file to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-l <numOfLevels>","Specify the number of PagedLOD levels to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-e <x> <y> <w> <h>","Extents of the model to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // create DataSet.
    osg::ref_ptr<DataSet> dataset = new DataSet;

    std::string filename;
    while (arguments.read("-d",filename))
    {
        if (!filename.empty()) dataset->addSource(new DataSet::Source(DataSet::Source::HEIGHT_FIELD,filename));
    }
    
    while (arguments.read("-t",filename))
    {
        if (!filename.empty()) dataset->addSource(new DataSet::Source(DataSet::Source::IMAGE,filename));
    }

    while (arguments.read("-m",filename))
    {
        if (!filename.empty()) dataset->addSource(new DataSet::Source(DataSet::Source::MODEL,filename));
    }

    float x,y,w,h;
    while (arguments.read("-e",x,y,w,h))
    {
        dataset->setDestinationExtents(osg::BoundingBox(x,y,0.0f,x+w,y+h,0.0f));
    }

    dataset->setDestinationTileBaseName("output");
    dataset->setDestinationTileExtension(".ive");

    std::string outputFileName("output.ive");
    while (arguments.read("-o",outputFileName)) 
    {
        std::string path = osgDB::getFilePath(outputFileName);
        std::string base = path.empty()?osgDB::getStrippedName(outputFileName):
                                        path +'/'+ osgDB::getStrippedName(outputFileName);
        std::string extension = '.'+osgDB::getLowerCaseFileExtension(outputFileName);

        dataset->setDestinationTileBaseName(base);
        dataset->setDestinationTileExtension(extension);

    }

    float numLevels = 6.0f;
    while (arguments.read("-l",numLevels)) {}

    float verticalScale;
    while (arguments.read("-v",verticalScale)) {}


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    dataset->loadSources();

    dataset->createDestination((unsigned int)numLevels);
    
    dataset->writeDestination(outputFileName);

    return 0;
}

