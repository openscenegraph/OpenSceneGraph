#include <cstdlib>
#include <sstream>
#include <osg/io_utils>
#include <osg/ArgumentParser>
#include <osg/Geode>
#include <osg/MatrixTransform>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/StateSetManipulator>

void textInfo(osgText::Text* text)
{
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
        minFilter(osg::Texture::LINEAR_MIPMAP_LINEAR),
        magFilter(osg::Texture::LINEAR),
        maxAnisotropy(16.0f),
        shaderTechnique(osgText::GREYSCALE),
        textColor(1.0f, 1.0f, 1.0f, 1.0f),
        backdropType(osgText::Text::NONE),
        backdropOffset(0.07f, 0.07f),
        backdropColor(0.0f, 0.0f, 0.0f, 1.0f),
        scaleFontSizeToFontResolution(false)
    {
    }


    void readFilterMode(const std::string& value, osg::Texture::FilterMode& filterMode)
    {
        if (value=="LINEAR") filterMode = osg::Texture::LINEAR;
        else if (value=="NEAREST") filterMode = osg::Texture::NEAREST;
        else if (value=="LINEAR_MIPMAP_LINEAR") filterMode = osg::Texture::LINEAR_MIPMAP_LINEAR;
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

        if (arguments.read("--GREYSCALE")) { shaderTechnique = osgText::GREYSCALE; }
        if (arguments.read("--SIGNED_DISTANCE_FIELD")) { shaderTechnique = osgText::SIGNED_DISTANCE_FIELD; }
        if (arguments.read("--ALL_FEATURES")) { shaderTechnique = osgText::ALL_FEATURES; }

        if (arguments.read("--font",fontFilename)) {}

        std::string value;
        if (arguments.read("--min", value)) { readFilterMode(value, minFilter); }
        if (arguments.read("--mag", value)) { readFilterMode(value, magFilter); }

        if (arguments.read("--anisotropy",maxAnisotropy)) {}


        if (arguments.read("--outline")) backdropType = osgText::Text::OUTLINE;
        if (arguments.read("--shadow")) backdropType = osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
        if (arguments.read("--shadow-br")) backdropType = osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
        if (arguments.read("--shadow-cr")) backdropType = osgText::Text::DROP_SHADOW_CENTER_RIGHT;
        if (arguments.read("--shadow-tr")) backdropType = osgText::Text::DROP_SHADOW_TOP_RIGHT;
        if (arguments.read("--shadow-bc")) backdropType = osgText::Text::DROP_SHADOW_BOTTOM_CENTER;
        if (arguments.read("--shadow-tc")) backdropType = osgText::Text::DROP_SHADOW_TOP_CENTER;
        if (arguments.read("--shadow-bl")) backdropType = osgText::Text::DROP_SHADOW_BOTTOM_LEFT;
        if (arguments.read("--shadow-cl")) backdropType = osgText::Text::DROP_SHADOW_CENTER_LEFT;
        if (arguments.read("--shadow-tl")) backdropType = osgText::Text::DROP_SHADOW_TOP_LEFT;



        float offset;
        if (arguments.read("--offset", offset)) backdropOffset.set(offset, offset);

        if (arguments.read("--text-color", textColor.r(), textColor.g(), textColor.b(), textColor.a())) {}
        if (arguments.read("--bd-color", backdropColor.r(), backdropColor.g(), backdropColor.b(), backdropColor.a())) {}
        if (arguments.read("--bg-color", backgroundColor.r(), backgroundColor.g(), backgroundColor.b(), backgroundColor.a())) {}

        if (arguments.read("--constant-size")) scaleFontSizeToFontResolution = false;
        if (arguments.read("--scale-size")) scaleFontSizeToFontResolution = true;

    }

    void setText(osgText::Text& text)
    {
        OSG_NOTICE<<"Settings::setText()"<<std::endl;

        osg::ref_ptr<osgText::Font> font;

        if (fontFilename!="default") font = osgText::readRefFontFile(fontFilename);

        if (!font) font = osgText::Font::getDefaultFont();

        font->setMinFilterHint(minFilter);
        font->setMagFilterHint(magFilter);
        font->setMaxAnisotropy(maxAnisotropy);

        text.setColor(textColor);
        text.setBackdropType(backdropType);
        text.setBackdropOffset(backdropOffset.x(), backdropOffset.y());
        text.setBackdropColor(backdropColor);
        text.setShaderTechnique(shaderTechnique);

        text.setFont(font.get());

    }

    std::string                     fontFilename;
    osg::Texture::FilterMode        minFilter;
    osg::Texture::FilterMode        magFilter;
    float                           maxAnisotropy;
    osgText::ShaderTechnique        shaderTechnique;

    osg::Vec4                       textColor;
    osgText::Text::BackdropType     backdropType;
    osg::Vec2                       backdropOffset;
    osg::Vec4                       backdropColor;
    osg::Vec4                       backgroundColor;
    Sizes                           sizes;
    bool                            scaleFontSizeToFontResolution;
};

osgText::Text* createLabel(const std::string& l, TextSettings& settings, unsigned int size, osg::Vec3& pos)
{
    osgText::Text* label = new osgText::Text();

    settings.setText(*label);

    if (settings.scaleFontSizeToFontResolution)
    {
        label->setCharacterSize(size);
    }

    label->setFontResolution(size, size);
    label->setPosition(pos);
    label->setAlignment(osgText::Text::LEFT_BOTTOM);

    // It seems to be important we do this last to get best results?
    label->setText(l);

    // textInfo(label);

    pos.y() += label->getCharacterHeight()*2.0;

    return label;
}

class KeyHandler : public osgGA::GUIEventHandler
{
public:

    KeyHandler() {}

    ~KeyHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
    {
        osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
        if (!view) return false;

#if 1
        osg::StateSet* stateset = view->getSceneData()->getOrCreateStateSet();
#else
        osg::StateSet* stateset = view->getCamera()->getOrCreateStateSet();
#endif
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::KEYUP):
            {
                if (ea.getKey()=='d')
                {
                    toggleDefine(stateset, "SIGNED_DISTANCE_FIELD");
                    return true;
                }
                else if (ea.getKey()=='o')
                {
                    toggleDefine(stateset, "OUTLINE");
                    return true;
                }
                break;
            }
            default:
                break;
        }
        return false;
    }

    void toggleDefine(osg::StateSet* stateset, const std::string& define)
    {
        osg::StateSet::DefinePair* dp = stateset->getDefinePair(define);
        if (dp)
        {
            OSG_NOTICE<<"Disabling "<<define<<std::endl;
            stateset->removeDefine(define);
        }
        else
        {
            OSG_NOTICE<<"Enabling "<<define<<std::endl;
            stateset->setDefine(define);
        }
    }
};


int main(int argc, char** argv)
{
    osg::ArgumentParser args(&argc, argv);
    osgViewer::Viewer viewer(args);


    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::StatsHandler());
    viewer.addEventHandler(new osgViewer::WindowSizeHandler());
    viewer.addEventHandler(new KeyHandler());

    TextSettings settings;
    settings.backgroundColor = viewer.getCamera()->getClearColor();

    settings.read(args);

    viewer.getCamera()->setClearColor(settings.backgroundColor);

    osg::ref_ptr<osg::Group> root = new osg::Group;

    bool split_screen = args.read("--split");

    if (split_screen)
    {
        viewer.realize();

        // quite an dirty divusion of the master Camera's window if one is assigned.
        if (viewer.getCamera()->getGraphicsContext())
        {
            viewer.stopThreading();

            osg::ref_ptr<osg::GraphicsContext> gc = viewer.getCamera()->getGraphicsContext();
            osg::ref_ptr<const osg::GraphicsContext::Traits> traits = gc->getTraits();

            // left half
            {
                osg::ref_ptr<osg::Camera> camera = new osg::Camera;
                camera->setCullMask(0x1);
                camera->setGraphicsContext(gc.get());
                camera->setViewport(new osg::Viewport(0,0, traits->width/2, traits->height));
                viewer.addSlave(camera.get(), osg::Matrixd::translate(1.0,0.0,0.0), osg::Matrixd::scale(2.0, 1.0, 1.0));
            }

            {
                osg::ref_ptr<osg::Camera> camera = new osg::Camera;
                camera->setCullMask(0x2);
                camera->setGraphicsContext(gc.get());
                camera->setViewport(new osg::Viewport(traits->width/2+2,0, traits->width/2, traits->height));
                viewer.addSlave(camera.get(), osg::Matrixd::translate(-1.0,0.0,0.0), osg::Matrixd::scale(2.0, 1.0, 1.0));
            }

            viewer.getCamera()->setGraphicsContext(0);

            viewer.startThreading();
        }
        else
        {
            split_screen = false;
        }
    }

    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
    transform->setMatrix(osg::Matrixd::rotate(osg::DegreesToRadians(90.0), 1.0, 0.0, 0.0));
    root->addChild(transform.get());
    root = transform;

    osg::ref_ptr<osg::Program> program = new osg::Program;
    std::string shaderFilename;
    while(args.read("--shader", shaderFilename))
    {
        osg::ref_ptr<osg::Shader> shader = osgDB::readRefShaderFile(shaderFilename);
        if (shader.get())
        {
            OSG_NOTICE<<"Loading shader "<<shaderFilename<<std::endl;
            program->addShader(shader.get());
        }
    }

    if (program->getNumShaders()>0)
    {
        OSG_NOTICE<<"Using shaders"<<std::endl;
        root->getOrCreateStateSet()->setAttribute(program.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);
        root->getOrCreateStateSet()->addUniform(new osg::Uniform("glyphTexture", 0));

        settings.shaderTechnique = osgText::ALL_FEATURES;
    }


    std::string outputFilename;
    if (args.read("-o", outputFilename)) {}

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

    osg::ref_ptr<osg::Geode> geode = new osg::Geode();

    osg::Vec3 pos(0.0f, 0.0f, 0.0f);

    // Add all of our osgText drawables.
    for(Sizes::const_iterator i = settings.sizes.begin(); i != settings.sizes.end(); i++)
    {
        std::stringstream ss;

        ss << *i << " 1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

        geode->addDrawable(createLabel(ss.str(), settings, *i, pos));
    }

    root->addChild(geode.get());

    if (split_screen)
    {
        geode->setNodeMask(0x1);

        osg::ref_ptr<osg::Geode> right_geode = new osg::Geode;
        right_geode->setNodeMask(0x2);

        settings.shaderTechnique = osgText::GREYSCALE;

        pos.set(0.0f, 0.0f, 0.0f);

        for(Sizes::const_iterator i = settings.sizes.begin(); i != settings.sizes.end(); i++)
        {
            std::stringstream ss;

            ss << *i << " 1234567890 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ";

            right_geode->addDrawable(createLabel(ss.str(), settings, *i, pos));
        }

        root->addChild(right_geode);
    }


    if (!outputFilename.empty())
    {
        osgDB::writeNodeFile(*root, outputFilename);
        return 0;
    }

    viewer.setSceneData(root.get());

    return viewer.run();
}

