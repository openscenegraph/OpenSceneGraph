#include <osg/Group>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Projection>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/LightSource>

#include <osgProducer/Viewer>

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

class RotateCallback: public osg::NodeCallback {
public:
	RotateCallback(): osg::NodeCallback(), enabled_(true) {}
	void operator()(osg::Node *node, osg::NodeVisitor *nv)
	{
		osg::MatrixTransform *xform = dynamic_cast<osg::MatrixTransform *>(node);
		if (xform && enabled_) {
			double t = nv->getFrameStamp()->getReferenceTime();
			xform->setMatrix(osg::Matrix::rotate(t, osg::Vec3(0, 0, 1)));
		}
		traverse(node, nv);
	}

	bool enabled_;
};


// yes, I know global variables are not good things in C++
// but in this case it is useful... :-P
RotateCallback *rotate_cb;


class EffectPanel: public Frame {
public:

	class KeyboardHandler: public osgGA::GUIEventHandler {
	public:
		KeyboardHandler(EffectPanel *ep): ep_(ep) {}

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
					osgDB::writeNodeFile(*ep_->getRoot(), "osgfx_model.osg");
					std::cout << "written nodes to \"osgfx_model.osg\"\n";
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
	:	Frame(),
		selected_fx_(-1),
		fxen_(true),
		root_(new osg::Group),
		hints_color_(0.75f, 0.75f, 0.75f, 1.0f),
		name_color_(1, 1, 1, 1),
		desc_color_(1, 1, 0.7f, 1)
	{
		setBackgroundColor(osg::Vec4(0.3f, 0.1f, 0.15f, 0.75f));

		std::cout << "INFO: available osgFX effects:\n";
		osgFX::Registry::Effect_map emap = osgFX::Registry::instance()->getEffectMap();
		for (osgFX::Registry::Effect_map::const_iterator i=emap.begin(); i!=emap.end(); ++i) {
			std::cout << "INFO: \t" << i->first << "\n";
			osg::ref_ptr<osgFX::Effect> effect = static_cast<osgFX::Effect *>(i->second->cloneType());
			effects_.push_back(effect.get());			
		}

		std::cout << "INFO: " << emap.size() << " effect(s) ready.\n";

		if (!effects_.empty()) {
			selected_fx_ = 0;
		}
	}

	inline osg::Group *getRoot() { return root_.get(); }
	inline void setRoot(osg::Group *node) { root_ = node; }

	inline osg::Node *getScene() { return scene_.get(); }
	inline void setScene(osg::Node *node) { scene_ = node; }

	inline bool getEffectsEnabled() const { return fxen_; }
	inline void setEffectsEnabled(bool v) 
	{ 
		fxen_ = v; 
		if (getSelectedEffect()) {
			getSelectedEffect()->setEnabled(fxen_);
		}
	}

	inline int getEffectIndex() const { return selected_fx_; }
	inline void setEffectIndex(int i)
	{
		if (i >= static_cast<int>(effects_.size())) i = 0;
		if (i < 0) i = static_cast<int>(effects_.size()-1);		
		selected_fx_ = i;
		rebuild();
	}

	inline osgFX::Effect *getSelectedEffect()
	{
		if (selected_fx_ >= 0 && selected_fx_ < static_cast<int>(effects_.size())) {
			return effects_[selected_fx_].get();
		}
		return 0;
	}

protected:
	void rebuild_client_area(const Rect &client_rect)
	{
		osg::ref_ptr<osgText::Font> arial = osgText::readFontFile("fonts/arial.ttf");

		osg::ref_ptr<osgText::Text> hints = new osgText::Text;
		hints->setFont(arial.get());
		hints->setColor(hints_color_);
		hints->setAlignment(osgText::Text::CENTER_BOTTOM);
		hints->setCharacterSize(13);
		hints->setFontResolution(13, 13);
		hints->setPosition(osg::Vec3((client_rect.x0+client_rect.x1)/2, client_rect.y0 + 4, 0.1f));
		hints->setText("<RETURN> show/hide this panel      <LEFT> previous effect      <RIGHT> next effect      <DEL> enable/disable effects      'x' save to file      'r' rotate/stop");
		addDrawable(hints.get());

		std::string effect_name = "No Effect Selected";
		std::string effect_description = "";

		if (selected_fx_ >= 0 && selected_fx_ < static_cast<int>(effects_.size())) {
			effect_name = effects_[selected_fx_]->effectName();
			std::string author_name = effects_[selected_fx_]->effectAuthor();
			if (!author_name.empty()) {
				effect_description = author_name = "AUTHOR: " + std::string(effects_[selected_fx_]->effectAuthor()) + std::string("\n\n");
			}
			effect_description += "DESCRIPTION:\n" + std::string(effects_[selected_fx_]->effectDescription());			

			if (scene_.valid() && root_.valid()) {
				root_->removeChild(0, root_->getNumChildren());
				osg::ref_ptr<osgFX::Effect> effect = effects_[selected_fx_].get();
				effect->setEnabled(fxen_);
				effect->setChild(scene_.get());
				effect->setUpDemo();
				root_->addChild(effect.get());
			}
		}

		osg::ref_ptr<osgText::Text> ename = new osgText::Text;
		ename->setFont(arial.get());
		ename->setColor(name_color_);
		ename->setAlignment(osgText::Text::CENTER_TOP);
		ename->setCharacterSize(32);
		ename->setFontResolution(32, 32);
		ename->setPosition(osg::Vec3((client_rect.x0 + client_rect.x1) / 2, client_rect.y1 - 22, 0.1f));
		ename->setText(effect_name);
		addDrawable(ename.get());

		osg::ref_ptr<osgText::Text> edesc = new osgText::Text;
		edesc->setMaximumWidth(client_rect.width() - 16);
		edesc->setFont(arial.get());
		edesc->setColor(desc_color_);
		edesc->setAlignment(osgText::Text::LEFT_TOP);
		edesc->setCharacterSize(16);
		edesc->setFontResolution(16, 16);
		edesc->setPosition(osg::Vec3(client_rect.x0 + 8, client_rect.y1 - 60, 0.1f));
		edesc->setText(effect_description);
		addDrawable(edesc.get());
	}

private:
	int selected_fx_;
	typedef std::vector<osg::ref_ptr<osgFX::Effect> > Effect_list;
	Effect_list effects_;
	bool fxen_;
	osg::ref_ptr<osg::Group> root_;
	osg::ref_ptr<osg::Node> scene_;
	osg::Vec4 hints_color_;
	osg::Vec4 name_color_;
	osg::Vec4 desc_color_;
};


osg::Group *build_hud_base(osg::Group *root)
{
	osg::ref_ptr<osg::Projection> proj = new osg::Projection(osg::Matrix::ortho2D(0, 1024, 0, 768));
	proj->setCullingActive(false);
	root->addChild(proj.get());

	osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform(osg::Matrix::identity());
	xform->setReferenceFrame(osg::Transform::RELATIVE_TO_ABSOLUTE);
	proj->addChild(xform.get());

	osg::StateSet *ss = xform->getOrCreateStateSet();
	ss->setRenderBinDetails(100, "RenderBin");
	ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
	ss->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

	osg::ref_ptr<osg::BlendFunc> bf = new osg::BlendFunc;
	ss->setAttributeAndModes(bf.get());

	return xform.take();
}

EffectPanel *build_gui(osg::Group *root)
{
	osg::ref_ptr<osg::Group> hud = build_hud_base(root);

	osg::ref_ptr<EffectPanel> effect_panel = new EffectPanel;
	effect_panel->setCaption("osgFX Effect Browser");
	effect_panel->setRect(Rect(20, 20, 1000, 280));	

	hud->addChild(effect_panel.get());

	return effect_panel.take();
}

void build_world(osg::Group *root, osg::Node *scene, osgProducer::Viewer &viewer)
{
	osg::ref_ptr<EffectPanel> effect_panel = build_gui(root);
	effect_panel->setScene(scene);
	effect_panel->rebuild();

	viewer.getEventHandlerList().push_front(new EffectPanel::KeyboardHandler(effect_panel.get()));

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
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help")) {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors()) {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    if (arguments.argc() <= 1) {
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) {
        std::cout << arguments.getApplicationName() << ": No data loaded" << std::endl;
        return 1;
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = " << timer.delta_s(start_tick,end_tick) << std::endl;

    // optimize the scene graph, remove rendundent nodes and state etc.
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

    // create the windows and run the threads.
    viewer.realize();

    while(!viewer.done())
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();

        // fire off the cull and draw traversals of the scene.
        viewer.frame();

    }

    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}
