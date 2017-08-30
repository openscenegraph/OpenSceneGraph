#include <cstdlib>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>

void textInfo(osgText::Text* text)
{
    const osgText::Text::TextureGlyphQuadMap& tgqm = text->getTextureGlyphQuadMap();

    const osgText::Text::TextureGlyphQuadMap::const_iterator tgqmi = tgqm.begin();


    osgText::String& s = text->getText();

    for(unsigned int i = 0; i < s.size(); i++)
    {
        osg::Vec2 ul; text->getCoord(0 + (i * 4), ul); // upperLeft
        osg::Vec2 ll; text->getCoord(1 + (i * 4), ll); // lowerLeft
        osg::Vec2 lr; text->getCoord(2 + (i * 4), lr); // lowerRight
        osg::Vec2 ur; text->getCoord(3 + (i * 4), ur); // upperRight

        osg::notify(osg::NOTICE)
            << "'" << static_cast<char>(s[i]) << "':"
            << " width(" << lr.x() - ll.x() << ")"
            << " height(" << ul.y() - ll.y() << ")" << std::endl << "\t"
            << "ul(" << ul << "), "
            << "ll(" << ll << "), "
            << "lr(" << lr << "), "
            << "ur(" << ur << ")"
            << std::endl
        ;
    }
}

osg::Camera* createOrthoCamera(double width, double height)
{
    osg::Camera* camera = new osg::Camera();

    camera->getOrCreateStateSet()->setMode(
        GL_LIGHTING,
        osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
    );

    osg::Matrix m = osg::Matrix::ortho2D(0.0f, width, 0.0f, height);

    camera->setProjectionMatrix(m);
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setViewMatrix(osg::Matrix::identity());
    camera->setClearMask(GL_DEPTH_BUFFER_BIT);
    camera->setRenderOrder(osg::Camera::POST_RENDER);

    return camera;
}

typedef std::list<unsigned int> Sizes;

struct TextSettings
{
    TextSettings():
        fontFilename("fonts/arial.ttf"),
        glyphImageMargin(1),
        glyphImageMarginRatio(0.02),
        glyphInterval(1),
        textColor(1.0f, 1.0f, 1.0f, 1.0f),
        backdropType(osgText::Text::NONE),
        backdropOffset(0.04f, 0.04f),
        backdropColor(0.0f, 0.0f, 0.0f, 1.0f)
    {
    }

    void read(osg::ArgumentParser& arguments)
    {
        if (arguments.read("--test"))
        {
            backgroundColor = osg::Vec4(1.0, 1.0, 1.0, 1.0);

            fontFilename = "fonts/arialbd.ttf";
            backdropType = osgText::Text::OUTLINE;

            sizes.clear();
            sizes.push_back(8);
            sizes.push_back(16);
            sizes.push_back(32);
            sizes.push_back(64);
            sizes.push_back(128);
        }

        if (arguments.read("--font",fontFilename)) {}


        if (arguments.read("--margin", glyphImageMargin)) {}
        if (arguments.read("--margin-ratio", glyphImageMarginRatio)) {}
        if (arguments.read("--interval", glyphInterval)) {}


        if (arguments.read("--outline")) backdropType = osgText::Text::OUTLINE;
        if (arguments.read("--shadow")) backdropType = osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;

        float offset;
        if (arguments.read("--offset", offset)) backdropOffset.set(offset, offset);

        if (arguments.read("--text-color", textColor.r(), textColor.g(), textColor.b(), textColor.a())) {}
        if (arguments.read("--bd-color", backdropColor.r(), backdropColor.g(), backdropColor.b(), backdropColor.a())) {}
        if (arguments.read("--bg-color", backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a())) {}

    }

    void setText(osgText::Text& text)
    {
        OSG_NOTICE<<"Settings::setText()"<<std::endl;
        text.setFont(fontFilename);
        text.setColor(textColor);
        text.setBackdropType(backdropType);
        text.setBackdropOffset(backdropOffset.x(), backdropOffset.y());
        text.setBackdropColor(backdropColor);
    }

    std::string                 fontFilename;
    unsigned int                glyphImageMargin;
    float                       glyphImageMarginRatio;
    int                         glyphInterval;

    osg::Vec4                   textColor;
    osgText::Text::BackdropType backdropType;
    osg::Vec2                   backdropOffset;
    osg::Vec4                   backdropColor;
    osg::Vec4                   backgroundColor;
    Sizes                       sizes;
};

osgText::Text* createLabel(const std::string& l, TextSettings& settings, unsigned int size)
{
    static osg::Vec3 pos(10.0f, 10.0f, 0.0f);

    osgText::Text* label = new osgText::Text();
    osg::ref_ptr<osgText::Font> font  = osgText::readRefFontFile(settings.fontFilename);

    font->setGlyphImageMargin(settings.glyphImageMargin);
    font->setGlyphImageMarginRatio(settings.glyphImageMarginRatio);

    settings.setText(*label);

    label->setCharacterSize(size);
    label->setFontResolution(size, size);
    label->setPosition(pos);
    label->setAlignment(osgText::Text::LEFT_BOTTOM);


    // It seems to be important we do this last to get best results?
    label->setText(l);


    // textInfo(label);

    pos.y() += size + 10.0f;

    return label;
}




int main(int argc, char** argv)
{
    osg::ArgumentParser args(&argc, argv);
    osgViewer::Viewer viewer(args);


    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());

    TextSettings settings;
    settings.backgroundColor = viewer.getCamera()->getClearColor();

    settings.read(args);

    viewer.getCamera()->setClearColor(settings.backgroundColor);

    osg::ref_ptr<osg::Group> root = new osg::Group;

    bool ortho = args.read("--ortho");
    if (ortho)
    {
        osg::ref_ptr<osg::Camera> camera = createOrthoCamera(1280.0f, 1024.0f);
        root->addChild(camera.get());
        root = camera;
    }
    else
    {
        osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
        transform->setMatrix(osg::Matrixd::rotate(osg::DegreesToRadians(90.0), 1.0, 0.0, 0.0));
        root->addChild(transform.get());
        root = transform;
    }

    if (args.argc() > 1)
    {
        settings.fontFilename = argv[1];

        // Create the list of desired sizes.
        for(int i = 2; i < args.argc(); i++)
        {
            if(!args.isNumber(i)) continue;

            settings.sizes.push_back(std::atoi(args[i]));
        }
    }

    if (settings.sizes.empty())
    {
        settings.sizes.push_back(8);
        settings.sizes.push_back(16);
        settings.sizes.push_back(32);
        settings.sizes.push_back(64);
    }

    osg::Geode* geode = new osg::Geode();

    // Add all of our osgText drawables.
    for(Sizes::const_iterator i = settings.sizes.begin(); i != settings.sizes.end(); i++)
    {
        std::stringstream ss;

        ss << *i << " 1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        geode->addDrawable(createLabel(ss.str(), settings, *i));
    }

    root->addChild(geode);

    std::string filename;
    if (args.read("-o", filename))
    {
        osgDB::writeNodeFile(*root, filename);
        return 0;
    }

    viewer.setSceneData(root.get());

    return viewer.run();
}

