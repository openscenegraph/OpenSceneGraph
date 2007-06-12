/* OpenSceneGraph example, osgshadowtexture.
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

#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Geode>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ShadowMap>

// include the call which creates the shadowed subgraph.
#include "CreateShadowedScene.h"


// for the grid data..
#include "../osghangglide/terrain_coords.h"



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

    osg::Geode* geode = new osg::Geode;
    
    // set up the texture of the base.
    osg::StateSet* stateset = new osg::StateSet();
    osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    
    geode->setStateSet( stateset );


    osg::HeightField* grid = new osg::HeightField;
    grid->allocate(38,39);
    grid->setOrigin(center+osg::Vec3(-radius,-radius,0.0f));
    grid->setXInterval(radius*2.0f/(float)(38-1));
    grid->setYInterval(radius*2.0f/(float)(39-1));
    
    float minHeight = FLT_MAX;
    float maxHeight = -FLT_MAX;


    unsigned int r;
    for(r=0;r<39;++r)
    {
        for(unsigned int c=0;c<38;++c)
        {
            float h = vertex[r+c*39][2];
            if (h>maxHeight) maxHeight=h;
            if (h<minHeight) minHeight=h;
        }
    }
    
    float hieghtScale = radius*0.5f/(maxHeight-minHeight);
    float hieghtOffset = -(minHeight+maxHeight)*0.5f;

    for(r=0;r<39;++r)
    {
        for(unsigned int c=0;c<38;++c)
        {
            float h = vertex[r+c*39][2];
            grid->setHeight(c,r,(h+hieghtOffset)*hieghtScale);
        }
    }
    
    geode->addDrawable(new osg::ShapeDrawable(grid));
     
    osg::Group* group = new osg::Group;
    group->addChild(geode);
     
    return group;

}

osg::Node* createMovingModel(const osg::Vec3& center, float radius)
{
    float animationLength = 10.0f;

    osg::AnimationPath* animationPath = createAnimationPath(center,radius,animationLength);

    osg::Group* model = new osg::Group;
 
    osg::Node* cessna = osgDB::readNodeFile("cessna.osg");
    if (cessna)
    {
        const osg::BoundingSphere& bs = cessna->getBound();

        float size = radius/bs.radius()*0.3f;
        osg::MatrixTransform* positioned = new osg::MatrixTransform;
        positioned->setDataVariance(osg::Object::STATIC);
        positioned->setMatrix(osg::Matrix::translate(-bs.center())*
                              osg::Matrix::scale(size,size,size)*
                              osg::Matrix::rotate(osg::inDegrees(180.0f),0.0f,0.0f,2.0f));
    
        positioned->addChild(cessna);
    
        osg::MatrixTransform* xform = new osg::MatrixTransform;
        xform->setUpdateCallback(new osg::AnimationPathCallback(animationPath,0.0f,2.0));
        xform->addChild(positioned);

        model->addChild(xform);
    }
    
    return model;
}




osg::Node* createModel(osg::ArgumentParser& arguments)
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;
    osg::Vec3 lightPosition(center+osg::Vec3(0.0f,0.0f,radius));

    // the shadower model
    osg::Node* shadower = createMovingModel(center,radius*0.5f);

    // the shadowed model
    osg::Node* shadowed = createBase(center-osg::Vec3(0.0f,0.0f,radius*0.25),radius);
    
    if (arguments.read("--sv"))
    {
        // hint to tell viewer to request stencil buffer when setting up windows
        osg::DisplaySettings::instance()->setMinimumNumStencilBits(8);

        osgShadow::ShadowedScene* shadowedScene = new osgShadow::ShadowedScene;
        
        osg::ref_ptr<osgShadow::ShadowVolume> shadowVolume = new osgShadow::ShadowVolume;
        shadowedScene->setShadowTechnique(shadowVolume.get());
        shadowVolume->setDynamicShadowVolumes(true);

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->getLight()->setPosition(osg::Vec4(lightPosition,1.0));

        shadowedScene->addChild(shadower);
        shadowedScene->addChild(shadowed);
        shadowedScene->addChild(ls.get());
        
        return shadowedScene;

    }
    else if (arguments.read("--st"))
    {

        osgShadow::ShadowedScene* shadowedScene = new osgShadow::ShadowedScene;
        
        osg::ref_ptr<osgShadow::ShadowTexture> shadowTexture = new osgShadow::ShadowTexture;
        shadowedScene->setShadowTechnique(shadowTexture.get());

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->getLight()->setPosition(osg::Vec4(lightPosition,1.0));
        
        shadowedScene->setReceivesShadowTraversalMask(0x1);
        shadowed->setNodeMask(shadowedScene->getReceivesShadowTraversalMask());
        
        shadowedScene->setCastsShadowTraversalMask(0x2);
        shadower->setNodeMask(shadowedScene->getCastsShadowTraversalMask());

        shadowedScene->addChild(shadower);
        shadowedScene->addChild(shadowed);
        shadowedScene->addChild(ls.get());
        
        return shadowedScene;

    }
    else if (arguments.read("--sm"))
    {

        osgShadow::ShadowedScene* shadowedScene = new osgShadow::ShadowedScene;
        
        osg::ref_ptr<osgShadow::ShadowMap> shadowMap = new osgShadow::ShadowMap;
        shadowedScene->setShadowTechnique(shadowMap.get());

        osg::ref_ptr<osg::LightSource> ls = new osg::LightSource;
        ls->getLight()->setPosition(osg::Vec4(lightPosition,1.0));
        
        shadowedScene->setReceivesShadowTraversalMask(0x1);
        shadowed->setNodeMask(shadowedScene->getReceivesShadowTraversalMask());
        
        shadowedScene->setCastsShadowTraversalMask(0x2);
        shadower->setNodeMask(shadowedScene->getCastsShadowTraversalMask());

        shadowedScene->addChild(shadower);
        shadowedScene->addChild(shadowed);
        shadowedScene->addChild(ls.get());
        
        return shadowedScene;

    }
    else
    {
        // combine the models together to create one which has the shadower and the shadowed with the required callback.
        osg::Group* root = createShadowedScene(shadower,shadowed,lightPosition,radius/100.0f,1);
        return root;
    }
    
}


int main(int argc, char ** argv)
{
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;
         
    // pass the model to the viewer.
    viewer.setSceneData( createModel(arguments) );

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);
    
    return viewer.run();
}
