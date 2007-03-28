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
#include <osgViewer/Viewer>
#include <osg/CoordinateSystemNode>
#include <osgGA/GUIEventAdapter>

#include <CEGUISystem.h>
#include <RendererModules/OpenGLGUIRenderer/openglrenderer.h>
#include <CEGUIScriptModule.h>
#include <CEGUIFontManager.h>
#include <CEGUISchemeManager.h>
#include <CEGUIWindowManager.h>
#include <CEGUIExceptions.h>

#include <iostream>

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

    void drawImplementation(osg::RenderInfo& renderInfo) const;

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

void CEGUIDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = renderInfo.getState();

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
    

    // construct the viewer.
    osgViewer::Viewer viewer;


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

    // run the viewer
    return viewer.run();
}

