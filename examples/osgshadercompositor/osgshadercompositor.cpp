#include <iostream>
#include <osg/Geode>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

extern osg::Node * CreateSimpleHierarchy( const char * file );
extern osg::Node * CreateAdvancedHierarchy( const char * file );

int main( int argc, char **argv )
{
    // construct the viewer.
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer( arguments );

    std::string filename = "cow.osg";
    bool useAdvancedTechnique = arguments.read("-a") || arguments.read("--advanced") ;

    if (arguments.argc()>1)
    {
        if (!arguments.isOption(1)) filename =  arguments[1];
    }

    // osg::Node * node = CreateSimpleHierarchy( file );
    osg::Node * node = useAdvancedTechnique ?
        CreateAdvancedHierarchy( filename.c_str() ) :
        CreateSimpleHierarchy( filename.c_str() );

    if ( !node ) {
        osg::notify( osg::NOTICE ) << "Error, cannot read " << filename << std::endl;
        return 1;
    }    

    viewer.setSceneData( node );
    viewer.realize(  );
    viewer.run();

    return 0;
}
