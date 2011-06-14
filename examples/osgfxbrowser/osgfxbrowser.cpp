/* OpenSceneGraph example, osgfxbrowser.
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

#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/LightSource>

#include <osgViewer/Viewer>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgText/Text>

#include <osgUtil/Optimizer>

#include <osgGA/GUIEventAdapter>
#include <osgGA/GUIActionAdapter>

#include <osgFX/Registry>
#include <osgFX/Effect>

#include "Frame.h"

#include <vector>
#include <string>
#include <iostream>

class RotateCallback: public osg::NodeCallback {
public:
    RotateCallback(): osg::NodeCallback(), enabled_(true) {}
    void operator()(osg::Node* node, osg::NodeVisitor *nv)
    {
        osg::MatrixTransform *xform = dynamic_cast<osg::MatrixTransform *>(node);
        if (xform && enabled_) {
            double t = nv->getFrameStamp()->getSimulationTime();
            xform->setMatrix(osg::Matrix::rotate(t, osg::Vec3(0, 0, 1)));
        }
        traverse(node, nv);
    }

    bool enabled_;
};


// yes, I know global variables are not good things in C++
// but in this case it is useful... :-P
RotateCallback *rotate_cb;


class EffectPanel: public osgfxbrowser::Frame {
public:

    class KeyboardHandler: public osgGA::GUIEventHandler {
    public:
        KeyboardHandler(EffectPanel* ep): ep_(ep) {}

        bool handle(const osgGA::GUIEventAdapter &ea, osgGA::GUIActionAdapter &)
        {
            if (ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN) {
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Right) {
                    ep_->setEffectIndex(ep_->getEffectIndex()+1);
                    return true;
                }
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Left) {
                    ep_->setEffectIndex(ep_->getEffectIndex()-1);
                    return true;
                }
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Return) {
                    ep_->setNodeMask(0xffffffff - ep_->getNodeMask());
                    return true;
                }
                if (ea.getKey() == osgGA::GUIEventAdapter::KEY_Delete) {
                    ep_->setEffectsEnabled(!ep_->getEffectsEnabled());
                    return true;
                }
                if (ea.getKey() == 'x') {
                    osgDB::writeNodeFile(*ep_->getRoot(), "osgfx_model.osgt");
                    std::cout << "written nodes to \"osgfx_model.osgt\"\n";
                    return true;
                }
                if (ea.getKey() == 'r') {
                    rotate_cb->enabled_ = !rotate_cb->enabled_;
                    return true;
                }
            }

            return false;
        }

    private:
        osg::ref_ptr<EffectPanel> ep_;
    };

    EffectPanel()
    :    osgfxbrowser::Frame(),
        _selected_fx(-1),
        _fxen(true),
        _root(new osg::Group),
        _hints_color(0.75f, 0.75f, 0.75f, 1.0f),
        _name_color(1, 1, 1, 1),
        _desc_color(1, 1, 0.7f, 1)
    {
        setBackgroundColor(osg::Vec4(0.3f, 0.1f, 0.15f, 0.75f));

        std::cout << "INFO: available osgFX effects:\n";
        osgFX::Registry::EffectMap emap = osgFX::Registry::instance()->getEffectMap();
        for (osgFX::Registry::EffectMap::const_iterator i=emap.begin(); i!=emap.end(); ++i) {
            std::cout << "INFO: \t" << i->first << "\n";
            osg::ref_ptr<osgFX::Effect> effect = static_cast<osgFX::Effect *>(i->second->cloneType());
            _effects.push_back(effect.get());            
        }

        std::cout << "INFO: " << emap.size() << " effect(s) ready.\n";

        if (!_effects.empty()) {
            _selected_fx = 0;
        }
    }

    inline osg::Group* getRoot() { return _root.get(); }
    inline void setRoot(osg::Group* node) { _root = node; }

    inline osg::Node* getScene() { return _scene.get(); }
    inline void setScene(osg::Node* node) { _scene = node; }

    inline bool getEffectsEnabled() const { return _fxen; }
    inline void setEffectsEnabled(bool v) 
    { 
        _fxen = v; 
        if (getSelectedEffect()) {
            getSelectedEffect()->setEnabled(_fxen);
        }
    }

    inline int getEffectIndex() const { return _selected_fx; }
    inline void setEffectIndex(int i)
    {
        if (i >= static_cast<int>(_effects.size())) i = 0;
        if (i < 0) i = static_cast<int>(_effects.size()-1);        
        _selected_fx = i;
        rebuild();
    }

    inline osgFX::Effect *getSelectedEffect()
    {
        if (_selected_fx >= 0 && _selected_fx < static_cast<int>(_effects.size())) {
            return _effects[_selected_fx].get();
        }
        return 0;
    }

protected:
    void rebuild_client_area(const osgfxbrowser::Rect &client_rect)
    {
                float zPos = -0.1; // note from Robert, was 0.1f, but now must be -0.1f to keep text visible??#!? due
                                   // to some other change in the OSG not tracked down yet...
        
        osg::ref_ptr<osgText::Font> arial = osgText::readFontFile("fonts/arial.ttf");

        osg::ref_ptr<osgText::Text> hints = new osgText::Text;
        hints->setFont(arial.get());
        hints->setColor(_hints_color);
        hints->setAlignment(osgText::Text::CENTER_BOTTOM);
        hints->setCharacterSize(13);
        hints->setPosition(osg::Vec3((client_rect.x0+client_rect.x1)/2, client_rect.y0 + 4, zPos));
        hints->setText("<RETURN> show/hide this panel      <LEFT> previous effect      <RIGHT> next effect      <DEL> enable/disable effects      'x' save to file      'r' rotate/stop");
        addDrawable(hints.get());

        std::string effect_name = "No Effect Selected";
        std::string effect_description = "";

        if (_selected_fx >= 0 && _selected_fx < static_cast<int>(_effects.size())) {
            effect_name = _effects[_selected_fx]->effectName();
            std::string author_name = _effects[_selected_fx]->effectAuthor();
            if (!author_name.empty()) {
                effect_description = author_name = "AUTHOR: " + std::string(_effects[_selected_fx]->effectAuthor()) + std::string("\n\n");
            }
            effect_description += "DESCRIPTION:\n" + std::string(_effects[_selected_fx]->effectDescription());            

            if (_scene.valid() && _root.valid()) {
                _root->removeChildren(0, _root->getNumChildren());
                osg::ref_ptr<osgFX::Effect> effect = _effects[_selected_fx].get();
                effect->setEnabled(_fxen);
                effect->removeChildren(0, effect->getNumChildren());
                effect->addChild(_scene.get());
                effect->setUpDemo();
                _root->addChild(effect.get());
            }
        }

        osg::ref_ptr<osgText::Text> ename = new osgText::Text;
        ename->setFont(arial.get());
        ename->setColor(_name_color);
        ename->setAlignment(osgText::Text::CENTER_TOP);
        ename->setCharacterSize(32);
        ename->setPosition(osg::Vec3((client_rect.x0 + client_rect.x1) / 2, client_rect.y1 - 22, zPos));
        ename->setText(effect_name);
        addDrawable(ename.get());

        osg::ref_ptr<osgText::Text> edesc = new osgText::Text;
        edesc->setMaximumWidth(client_rect.width() - 16);
        edesc->setFont(arial.get());
        edesc->setColor(_desc_color);
        edesc->setAlignment(osgText::Text::LEFT_TOP);
        edesc->setCharacterSize(16);
        edesc->setPosition(osg::Vec3(client_rect.x0 + 8, client_rect.y1 - 60, zPos));
        edesc->setText(effect_description);
        addDrawable(edesc.get());
    }

private:
    int _selected_fx;
    typedef std::vector<osg::ref_ptr<osgFX::Effect> > Effect_list;
    Effect_list _effects;
    bool _fxen;
    osg::ref_ptr<osg::Group> _root;
    osg::ref_ptr<osg::Node> _scene;
    osg::Vec4 _hints_color;
    osg::Vec4 _name_color;
    osg::Vec4 _desc_color;
};


osg::Group* build_hud_base(osg::Group* root)
{
    osg::ref_ptr<osg::Projection> proj = new osg::Projection(osg::Matrix::ortho2D(0, 1024, 0, 768));
    proj->setCullingActive(false);
    root->addChild(proj.get());

    osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform(osg::Matrix::identity());
    xform->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    proj->addChild(xform.get());

    osg::StateSet *ss = xform->getOrCreateStateSet();
    ss->setRenderBinDetails(100, "RenderBin");
    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

    osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
    ss->setAttributeAndModes(bf.get());

    return xform.release();
}

EffectPanel* build_gui(osg::Group* root)
{
    osg::ref_ptr<osg::Group> hud = build_hud_base(root);

    osg::ref_ptr<EffectPanel> effect_panel = new EffectPanel;
    effect_panel->setCaption("osgFX Effect Browser");
    effect_panel->setRect(osgfxbrowser::Rect(20, 20, 1000, 280));    

    hud->addChild(effect_panel.get());

    return effect_panel.release();
}

void build_world(osg::Group* root, osg::Node* scene, osgViewer::Viewer& viewer)
{
    osg::ref_ptr<EffectPanel> effect_panel = build_gui(root);
    effect_panel->setScene(scene);
    effect_panel->rebuild();

    viewer.addEventHandler(new EffectPanel::KeyboardHandler(effect_panel.get()));

    root->addChild(effect_panel->getRoot());
}

int main(int argc, char *argv[])
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc, argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is a simple browser that allows you to apply osgFX effects to models interactively.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName() + " [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help", "Display this information");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("Left", "Apply previous effect");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("Right", "Apply next effect");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("Del", "Enable or disable osgFX");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("Return", "Show or hide the effect information panel");
    arguments.getApplicationUsage()->addKeyboardMouseBinding("x", "Save the scene graph with current effect applied");


    // construct the viewer.
    osgViewer::Viewer viewer;

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help")) {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    osgViewer::Viewer::ThreadingModel threading = osgViewer::Viewer::SingleThreaded;
    while (arguments.read("--SingleThreaded")) threading = osgViewer::Viewer::SingleThreaded;
    while (arguments.read("--CullDrawThreadPerContext")) threading = osgViewer::Viewer::CullDrawThreadPerContext;
    while (arguments.read("--DrawThreadPerContext")) threading = osgViewer::Viewer::DrawThreadPerContext;
    while (arguments.read("--CullThreadPerCameraDrawThreadPerContext")) threading = osgViewer::Viewer::CullThreadPerCameraDrawThreadPerContext;

    viewer.setThreadingModel(threading);

    // setup stencil buffer for Outline f/x.
    osg::DisplaySettings::instance()->setMinimumNumStencilBits(1);
    unsigned int clearMask = viewer.getCamera()->getClearMask();
    viewer.getCamera()->setClearMask(clearMask | GL_STENCIL_BUFFER_BIT);
    viewer.getCamera()->setClearStencil(0);
 
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors()) {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readNodeFile("dumptruck.osgt");
  
    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // set up a transform to rotate the model
    osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
    rotate_cb = new RotateCallback;
    xform->setUpdateCallback(rotate_cb);
    xform->addChild(loadedModel.get());

    osg::ref_ptr<osg::Light> light = new osg::Light;
    light->setLightNum(0);
    light->setDiffuse(osg::Vec4(1, 1, 1, 1));
    light->setSpecular(osg::Vec4(1, 1, 0.8f, 1));
    light->setAmbient(osg::Vec4(0.2f, 0.2f, 0.2f, 0.2f));
    light->setPosition(osg::Vec4(1, -1, 1, 0));

    osg::ref_ptr<osg::LightSource> root = new osg::LightSource;
    root->setLight(light.get());
    root->setLocalStateSetModes();

    build_world(root.get(), xform.get(), viewer);

    // set the scene to render
    viewer.setSceneData(root.get());

    return viewer.run();
}
