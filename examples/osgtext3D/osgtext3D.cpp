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
#include <osg/Material>
#include <osg/PositionAttitudeTransform>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgText/Text3D>

#include "TextNode.h"


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

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
    
    
    if (bevel.valid()) 
    {
        while(arguments.read("--smooth-concave-Junctions") || arguments.read("--scj"))
        {
            bevel->setSmoothConcaveJunctions(true);
        }
    }

        
    style->setBevel(bevel.get());

    // set up outline.
    while(arguments.read("--outline",r)) { style->setOutlineRatio(r); }


    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler);

#if 1
    osg::ref_ptr<osg::Group> group = new osg::Group;

    float characterSize = 1.0f;
    while(arguments.read("--size",characterSize)) {}

    if (arguments.read("--2d"))
    {
        osgText::Text* text2D = new osgText::Text;
        text2D->setFont(font.get());
        text2D->setCharacterSize(characterSize);
        text2D->setFontResolution(256,256);
        text2D->setDrawMode(osgText::Text::TEXT | osgText::Text::BOUNDINGBOX);
        text2D->setAxisAlignment(osgText::Text::XZ_PLANE);
        text2D->setText(word);
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(text2D);
        group->addChild(geode);
    }

    if (arguments.read("--TextNode"))
    {
        // experimental text node
        osgText::TextNode* text = new osgText::TextNode;
        text->setFont(font.get());
        text->setStyle(style.get());
        text->setTextTechnique(new osgText::TextTechnique);
        text->setText(word);
        text->update();

        group->addChild(text);
    }
    else if (!arguments.read("--no-3d"))
    {
        osgText::Text3D* text3D = new osgText::Text3D;
        text3D->setFont(font.get());
        text3D->setStyle(style.get());
        text3D->setCharacterSize(characterSize);
        text3D->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
        text3D->setAxisAlignment(osgText::Text3D::XZ_PLANE);
        text3D->setText(word);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(text3D);
        group->addChild(geode);

        osg::Vec4 color(1.0f, 1.0f, 1.0f, 1.0f);
        while(arguments.read("--color",color.r(),color.g(),color.b(),color.a()))
        {
            OSG_NOTICE<<"--color "<<color<<std::endl;
            text3D->setColor(color);
        }

        std::string imageFilename;
        while(arguments.read("--image",imageFilename))
        {
            OSG_NOTICE<<"--image "<<imageFilename<<std::endl;
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageFilename);
            if (image.valid())
            {
                OSG_NOTICE<<"  loaded image "<<imageFilename<<std::endl;
                osg::StateSet* stateset = text3D->getOrCreateStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image.get()), osg::StateAttribute::ON);
            }
        }

        while(arguments.read("--wall-color",color.r(),color.g(),color.b(),color.a()))
        {
            osg::StateSet* stateset = text3D->getOrCreateWallStateSet();
            osg::Material* material = new osg::Material;
            material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
            stateset->setAttribute(material);
        }

        while(arguments.read("--wall-image",imageFilename))
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageFilename);
            if (image.valid())
            {
                osg::StateSet* stateset = text3D->getOrCreateWallStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image.get()), osg::StateAttribute::ON);
            }
        }

        while(arguments.read("--back-color",color.r(),color.g(),color.b(),color.a()))
        {
            osg::StateSet* stateset = text3D->getOrCreateBackStateSet();
            osg::Material* material = new osg::Material;
            material->setDiffuse(osg::Material::FRONT_AND_BACK, color);
            stateset->setAttribute(material);
        }

        while(arguments.read("--back-image",imageFilename))
        {
            osg::ref_ptr<osg::Image> image = osgDB::readImageFile(imageFilename);
            if (image.valid())
            {
                osg::StateSet* stateset = text3D->getOrCreateBackStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image.get()), osg::StateAttribute::ON);
            }
        }

        if (arguments.read("--size-quad"))
        {
            geode->addDrawable( osg::createTexturedQuadGeometry(osg::Vec3(0.0f,characterSize*thickness,0.0f),osg::Vec3(characterSize,0.0,0.0),osg::Vec3(0.0f,0.0,characterSize), 0.0, 0.0, 1.0, 1.0) );
        }
    }

    
    viewer.setSceneData(group.get());

#endif

    return viewer.run();
}
