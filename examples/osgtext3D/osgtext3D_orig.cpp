/* OpenSceneGraph example, osgtext.
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
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgText/Text3D>

#include <iostream>
#include <sstream>



// create text which sits in 3D space such as would be inserted into a normal model
osg::Group* create3DText(const osg::Vec3& center,float radius)
{

    osg::Geode* geode  = new osg::Geode;

////////////////////////////////////////////////////////////////////////////////////////////////////////
//    
// Examples of how to set up axis/orientation alignments
//

    float characterSize=radius*0.2f;
    float characterDepth=characterSize*0.2f;
    
    osg::Vec3 pos(center.x()-radius*.5f,center.y()-radius*.5f,center.z()-radius*.5f);

    osgText::Text3D* text1 = new osgText::Text3D;
    text1->setFont("fonts/arial.ttf");
    text1->setCharacterSize(characterSize);
    text1->setCharacterDepth(characterDepth);
    text1->setPosition(pos);
    text1->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text1->setAxisAlignment(osgText::Text3D::XY_PLANE);
    text1->setText("XY_PLANE");
    geode->addDrawable(text1);

    osgText::Text3D* text2 = new osgText::Text3D;
    text2->setFont("fonts/times.ttf");
    text2->setCharacterSize(characterSize);
    text2->setCharacterDepth(characterDepth);
    text2->setPosition(pos);
    text2->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text2->setAxisAlignment(osgText::Text3D::YZ_PLANE);
    text2->setText("YZ_PLANE");
    geode->addDrawable(text2);

    osgText::Text3D* text3 = new osgText::Text3D;
    text3->setFont("fonts/dirtydoz.ttf");
    text3->setCharacterSize(characterSize);
    text3->setCharacterDepth(characterDepth);
    text3->setPosition(pos);
    text3->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text3->setAxisAlignment(osgText::Text3D::XZ_PLANE);
    text3->setText("XZ_PLANE");
    geode->addDrawable(text3);

    osg::ref_ptr<osgText::Style> style = new osgText::Style;
    osg::ref_ptr<osgText::Bevel> bevel = new osgText::Bevel;
    bevel->roundedBevel2(0.25);
    style->setBevel(bevel.get());
    style->setWidthRatio(0.4f);

    osgText::Text3D* text7 = new osgText::Text3D;
    text7->setFont("fonts/times.ttf");
    text7->setStyle(style.get());
    text7->setCharacterSize(characterSize);
    text7->setCharacterDepth(characterSize*0.2f);
    text7->setPosition(center - osg::Vec3(0.0, 0.0, 0.6));
    text7->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text7->setAxisAlignment(osgText::Text3D::SCREEN);
    text7->setCharacterSizeMode(osgText::Text3D::OBJECT_COORDS);
    text7->setText("CharacterSizeMode OBJECT_COORDS (default)");
    geode->addDrawable(text7);

    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(center,characterSize*0.2f));
    shape->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    geode->addDrawable(shape);

    osg::Group* rootNode = new osg::Group;
    rootNode->addChild(geode);

    osg::Material* front = new osg::Material;
    front->setAlpha(osg::Material::FRONT_AND_BACK,1);
    front->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.2,0.2,0.2,1.0));
    front->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(.0,.0,1.0,1.0));
    rootNode->getOrCreateStateSet()->setAttributeAndModes(front);
    
    
    return rootNode;    
}

int main_orig(int, char**)
{
    osgViewer::Viewer viewer;

    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 1.0f;
    
    osg::Group* root = new osg::Group;
    root->addChild(create3DText(center, radius));

    viewer.setSceneData(root);
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    viewer.addEventHandler(new osgViewer::ThreadingHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    viewer.addEventHandler(new osgViewer::StatsHandler);


    viewer.run();
    
    return 0;
}


