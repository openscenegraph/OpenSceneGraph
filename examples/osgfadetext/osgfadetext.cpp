/* OpenSceneGraph example, osgfadetext.
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
#include <osg/CoordinateSystemNode>
#include <osg/ClusterCullingCallback>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>

#include <osgText/FadeText>

#include <osgSim/OverlayNode>
#include <osgSim/SphereSegment>

#include <osgGA/TerrainManipulator>

#include <iostream>

osg::Node* createEarth()
{
    osg::TessellationHints* hints = new osg::TessellationHints;
    hints->setDetailRatio(5.0f);


    osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0,0.0,0.0), osg::WGS_84_RADIUS_POLAR), hints);

    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(sd);

    std::string filename = osgDB::findDataFile("Images/land_shallow_topo_2048.jpg");
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(0, new osg::Texture2D(osgDB::readRefImageFile(filename)));

    osg::CoordinateSystemNode* csn = new osg::CoordinateSystemNode;
    csn->setEllipsoidModel(new osg::EllipsoidModel());
    csn->addChild(geode);

    return csn;

}

osgText::Text* createText(osg::EllipsoidModel* ellipsoid, double latitude, double longitude, double height, const std::string& str)
{
    double X,Y,Z;
    ellipsoid->convertLatLongHeightToXYZ( osg::DegreesToRadians(latitude), osg::DegreesToRadians(longitude), height, X, Y, Z);


    osgText::Text* text = new osgText::FadeText;

    osg::Vec3 normal = ellipsoid->computeLocalUpVector( X, Y, Z);
    text->setCullCallback(new osg::ClusterCullingCallback(osg::Vec3(X,Y,Z), normal, 0.0));

    text->setText(str);
    text->setFont("fonts/arial.ttf");
    text->setPosition(osg::Vec3(X,Y,Z));
    text->setCharacterSize(300000.0f);
    text->setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
    text->setAutoRotateToScreen(true);

    return text;
}

osg::Node* createFadeText(osg::EllipsoidModel* ellipsoid)
{
    osg::Group* group = new osg::Group;

    group->getOrCreateStateSet()->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);

    osg::Geode* geode = new osg::Geode;
    group->addChild(geode);

    std::vector<std::string> textList;
    textList.push_back("Town");
    textList.push_back("City");
    textList.push_back("Village");
    textList.push_back("River");
    textList.push_back("Mountain");
    textList.push_back("Road");
    textList.push_back("Lake");

    unsigned int numLat = 15;
    unsigned int numLong = 20;
    double latitude = 0.0;
    double longitude = -100.0;
    double deltaLatitude = 1.0f;
    double deltaLongitude = 1.0f;

    unsigned int t = 0;
    for(unsigned int i = 0; i < numLat; ++i, latitude += deltaLatitude)
    {
        double lgnt = longitude;
        for(unsigned int j = 0; j < numLong; ++j, ++t, lgnt += deltaLongitude)
        {
            geode->addDrawable( createText( ellipsoid, latitude, lgnt, 0, textList[t % textList.size()]) );
        }
    }

    return group;
}


class TextSettings : public osg::NodeVisitor
{
public:
    TextSettings(osg::ArgumentParser& arguments):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _backdropTypeSet(false),
        _backdropType(osgText::Text::NONE),
        _shaderTechniqueSet(false),
        _shaderTechnique(osgText::GREYSCALE)
    {
        if (arguments.read("--outline"))
        {
            _backdropTypeSet = true;
            _backdropType = osgText::Text::OUTLINE;
        }
        if (arguments.read("--sdf"))
        {
            _shaderTechniqueSet = true;
            _shaderTechnique = osgText::SIGNED_DISTANCE_FIELD;
        }
        if (arguments.read("--all"))
        {
            _shaderTechniqueSet = true;
            _shaderTechnique = osgText::ALL_FEATURES;
        }
        if (arguments.read("--greyscale"))
        {
            _shaderTechniqueSet = true;
            _shaderTechnique = osgText::GREYSCALE;
        }
        if (arguments.read("--no-shader"))
        {
            _shaderTechniqueSet = true;
            _shaderTechnique = osgText::NO_TEXT_SHADER;
        }
    }

    void apply(osg::Drawable& drawable)
    {
        osgText::Text* text = dynamic_cast<osgText::Text*>(&drawable);
        if (text)
        {
            if (_backdropTypeSet)
            {
                text->setBackdropType(_backdropType);
                text->setBackdropOffset(0.1f);
                text->setBackdropColor(osg::Vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
            if (_shaderTechniqueSet) text->setShaderTechnique(_shaderTechnique);
        }
    }

    bool                        _backdropTypeSet;
    osgText::Text::BackdropType _backdropType;
    bool                        _shaderTechniqueSet;
    osgText::ShaderTechnique    _shaderTechnique;
};

int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    viewer.getCamera()->setComputeNearFarMode(osg::CullSettings::COMPUTE_NEAR_FAR_USING_PRIMITIVES);
    viewer.getCamera()->setNearFarRatio(0.00001f);

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> root = createEarth();

    if (!root) return 0;

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root.get());

    osg::CoordinateSystemNode* csn = dynamic_cast<osg::CoordinateSystemNode*>(root.get());
    if (csn)
    {
        // add fade text around the globe
        csn->addChild(createFadeText(csn->getEllipsoidModel()));
    }

    if (arguments.argc()>1)
    {
        TextSettings textSettings(arguments);
        root->accept(textSettings);
    }

    viewer.setCameraManipulator(new osgGA::TerrainManipulator);

    return viewer.run();
}
