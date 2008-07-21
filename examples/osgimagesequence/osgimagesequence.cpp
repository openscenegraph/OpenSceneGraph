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
#include <osg/Texture2D>
#include <osg/ImageSequence>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <iostream>

//
// A simple demo demonstrating how to set on an animated texture using an osg::ImageSequence
//

osg::StateSet* createState()
{
    // read 4 2d images
    osg::ref_ptr<osg::Image> image_0 = osgDB::readImageFile("Images/lz.rgb");
    osg::ref_ptr<osg::Image> image_1 = osgDB::readImageFile("Images/reflect.rgb");
    osg::ref_ptr<osg::Image> image_2 = osgDB::readImageFile("Images/tank.rgb");
    osg::ref_ptr<osg::Image> image_3 = osgDB::readImageFile("Images/skymap.jpg");

    osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
    imageSequence->addImage(image_0.get());
    imageSequence->addImage(image_1.get());
    imageSequence->addImage(image_2.get());
    imageSequence->addImage(image_3.get());
    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_R,osg::Texture2D::REPEAT);
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setImage(imageSequence.get());
    //texture->setTextureSize(512,512);
    
    texture->setUpdateCallback(new osg::ImageSequence::UpdateCallback);

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


int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // create a model from the images and pass it to the viewer.
    viewer.setSceneData(createModel());

    return viewer.run();
}
