/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgGA/GUIEventAdapter>

#include <CEGUISystem.h>
#include <RendererModules/OpenGLGUIRenderer/openglrenderer.h>
#include <CEGUIScriptModule.h>
#include <CEGUIFontManager.h>
#include <CEGUISchemeManager.h>
#include <CEGUIWindowManager.h>
#include <CEGUIExceptions.h>

class CEGUIDrawable : public osg::Drawable
{
public:

    CEGUIDrawable();

    /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
    CEGUIDrawable(const CEGUIDrawable& drawable,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        Drawable(drawable,copyop) {}
    
    META_Object(osg,CEGUIDrawable);
    
    void loadScheme(const std::string& scheme);
    void loadFont(const std::string& font);
    void loadLayout(const std::string& layout);

    void drawImplementation(osg::State& state) const;

protected:    

    virtual ~CEGUIDrawable();

    unsigned int _activeContextID;

};


struct CEGUIEventCallback : public osgGA::GUIEventHandler
{
    CEGUIEventCallback() {}
    
    /** do customized Event code. */
    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
    {
        osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
        CEGUIDrawable* cd = dynamic_cast<CEGUIDrawable*>(obj);
        
        if (!ev || !cd) return false;
        
        switch(ea.getEventType())
        {
            case(osgGA::GUIEventAdapter::DRAG):
            case(osgGA::GUIEventAdapter::MOVE):
                CEGUI::System::getSingleton().injectMousePosition(ea.getX(),ea.getY());
                return true;
            case(osgGA::GUIEventAdapter::PUSH):
            {
                CEGUI::System::getSingleton().injectMousePosition(ea.getX(), ea.getY());

                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)  // left
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)  // middle
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)  // right
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);
      
                return true;
            }
            case(osgGA::GUIEventAdapter::RELEASE):
            {
                CEGUI::System::getSingleton().injectMousePosition(ea.getX(), ea.getY());

                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)  // left
                  CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::LeftButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)  // middle
                  CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::MiddleButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)  // right
                  CEGUI::System::getSingleton().injectMouseButtonUp(CEGUI::RightButton);
      
                return true;
            }
            case(osgGA::GUIEventAdapter::DOUBLECLICK):
            {
                // do we need to do something special here to handle double click???  Will just assume button down for now.
                CEGUI::System::getSingleton().injectMousePosition(ea.getX(), ea.getY());

                if (ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)  // left
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::LeftButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON)  // middle
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::MiddleButton);

                else if (ea.getButton() == osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON)  // right
                  CEGUI::System::getSingleton().injectMouseButtonDown(CEGUI::RightButton);

                return true;
            }
            case(osgGA::GUIEventAdapter::KEYDOWN):
                CEGUI::System::getSingleton().injectKeyDown( static_cast<CEGUI::uint>(ea.getKey()) );
                CEGUI::System::getSingleton().injectChar( static_cast<CEGUI::utf32>( ea.getKey() ) );
                return true;
            case(osgGA::GUIEventAdapter::KEYUP):
                CEGUI::System::getSingleton().injectKeyUp( static_cast<CEGUI::uint>(ea.getKey()) );
                return true;
            default:
                break;
        }

        return false;
    }
};

CEGUIDrawable::CEGUIDrawable()
{
    setSupportsDisplayList(false);

    setEventCallback(new CEGUIEventCallback());
    
    new CEGUI::System( new CEGUI::OpenGLRenderer(0) );
    
    _activeContextID = 0;
}

CEGUIDrawable::~CEGUIDrawable()
{
    // delete CEGUI??
}

void CEGUIDrawable::loadScheme(const std::string& scheme)
{
    try
    {
        CEGUI::SchemeManager::getSingleton().loadScheme(scheme.c_str());
    }
    catch (CEGUI::Exception e)
    {
        std::cout<<"CEGUIDrawable::loadScheme Error: "<<e.getMessage()<<std::endl;
    }
}

void CEGUIDrawable::loadFont(const std::string& font)
{
    try
    {
        CEGUI::FontManager::getSingleton().createFont(font.c_str());
    }
    catch (CEGUI::Exception e)
    {
        std::cout<<"CEGUIDrawable::loadFont Error: "<<e.getMessage()<<std::endl;
    }
}

void CEGUIDrawable::loadLayout(const std::string& layout)
{
    try
    {
        CEGUI::Window* myRoot = CEGUI::WindowManager::getSingleton().loadWindowLayout(layout.c_str());
        CEGUI::System::getSingleton().setGUISheet(myRoot);
    }
    catch (CEGUI::Exception e)
    {
        std::cout<<"CEGUIDrawable::loadLayout error: "<<e.getMessage()<<std::endl;
    }

}

void CEGUIDrawable::drawImplementation(osg::State& state) const
{
    if (state.getContextID()!=_activeContextID) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    state.disableAllVertexArrays();

    CEGUI::System::getSingleton().renderGUI();

    glPopAttrib();
    
    state.checkGLErrors("CEGUIDrawable::drawImplementation");
}

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the standard OpenSceneGraph example which loads and visualises 3d models.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("--image <filename>","Load an image and render it on a quad");
    arguments.getApplicationUsage()->addCommandLineOption("--dem <filename>","Load an image/DEM and render it on a HeightField");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line parameters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");
    arguments.getApplicationUsage()->addCommandLineOption("--help-keys","Display keyboard & mouse bindings available");

    arguments.getApplicationUsage()->addCommandLineOption("--help-all","Display all command line, env vars and keyboard & mouse bindings.");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    bool helpAll = arguments.read("--help-all");
    unsigned int helpType = ((helpAll || arguments.read("-h") || arguments.read("--help"))? osg::ApplicationUsage::COMMAND_LINE_OPTION : 0 ) |
                            ((helpAll ||  arguments.read("--help-env"))? osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE : 0 ) |
                            ((helpAll ||  arguments.read("--help-keys"))? osg::ApplicationUsage::KEYBOARD_MOUSE_BINDING : 0 );
    if (helpType)
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }

    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    osg::ref_ptr<CEGUIDrawable> cd = new CEGUIDrawable();
    geode->addDrawable(cd.get());

    std::string scheme;
    while(arguments.read("--scheme",scheme))
    {
        cd->loadScheme(scheme);
    }

    std::string font;
    while(arguments.read("--font",font))
    {
        cd->loadFont(font);
    }

    std::string layout;
    while(arguments.read("--layout",layout))
    {
        cd->loadLayout(layout);
    }


    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // read the scene from the list of file specified command line args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(loadedModel.get());
    
    group->addChild(geode.get());


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    osg::Timer_t end_tick = osg::Timer::instance()->tick();

    std::cout << "Time to load = "<<osg::Timer::instance()->delta_s(start_tick,end_tick)<<std::endl;


    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    // pass the loaded scene graph to the viewer.
    viewer.setSceneData(group.get());

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}

