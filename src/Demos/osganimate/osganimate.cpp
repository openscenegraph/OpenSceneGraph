#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgGLUT/Viewer>


osg::AnimationPath* createAnimationPath(const osg::Vec3& center,float radius,double looptime)
{
    // set up the animation path 
    osg::AnimationPath* animationPath = new osg::AnimationPath;
    animationPath->setLoopMode(osg::AnimationPath::LOOP);
    
    int numSamples = 40;
    float yaw = 0.0f;
    float yaw_delta = 2.0f*osg::PI/((float)numSamples-1.0f);
    float roll = osg::inDegrees(30.0f);
    
    double time=0.0f;
    double time_delta = looptime/(double)numSamples;
    for(int i=0;i<numSamples;++i)
    {
        osg::Vec3 position(center+osg::Vec3(sinf(yaw)*radius,cosf(yaw)*radius,0.0f));
        osg::Quat rotation(osg::Quat(roll,osg::Vec3(0.0,1.0,0.0))*osg::Quat(-(yaw+osg::inDegrees(90.0f)),osg::Vec3(0.0,0.0,1.0)));
        
        animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));

        yaw += yaw_delta;
        time += time_delta;

    }
    return animationPath;    
}

osg::Node* createBase(const osg::Vec3& center,float radius)
{

    

    int numTilesX = 10;
    int numTilesY = 10;
    
    float width = 2*radius;
    float height = 2*radius;
    
    osg::Vec3 v000(center - osg::Vec3(width*0.5f,height*0.5f,0.0f));
    osg::Vec3 dx(osg::Vec3(width/((float)numTilesX),0.0,0.0f));
    osg::Vec3 dy(osg::Vec3(0.0f,height/((float)numTilesY),0.0f));
    
    // fill in vertices for grid, note numTilesX+1 * numTilesY+1...
    osg::Vec3Array* coords = new osg::Vec3Array;
    int iy;
    for(iy=0;iy<=numTilesY;++iy)
    {
        for(int ix=0;ix<=numTilesX;++ix)
        {
            coords->push_back(v000+dx*(float)ix+dy*(float)iy);
        }
    }
    
    //Just two colours - black and white.
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f)); // white
    colors->push_back(osg::Vec4(0.0f,0.0f,0.0f,1.0f)); // black
    int numColors=colors->size();
    
    
    int numIndicesPerRow=numTilesX+1;
    osg::UByteArray* coordIndices = new osg::UByteArray; // assumes we are using less than 256 points...
    osg::UByteArray* colorIndices = new osg::UByteArray;
    for(iy=0;iy<numTilesY;++iy)
    {
        for(int ix=0;ix<numTilesX;++ix)
        {
            // four vertices per quad.
            coordIndices->push_back(ix    +(iy+1)*numIndicesPerRow);
            coordIndices->push_back(ix    +iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+iy*numIndicesPerRow);
            coordIndices->push_back((ix+1)+(iy+1)*numIndicesPerRow);
            
            // one color per quad
            colorIndices->push_back((ix+iy)%numColors);
        }
    }
    

    // set up a single normal
    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0f,0.0f,1.0f));
    

    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(coords);
    geom->setVertexIndices(coordIndices);
    
    geom->setColorArray(colors);
    geom->setColorIndices(colorIndices);
    geom->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,coordIndices->size()));
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geom);
    
    return geode;
}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("glider.osg");
    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(glider);
    
        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;    
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
 
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(cessna);
    
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
    
    return model;
}

osg::Node* createModel()
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    osg::Group* root = new osg::Group;

    root->addChild(createMovingModel(center,radius*0.8f));

    root->addChild(createBase(center-osg::Vec3(0.0f,0.0f,radius*0.5),radius));

    return root;
}


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");

    // initialize the viewer.
    osgGLUT::Viewer viewer(arguments);

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // load the nodes from the commandline arguments.
    osg::Node* model = createModel();
    if (!model)
    {
        return 1;
    }
    
    // tilt the scene so the default eye position is looking down on the model.
    osg::MatrixTransform* rootnode = new osg::MatrixTransform;
    rootnode->setMatrix(osg::Matrix::rotate(osg::inDegrees(30.0f),1.0f,0.0f,0.0f));
    rootnode->addChild(model);

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );
    
    // register trackball, flight and drive.
    viewer.registerCameraManipulator(new osgGA::TrackballManipulator);
    viewer.registerCameraManipulator(new osgGA::FlightManipulator);
    viewer.registerCameraManipulator(new osgGA::DriveManipulator);


    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
