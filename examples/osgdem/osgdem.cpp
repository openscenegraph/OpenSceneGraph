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
#include <osg/Switch>

#include <osgTerrain/DataSet>

#include <ogr_spatialref.h>


char *SanitizeSRS( const char *pszUserInput )

{
    OGRSpatialReferenceH hSRS;
    char *pszResult = NULL;

    CPLErrorReset();
    
    hSRS = OSRNewSpatialReference( NULL );
    if( OSRSetFromUserInput( hSRS, pszUserInput ) == OGRERR_NONE )
        OSRExportToWkt( hSRS, &pszResult );
    else
    {
        CPLError( CE_Failure, CPLE_AppDefined,
                  "Translating source or target SRS failed:\n%s",
                  pszUserInput );
        exit( 1 );
    }
    
    OSRDestroySpatialReference( hSRS );

    return pszResult;
}


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
    arguments.getApplicationUsage()->addCommandLineOption("--o_cs <coordinates system string>","Set the output coordinates system. The string may be any of the usual GDAL/OGR forms, complete WKT, PROJ.4, EPS");     
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    // create DataSet.
    osg::ref_ptr<osgTerrain::DataSet> dataset = new osgTerrain::DataSet;


    float x,y,w,h;
    while (arguments.read("-e",x,y,w,h))
    {
        dataset->setDestinationExtents(osg::BoundingBox(x,y,0.0f,x+w,y+h,0.0f));
    }
    
    while (arguments.read("--LOD"))
    {
        dataset->setDatabaseType(osgTerrain::DataSet::LOD_DATABASE);
    }
    
    while (arguments.read("--PagedLOD"))
    {
        dataset->setDatabaseType(osgTerrain::DataSet::PagedLOD_DATABASE);
    }

    dataset->setDestinationTileBaseName("output");
    dataset->setDestinationTileExtension(".ive");


    float numLevels = 6.0f;
    while (arguments.read("-l",numLevels)) {}

    float verticalScale;
    while (arguments.read("-v",verticalScale))
    {
        dataset->setVerticalScale(verticalScale);
    }


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }


    // read the input data

    std::string filename;
    std::string currentCS;
    osg::Matrixd geoTransform;
    bool geoTransformSet = false; 
    
    bool argumentRead = true;
    int pos = 1;
    while(pos<arguments.argc())
    {
        argumentRead = false;
    
        std::string def;
        if (arguments.read(pos, "--cs",def))
        {
            currentCS = !def.empty() ? SanitizeSRS(def.c_str()) : "";
            std::cout<<"--cs "<<currentCS<<std::endl;
            argumentRead = true;
        }
        
        

        osgTerrain::DataSet::Source* source = 0;
        if (arguments.read(pos, "-d",filename))
        {
            argumentRead = true;

            if (!filename.empty())
            {
                std::cout<<"-d "<<filename<<std::endl;
            
                source = new osgTerrain::DataSet::Source(osgTerrain::DataSet::Source::HEIGHT_FIELD,filename);                
            }
        }

        if (arguments.read(pos, "-t",filename))
        {
            argumentRead = true;

            if (!filename.empty())
            {
                std::cout<<"-t "<<filename<<std::endl;
                source = new osgTerrain::DataSet::Source(osgTerrain::DataSet::Source::IMAGE,filename);
            }
        }

        if (arguments.read(pos, "-m",filename))
        {
            argumentRead = true;

            if (!filename.empty())
            {
                std::cout<<"-m "<<filename<<std::endl;
                source = new osgTerrain::DataSet::Source(osgTerrain::DataSet::Source::MODEL,filename);
            }
        }

        if (source)
        {
            if (!currentCS.empty())
            {
                std::cout<<"source->setCoordySystem "<<currentCS<<std::endl;
                source->setCoordinateSystemPolicy(osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS);
                source->setCoordinateSystem(currentCS);
            } 

            if (geoTransformSet)
            {
                std::cout<<"source->setGeoTransform "<<geoTransform<<std::endl;
                source->setGeoTransformPolicy(osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS);
                source->setGeoTransform(geoTransform);
            }

            dataset->addSource(source);
        }

        if (arguments.read(pos, "-o",filename)) 
        {
            std::cout<<"-o "<<filename<<std::endl;

            argumentRead = true;

            std::string path = osgDB::getFilePath(filename);
            std::string base = path.empty()?osgDB::getStrippedName(filename):
                                            path +'/'+ osgDB::getStrippedName(filename);
            std::string extension = '.'+osgDB::getLowerCaseFileExtension(filename);

            dataset->setDestinationTileBaseName(base);
            dataset->setDestinationTileExtension(extension);
            
            if (!currentCS.empty()) dataset->setDestinationCoordinateSystem(currentCS);
        }
        
        // if no argument read advance to next argument.
        if (!argumentRead) ++pos;

    }
    
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    return 0;
    
    dataset->loadSources();

    dataset->createDestination((unsigned int)numLevels);
    
    dataset->writeDestination();

    return 0;
}

