/* OpenSceneGraph example, osgspotlight.
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
#include <osg/LightSource>
#include <osg/TexGenNode>

#include <osgUtil/Optimizer>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>


// for the grid data..
#include "../osghangglide/terrain_coords.h"


osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
{
    osg::Image* image = new osg::Image;
    image->allocateImage(size,size,1,
                         GL_RGBA,GL_UNSIGNED_BYTE);


    float mid = (float(size)-1)*0.5f;
    float div = 2.0f/float(size);
    for(unsigned int r=0;r<size;++r)
    {
        unsigned char* ptr = image->data(0,r,0);
        for(unsigned int c=0;c<size;++c)
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float pr = powf(1.0f-sqrtf(dx*dx+dy*dy),power);
            if (pr<0.0f) pr=0.0f;
            osg::Vec4 color = centerColour*pr+backgroudColour*(1.0f-pr);
            *ptr++ = (unsigned char)((color[0])*255.0f);
            *ptr++ = (unsigned char)((color[1])*255.0f);
            *ptr++ = (unsigned char)((color[2])*255.0f);
            *ptr++ = (unsigned char)((color[3])*255.0f);
        }
    }
    return image;

    //return osgDB::readImageFile("spot.dds");
}

osg::StateSet* createSpotLightDecoratorState(unsigned int lightNum, unsigned int textureUnit)
{
    osg::StateSet* stateset = new osg::StateSet;

    stateset->setMode(GL_LIGHT0+lightNum, osg::StateAttribute::ON);

    osg::Vec4 centerColour(1.0f,1.0f,1.0f,1.0f);
    osg::Vec4 ambientColour(0.05f,0.05f,0.05f,1.0f);

    // set up spot light texture
    osg::Texture2D* texture = new osg::Texture2D();
    texture->setImage(createSpotLightImage(centerColour, ambientColour, 64, 1.0));
    texture->setBorderColor(osg::Vec4(ambientColour));
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_BORDER);

    stateset->setTextureAttributeAndModes(textureUnit, texture, osg::StateAttribute::ON);

    // set up tex gens
    stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_S, osg::StateAttribute::ON);
    stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_T, osg::StateAttribute::ON);
    stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_R, osg::StateAttribute::ON);
    stateset->setTextureMode(textureUnit, GL_TEXTURE_GEN_Q, osg::StateAttribute::ON);

    return stateset;
}


osg::Node* createSpotLightNode(const osg::Vec3& position, const osg::Vec3& direction, float angle, unsigned int lightNum, unsigned int textureUnit)
{
    osg::Group* group = new osg::Group;

    // create light source.
    osg::LightSource* lightsource = new osg::LightSource;
    osg::Light* light = lightsource->getLight();
    light->setLightNum(lightNum);
    light->setPosition(osg::Vec4(position,1.0f));
    light->setAmbient(osg::Vec4(0.00f,0.00f,0.05f,1.0f));
    light->setDiffuse(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    group->addChild(lightsource);

    // create tex gen.

    osg::Vec3 up(0.0f,0.0f,1.0f);
    up = (direction ^ up) ^ direction;
    up.normalize();

    osg::TexGenNode* texgenNode = new osg::TexGenNode;
    texgenNode->setTextureUnit(textureUnit);
    osg::TexGen* texgen = texgenNode->getTexGen();
    texgen->setMode(osg::TexGen::EYE_LINEAR);
    texgen->setPlanesFromMatrix(osg::Matrixd::lookAt(position, position+direction, up)*
                                osg::Matrixd::perspective(angle,1.0,0.1,100)*
                                osg::Matrixd::translate(1.0,1.0,1.0)*
                                osg::Matrixd::scale(0.5,0.5,0.5));


    group->addChild(texgenNode);

    return group;

}


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
    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile("Images/lz.rgb");
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

    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile("cessna.osgt");
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

        xform->addChild(createSpotLightNode(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(0.0f,1.0f,-1.0f), 60.0f, 0, 1));

        model->addChild(xform);
    }

    return model;
}




osg::Node* createModel()
{
    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 100.0f;

    // the shadower model
    osg::Node* shadower = createMovingModel(center,radius*0.5f);

    // the shadowed model
    osg::Node* shadowed = createBase(center-osg::Vec3(0.0f,0.0f,radius*0.1),radius);

    // combine the models together to create one which has the shadower and the shadowed with the required callback.
    osg::Group* root = new osg::Group;

    root->setStateSet(createSpotLightDecoratorState(0,1));

    root->addChild(shadower);
    root->addChild(shadowed);

    return root;
}


int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add the spoit light model to the viewer
    viewer.setSceneData( createModel() );

    // run the viewer main frame loop.
    return viewer.run();
}
