// simple animation demo written by Graeme Harkness.


#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Material>
#include <osg/Vec3>
#include <osg/Transform>

#include <osgUtil/TrackballManipulator>

#include <osgGLUT/Viewer>
#include <osgGLUT/glut>

#include <osg/Math>

// ----------------------------------------------------------------------
// Global variables - this is basically the stuff which will be animated
// ----------------------------------------------------------------------

class MyTransformCallback : public osg::NodeCallback{

    public:

        MyTransformCallback(osg::Transform* node,float angularVelocity)
        {
            _nodeToOperateOn = node;
            _angular_velocity = angularVelocity;
            _previousTraversalNumber = -1;
            _orig_t = _timer.tick();
        }

        virtual void operator() (osg::Node* node, osg::NodeVisitor* nv)
        {
            if (nv)
            {
                if (_nodeToOperateOn && node==_nodeToOperateOn)
                {
                    // ensure that we do not operate on this node more than
                    // once during this traversal.  This is an issue since node
                    // can be shared between multiple parents.
                    if (nv->getTraversalNumber()!=_previousTraversalNumber)
                    {
                        osg::Timer_t new_t = _timer.tick();
                        float delta_angle = _angular_velocity*_timer.delta_s(_orig_t,new_t);
                        
                        osg::Matrix matrix;
                        matrix.makeRotate(delta_angle,1.0f,1.0f,1.0f);
                        matrix *= osg::Matrix::translate(1.0f,0.0f,0.0f);
                        matrix *= osg::Matrix::rotate(delta_angle,0.0f,0.0f,1.0f);
                                                
                        _nodeToOperateOn->setMatrix(matrix);

                        _previousTraversalNumber = nv->getTraversalNumber();
                    }
                }
            }
            
            // must continue subgraph traversal.
            traverse(node,nv);            
            
        }
        
    protected:
    
        osg::Transform*     _nodeToOperateOn;
        float               _angular_velocity;

        int                 _previousTraversalNumber;
        osg::Timer          _timer;
        osg::Timer_t        _orig_t;

};

osg::Geode* createCube()
{
    osg::Geode* geode = new osg::Geode();

    // -------------------------------------------
    // Set up a new GeoSet which will be our cube
    // -------------------------------------------
    osg::GeoSet* cube = new osg::GeoSet();

    // set up the primitives
    cube->setPrimType( osg::GeoSet::POLYGON );
    cube->setNumPrims( 6 );      // the six square faces

    // set up the primitive indices
    int* cubeLengthList = new int[6];
    cubeLengthList[0] = 4; // each side of the cube has 4 vertices
    cubeLengthList[1] = 4;
    cubeLengthList[2] = 4;
    cubeLengthList[3] = 4;
    cubeLengthList[4] = 4;
    cubeLengthList[5] = 4;

    cube->setPrimLengths( cubeLengthList );


    // set up the coordinates.
    osg::Vec3 *cubeCoords = new osg::Vec3[24];
    cubeCoords[0].set( -1.0000f,   1.0000f,  -1.000f );
    cubeCoords[1].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[2].set( 1.0000f,  -1.0000f,  -1.0000f );
    cubeCoords[3].set( -1.0000f,  -1.0000f,  -1.000 );

    cubeCoords[4].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[5].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[6].set( 1.0000f,  -1.0000f,   1.0000f );
    cubeCoords[7].set( 1.0000f,  -1.0000f,  -1.0000f );

    cubeCoords[8].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[9].set( -1.0000f,   1.0000f,   1.000f );
    cubeCoords[10].set( -1.0000f,  -1.0000f,   1.000f );
    cubeCoords[11].set( 1.0000f,  -1.0000f,   1.0000f );

    cubeCoords[12].set( -1.0000f,   1.0000f,   1.000 );
    cubeCoords[13].set( -1.0000f,   1.0000f,  -1.000 );
    cubeCoords[14].set( -1.0000f,  -1.0000f,  -1.000 );
    cubeCoords[15].set( -1.0000f,  -1.0000f,   1.000 );

    cubeCoords[16].set( -1.0000f,   1.0000f,   1.000 );
    cubeCoords[17].set( 1.0000f,   1.0000f,   1.0000f );
    cubeCoords[18].set( 1.0000f,   1.0000f,  -1.0000f );
    cubeCoords[19].set( -1.0000f,   1.0000f,  -1.000f );

    cubeCoords[20].set( -1.0000f,  -1.0000f,   1.000f );
    cubeCoords[21].set( -1.0000f,  -1.0000f,  -1.000f );
    cubeCoords[22].set( 1.0000f,  -1.0000f,  -1.0000f );
    cubeCoords[23].set( 1.0000f,  -1.0000f,   1.0000f );

    cube->setCoords( cubeCoords );
    
    
    // set up the normals.
    osg::Vec3 *cubeNormals = new osg::Vec3[6];
    cubeNormals[0].set(0.0f,0.0f,-1.0f);
    cubeNormals[1].set(1.0f,0.0f,0.0f);
    cubeNormals[2].set(0.0f,0.0f,1.0f);
    cubeNormals[3].set(-1.0f,0.0f,0.0f);
    cubeNormals[4].set(0.0f,1.0f,0.0f);
    cubeNormals[5].set(0.0f,-1.0f,0.0f);
    cube->setNormals( cubeNormals );
    cube->setNormalBinding( osg::GeoSet::BIND_PERPRIM );

    // ---------------------------------------
    // Set up a StateSet to make the cube red
    // ---------------------------------------
    osg::StateSet* cubeState = new osg::StateSet();
    osg::Material* redMaterial = new osg::Material();
    osg::Vec4 red( 1.0f, 0.0f, 0.0f, 1.0f );
    redMaterial->setDiffuse( osg::Material::FRONT_AND_BACK, red );
    cubeState->setAttribute( redMaterial );

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

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    osg::Transform* myTransform = new osg::Transform();
    myTransform->addChild( createCube() );
    
    // move node in a circle at 90 degrees a sec.
    myTransform->setAppCallback(new MyTransformCallback(myTransform,osg::inDegrees(90.0f)));

    // add model to viewer.
    viewer.addViewport( myTransform );

    // register trackball maniupulators.
    viewer.registerCameraManipulator(new osgUtil::TrackballManipulator);
    
    viewer.open();

    viewer.run();

    return 0;
}
