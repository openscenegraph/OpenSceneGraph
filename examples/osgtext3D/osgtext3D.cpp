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

#include <osg/ArgumentParser>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/TriangleIndexFunctor>
#include <osg/PositionAttitudeTransform>
#include <osgUtil/SmoothingVisitor>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/Tessellator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/io_utils>

#include <osgText/Text3D>

#include "TextNode.h"

extern int main_orig(int, char**);
extern int main_test(int, char**);


int main_size(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    std::string fontFile("arial.ttf");
    while(arguments.read("-f",fontFile)) {}

    osg::ref_ptr<osgText::Font> font = osgText::readFontFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;

    osg::Geode* geode = new osg::Geode;

    geode->addDrawable( osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(1.0f,0.0,0.0),osg::Vec3(0.0f,0.0,1.0), 0.0, 0.0, 1.0, 1.0) );

    osgText::Text3D* text3d = new osgText::Text3D;
    text3d->setPosition(osg::Vec3(1.0f,0.0f,0.0f));
    text3d->setFont(osgText::readFontFile("arial.ttf"));
    text3d->setCharacterSizeMode(osgText::Text3D::OBJECT_COORDS);
    text3d->setCharacterSize(1.0f);
    text3d->setCharacterDepth(0.1f);
    text3d->setAxisAlignment(osgText::Text3D::XZ_PLANE);
    text3d->setText("This is a size test");

    geode->addDrawable(text3d);

    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.setSceneData(geode);

    return viewer.run();
}



int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    if (arguments.read("--test"))
    {
        return main_test(argc,argv);
    }
    else if (arguments.read("--original") || arguments.read("--orig"))
    {
        return main_orig(argc,argv);
    }
    else if (arguments.read("--size-test"))
    {
        return main_size(argc,argv);
    }

    osgViewer::Viewer viewer(arguments);

    std::string fontFile("arial.ttf");
    while(arguments.read("-f",fontFile)) {}

    osg::ref_ptr<osgText::Font> font = osgText::readFontFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;

    std::string word("This is a new test.");
    while (arguments.read("-w",word)) {}

    osg::ref_ptr<osgText::Style> style = new osgText::Style;

    float thickness = 0.1f;
    while(arguments.read("--thickness",thickness)) {}
    style->setThicknessRatio(thickness);

    // set up any bevel if required
    float r;
    osg::ref_ptr<osgText::Bevel> bevel;
    while(arguments.read("--rounded",r)) { bevel = new osgText::Bevel; bevel->roundedBevel2(r); }
    while(arguments.read("--rounded")) { bevel = new osgText::Bevel; bevel->roundedBevel2(0.25); }
    while(arguments.read("--flat",r)) { bevel = new osgText::Bevel; bevel->flatBevel(r); }
    while(arguments.read("--flat")) { bevel = new osgText::Bevel; bevel->flatBevel(0.25); }
    while(arguments.read("--bevel-thickness",r)) { if (bevel.valid()) bevel->setBevelThickness(r); }

    style->setBevel(bevel.get());

    // set up outline.
    while(arguments.read("--outline",r)) { style->setOutlineRatio(r); }


    osgText::TextNode* text = new osgText::TextNode;
    text->setText(word);
    text->setFont(font.get());
    text->setStyle(style.get());
    text->setTextTechnique(new osgText::TextTechnique);
    text->update();

    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.setSceneData(text);

        return viewer.run();
}
