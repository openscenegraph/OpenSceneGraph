#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Vec3>
#include <osg/VertexProgram>

#include <osgGA/TrackballManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>

// A simple ambient, specular, and diffuse infinite lighting computation
// with a single light and an eye-space normal
// This vertex program is a slightly modified version of an example in
// the ARB_vertex_program spec. It uses local parameter 0 for ambient.
// If your cube have a blue ambient component it shows the vertex-
// program is working.

const char vpstr[] =
    "!!ARBvp1.0                                                 \n"
    "ATTRIB iPos         = vertex.position;                     \n"
    "ATTRIB iNormal      = vertex.normal;                       \n"
    "PARAM  mvinv[4]     = { state.matrix.modelview.invtrans }; \n"
    "PARAM  mvp[4]       = { state.matrix.mvp };                \n"
    "PARAM  lightDir     = state.light[0].position;             \n"
    "PARAM  halfDir      = state.light[0].half;                 \n"
    "PARAM  specExp      = state.material.shininess;            \n"
    "PARAM  ambientCol   = program.local[0];                    \n"
    "PARAM  diffuseCol   = state.lightprod[0].diffuse;          \n"
    "PARAM  specularCol  = state.lightprod[0].specular;         \n"
    "TEMP   xfNormal, temp, dots, newDiffuse;                   \n"
    "OUTPUT oPos         = result.position;                     \n"
    "OUTPUT oColor       = result.color;                        \n"
    "                                                           \n"
    "# Transform the vertex to clip coordinates.                \n"
    "DP4   oPos.x, mvp[0], iPos;                                \n"
    "DP4   oPos.y, mvp[1], iPos;                                \n"
    "DP4   oPos.z, mvp[2], iPos;                                \n"
    "DP4   oPos.w, mvp[3], iPos;                                \n"
    "                                                           \n"
    "# Transform the normal to eye coordinates.                 \n"
    "DP3   xfNormal.x, mvinv[0], iNormal;                       \n"
    "DP3   xfNormal.y, mvinv[1], iNormal;                       \n"
    "DP3   xfNormal.z, mvinv[2], iNormal;                       \n"
    "                                                           \n"
    "# Compute diffuse and specular dot products and use LIT to compute \n"
    "# lighting coefficients.                                   \n"
    "DP3   dots.x, xfNormal, lightDir;                          \n"
    "DP3   dots.y, xfNormal, halfDir;                           \n"
    "MOV   dots.w, specExp.x;                                   \n"
    "LIT   dots, dots;                                          \n"
    "                                                           \n"
    "# Accumulate color contributions.                          \n"
    "MAD   temp, dots.y, diffuseCol, ambientCol;                \n"
    "MAD   oColor.xyz, dots.z, specularCol, temp;               \n"
    "MOV   oColor.w, diffuseCol.w;                              \n"
    "END                                                        \n";
 

osg::Geode* createGeometryCube()
{
    osg::Geode* geode = new osg::Geode();

    // -------------------------------------------
    // Set up a new Geometry which will be our cube
    // -------------------------------------------
    osg::Geometry* cube = new osg::Geometry();

    // set up the primitives
    
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,4));
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,4,4));
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,8,4));
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,12,4));
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,16,4));
    cube->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,20,4));
    

    // set up coords.
    osg::Vec3Array* coords = new osg::Vec3Array;
    coords->resize(24);
    
    (*coords)[0].set( -1.0000f,   1.0000f,  -1.000f );
    (*coords)[1].set( 1.0000f,   1.0000f,  -1.0000f );
    (*coords)[2].set( 1.0000f,  -1.0000f,  -1.0000f );
    (*coords)[3].set( -1.0000f,  -1.0000f,  -1.000 );

    (*coords)[4].set( 1.0000f,   1.0000f,  -1.0000f );
    (*coords)[5].set( 1.0000f,   1.0000f,   1.0000f );
    (*coords)[6].set( 1.0000f,  -1.0000f,   1.0000f );
    (*coords)[7].set( 1.0000f,  -1.0000f,  -1.0000f );

    (*coords)[8].set( 1.0000f,   1.0000f,   1.0000f );
    (*coords)[9].set( -1.0000f,   1.0000f,   1.000f );
    (*coords)[10].set( -1.0000f,  -1.0000f,   1.000f );
    (*coords)[11].set( 1.0000f,  -1.0000f,   1.0000f );

    (*coords)[12].set( -1.0000f,   1.0000f,   1.000 );
    (*coords)[13].set( -1.0000f,   1.0000f,  -1.000 );
    (*coords)[14].set( -1.0000f,  -1.0000f,  -1.000 );
    (*coords)[15].set( -1.0000f,  -1.0000f,   1.000 );

    (*coords)[16].set( -1.0000f,   1.0000f,   1.000 );
    (*coords)[17].set( 1.0000f,   1.0000f,   1.0000f );
    (*coords)[18].set( 1.0000f,   1.0000f,  -1.0000f );
    (*coords)[19].set( -1.0000f,   1.0000f,  -1.000f );

    (*coords)[20].set( -1.0000f,  -1.0000f,   1.000f );
    (*coords)[21].set( -1.0000f,  -1.0000f,  -1.000f );
    (*coords)[22].set( 1.0000f,  -1.0000f,  -1.0000f );
    (*coords)[23].set( 1.0000f,  -1.0000f,   1.0000f );


    cube->setVertexArray( coords );
    
    
    // set up the normals.
    osg::Vec3Array* cubeNormals = new osg::Vec3Array;
    cubeNormals->resize(6);
    
    (*cubeNormals)[0].set(0.0f,0.0f,-1.0f);
    (*cubeNormals)[1].set(1.0f,0.0f,0.0f);
    (*cubeNormals)[2].set(0.0f,0.0f,1.0f);
    (*cubeNormals)[3].set(-1.0f,0.0f,0.0f);
    (*cubeNormals)[4].set(0.0f,1.0f,0.0f);
    (*cubeNormals)[5].set(0.0f,-1.0f,0.0f);
    
    cube->setNormalArray( cubeNormals );    
    cube->setNormalBinding( osg::Geometry::BIND_PER_PRIMITIVE );

    // -------------------------
    // make diffuse material red
    // -------------------------
    osg::StateSet* cubeState = new osg::StateSet();
    osg::Material* redMaterial = new osg::Material();
    const osg::Vec4 red( 1.0f, 0.0f, 0.0f, 1.0f );
    redMaterial->setAmbient( osg::Material::FRONT_AND_BACK, red );
    redMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, red );
    cubeState->setAttribute( redMaterial );

    // ---------------------------------------------------
    // Use vp local parameter 0 for ambient product (blue)
    // ---------------------------------------------------
    osg::VertexProgram* vp = new osg::VertexProgram();
    vp->setVertexProgram( vpstr );
    const osg::Vec4 blue( 0.0f, 0.0f, 1.0f, 1.0f );
    vp->setProgramLocalParameter( 0, blue );
    cubeState->setAttributeAndModes( vp, osg::StateAttribute::ON );

    cube->setStateSet( cubeState );

    geode->addDrawable( cube );
    
    return geode;
}


int main( int argc, char **argv )
{

    glutInit( &argc, argv );

    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);

    // create the viewer and the model to it.
    osgGLUT::Viewer viewer;

    viewer.setWindowTitle(argv[0]);

 
    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // add model to viewer.
    viewer.addViewport( createGeometryCube() );

    // register trackball maniupulators.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    
    viewer.open();

    viewer.run();

    return 0;
}
