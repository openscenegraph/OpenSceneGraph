#include <osg/Group>
#include <osg/Notify>
#include <osg/Depth>
#include <osg/StateSet>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <GL/glut.h>
#include <osgGLUT/Viewer>

#include "GliderManipulator.h"

extern osg::Node *makeTerrain( void );
extern osg::Node *makeTrees( void );
extern osg::Node *makeTank( void );
extern osg::Node *makeWindsocks( void );
extern osg::Node *makeGliders( void );
extern osg::Node *makeGlider( void );
extern osg::Node *makeSky( void );
extern osg::Node *makeBase( void );
extern osg::Node *makeClouds( void );

/*
 * Function to read several files (typically one) as specified on the command
 * line, and return them in an osg::Node
 */
osg::Node* getNodeFromFiles(int argc,char **argv)
{

    int i;

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;
    for( i = 1; i < argc; i++ )
    {

        if (argv[i][0]=='-')
        {
            switch(argv[i][1])
            {
                case('l'):
                    ++i;
                    if (i<argc)
                    {
                        osgDB::Registry::instance()->loadLibrary(argv[i]);
                    }
                    break;
                case('e'):
                    ++i;
                    if (i<argc)
                    {
                        std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(argv[i]);
                        osgDB::Registry::instance()->loadLibrary(libName);
                    }
                    break;
            }
        } else
        {
            osg::Node *node = osgDB::readNodeFile( argv[i] );

            if( node != (osg::Node *)0L )
            {
                if (node->getName().empty()) node->setName( argv[i] );
                nodeList.push_back(node);
            }
        }

    }

    if (nodeList.size()==0)
    {
        osg::notify(osg::INFO) << "No data loaded."<<endl;
        return 0;
    }

    osg::Node *rootnode = new osg::Node;
    if (nodeList.size()==1)
    {
        rootnode = nodeList.front();
    }
    else                         // size >1
    {
        osg::Group* group = new osg::Group();
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        rootnode = group;
    }

    return rootnode;
}


int main( int argc, char **argv )
{

    //     if (argc<2)
    //     {
    //         osg::notify(osg::NOTICE)<<"usage:"<<endl;
    //         osg::notify(osg::NOTICE)<<"    sgv [options] infile1 [infile2 ...]"<<endl;
    //         osg::notify(osg::NOTICE)<<endl;
    //         osg::notify(osg::NOTICE)<<"options:"<<endl;
    //         osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<<endl;
    //         osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<<endl;
    //         osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<<endl;
    //         osg::notify(osg::NOTICE)<<endl;
    //
    //         return 0;
    //     }

    osg::Node* rootnode = getNodeFromFiles( argc, argv);
    if (rootnode==NULL)
    {
        // no database loaded so automatically create Ed Levin Park..
        osg::Group* group = new osg::Group;
        rootnode = group;
        
        // the base and sky subgraphs go to set the earth sky of the
        // model and clear the color and depth buffer for us, by using
        // osg::Depth, and setting their bin numbers to less than 0,
        // to force them to draw before the rest of the scene.
        
        group->addChild(makeSky());  // bin number -2 so drawn first.
        group->addChild(makeBase()); // bin number -1 so draw second.

        // the rest of the scene drawn after the base and sky above.
        group->addChild(makeTrees()); // will drop into a transparent, depth sorted bin (1)
        group->addChild(makeTerrain()); // will drop into default bin - state sorted 0
        group->addChild(makeTank()); // will drop into default bin - state sorted 0
        // add the following in the future...
        // makeGliders
        // makeClouds

    }

    glutInit( &argc, argv );

    osgGLUT::Viewer viewer;
    viewer.addViewport( rootnode );

    unsigned int pos = viewer.registerCameraManipulator(new GliderManipulator());

    // Open window so camera manipulator's warp pointer request will succeed
    viewer.open();

    viewer.selectCameraManipulator(pos);

    osgUtil::SceneView* sv = viewer.getViewportSceneView(0);
    
    // switch off the render stages clear mask as we use the earth/sky
    // to clear it for us, see above.
    sv->getRenderStage()->setClearMask(0);

    viewer.run();

    return 0;
}
