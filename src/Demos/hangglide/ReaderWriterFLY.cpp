#include <stdio.h>
#include <string.h>

#include <osg/Geode>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

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

static struct _nodes
{
    char *name;
    Node *(*fptr)(void);
}


nodes[] =
{
    { "terrain", makeTerrain },
    { "tank",    makeTank   },
    { "sky",     makeSky       },
    { "base",    makeBase       },
    { "trees",   makeTrees   },
    //    { "gliders", makeGliders   },
    //    { "clouds",  makeClouds       },

    { 0L, 0L }
};

class ReaderWriterFLY : public osgDB::ReaderWriter
{
    public:
        virtual const char* className() { return "FLY Database Reader"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return osgDB::equalCaseInsensitive(extension,"fly");
        }

        virtual ReadResult readNode(const std::string& fileName,const osgDB::ReaderWriter::Options*)
        {
        
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            char buff[256];

            notify(INFO)<<   "ReaderWriterFLY::readNode( "<<fileName.c_str()<<" )\n";

            FILE *fp;

            if( (fp = fopen( fileName.c_str(), "r" )) == (FILE *)0L )
            {
                notify(WARN)<<  "Unable to open file \""<<fileName.c_str()<<"\"\n";
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

};

// now register with osgDB::Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterFLY> g_readerWriter_FLY_Proxy;

