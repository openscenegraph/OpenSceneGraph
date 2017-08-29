#include <cstdlib>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Geode>
#include <osg/MatrixTransform>
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

osgText::Text* createLabel(const std::string& l, const std::string& fontfile, unsigned int size)
{
    static osg::Vec3 pos(10.0f, 10.0f, 0.0f);

    osgText::Text* label = new osgText::Text();
    osg::ref_ptr<osgText::Font> font  = osgText::readRefFontFile(fontfile);

    label->setFont(font);
    label->setCharacterSize(size);
    label->setFontResolution(size, size);
    label->setColor(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    label->setPosition(pos);
    label->setAlignment(osgText::Text::LEFT_BOTTOM);

    // It seems to be important we do this last to get best results?
    label->setText(l);

    textInfo(label);

    pos.y() += size + 10.0f;

    return label;
}

typedef std::list<unsigned int> Sizes;

int main(int argc, char** argv)
{
    osg::ArgumentParser args(&argc, argv);
    osgViewer::Viewer viewer(args);

    // Make sure we have the minimum args...
    if(argc <= 2)
    {
        osg::notify(osg::FATAL) << "usage: " << args[0] << " fontfile size1 [size2 ...]" << std::endl;

        return 1;
    }


    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());


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

    std::string fontfile("arial.ttf");

    fontfile = argv[1];

    // Create the list of desired sizes.
    Sizes sizes;

    for(int i = 2; i < argc; i++)
    {
        if(!args.isNumber(i)) continue;

        sizes.push_back(std::atoi(args[i]));
    }

    osg::Geode* geode = new osg::Geode();

    // Add all of our osgText drawables.
    for(Sizes::const_iterator i = sizes.begin(); i != sizes.end(); i++)
    {
        std::stringstream ss;

        ss << *i << " 1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        geode->addDrawable(createLabel(ss.str(), fontfile, *i));
    }

    root->addChild(geode);

    viewer.setSceneData(root.get());

    return viewer.run();
}

