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
#include <osgText/Font3D>
#include <osgDB/WriteFile>
#include <osgGA/StateSetManipulator>
#include <osgUtil/Tessellator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osg/io_utils>

#include "GlyphGeometry.h"

extern int main_orig(int, char**);
extern int main_test(int, char**);

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

    std::string fontFile("arial.ttf");
    while(arguments.read("-f",fontFile)) {}

    std::string word("This is a simple test");

    while(arguments.read("--ascii"))
    {
        word.clear();
        for(unsigned int c=' '; c<=127;++c)
        {
            word.push_back(c);
        }
    }

    while(arguments.read("-w",word)) {}

    osg::ref_ptr<osgText::Font3D> font = osgText::readFont3DFile(fontFile);
    if (!font) return 1;
    OSG_NOTICE<<"Read font "<<fontFile<<" font="<<font.get()<<std::endl;

    bool useTessellator = true;
    while(arguments.read("-t") || arguments.read("--tessellate")) { useTessellator = true; }
    while(arguments.read("--no-tessellate")) { useTessellator = false; }

    float thickness = 5.0;
    while(arguments.read("--thickness",thickness)) {}

    float width = 20.0;
    while(arguments.read("--width",width)) {}

    float creaseAngle = 30.0f;
    while(arguments.read("--crease-angle",creaseAngle)) {}

    OSG_NOTICE<<"creaseAngle="<<creaseAngle<<std::endl;

    osgText::BevelProfile profile;
    float ratio = 0.5;
    while(arguments.read("--rounded",ratio)) { profile.roundedBevel(ratio); }
    while(arguments.read("--rounded2",ratio)) { profile.roundedBevel2(ratio); }
    while(arguments.read("--flat",ratio)) { profile.flatBevel(ratio); }

    bool outline = false;
    while(arguments.read("--outline")) { outline = true; }
    while(arguments.read("--no-outline")) { outline = false; }

    bool smooth = true;
    while(arguments.read("--flat-shaded")) { smooth = false; }
    while(arguments.read("--smooth")) { smooth = false; }

    unsigned int numSamples = 10;
    while(arguments.read("--samples", numSamples)) {}
    font->setNumberCurveSamples(numSamples);

    profile.print(std::cout);


    osg::ref_ptr<osg::Group> group = new osg::Group;
    osg::Vec3 position;

    for(unsigned int i=0; i<word.size(); ++i)
    {
        osg::ref_ptr<osgText::Font3D::Glyph3D> glyph = font->getGlyph(word[i]);
        if (!glyph) return 1;

        osg::ref_ptr<osg::PositionAttitudeTransform> transform = new osg::PositionAttitudeTransform;
        transform->setPosition(position);
        transform->setAttitude(osg::Quat(osg::inDegrees(90.0),osg::Vec3d(1.0,0.0,0.0)));

        position.x() += glyph->getHorizontalWidth();

        osg::ref_ptr<osg::Geode> geode = new osg::Geode;

        osg::ref_ptr<osg::Geometry> glyphGeometry = osgText::computeGlyphGeometry(glyph.get(), thickness, width);
        osg::ref_ptr<osg::Geometry> textGeometry = osgText::computeTextGeometry(glyphGeometry.get(), profile, width);
        osg::ref_ptr<osg::Geometry> shellGeometry = outline ? osgText::computeShellGeometry(glyphGeometry.get(), profile, width) : 0;
        if (textGeometry.valid()) geode->addDrawable(textGeometry.get());
        if (shellGeometry.valid()) geode->addDrawable(shellGeometry.get());

        // create the normals
        if (smooth && textGeometry.valid())
        {
            osgUtil::SmoothingVisitor::smooth(*textGeometry, osg::DegreesToRadians(creaseAngle));
        }

        transform->addChild(geode.get());

        group->addChild(transform.get());
    }

    std::string filename;
    if (arguments.read("-o", filename)) osgDB::writeNodeFile(*group, filename);

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData(group.get());
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler);
    return viewer.run();
}
