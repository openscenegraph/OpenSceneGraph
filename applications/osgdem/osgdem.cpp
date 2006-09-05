/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#ifdef WIN32
    #pragma warning(disable : 4503)
#endif // WIN32

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/State>
#include <osg/ShapeDrawable>
#include <osg/io_utils>

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
                   bool minmaxLevelSet, unsigned int min_level, unsigned int max_level,
                   unsigned int layerNum,
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
            if (minmaxLevelSet) 
            {
                source->setMinMaxLevel(min_level, max_level);
            }
            
            source->setLayer(layerNum);
            
            std::cout<<"Reading as layer = "<<layerNum<<std::endl;
            
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
                processFile(fullfilename, type, currentCS, 
                            geoTransform, geoTransformSet, geoTransformScale, 
                            minmaxLevelSet, min_level, max_level,
                            layerNum,
                            dataset);
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
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" application is utility tools which can be used to generate paged geospatial terrain databases.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-d <filename>","Specify the digital elevation map input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-t <filename>","Specify the texture map input file to process");
//    arguments.getApplicationUsage()->addCommandLineOption("-m <filename>","Specify the 3D database model input file to process");
    arguments.getApplicationUsage()->addCommandLineOption("-a <archivename>","Specify the archive to place the generated database");
    arguments.getApplicationUsage()->addCommandLineOption("-o <outputfile>","Specify the output master file to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-l <numOfLevels>","Specify the number of PagedLOD levels to generate");
    arguments.getApplicationUsage()->addCommandLineOption("--image-ext <ext>","Specify the Image format to output to via its plugin name, i.e. rgb, dds, jp2, jpeg.");
    arguments.getApplicationUsage()->addCommandLineOption("--levels <begin_level> <end_level>","Specify the range of lavels that the next source Texture or DEM will contribute to.");
    arguments.getApplicationUsage()->addCommandLineOption("--layer <layer_num>","Specify the layer that the next source Texture will contribute to..");
    arguments.getApplicationUsage()->addCommandLineOption("-e <x> <y> <w> <h>","Extents of the model to generate");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("--cs <coordinates system string>","Set the coordinates system of source imagery, DEM or destination database. The string may be any of the usual GDAL/OGR forms, complete WKT, PROJ.4, EPS");     
    arguments.getApplicationUsage()->addCommandLineOption("--wkt <WKT string>","Set the coordinates system of source imagery, DEM or destination database in WellKownText form.");     
    arguments.getApplicationUsage()->addCommandLineOption("--wkt-file <WKT file>","Set the coordinates system of source imagery, DEM or destination database by as file containing WellKownText definition.");     
    arguments.getApplicationUsage()->addCommandLineOption("--skirt-ratio <float>","Set the ratio of skirt height to tile size");     
    arguments.getApplicationUsage()->addCommandLineOption("--HEIGHT_FIELD","Create a height field database");     
    arguments.getApplicationUsage()->addCommandLineOption("--POLYGONAL","Create a height field database");     
    arguments.getApplicationUsage()->addCommandLineOption("--LOD","Create a LOD'd database");     
    arguments.getApplicationUsage()->addCommandLineOption("--PagedLOD","Create a PagedLOD'd database");     
    arguments.getApplicationUsage()->addCommandLineOption("-v","Set the vertical multiplier");     
    arguments.getApplicationUsage()->addCommandLineOption("--compressed","Use OpenGL compression on RGB destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--RGBA-compressed","Use OpenGL compression on RGBA destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--RGB-16","Use 16bit RGB destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--RGBA-24","Use 24bit RGB destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--RGB-16","Use 16bit RGBA destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--RGBA","Use 32bit RGBA destination imagery");     
    arguments.getApplicationUsage()->addCommandLineOption("--max-visible-distance-of-top-level","Set the maximum visible distance that the top most tile can be viewed at");     
    arguments.getApplicationUsage()->addCommandLineOption("--no-terrain-simplification","Switch off terrain simplification.");
    arguments.getApplicationUsage()->addCommandLineOption("--default-color <r,g,b,a>","Sets the default color of the terrain.");
    arguments.getApplicationUsage()->addCommandLineOption("--radius-to-max-visible-distance-ratio","Set the maximum visible distance ratio for all tiles apart from the top most tile. The maximum visuble distance is computed from the ratio * tile radius.");     
    arguments.getApplicationUsage()->addCommandLineOption("--no-mip-mapping","Disable mip mapping of textures");     
    arguments.getApplicationUsage()->addCommandLineOption("--mip-mapping-hardware","Use mip mapped textures, and generate the mipmaps in hardware when available.");     
    arguments.getApplicationUsage()->addCommandLineOption("--mip-mapping-imagery","Use mip mapped textures, and generate the mipmaps in imagery.");     
    arguments.getApplicationUsage()->addCommandLineOption("--max-anisotropy","Max anisotropy level to use when texturing, defaults to 1.0.");
    arguments.getApplicationUsage()->addCommandLineOption("--bluemarble-east","Set the coordinates system for next texture or dem to represent the eastern hemisphere of the earth.");     
    arguments.getApplicationUsage()->addCommandLineOption("--bluemarble-west","Set the coordinates system for next texture or dem to represent the western hemisphere of the earth.");     
    arguments.getApplicationUsage()->addCommandLineOption("--whole-globe","Set the coordinates system for next texture or dem to represent the whole hemisphere of the earth.");
    arguments.getApplicationUsage()->addCommandLineOption("--geocentric","");
    arguments.getApplicationUsage()->addCommandLineOption("--range","");     
    arguments.getApplicationUsage()->addCommandLineOption("--xx","");     
    arguments.getApplicationUsage()->addCommandLineOption("--xt","");     
    arguments.getApplicationUsage()->addCommandLineOption("--yy","");     
    arguments.getApplicationUsage()->addCommandLineOption("--yt","");     
    arguments.getApplicationUsage()->addCommandLineOption("--zz","");     
    arguments.getApplicationUsage()->addCommandLineOption("--zt","");     
    arguments.getApplicationUsage()->addCommandLineOption("--tile-image-size","Set the tile maximum image size");
    arguments.getApplicationUsage()->addCommandLineOption("--tile-terrain-size","Set the tile maximum terrain size");
    arguments.getApplicationUsage()->addCommandLineOption("--comment","Added a comment/description string to the top most node in the dataset");     
        
    // create DataSet.
    osg::ref_ptr<osgTerrain::DataSet> dataset = new osgTerrain::DataSet;


    float x,y,w,h;
    while (arguments.read("-e",x,y,w,h))
    {
        dataset->setDestinationExtents(osgTerrain::GeospatialExtents(x,y,x+w,y+h,false)); // FIXME - need to check whether we a geographic extents of not
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

    while (arguments.read("--compressed")) { dataset->setTextureType(osgTerrain::DataSet::COMPRESSED_TEXTURE); }
    while (arguments.read("--RGBA-compressed")) { dataset->setTextureType(osgTerrain::DataSet::COMPRESSED_RGBA_TEXTURE); }
    while (arguments.read("--RGB_16") || arguments.read("--RGB-16") ) { dataset->setTextureType(osgTerrain::DataSet::RGB_16); }
    while (arguments.read("--RGBA_16") || arguments.read("--RGBA-16") ) { dataset->setTextureType(osgTerrain::DataSet::RGBA_16); }
    while (arguments.read("--RGB_24") || arguments.read("--RGB-24") ) { dataset->setTextureType(osgTerrain::DataSet::RGB_24); }
    while (arguments.read("--RGBA") || arguments.read("--RGBA") ) { dataset->setTextureType(osgTerrain::DataSet::RGBA); }

    while (arguments.read("--no_mip_mapping") || arguments.read("--no-mip-mapping")) { dataset->setMipMappingMode(osgTerrain::DataSet::NO_MIP_MAPPING); }
    while (arguments.read("--mip_mapping_hardware") || arguments.read("--mip-mapping-hardware")) { dataset->setMipMappingMode(osgTerrain::DataSet::MIP_MAPPING_HARDWARE); }
    while (arguments.read("--mip_mapping_imagery") || arguments.read("--mip-mapping-imagery")) { dataset->setMipMappingMode(osgTerrain::DataSet::MIP_MAPPING_IMAGERY); }

    float maxAnisotropy;
    while (arguments.read("--max_anisotropy",maxAnisotropy) || arguments.read("--max-anisotropy",maxAnisotropy))
    {
        dataset->setMaxAnisotropy(maxAnisotropy);
    }

    unsigned int image_size;
    while (arguments.read("--tile-image-size",image_size)) { dataset->setMaximumTileImageSize(image_size); }

    unsigned int terrain_size;
    while (arguments.read("--tile-terrain-size",terrain_size)) { dataset->setMaximumTileTerrainSize(terrain_size); }

    std::string comment;
    while (arguments.read("--comment",comment)) { dataset->setCommentString(comment); }

    std::string archiveName;
    while (arguments.read("-a",archiveName)) { dataset->setArchiveName(archiveName); }

    dataset->setDestinationName("output.ive");


    unsigned int numLevels = 10;
    while (arguments.read("-l",numLevels)) {}

    float verticalScale;
    while (arguments.read("-v",verticalScale))
    {
        dataset->setVerticalScale(verticalScale);
    }

    float skirtRatio;
    while (arguments.read("--skirt-ratio",skirtRatio))
    {
        dataset->setSkirtRatio(skirtRatio);
    }

    float maxVisibleDistanceOfTopLevel;
    while (arguments.read("--max_visible_distance_of_top_level",maxVisibleDistanceOfTopLevel) ||
          arguments.read("--max-visible-distance-of-top-level",maxVisibleDistanceOfTopLevel) )
    {
        dataset->setMaximumVisibleDistanceOfTopLevel(maxVisibleDistanceOfTopLevel);
    }

    float radiusToMaxVisibleDistanceRatio;
    while (arguments.read("--radius_to_max_visible_distance_ratio",radiusToMaxVisibleDistanceRatio) ||
           arguments.read("--radius-to-max-visible-distance-ratio",radiusToMaxVisibleDistanceRatio))
    {
        dataset->setRadiusToMaxVisibleDistanceRatio(radiusToMaxVisibleDistanceRatio);
    }

    while (arguments.read("--no_terrain_simplification") ||
           arguments.read("--no-terrain-simplification"))
    {
        dataset->setSimplifyTerrain(false);
    }

    std::string str;
    while (arguments.read("--default-color",str) ||
           arguments.read("--default_color",str))
    {
        osg::Vec4 defaultColor;
        if( sscanf( str.c_str(), "%f,%f,%f,%f",
                &defaultColor[0], &defaultColor[1], &defaultColor[2], &defaultColor[3] ) != 4 )
        {
            std::cout<<"Color argument format incorrect."<<std::endl;
            return 1;
        }
        dataset->setDefaultColor(defaultColor);
    }

    std::string image_ext;
    while (arguments.read("--image-ext",image_ext))
    {
        std::string::size_type dot = image_ext.find_last_of('.');
        if (dot!=std::string::npos) image_ext.erase(0,dot+1);

        osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension(image_ext);
        if (rw) 
        {
            image_ext.insert(0,".");
            dataset->setDestinationImageExtension(image_ext);
            return 1;
        }
        else
        {
            std::cout<<"Error: can not find plugin to write out image with extension '"<<image_ext<<"'"<<std::endl;
            return 1;
        }
    }


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    unsigned int maximumPossibleLevel = 30;


    // read the input data

    std::string filename;
    std::string currentCS;
    osg::Matrixd geoTransform;
    bool geoTransformSet = false; 
    bool geoTransformScale = false; 
    double xMin, xMax, yMin, yMax;
    bool minmaxLevelSet = false;
    unsigned int min_level=0, max_level=maximumPossibleLevel;
    unsigned int currentLayerNum = 0;
         
    int pos = 1;
    while(pos<arguments.argc())
    {
        std::string def;

        if (arguments.read(pos, "--cs",def))
        {
            currentCS = !def.empty() ? osgTerrain::DataSet::coordinateSystemStringToWTK(def) : "";
            std::cout<<"--cs \""<<def<<"\" converted to "<<currentCS<<std::endl;
        }
        else if (arguments.read(pos, "--wkt",def))
        {
            currentCS = def;
            std::cout<<"--wkt "<<currentCS<<std::endl;
        }
        else if (arguments.read(pos, "--wkt-file",def))
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
                std::cout<<"--wkt-file "<<currentCS<<std::endl;
            }
        }
        else if (arguments.read(pos, "--geocentric"))
        {
            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--geocentric "<<currentCS<<std::endl;
        }

        else if (arguments.read(pos, "--bluemarble-east"))
        {
            currentCS = osgTerrain::DataSet::coordinateSystemStringToWTK("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(0.0, 180.0, -90.0, 90.0);
            
//            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--bluemarble-west"<<currentCS<<" matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--bluemarble-west"))
        {
            currentCS = osgTerrain::DataSet::coordinateSystemStringToWTK("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(-180.0, 0.0, -90.0, 90.0);
            
//            dataset->setConvertFromGeographicToGeocentric(true);
            std::cout<<"--bluemarble-west "<<currentCS<<" matrix="<<geoTransform<<std::endl;
        }

        else if (arguments.read(pos, "--whole-globe"))
        {
            currentCS = osgTerrain::DataSet::coordinateSystemStringToWTK("WGS84");
            geoTransformSet = true;
            geoTransformScale = true;
            geoTransform = computeGeoTransForRange(-180.0, 180.0, -90.0, 90.0);
            
//            dataset->setConvertFromGeographicToGeocentric(true);
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

        else if (arguments.read(pos, "--levels", min_level, max_level))
        {
            minmaxLevelSet = true;
            std::cout<<"--levels, min_level="<<min_level<<"  max_level="<<max_level<<std::endl;
        }
        
        else if (arguments.read(pos, "--layer", currentLayerNum))
        {
            std::cout<<"--layer layeNumber="<<currentLayerNum<<std::endl;
        }

        else if (arguments.read(pos, "-d",filename))
        {
            std::cout<<"-d "<<filename<<std::endl;
            processFile(filename, osgTerrain::DataSet::Source::HEIGHT_FIELD, currentCS, 
                        geoTransform, geoTransformSet, geoTransformScale,
                        minmaxLevelSet, min_level, max_level,
                        currentLayerNum,
                        dataset);

            minmaxLevelSet = false;
            min_level=0; max_level=maximumPossibleLevel;
            currentLayerNum = 0;
            
            currentCS = "";
            geoTransformSet = false;
            geoTransformScale = false;
            geoTransform.makeIdentity();

        }
        else if (arguments.read(pos, "-t",filename))
        {
            std::cout<<"-t "<<filename<<std::endl;
            processFile(filename, osgTerrain::DataSet::Source::IMAGE, currentCS, 
                        geoTransform, geoTransformSet, geoTransformScale, 
                        minmaxLevelSet, min_level, max_level, 
                        currentLayerNum,
                        dataset);

            minmaxLevelSet = false;
            min_level=0; max_level=maximumPossibleLevel;
            currentLayerNum = 0;
            
            currentCS = "";
            geoTransformSet = false;
            geoTransformScale = false;
            geoTransform.makeIdentity();            
        }
/*        
        else if (arguments.read(pos, "-m",filename))
        {
            std::cout<<"-m "<<filename<<std::endl;
            processFile(filename, osgTerrain::DataSet::Source::MODEL, currentCS, 
                        geoTransform, geoTransformSet, geoTransformScale, 
                        minmaxLevelSet, min_level, max_level, 
                        currentLayerNum, 
                        dataset);

            minmaxLevelSet = false;
            min_level=0; max_level=maximumPossibleLevel;
            currentLayerNum = 0;
            
            currentCS = "";
            geoTransformSet = false;
            geoTransformScale = false;
            geoTransform.makeIdentity();            
        }
*/
        else if (arguments.read(pos, "-o",filename)) 
        {
            std::cout<<"-o "<<filename<<std::endl;
            dataset->setDestinationName(filename);
            
            if (!currentCS.empty()) dataset->setDestinationCoordinateSystem(currentCS);

            minmaxLevelSet = false;
            min_level=0; max_level=maximumPossibleLevel;
            
            currentCS = "";
            geoTransformSet = false;
            geoTransformScale = false;
            geoTransform.makeIdentity();            

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

