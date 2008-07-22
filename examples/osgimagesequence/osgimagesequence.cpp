/* OpenSceneGraph example, osgtexture3D.
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

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/TextureRectangle>
#include <osg/ImageSequence>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>

#include <iostream>

//
// A simple demo demonstrating how to set on an animated texture using an osg::ImageSequence
//

osg::StateSet* createState()
{
    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;

    imageSequence->setDuration(2.0);
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posx.png"));
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negx.png"));
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posy.png"));
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negy.png"));
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/posz.png"));
    imageSequence->addImage(osgDB::readImageFile("Cubemap_axis/negz.png"));

#if 1
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setImage(imageSequence.get());
    //texture->setTextureSize(512,512);
    
    //texture->setUpdateCallback(new osg::ImageSequence::UpdateCallback);
#else    
    osg::TextureRectangle* texture = new osg::TextureRectangle;
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::REPEAT);
    // texture->setResizeNonPowerOfTwoHint(false);
    texture->setImage(imageSequence.get());
    //texture->setTextureSize(512,512);
    
    //texture->setUpdateCallback(new osg::ImageSequence::UpdateCallback);
#endif

    // create the StateSet to store the texture data
    osg::StateSet* stateset = new osg::StateSet;

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);

    return stateset;
}

osg::Node* createModel()
{

    // create the geometry of the model, just a simple 2d quad right now.    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0), osg::Vec3(1.0f,0.0f,0.0), osg::Vec3(0.0f,0.0f,1.0f)));

    geode->setStateSet(createState());
    
    return geode;

}


int main(int argc, char **argv)
{
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // create a model from the images and pass it to the viewer.
    viewer.setSceneData(createModel());

    //osgDB::writeNodeFile(*viewer.getSceneData(),"test.osg");

    return viewer.run();
}
