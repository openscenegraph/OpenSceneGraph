#include <stdio.h>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include "OrientationConverter.h"

typedef std::vector<std::string> FileNameList;

static bool do_convert = false;


static void usage( const char *prog, const char *msg )
{
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE) << msg << endl;
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"usage:"<<endl;
    osg::notify(osg::NOTICE)<<"    " << prog << " [options] infile1 [infile2 ...] outfile"<<endl;
    osg::notify(osg::NOTICE)<<endl;
    osg::notify(osg::NOTICE)<<"options:"<<endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
    osg::notify(osg::NOTICE)<<"    -o orientation     - Convert geometry from input files to output files."<<endl;

    osg::notify(osg::NOTICE)<<
                              "                         Format of orientation argument must be the following:\n"
			      "\n"
                              "                             X1,Y1,Z1-X2,Y2,Z2\n"
			      "\n"
                              "                         where X1,Y1,Z1 represent the UP vector in the input\n"
			      "                         files and X2,Y2,Z2 represent the UP vector of the output file.\n"
			      "                         For example, to convert a model built in a Y-Up coordinate system\n"
			      "                         to a model with a Z-up coordinate system, the argument looks like\n"
			      "\n"
			      "                             0,1,0-0,0,1"
			      "\n"
			      << endl;
}

static bool 
parse_args( int argc, char **argv, FileNameList &fileNames, OrientationConverter &oc )
{
    int nexti;

    for(int i = 1; i < argc; i=nexti )
    {
        nexti = i+1;

        if (argv[i][0]=='-')
        {
  	    for( unsigned int j = 1; j < strlen( argv[i] ); j++ )
	    {
                switch(argv[i][j])
                {
                    case('e'):
                        if (nexti<argc)
                        {
                            std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[nexti++]);
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
			    	&to[0], &to[1], &to[2] ) != 6 )
			    {
			        usage( argv[0], "Orientation argument format incorrect." );
				return false;
			    }
			    oc.setConversion( from, to );
			    do_convert = true;
			}
			else
			{
			    usage( argv[0], "Orientation conversion option requires an argument." );
			    return false;
			}
		        break;

		    default :
		        std::string a = "Invalide option " ;
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
    return true;
}

int main( int argc, char **argv )
{
    FileNameList fileNames;
    OrientationConverter oc;

    if( parse_args( argc, argv, fileNames, oc ) == false )
        return -1;

    if (fileNames.size()==1)
    {
        osg::Node* root = osgDB::readNodeFile(fileNames.front());

	if( do_convert )
	    oc.convert( *root );

        if (root)
        {
            osgDB::writeNodeFile(*root,"converted.osg");
            osg::notify(osg::NOTICE)<<"Data written to 'converted.osg'."<<endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Error no data loaded."<<endl;
            return 1;
        }
    }
    else
    {
        std::string fileNameOut = fileNames.back();
        fileNames.pop_back();

        osg::Group* group = new osg::Group();
        for(FileNameList::iterator itr=fileNames.begin();
            itr<fileNames.end();
            ++itr)
        {
            osg::Node* child = osgDB::readNodeFile(*itr);
            if (child)
            {
                group->addChild(child);
            }
        }
	if( do_convert )
	    oc.convert(*group);

        if (group->getNumChildren()==0)
        {

            osg::notify(osg::NOTICE)<<"Error no data loaded."<<endl;
            return 1;
        }
        else if (group->getNumChildren()==1)
        {
            osgDB::writeNodeFile(*(group->getChild(0)),fileNameOut);
        }
        else
        {
            osgDB::writeNodeFile(*group,fileNameOut);
        }
    }

    return 0;
}
