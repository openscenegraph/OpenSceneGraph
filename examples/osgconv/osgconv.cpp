#include <stdio.h>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Geometry>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/ReaderWriter>

#include <osgUtil/Optimizer>

#include "OrientationConverter.h"
#include "GeoSet.h"

typedef std::vector<std::string> FileNameList;

static bool do_convert = false;

////////////////////////////////////////////////////////////////////////////
// Convert GeoSet To Geometry Visitor.
////////////////////////////////////////////////////////////////////////////

/** ConvertGeoSetsToGeometryVisitor all the old GeoSet Drawables to the new Geometry Drawables.*/
class ConvertGeoSetsToGeometryVisitor : public osg::NodeVisitor
{
public:

    ConvertGeoSetsToGeometryVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(osg::Geode& geode)
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::GeoSet* geoset = dynamic_cast<osg::GeoSet*>(geode.getDrawable(i));
            if (geoset)
            {
                osg::Geometry* geom = geoset->convertToGeometry();
                if (geom)
                {
                    std::cout<<"Successfully converted GeoSet to Geometry"<<std::endl;
                    geode.replaceDrawable(geoset,geom);
                }
                else
                {
                    std::cout<<"*** Failed to convert GeoSet to Geometry"<<std::endl;
                }

            }
        }
    }

    virtual void apply(osg::Node& node) { traverse(node); }

};

static void usage( const char *prog, const char *msg )
{
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE) << msg << std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"usage:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    " << prog << " [options] infile1 [infile2 ...] outfile"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"options:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    -O option          - ReaderWriter option"<< std::endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -o orientation     - Convert geometry from input files to output files."<< std::endl;
    osg::notify(osg::NOTICE)<<
                              "                         Format of orientation argument must be the following:\n"
			      "\n"
                              "                             X1,Y1,Z1-X2,Y2,Z2\n"
			      "\n"
                              "                         where X1,Y1,Z1 represent the UP vector in the input\n"
			      "                         files and X2,Y2,Z2 represent the UP vector of the\n"
			      "                         output file.  For example, to convert a model built\n"
			      "                         in a Y-Up coordinate system to a model with a Z-up\n"
			      "                         coordinate system, the argument looks like\n"
			      "\n"
			      "                             0,1,0-0,0,1"
			      "\n"
			      << std::endl;
    osg::notify(osg::NOTICE)<<"    -t translation     - Convert spatial position of output files.  Format of\n"
                              "                         translation argument must be the following :\n"
			      "\n"
			      "                             X,Y,Z\n"
			      "\n"
			      "                         where X, Y, and Z represent the coordinates of the\n"
			      "                         absolute position in world space\n"
			      << std::endl;
    osg::notify(osg::NOTICE)<<"    -s scale           - Scale size of model.  Scale argument must be the \n"
                              "                         following :\n"
			      "\n"
			      "                             SX,SY,SZ\n"
			      "\n"
			      "                         where SX, SY, and SZ represent the scale factors\n"
			      "                         Caution: Scaling will be done in destination orientation\n"
			      << std::endl;
}

static bool 
parse_args( int argc, char **argv, FileNameList &fileNames,
            OrientationConverter &oc, osgDB::ReaderWriter::Options* options )
{
    int nexti;
    std::string opt = "";

    for(int i = 1; i < argc; i=nexti )
    {
        nexti = i+1;

        if (argv[i][0]=='-')
        {
  	    for( unsigned int j = 1; j < strlen( argv[i] ); j++ )
	    {
                switch(argv[i][j])
                {
                    case 'O':
                        if (nexti<argc) {
                            if (opt.size() == 0)
                                opt = argv[nexti++];
                            else
                                opt = opt+" "+argv[nexti++];
                        }
                        else {
			    usage( argv[0], "ReaderWriter option requires an argument." );
			    return false;
                        }
                        break;

                    case('e'):
                        if (nexti<argc)
                        {
                            std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension(argv[nexti++]);
                            osgDB::Registry::instance()->loadLibrary(libName);
                        }
			else
			{
			    usage( argv[0], "Extension option requires an argument." );
			    return false;
			}
                        break;

                    case('l'):
                        if (nexti<argc)
                        {
                            osgDB::Registry::instance()->loadLibrary(argv[nexti++]);
                        }
			else
			{
			    usage( argv[0], "Library option requires an argument." );
			    return false;
			}
                        break;

		    case 'o' :
		        if( nexti < argc )
			{
			    osg::Vec3 from, to;
			    if( sscanf( argv[nexti++], "%f,%f,%f-%f,%f,%f",
			    	&from[0], &from[1], &from[2],
			    	&to[0], &to[1], &to[2]  )
				!= 6 )
			    {
			        usage( argv[0], "Orientation argument format incorrect." );
				return false;
			    }
			    oc.setRotation( from, to );
			    do_convert = true;
			}
			else
			{
			    usage( argv[0], "Orientation conversion option requires an argument." );
			    return false;
			}
		        break;

		    case 't' :
		        if( nexti < argc )
			{
			    osg::Vec3 trans(0,0,0);
			    if( sscanf( argv[nexti++], "%f,%f,%f",
			    	&trans[0], &trans[1], &trans[2] ) != 3 )
			    {
			        usage( argv[0], "Translation argument format incorrect." );
				return false;
			    }
			    oc.setTranslation( trans );
			    do_convert = true;
			}
			else
			{
			    usage( argv[0], "Translation conversion option requires an argument." );
			    return false;
			}
		        break;

		    case 's' :
		        if( nexti < argc )
			{
			    osg::Vec3 scale(0,0,0);
			    if( sscanf( argv[nexti++], "%f,%f,%f",
			    	&scale[0], &scale[1], &scale[2] ) != 3 )
			    {
			        usage( argv[0], "Scale argument format incorrect." );
				return false;
			    }
			    oc.setScale( scale );
			    do_convert = true;
			}
			else
			{
			    usage( argv[0], "Scale conversion option requires an argument." );
			    return false;
			}
		        break;

		    default :
		        std::string a = "Invalid option " ;
			a += "'"; 
			a += argv[i][j] ;
			a += "'.";
		        usage( argv[0], a.c_str() );
			return false;
			break;
                }
	    }
        } else
        {
            fileNames.push_back(argv[i]);
        }
    }

    if (fileNames.empty())
    {
	usage( argv[0], "No files specified." );
        return false;
    }

    options->setOptionString(opt);

    return true;
}

int main( int argc, char **argv )
{
    FileNameList fileNames;
    OrientationConverter oc;

    osgDB::ReaderWriter::Options* options = new osgDB::ReaderWriter::Options;
    osgDB::Registry::instance()->setOptions(options);

    if( parse_args( argc, argv, fileNames, oc, options ) == false )
        return -1;
     
    std::string fileNameOut("converted.osg");
    if (fileNames.size()>1)
    {
        fileNameOut = fileNames.back();
        fileNames.pop_back();
    }

    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(fileNames);
    
    // convert the old style GeoSet to Geometry
    ConvertGeoSetsToGeometryVisitor cgtg;
    root->accept(cgtg);

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(root.get());
    

    if( do_convert )
	root = oc.convert( root.get() );

    if (root.valid())
    {
        if (osgDB::writeNodeFile(*root,fileNameOut))
        {
            osg::notify(osg::NOTICE)<<"Data written to '"<<fileNameOut<<"'."<< std::endl;
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Error no data loaded."<< std::endl;
        return 1;
    }

    return 0;
}
