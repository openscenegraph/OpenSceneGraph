#include <iostream>
#include <osg/Geode>
#include <osg/TexGen>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/ShapeDrawable>
#include <osg/Material>

extern osg::Node * CreateSimpleHierarchy( osg::Node * model );
extern osg::Node * CreateAdvancedHierarchy( osg::Node * model );

////////////////////////////////////////////////////////////////////////////////
osg::Node * CreateGlobe( void )
{
    // File not found - create textured sphere 
    osg::Geode * geode = new osg::Geode;
    osg::ref_ptr<osg::TessellationHints> hints = new osg::TessellationHints;
    hints->setDetailRatio( 0.3 );

#if 1
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable
        ( new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), 4.0 ), hints.get() );
#else
    osg::ref_ptr<osg::ShapeDrawable> shape = new osg::ShapeDrawable
        ( new osg::Box( osg::Vec3(-1.0f, -1.0f, -1.0f), 2.0, 2.0, 2.0 ) );
#endif

    shape->setColor(osg::Vec4(0.8f, 0.8f, 0.8f, 1.0f));

    geode->addDrawable( shape.get() );

    osg::StateSet * stateSet = new osg::StateSet;

    osg::Texture2D * texture =  new osg::Texture2D( 
        osgDB::readImageFile("Images/land_shallow_topo_2048.jpg") 
    );

    osg::Material * material = new osg::Material;

    material->setAmbient
        ( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.9, 0.9, 0.9, 1.0 ) );

    material->setDiffuse
        ( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.9, 0.9, 0.9, 1.0 ) );

#if 1
    material->setSpecular
        ( osg::Material::FRONT_AND_BACK, osg::Vec4( 0.7, 0.3, 0.3, 1.0 ) );

    material->setShininess( osg::Material::FRONT_AND_BACK, 25 );

#endif

    stateSet->setAttributeAndModes( material );
    stateSet->setTextureAttributeAndModes( 0,texture, osg::StateAttribute::ON );

    geode->setStateSet( stateSet );
    return geode;
}
////////////////////////////////////////////////////////////////////////////////
int main( int argc, char **argv )
{
    // construct the viewer.
    osg::ArgumentParser arguments( &argc, argv );
    osgViewer::Viewer viewer( arguments );

    bool useSimpleExample = arguments.read("-s") || arguments.read("--simple") ;

    osg::Node * model = NULL;

    if (arguments.argc()>1 && !arguments.isOption(1) ) {
        std::string filename = arguments[1];
        model = osgDB::readNodeFile( filename );
        if ( !model ) {
            osg::notify( osg::NOTICE ) 
                << "Error, cannot read " << filename 
                << ". Loading default earth model instead." << std::endl;
        }
    }

    if( model == NULL )
        model = CreateGlobe( );

    osg::Node * node = useSimpleExample ?        
        CreateSimpleHierarchy( model ):
        CreateAdvancedHierarchy( model );

    viewer.setSceneData( node );
    viewer.realize(  );
    viewer.run();

    return 0;
}
