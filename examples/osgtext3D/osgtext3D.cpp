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

class TextAttributeHandler : public osgGA::GUIEventHandler
{
public:
    TextAttributeHandler()
    {
    }

    virtual ~TextAttributeHandler()
    {
    }

    void addText(osgText::TextBase* aText)
    {
        m_Texts.push_back(aText);
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter&)
    {
        for (unsigned int i = 0; i < m_Texts.size(); ++i)
            process(ea, m_Texts[i]);

        return false;
    }

    void process(const osgGA::GUIEventAdapter& ea, osgText::TextBase* aText)
    {
        if (ea.getEventType() == osgGA::GUIEventAdapter::KEYUP)
        {
            if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Up)
            {
                aText->setCharacterSize(aText->getCharacterHeight() + 0.1);
                OSG_NOTICE<<"aText->getCharacterHeight() = " << aText->getCharacterHeight() << std::endl;
            }
            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Down)
            {
                osgText::Text3D* text3D = dynamic_cast<osgText::Text3D*>(aText);
                if (text3D)
                {
                    text3D->setCharacterDepth(text3D->getCharacterDepth() + 0.1);
                    OSG_NOTICE<<"text3D->getCharacterDepth() = " << text3D->getCharacterDepth() << std::endl;
                }
            }
            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left)
            {
                static int counter = 1;
                if (counter%4 == 0)
                    aText->setText("Press arrow keys.", osgText::String::ENCODING_UTF8);
                else if (counter%4 == 1)
                    aText->setText("setText\nworks\nPress enter\nto change alignment!", osgText::String::ENCODING_UTF8);
                else if (counter%4 == 2)
                    aText->setText("setText really works?", osgText::String::ENCODING_UTF8);
                else if (counter%4 == 3)
                    aText->setText("setText works, really!", osgText::String::ENCODING_UTF8);

                if (aText == m_Texts.back())
                    ++counter;

                OSG_NOTICE<<"aText->getText().size() = " << aText->getText().size() << std::endl;
            }
            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right)
            {
                aText->setLineSpacing(aText->getLineSpacing() + 0.1);
                OSG_NOTICE<<"aText->getLineSpacing() = " << aText->getLineSpacing() << std::endl;
            }
            else if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Return)
            {
                static int counter = 1;
                if (counter%3 == 0)
                    aText->setAlignment(osgText::TextBase::LEFT_BOTTOM_BASE_LINE);
                else if (counter%3 == 1)
                    aText->setAlignment(osgText::TextBase::CENTER_BOTTOM_BASE_LINE);
                else if (counter%3 == 2)
                    aText->setAlignment(osgText::TextBase::RIGHT_BOTTOM_BASE_LINE);

                if (aText == m_Texts.back())
                    ++counter;
            }
        }
    }

private:
    std::vector<osgText::TextBase*> m_Texts;
};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    std::string fontFile("arial.ttf");
    while(arguments.read("-f",fontFile)) {}

    osg::ref_ptr<osgText::Font> font = osgText::readRefFontFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;

    std::string word("Press arrow keys.");
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


    osg::ref_ptr<TextAttributeHandler> textAttributeHandler = new TextAttributeHandler;

#if 1
    osg::ref_ptr<osg::Group> group = new osg::Group;

    float characterSize = 1.0f;
    while(arguments.read("--size",characterSize)) {}

    if (arguments.read("--2d"))
    {
        osgText::Text* text2D = new osgText::Text;
        text2D->setDataVariance(osg::Object::DYNAMIC);

        text2D->setFont(font.get());
        text2D->setCharacterSize(characterSize);
        text2D->setFontResolution(256,256);
        text2D->setDrawMode(osgText::Text::TEXT | osgText::Text::BOUNDINGBOX);
        text2D->setAxisAlignment(osgText::Text::XZ_PLANE);
        text2D->setText(word);
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(text2D);
        group->addChild(geode);

        textAttributeHandler->addText(text2D);
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
        text3D->setDataVariance(osg::Object::DYNAMIC);

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
            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(imageFilename);
            if (image.valid())
            {
                OSG_NOTICE<<"  loaded image "<<imageFilename<<std::endl;
                osg::StateSet* stateset = text3D->getOrCreateStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image), osg::StateAttribute::ON);
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
            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(imageFilename);
            if (image.valid())
            {
                osg::StateSet* stateset = text3D->getOrCreateWallStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image), osg::StateAttribute::ON);
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
            osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(imageFilename);
            if (image)
            {
                osg::StateSet* stateset = text3D->getOrCreateBackStateSet();
                stateset->setTextureAttributeAndModes(0, new osg::Texture2D(image), osg::StateAttribute::ON);
            }
        }

        if (arguments.read("--size-quad"))
        {
            geode->addDrawable( osg::createTexturedQuadGeometry(osg::Vec3(0.0f,characterSize*thickness,0.0f),osg::Vec3(characterSize,0.0,0.0),osg::Vec3(0.0f,0.0,characterSize), 0.0, 0.0, 1.0, 1.0) );
        }

        if (arguments.read("--add-axes"))
        {
            group->addChild(osgDB::readRefNodeFile("axes.osgt"));
        }

        std::string mode;
        if (arguments.read("--character-size-mode", mode))
        {
            if (mode == "screen_coords")
            {
                text3D->setCharacterSizeMode(osgText::TextBase::SCREEN_COORDS);
                text3D->setCharacterSize(1080/4);
            }
        }

        textAttributeHandler->addText(text3D);
    }

    viewer.addEventHandler(textAttributeHandler);
    viewer.setSceneData(group);

#endif

    return viewer.run();
}
