#include <osgProducer/Viewer>

#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/io_utils>

#include <osgUtil/Optimizer>

#include <osgDB/ReadFile>

#include <osgText/Text>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebrisEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

osg::Vec3 wind(1.0f,0.0f,0.0f);            

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

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;

    osg::Node* glider = osgDB::readNodeFile("glider.osg");
    if (glider)
    {
        const osg::BoundingSphere& bs = glider->getBound();
        float size = radius/bs.radius()*0.15f;

        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(-90.0f),0.0f,0.0f,1.0f));
    
        positioned->addChild(glider);
    
        osg::PositionAttitudeTransform* xform = new osg::PositionAttitudeTransform;    
        xform->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,0.5));
        xform->addChild(positioned);

        model->addChild(xform);
    }
 
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();
        float size = radius/bs.radius()*0.15f;

        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                                     osg::Matrix::scale(size,size,size)*
                                     osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,1.0f));
    
        //positioned->addChild(cessna);
        positioned->addChild(cessna);
    
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
    
    return model;
}


osg::Vec3 computeTerrainIntersection(osg::Node* subgraph,float x,float y)
{
    osgUtil::IntersectVisitor iv;
    osg::ref_ptr<osg::LineSegment> segDown = new osg::LineSegment;

    const osg::BoundingSphere& bs = subgraph->getBound();
    float zMax = bs.center().z()+bs.radius();
    float zMin = bs.center().z()-bs.radius();
    
    segDown->set(osg::Vec3(x,y,zMin),osg::Vec3(x,y,zMax));
    iv.addLineSegment(segDown.get());

    subgraph->accept(iv);

    if (iv.hits())
    {
        osgUtil::IntersectVisitor::HitList& hitList = iv.getHitList(segDown.get());
        if (!hitList.empty())
        {
            osg::Vec3 ip = hitList.front().getWorldIntersectPoint();
            return  ip;
        }
    }

    return osg::Vec3(x,y,0.0f);
}


//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////

void build_world(osg::Group *root)
{

    osg::Geode* terrainGeode = new osg::Geode;
    // create terrain
    {
        osg::StateSet* stateset = new osg::StateSet();
        osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
        if (image)
        {
            osg::Texture2D* texture = new osg::Texture2D;
            texture->setImage(image);
            stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        }

        terrainGeode->setStateSet( stateset );

        float size = 1000; // 10km;
        float scale = size/39.0f; // 10km;
        float z_scale = scale*3.0f;

        osg::HeightField* grid = new osg::HeightField;
        grid->allocate(38,39);
        grid->setXInterval(scale);
        grid->setYInterval(scale);

        for(unsigned int r=0;r<39;++r)
        {
            for(unsigned int c=0;c<38;++c)
            {
                grid->setHeight(c,r,z_scale*vertex[r+c*39][2]);
            }
        }
        terrainGeode->addDrawable(new osg::ShapeDrawable(grid));
        
        root->addChild(terrainGeode);
    }    


    // create particle effects
    {    
        osg::Vec3 position = computeTerrainIntersection(terrainGeode,100.0f,100.0f);

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, 10.0f);
        osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, 10.0f);
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(position, 10.0f);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, 10.0f);

        explosion->setWind(wind);
        explosionDebri->setWind(wind);
        smoke->setWind(wind);
        fire->setWind(wind);

        root->addChild(explosion);
        root->addChild(explosionDebri);
        root->addChild(smoke);
        root->addChild(fire);
    }
    
    // create particle effects
    {    
        osg::Vec3 position = computeTerrainIntersection(terrainGeode,200.0f,100.0f);

        osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, 1.0f);
        osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, 1.0f);
        osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(position, 1.0f);
        osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, 1.0f);

        explosion->setWind(wind);
        explosionDebri->setWind(wind);
        smoke->setWind(wind);
        fire->setWind(wind);

        root->addChild(explosion);
        root->addChild(explosionDebri);
        root->addChild(smoke);
        root->addChild(fire);
    }

    // create the moving models.
    {
        root->addChild(createMovingModel(osg::Vec3(500.0f,500.0f,500.0f),300.0f));
    }
}


// class to handle events with a pick
class PickHandler : public osgGA::GUIEventHandler {
public: 

    PickHandler() {}        
    
    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
    {
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::PUSH):
            {
                osgProducer::Viewer* viewer = dynamic_cast<osgProducer::Viewer*>(&aa);
                pick(viewer,ea);
            }
            return false;

        default:
            return false;
        }
    }

    void pick(osgProducer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
    {
        osg::Group* root = dynamic_cast<osg::Group*>(viewer->getSceneData());       
        if (!root) return;

        osgUtil::IntersectVisitor::HitList hlist;
        if (viewer->computeIntersections(ea.getX(),ea.getY(),hlist))
        {
            osgUtil::Hit& hit = hlist.front();

            bool handleMovingModels = false;
            const osg::NodePath& nodePath = hit.getNodePath();
            for(osg::NodePath::const_iterator nitr=nodePath.begin();
                nitr!=nodePath.end();
                ++nitr)
            {
                const osg::Transform* transform = dynamic_cast<const osg::Transform*>(*nitr);
                if (transform)
                {
                    if (transform->getDataVariance()==osg::Object::DYNAMIC) handleMovingModels=true;
                }
            }
            
            osg::Vec3 position = handleMovingModels ? hit.getLocalIntersectPoint() : hit.getWorldIntersectPoint();
            float scale = 10.0f * ((float)rand() / (float)RAND_MAX);
            float intensity = 1.0f;

            osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, scale, intensity);
            osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, scale, intensity);
            osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, scale, intensity);
            osgParticle::ParticleEffect* smoke = 0;
            if (handleMovingModels)
                smoke =  new osgParticle::SmokeTrailEffect(position, scale, intensity);
            else
                smoke =  new osgParticle::SmokeEffect(position, scale, intensity);
            
            explosion->setWind(wind);
            explosionDebri->setWind(wind);
            smoke->setWind(wind);
            fire->setWind(wind);

            osg::Group* effectsGroup = new osg::Group;
            effectsGroup->addChild(explosion);
            effectsGroup->addChild(explosionDebri);
            effectsGroup->addChild(smoke);
            effectsGroup->addChild(fire);
            

            if (handleMovingModels)
            {
                // insert particle effects alongside the hit node, therefore able to track that nodes movement,
                // however, this does require us to insert the ParticleSystem itself into the root of the scene graph
                // seperately from the the main particle effects group which contains the emitters and programs.
                // the follow code block implements this, note the path for handling particle effects which arn't attached to 
                // moving models is easy - just a single line of code!
            
                // tell the effects not to attach to the particle system locally for rendering, as we'll handle add it into the 
                // scene graph ourselves.
                explosion->setUseLocalParticleSystem(false);
                explosionDebri->setUseLocalParticleSystem(false);
                smoke->setUseLocalParticleSystem(false);
                fire->setUseLocalParticleSystem(false);

                // find a place to insert the particle effects group alongside the hit node.
                // there are two possible ways that this can be done, either insert it into
                // a pre-existing group along side the hit node, or if no pre existing group
                // is found then this needs to be inserted above the hit node, and then the
                // particle effect can be inserted into this.
                osg::ref_ptr<osg::Geode> hitGeode = hit.getGeode();
                osg::Node::ParentList parents = hitGeode->getParents();                
                osg::Group* insertGroup = 0;
                unsigned int numGroupsFound = 0;
                for(osg::Node::ParentList::iterator itr=parents.begin();
                    itr!=parents.end();
                    ++itr)
                {
                    if (typeid(*(*itr))==typeid(osg::Group))
                    {
                        ++numGroupsFound;
                        insertGroup = *itr;
                    }
                }                
                if (numGroupsFound==parents.size() && numGroupsFound==1 && insertGroup)
                {
                    osg::notify(osg::INFO)<<"PickHandler::pick(,) hit node's parent is a single osg::Group so we can simple the insert the particle effects group here."<<std::endl;

                    // just reuse the existing group.
                    insertGroup->addChild(effectsGroup);
                }
                else
                {            
                    osg::notify(osg::INFO)<<"PickHandler::pick(,) hit node doesn't have an appropriate osg::Group node to insert particle effects into, inserting a new osg::Group."<<std::endl;
                    insertGroup = new osg::Group;
                    for(osg::Node::ParentList::iterator itr=parents.begin();
                        itr!=parents.end();
                        ++itr)
                    {
                        (*itr)->replaceChild(hit.getGeode(),insertGroup);
                    }
                    insertGroup->addChild(hitGeode.get());
                    insertGroup->addChild(effectsGroup);
                }

                // finally insert the particle systems into a Geode and attach to the root of the scene graph so the particle system
                // can be rendered.
                osg::Geode* geode = new osg::Geode;
                geode->addDrawable(explosion->getParticleSystem());
                geode->addDrawable(explosionDebri->getParticleSystem());
                geode->addDrawable(smoke->getParticleSystem());
                geode->addDrawable(fire->getParticleSystem());
                
                root->addChild(geode);

            }
            else
            {
                // when we don't have moving models we can simple insert the particle effect into the root of the scene graph
                osg::notify(osg::INFO)<<"PickHandler::pick(,) adding particle effects to root node."<<std::endl;
                root->addChild(effectsGroup);
            }

#if 0            
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(position,scale)));
            group->addChild(geode);
#endif
 
        }
    }
    
protected:
    virtual ~PickHandler() {}
};

// function used in debugging
void insertParticle(osg::Group* root, const osg::Vec3& center, float radius)
{
    bool handleMovingModels = false;

    osg::Vec3 position = center + 
               osg::Vec3( radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
                          radius * (((float)rand() / (float)RAND_MAX)-0.5)*2.0,
                          0.0f);

    float scale = 10.0f * ((float)rand() / (float)RAND_MAX);
    float intensity = 1.0f;

    osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, scale, intensity);
    osgParticle::ExplosionDebrisEffect* explosionDebri = new osgParticle::ExplosionDebrisEffect(position, scale, intensity);
    osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, scale, intensity);
    osgParticle::ParticleEffect* smoke = 0;
    if (handleMovingModels)
        smoke =  new osgParticle::SmokeTrailEffect(position, scale, intensity);
    else
        smoke =  new osgParticle::SmokeEffect(position, scale, intensity);

    explosion->setWind(wind);
    explosionDebri->setWind(wind);
    smoke->setWind(wind);
    fire->setWind(wind);

    osg::Group* effectsGroup = new osg::Group;
    effectsGroup->addChild(explosion);
    effectsGroup->addChild(explosionDebri);
    effectsGroup->addChild(smoke);
    effectsGroup->addChild(fire);

    root->addChild(effectsGroup);
}

//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////

int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of particle systems.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());
    
    // register the pick handler
    viewer.getEventHandlerList().push_front(new PickHandler());

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
    
    osg::Group *root = new osg::Group;
    build_world(root);


    osgUtil::Optimizer optimizer;
    optimizer.optimize(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
    // create the windows and run the threads.
    viewer.realize();

    // osg::Vec3 center = root->getBound().center();
    // float radius = root->getBound().radius();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // insertParticle(root, center, radius);

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
