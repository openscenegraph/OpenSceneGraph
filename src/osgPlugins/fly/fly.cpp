#include <stdio.h>
#include <string.h>
#include <osg/OSG>
#include <osg/Geode> 
#include <osg/Group>
#include <osg/Registry>
#include <osg/Notify>

using namespace osg;

extern Node *makeTerrain( void );
extern Node *makeTrees( void );
extern Node *makeTank( void );
extern Node *makeWindsocks( void );
extern Node *makeGliders( void );
extern Node *makeGlider( void );
extern Node *makeSky( void );
extern Node *makeBase( void );
extern Node *makeClouds( void );


extern "C" {
Node* LoadFile_fly( const char *file );
void InitConverter_fly( void );
}

static struct _nodes {
    char *name;
    Node *(*fptr)(void);
}nodes[] = {
    { "terrain", makeTerrain },
    { "tank",    makeTank   },
    { "sky",     makeSky       },
    { "base",    makeBase       },
    { "trees",   makeTrees   },
//    { "gliders", makeGliders   },
//    { "clouds",  makeClouds       },

    { 0L, 0L }
};

void InitConverter_fly( void )
{
}


Node* LoadFile_fly( const char *file )
{
    char buff[256];

    osg::notify(osg::INFO)<< "sgLoadFile_fly( "<<file<< ")\n";

    FILE *fp; 

    if( (fp = fopen( file, "r" )) == (FILE *)0L )
    {
        osg::notify(osg::WARN)<< "Unable to open file \""<<file<<"\"\n";
	return 0L;
    }
    Group *grp = new Group;

    while( !feof( fp ) )
    {
        _nodes *nptr;
        fgets( buff, sizeof( buff ), fp );
	if( buff[0] == '#' )
	    continue;

	for( nptr = nodes; nptr->name; nptr ++ )
	{
	    if( !strncmp( buff, nptr->name, strlen( nptr->name ) ))
	    {
	        Node *node = nptr->fptr();
		node->setName( nptr->name );
	    	grp->addChild( node );
		break;
	    }
	}
    }
    fclose( fp );

    return grp;
}

class sgReaderWriterFLY : public ReaderWriter {
  public:
	virtual const char* className() { return "Default FLY Database Reader/Writer"; }
	virtual bool acceptsExtension(const std::string& extension) { return extension=="fly"; }

	virtual Node* readNode(const std::string& fileName)
        {
            char buff[256];

            osg::notify(osg::INFO)<<   "sgReaderWriterFLY::readNode( "<<fileName.c_str()<<" )\n";

            FILE *fp; 

            if( (fp = fopen( fileName.c_str(), "r" )) == (FILE *)0L )
            {
                osg::notify(osg::WARN)<<  "Unable to open file \""<<fileName.c_str()<<"\"\n";
	        return 0L;
            }
            Group *grp = new Group;

            while( !feof( fp ) )
            {
                _nodes *nptr;
                fgets( buff, sizeof( buff ), fp );
	        if( buff[0] == '#' )
	            continue;

	        for( nptr = nodes; nptr->name; nptr ++ )
	        {
	            if( !strncmp( buff, nptr->name, strlen( nptr->name ) ))
	            {
	                Node *node = nptr->fptr();
		        node->setName( nptr->name );
	    	        grp->addChild( node );
		        break;
	            }
	        }
            }
            fclose( fp );

            return grp;
        
        }
        
	virtual bool writeNode(Node& obj,const std::string& fileName) {
                return false;
	}

};

// now register with sgRegistry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<sgReaderWriterFLY> g_readerWriter_FLY_Proxy;
