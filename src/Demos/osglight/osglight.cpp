#include <osg/GL>
#include <osgGLUT/glut>
#include <osgGLUT/Viewer>

#include <osg/Group>
#include <osg/Node>

#include <osg/Light>
#include <osg/LightSource>
#include <osg/StateAttribute>
#include <osg/Geometry>

#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include "stdio.h"


// callback to make the loaded model oscilate up and down.
class ModelTransformCallback : public osg::NodeCallback
{
    public:

        ModelTransformCallback(const osg::BoundingSphere& bs)
        {
            _firstTime = 0.0;
            _period = 4.0f;
            _range = bs.radius()*0.5f;
        }
    
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osg::PositionAttitudeTransform* pat = dynamic_cast<osg::PositionAttitudeTransform*>(node);
            const osg::FrameStamp* frameStamp = nv->getFrameStamp();
            if (pat && frameStamp)
            {
                if (_firstTime==0.0) 
                {
                    _firstTime = frameStamp->getReferenceTime();
                }
                
                double phase = (frameStamp->getReferenceTime()-_firstTime)/_period;
                phase -= floor(phase);
                phase *= (2.0 * osg::PI);
            
                osg::Quat rotation;
                rotation.makeRotate(phase,1.0f,1.0f,1.0f);
                
                pat->setAttitude(rotation); 
                
                pat->setPosition(osg::Vec3(0.0f,0.0f,sin(phase))*_range);
            }
        
            // must traverse the Node's subgraph            
            traverse(node,nv);
        }
        
        double _firstTime;
        double _period;
        double _range;

};


osg::Node* createLights(osg::BoundingBox& bb,osg::StateSet* rootStateSet)
{
    osg::Group* lightGroup = new osg::Group;
    
    float modelSize = bb.radius();

    // create a spot light.
    osg::Light* myLight1 = new osg::Light;
    myLight1->setLightNum(0);
    myLight1->setPosition(osg::Vec4(bb.corner(4),1.0f));
    myLight1->setAmbient(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setDiffuse(osg::Vec4(1.0f,0.0f,0.0f,1.0f));
    myLight1->setSpotCutoff(20.0f);
    myLight1->setSpotExponent(50.0f);
    myLight1->setDirection(osg::Vec3(1.0f,1.0f,-1.0f));

    osg::LightSource* lightS1 = new osg::LightSource;	
    lightS1->setLight(myLight1);
    lightS1->setLocalStateSetModes(osg::StateAttribute::ON); 

    lightS1->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
    lightGroup->addChild(lightS1);
    

    // create a local light.
    osg::Light* myLight2 = new osg::Light;
    myLight2->setLightNum(1);
    myLight2->setPosition(osg::Vec4(bb.corner(1),1.0f));
    myLight2->setAmbient(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setDiffuse(osg::Vec4(0.0f,1.0f,1.0f,1.0f));
    myLight2->setConstantAttenuation(1.0f);
    myLight2->setLinearAttenuation(2.0f/modelSize);
    myLight2->setQuadraticAttenuation(2.0f/osg::square(modelSize));

    osg::LightSource* lightS2 = new osg::LightSource;	
    lightS2->setLight(myLight2);
    lightS2->setLocalStateSetModes(osg::StateAttribute::ON); 

    lightS2->setStateSetModes(*rootStateSet,osg::StateAttribute::ON);
    
    
    osg::Transform* pat = new osg::Transform();
    
    pat->addChild(lightS2);
    
    lightGroup->addChild(lightS2);

    lightGroup->addChild(pat);

    return lightGroup;
}

osg::Geometry* createWall(const osg::Vec3& v1,const osg::Vec3& v2,const osg::Vec3& v3,osg::StateSet* stateset)
{

   // create a drawable for occluder.
    osg::Geometry* geom = osgNew osg::Geometry;
    
    geom->setStateSet(stateset);

    unsigned int noXSteps = 100;
    unsigned int noYSteps = 100;
    
    osg::Vec3Array* coords = osgNew osg::Vec3Array;
    coords->reserve(noXSteps*noYSteps);
    
    
    osg::Vec3 dx = (v2-v1)/((float)noXSteps-1.0f);
    osg::Vec3 dy = (v3-v1)/((float)noYSteps-1.0f);
    
    unsigned int row;
    osg::Vec3 vRowStart = v1;
    for(row=0;row<noYSteps;++row)
    {
        osg::Vec3 v = vRowStart;
        for(unsigned int col=0;col<noXSteps;++col)        
        {
            coords->push_back(v);
            v += dx;
        }
        vRowStart+=dy;
    }
    
    geom->setVertexArray(coords);
    
    osg::Vec4Array* colors = osgNew osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    
    
    for(row=0;row<noYSteps-1;++row)
    {
        osg::DrawElementsUShort* quadstrip = new osg::DrawElementsUShort(osg::Primitive::QUAD_STRIP);
        quadstrip->reserve(noXSteps*2);
        for(unsigned int col=0;col<noXSteps;++col)        
        {
            quadstrip->push_back((row+1)*noXSteps+col);
            quadstrip->push_back(row*noXSteps+col);
        }   
        geom->addPrimitive(quadstrip);
    }
    
    // create the normals.    
    osgUtil::SmoothingVisitor::smooth(*geom);
    
    return geom;
 
}


osg::Node* createRoom(osg::Node* loadedModel)
{
    // default scale for this model.
    osg::BoundingSphere bs(osg::Vec3(0.0f,0.0f,0.0f),1.0f);

    osg::Group* root = new osg::Group;

    if (loadedModel)
    {
        const osg::BoundingSphere& loaded_bs = loadedModel->getBound();

        osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform();
        pat->setPivotPoint(loaded_bs.center());
        
        pat->setAppCallback(new ModelTransformCallback(loaded_bs));
        pat->addChild(loadedModel);
        
        bs = pat->getBound();
        
        root->addChild(pat);

    }

    bs.radius()*=1.5f;

    // create a bounding box, which we'll use to size the room.
    osg::BoundingBox bb;
    bb.expandBy(bs);


    // create statesets.
    osg::StateSet* rootStateSet = new osg::StateSet;
    root->setStateSet(rootStateSet);

    osg::StateSet* wall = new osg::StateSet;
    wall->setMode(GL_CULL_FACE,osg::StateAttribute::ON);
    
    osg::StateSet* floor = new osg::StateSet;
    floor->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::StateSet* roof = new osg::StateSet;
    roof->setMode(GL_CULL_FACE,osg::StateAttribute::ON);

    osg::Geode* geode = new osg::Geode;
    
    // create front side.
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(4),
                                  bb.corner(1),
                                  wall));

    // right side
    geode->addDrawable(createWall(bb.corner(1),
                                  bb.corner(5),
                                  bb.corner(3),
                                  wall));

    // left side
    geode->addDrawable(createWall(bb.corner(2),
                                  bb.corner(6),
                                  bb.corner(0),
                                  wall));
    // back side
    geode->addDrawable(createWall(bb.corner(3),
                                  bb.corner(7),
                                  bb.corner(2),
                                  wall));

    // floor
    geode->addDrawable(createWall(bb.corner(0),
                                  bb.corner(1),
                                  bb.corner(2),
                                  floor));

    // roof
    geode->addDrawable(createWall(bb.corner(6),
                                  bb.corner(7),
                                  bb.corner(4),
                                  roof));

    root->addChild(geode);
    
    root->addChild(createLights(bb,rootStateSet));

    return root;
    
}    

int main( int argc, char **argv )
{

    // initialize the GLUT
    glutInit( &argc, argv );
    
    // create the commandline args.
    std::vector<std::string> commandLine;
    for(int i=1;i<argc;++i) commandLine.push_back(argv[i]);
    

    // initialize the viewer.
    osgGLUT::Viewer viewer;
    viewer.setWindowTitle(argv[0]);

    // configure the viewer from the commandline arguments, and eat any
    // parameters that have been matched.
    viewer.readCommandLine(commandLine);
    
    // configure the plugin registry from the commandline arguments, and 
    // eat any parameters that have been matched.
    osgDB::readCommandLine(commandLine);

    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(commandLine);

    // create a room made of foor walls, a floor, a roof, and swinging light fitting.
    osg::Node* rootnode = createRoom(loadedModel);

    // run optimization over the scene graph
    osgUtil::Optimizer optimzer;
    optimzer.optimize(rootnode);
     
    // add a viewport to the viewer and attach the scene graph.
    viewer.addViewport( rootnode );

    // open the viewer window.
    viewer.open();
    
    // fire up the event loop.
    viewer.run();

    return 0;
}
