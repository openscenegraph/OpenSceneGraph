#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <osg/OSG>
#include <osg/Geode> 
#include <osg/Group>
#include <osg/Registry>
#include <osg/Notify>
#include "osg/FileNameUtils"

using namespace osg;

#ifdef __sgi
static int dirent_select( dirent *dent )
#else
static int dirent_select( const dirent *dent )
#endif
{
    // if blank name don't pass selection.
    if (dent->d_name[0]==0) return 0;

    // if current directory '.' don't pass selection.
    if (strcmp(dent->d_name,".")==0) return 0;

    // if parent directory '..' don't pass selection.
    if (strcmp(dent->d_name,"..")==0) return 0;

    // should test for file being a directory?

    // if length < 4 chars then can't be .tgz extension three pass test.
    if (strlen(dent->d_name)<4) return 1;

    // return 1 (for pass) if 
    return strncmp( ".tgz", &dent->d_name[strlen(dent->d_name)-4], 4 );
}

class ReaderWriterTGZ : public ReaderWriter {
  public:
	virtual const char* className() { return "TGZ Database Reader/Writer"; }
	virtual bool acceptsExtension(const std::string& extension) { return extension=="tgz"; }

	virtual Node* readNode(const std::string& fileName)
        {
            std::string ext = getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return NULL;

            osg::notify(osg::INFO)<<   "ReaderWriterTGZ::readNode( "<<fileName.c_str()<<" )\n";

	    char dirname[128];
	    char command[1024];
	    struct dirent **dent;
	    int ndent;

	    sprintf( dirname, "/tmp/.tgz%06d", getpid());
	    mkdir( dirname, 0700 );

#ifdef __linux
	    sprintf( command,
	    	"tar xfCz %s %s", 
		fileName.c_str(), dirname );
#endif
#ifdef __sgi
	    sprintf( command,
		"cp %s %s; cd %s;"
	    	"gzcat %s | tar xf -",
		fileName.c_str(), dirname, dirname,
		fileName.c_str());

#endif
	    system( command );

	    osg::Group *grp = new osg::Group;
	    osg::SetFilePath( dirname );

	    ndent = scandir( dirname, &dent, dirent_select, alphasort );

	    for( int i = 0; i < ndent; i++ )
	    {
		osg::Node *node = osg::loadNodeFile( dent[i]->d_name );
		grp->addChild( node );
	    }


	    sprintf( command, "rm -rf %s", dirname );
	    system( command );

	    if( grp->getNumChildren() == 0 )
	    {
	        grp->unref();
		return NULL;
	    }

	    else
	    	return grp;
        }
        
	virtual bool writeNode(Node& obj,const std::string& fileName) {
                return false;
	}

};

// now register with sgRegistry to instantiate the above
// reader/writer.
RegisterReaderWriterProxy<ReaderWriterTGZ> g_readerWriter_TGZ_Proxy;
