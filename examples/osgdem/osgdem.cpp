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
#include <osgDB/FileUtils>

#include <osgUtil/Optimizer>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TangentSpaceGenerator>

#include <osgFX/BumpMapping>

#include <osgProducer/Viewer>
#include <osg/Switch>

#include <osgTerrain/DataSet>

#include <ogr_spatialref.h>

class GraphicsContext {
    public:
        GraphicsContext()
        {
            rs = new Producer::RenderSurface;
            rs->setWindowRectangle(0,0,1,1);
            rs->useBorder(false);
            rs->useConfigEventThread(false);
            rs->realize();
            std::cout<<"Realized window"<<std::endl;
        }

        virtual ~GraphicsContext()
        {
        }
        
    private:
        Producer::ref_ptr<Producer::RenderSurface> rs;
};

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

osg::Matrixd computeGeoTransForRange(double xMin, double xMax, double yMin, double yMax)
{
    osg::Matrixd matrix;
    matrix(0,0) = xMax-xMin;
    matrix(3,0) = xMin;

    matrix(1,1) = yMax-yMin;
    matrix(3,1) = yMin;
    
    return matrix;
}


void ellipsodeTransformTest(double latitude, double longitude, double height)
{
    osg::ref_ptr<osg::EllipsoidModel> transform = new osg::EllipsoidModel;
    
    double X,Y,Z;
    double newLat, newLong, newHeight;
    
    transform->convertLatLongHeightToXYZ(latitude,longitude,height,
                                        X,Y,Z);
    
    transform->convertXYZToLatLongHeight(X,Y,Z,
                                        newLat,newLong,newHeight);
                                        
    std::cout<<"lat = "<<osg::RadiansToDegrees(latitude)<<"\tlong="<<osg::RadiansToDegrees(longitude)<<"\t"<<height<<std::endl;  
    std::cout<<"X = "<<X<<"\tY="<<Y<<"\tZ="<<Z<<std::endl;  
    std::cout<<"lat = "<<osg::RadiansToDegrees(newLat)<<"\tlong="<<osg::RadiansToDegrees(newLong)<<"\t"<<newHeight<<std::endl;  
}

void processFile(std::string filename,
		   osgTerrain::DataSet::Source::Type type,
		   std::string currentCS, 
		   osg::Matrixd &geoTransform,
		   bool geoTransformSet,
		   bool geoTransformScale,
		   osg::ref_ptr<osgTerrain::DataSet> dataset) {

    if(filename.empty()) return;

    if(osgDB::fileType(filename) == osgDB::REGULAR_FILE) {
	
	osgTerrain::DataSet::Source* source = new osgTerrain::DataSet::Source(type, filename);                
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
		source->setGeoTransformPolicy(geoTransformScale ? 
					      osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS_BUT_SCALE_BY_FILE_RESOLUTION : 
					      osgTerrain::DataSet::Source::PREFER_CONFIG_SETTINGS);
		source->setGeoTransform(geoTransform);
	    }
	    
	    dataset->addSource(source);
	}
    } else if (osgDB::fileType(filename) == osgDB::DIRECTORY) {

	osgDB::DirectoryContents dirContents= osgDB::getDirectoryContents(filename);
	
	// loop through directory contents and call processFile
	std::vector<std::string>::iterator i;
	std::string fullfilename;
	for(i = dirContents.begin(); i != dirContents.end(); ++i) {
	    if((*i != ".") && (*i != "..")) {
		fullfilename = filename + '/' + *i;
		processFile(fullfilename, type, currentCS, geoTransform, geoTransformSet, geoTransformScale, dataset);
	    }
	}
    }
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
    
    while (arguments.read("--HEIGHT_FIELD"))
    {
        dataset->setGeometryType(osgTerrain::DataSet::HEIGHT_FIELD);
    }

    while (arguments.read("--POLYGONAL"))
    {
        dataset->setGeometryType(osgTerrain::DataSet::POLYGONAL);
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
    bool geoTransformScale = false; 
    double xMin, xMax, yMin, yMax;
         
    int pos = 1;
    while(pos<arguments.argc())
    {
        std::string def;

        if (arguments.read(pos, "--cs",def))
        {
            currentCS = !def.empty() ? SanitizeSRS(def.c_str()) : "";
            std::cout<<"--cs "<<currentCS<<std::endl;
        }
        else if (arguments.read(pos, "--wkt",def))
        {
            currentCS = def;
            std::cout<<"--wkt "<<currentCS<<std::endl;
        }
        else if (arguments.read(pos, "--wkt_file",def))
        {
            std::ifstream in(def.c_str());
            if (in)
            {   
                currentCS = "";
                while (!in.eof())
                {
                    std::string line;
                    in >> line;
                    currentCS += line;
                }
                std::cout<<"--wkt_file "<<currentCS<<std::endl;
            }
        }
        else if (arguments.read(pos, "--geocentric"))
        {
            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--geocentric "<<currentCS<<std::endl;
        }

        else if (arguments.read(pos, "--bluemarble-east"))
        {
            currentCS = SanitizeSRS("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(0.0, 180.0, -90.0, 90.0);
            
            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--bluemarble-west"<<currentCS<<" matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--bluemarble-west"))
        {
            currentCS = SanitizeSRS("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(-180.0, 0.0, -90.0, 90.0);
            
            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--bluemarble-west "<<currentCS<<" matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--whole-globe"))
        {
            currentCS = SanitizeSRS("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(-180.0, 180.0, -90.0, 90.0);
            
            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--whole-globe "<<currentCS<<" matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--range", xMin, xMax, yMin, yMax))
        {
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange( xMin, xMax, yMin, yMax);
            
            std::cout<<"--range, matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--identity"))
        {
            geoTransformSet = false;
            geoTransform.makeIdentity();            
        }

        // x vector
        else if (arguments.read(pos, "--xx",geoTransform(0,0)))
        {
           geoTransformSet = true;
           geoTransformScale = false;
            std::cout<<"--xx "<<geoTransform(0,0)<<std::endl;
        }
        else if (arguments.read(pos, "--xy",geoTransform(1,0)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--xy "<<geoTransform(1,0)<<std::endl;
        }
        else if (arguments.read(pos, "--xz",geoTransform(2,0)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--xz "<<geoTransform(2,0)<<std::endl;
        }
        else if (arguments.read(pos, "--xt",geoTransform(3,0)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--xo "<<geoTransform(3,0)<<std::endl;
        }

        // y vector
        else if (arguments.read(pos, "--yx",geoTransform(0,1)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--yx "<<geoTransform(0,1)<<std::endl;
        }
        else if (arguments.read(pos, "--yy",geoTransform(1,1)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--yy "<<geoTransform(1,1)<<std::endl;
        }
        else if (arguments.read(pos, "--yz",geoTransform(2,1)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--yz "<<geoTransform(2,1)<<std::endl;
        }
        else if (arguments.read(pos, "--yt",geoTransform(3,1)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--yt "<<geoTransform(3,1)<<std::endl;
        }

        // z vector
        else if (arguments.read(pos, "--zx",geoTransform(0,2)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--zx "<<geoTransform(0,2)<<std::endl;
        }
        else if (arguments.read(pos, "--zy",geoTransform(1,2)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--zy "<<geoTransform(1,2)<<std::endl;
        }
        else if (arguments.read(pos, "--zz",geoTransform(2,2)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--zz "<<geoTransform(2,2)<<std::endl;
        }
        else if (arguments.read(pos, "--zt",geoTransform(3,2)))
        {
            geoTransformSet = true;
            geoTransformScale = false;
            std::cout<<"--zt "<<geoTransform(3,2)<<std::endl;
        }

        else if (arguments.read(pos, "-d",filename))
        {
	    std::cout<<"-d "<<filename<<std::endl;
	    processFile(filename, osgTerrain::DataSet::Source::HEIGHT_FIELD, currentCS, geoTransform, geoTransformSet, geoTransformScale, dataset);
        }
        else if (arguments.read(pos, "-t",filename))
        {
	    std::cout<<"-t "<<filename<<std::endl;
	    processFile(filename, osgTerrain::DataSet::Source::IMAGE, currentCS, geoTransform, geoTransformSet, geoTransformScale, dataset);
        }
        else if (arguments.read(pos, "-m",filename))
        {
	    std::cout<<"-m "<<filename<<std::endl;
	    processFile(filename, osgTerrain::DataSet::Source::MODEL, currentCS, geoTransform, geoTransformSet, geoTransformScale, dataset);
        }
        else if (arguments.read(pos, "-o",filename)) 
        {
            std::cout<<"-o "<<filename<<std::endl;

            std::string path = osgDB::getFilePath(filename);
            std::string base = path.empty()?osgDB::getStrippedName(filename):
                                            path +'/'+ osgDB::getStrippedName(filename);
            std::string extension = '.'+osgDB::getLowerCaseFileExtension(filename);

            dataset->setDestinationTileBaseName(base);
            dataset->setDestinationTileExtension(extension);
            
            if (!currentCS.empty()) dataset->setDestinationCoordinateSystem(currentCS);
        }
        else
        {
            // if no argument read advance to next argument.
            ++pos;
        }
    }
    
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    // generate the database
    {
        GraphicsContext context;

        dataset->loadSources();

        dataset->createDestination((unsigned int)numLevels);

        dataset->writeDestination();        
    }

    return 0;
}

