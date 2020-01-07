/* OpenSceneGraph example, osgparticleeffects.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include <osgViewer/Viewer>

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

    osg::ref_ptr<osg::Group> model = new osg::Group;

    osg::ref_ptr<osg::Node> glider = osgDB::readRefNodeFile("glider.osgt");
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
        xform->setDataVariance(osg::Object::DYNAMIC);
        xform->getOrCreateStateSet()->setMode(GL_NORMALIZE, osg::StateAttribute::ON);
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0,0.5));
        xform->addChild(positioned);

        model->addChild(xform);
    }

    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile("cessna.osgt");
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
        xform->setDataVariance(osg::Object::DYNAMIC);
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,1.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }

    return model.release();
}


osg::Vec3 computeTerrainIntersection(osg::Node* subgraph,float x,float y)
{
    const osg::BoundingSphere& bs = subgraph->getBound();
    float zMax = bs.center().z()+bs.radius();
    float zMin = bs.center().z()-bs.radius();

    osg::ref_ptr<osgUtil::LineSegmentIntersector> intersector =
        new osgUtil::LineSegmentIntersector(osg::Vec3(x,y,zMin),osg::Vec3(x,y,zMax));

    osgUtil::IntersectionVisitor iv(intersector.get());

    subgraph->accept(iv);

    if (intersector->containsIntersections())
    {
        return intersector->getFirstIntersection().getWorldIntersectPoint();
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
        osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
        osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/lz.rgb");
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
                osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
                pick(viewer,ea);
            }
            return false;

        default:
            return false;
        }
    }

    void pick(osgViewer::Viewer* viewer, const osgGA::GUIEventAdapter& ea)
    {
        osg::Group* root = dynamic_cast<osg::Group*>(viewer->getSceneData());
        if (!root) return;

        osgUtil::LineSegmentIntersector::Intersections intersections;
        if (viewer->computeIntersections(ea,intersections))
        {
            const osgUtil::LineSegmentIntersector::Intersection& hit = *intersections.begin();

            bool handleMovingModels = false;
            const osg::NodePath& nodePath = hit.nodePath;
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
                // separately from the main particle effects group which contains the emitters and programs.
                // the follow code block implements this, note the path for handling particle effects which aren't attached to
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
                osg::ref_ptr<osg::Node> hitNode = hit.nodePath.back();
                osg::Node::ParentList parents = hitNode->getParents();
                osg::Group* insertGroup = 0;
                unsigned int numGroupsFound = 0;
                for(osg::Node::ParentList::iterator itr=parents.begin();
                    itr!=parents.end();
                    ++itr)
                {
                    osg::Group* parent = (*itr);
                    if (typeid(*parent)==typeid(osg::Group))
                    {
                        ++numGroupsFound;
                        insertGroup = parent;
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
                        (*itr)->replaceChild(hit.nodePath.back(),insertGroup);
                    }
                    insertGroup->addChild(hitNode.get());
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

int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // register the pick handler
    viewer.addEventHandler(new PickHandler());

    osg::Group *root = new osg::Group;
    build_world(root);

    osgUtil::Optimizer optimizer;
    optimizer.optimize(root);

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);

    return viewer.run();
}
