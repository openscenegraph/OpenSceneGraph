#include <osgProducer/Viewer>

#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/Group>
#include <osg/Switch>
#include <osg/Notify>
#include <osg/Geometry>

#include <osgText/Text>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>


class KeyboardModel : public osg::Referenced
{
public:

        KeyboardModel() { createKeyboard(); }
        
        osg::Group* getScene() { return _scene.get(); }
        
        void keyChange(int key,int value);
        
protected:
        
        ~KeyboardModel() {}

        void addKey(osg::Vec3& pos, int key,const std::string& text,float width, float height);

        void createKeyboard();

        typedef std::map<int, osg::ref_ptr<osg::Switch> > KeyModelMap;
        
        osg::ref_ptr<osg::Group>    _scene;
        KeyModelMap                 _keyModelMap;

};

void KeyboardModel::keyChange(int key,int value)
{
    std::cout << std::hex << key << "\t"<< value << std::dec  << std::endl;

    KeyModelMap::iterator itr = _keyModelMap.find(key);
    if (itr!=_keyModelMap.end())
    {
        itr->second->setSingleChildOn(value);
        
    }
}

void KeyboardModel::addKey(osg::Vec3& pos, int key,const std::string& text,float width, float height)
{

    osg::Geode* geodeUp = new osg::Geode;
    {
        osgText::Text* textUp = new osgText::Text;
        textUp->setFont("fonts/arial.ttf");
        textUp->setColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        textUp->setCharacterSize(height);
        textUp->setPosition(pos);
        textUp->setDrawMode(osgText::Text::TEXT/*|osgText::Text::BOUNDINGBOX*/);
        textUp->setAlignment(osgText::Text::LEFT_CENTER);
        textUp->setAxisAlignment(osgText::Text::XZ_PLANE);
        textUp->setText(text);
        
        geodeUp->addDrawable(textUp);
    }
    
    osg::Geode* geodeDown = new osg::Geode;
    {
        osgText::Text* textDown = new osgText::Text;
        textDown->setFont("fonts/arial.ttf");
        textDown->setColor(osg::Vec4(1.0f,0.0f,1.0f,1.0f));
        textDown->setCharacterSize(height);
        textDown->setPosition(pos);
        textDown->setDrawMode(osgText::Text::TEXT/*||osgText::Text::BOUNDINGBOX*/);
        textDown->setAlignment(osgText::Text::LEFT_CENTER);
        textDown->setAxisAlignment(osgText::Text::XZ_PLANE);
        textDown->setText(text);
        
        geodeDown->addDrawable(textDown);
    }

    osg::Switch* model = new osg::Switch;
    model->addChild(geodeUp,true);
    model->addChild(geodeDown,false);
    
    _scene->addChild(model);

    _keyModelMap[key] = model;
    
    pos.x() += width;
    
}

void KeyboardModel::createKeyboard()
{
    _scene = new osg::Group;
    
    osg::Vec3 pos(0.0f,0.0f,0.0f);
    
    addKey(pos,osgGA::GUIEventAdapter::KEY_Control_L,"Ctrl",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Super_L,"Super",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Alt_L,"Alt",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Space,"Space",3.0f,1.0f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Mode_switch,"Switch",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Super_R,"Super",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Menu,"Menu",2.0f,0.5f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Control_R,"Ctrl",2.0f,0.5f);

    osg::Vec3 middle(pos.x()+1.0f,0.0f,0.0f);
    
    pos.x() = 0.0f;
    pos.z() += 1.0f;

    addKey(pos,osgGA::GUIEventAdapter::KEY_Shift_L,"Shift",2.0f,0.5f);
    addKey(pos,'\\',"\\",1.0f,1.0f);
    addKey(pos,'z',"Z",1.0f,1.0f);
    addKey(pos,'x',"X",1.0f,1.0f);
    addKey(pos,'c',"C",1.0f,1.0f);
    addKey(pos,'v',"V",1.0f,1.0f);
    addKey(pos,'b',"B",1.0f,1.0f);
    addKey(pos,'n',"N",1.0f,1.0f);
    addKey(pos,'m',"M",1.0f,1.0f);
    addKey(pos,',',",",1.0f,1.0f);
    addKey(pos,'.',".",1.0f,1.0f);
    addKey(pos,'/',"/",1.0f,1.0f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Shift_R,"Shift",2.0f,0.5f);

    pos.x() = 0.0f;
    pos.z() += 1.0f;
    
    addKey(pos,osgGA::GUIEventAdapter::KEY_Caps_Lock,"Caps",2.0f,0.5f);
    addKey(pos,'a',"A",1.0f,1.0f);
    addKey(pos,'s',"S",1.0f,1.0f);
    addKey(pos,'d',"D",1.0f,1.0f);
    addKey(pos,'f',"F",1.0f,1.0f);
    addKey(pos,'g',"G",1.0f,1.0f);
    addKey(pos,'h',"H",1.0f,1.0f);
    addKey(pos,'j',"J",1.0f,1.0f);
    addKey(pos,'k',"K",1.0f,1.0f);
    addKey(pos,'l',"L",1.0f,1.0f);
    addKey(pos,';',";",1.0f,1.0f);
    addKey(pos,'\'',"'",1.0f,1.0f);
    addKey(pos,'#',"#",1.0f,1.0f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Return,"Return",4.0f,0.5f);

    pos.x() = 0.0f;
    pos.z() += 1.0f;
    
    addKey(pos,osgGA::GUIEventAdapter::KEY_Tab,"Tab",2.0f,0.5f);
    addKey(pos,'q',"Q",1.0f,1.0f);
    addKey(pos,'w',"W",1.0f,1.0f);
    addKey(pos,'e',"E",1.0f,1.0f);
    addKey(pos,'r',"R",1.0f,1.0f);
    addKey(pos,'t',"T",1.0f,1.0f);
    addKey(pos,'y',"Y",1.0f,1.0f);
    addKey(pos,'u',"U",1.0f,1.0f);
    addKey(pos,'i',"I",1.0f,1.0f);
    addKey(pos,'o',"O",1.0f,1.0f);
    addKey(pos,'p',"P",1.0f,1.0f);
    addKey(pos,'[',"[",1.0f,1.0f);
    addKey(pos,']',"]",1.0f,1.0f);

    pos.x() = 0.0f;
    pos.z() += 1.0f;
    
    addKey(pos,'`',"`",1.0f,1.0f);
    addKey(pos,'1',"1",1.0f,1.0f);
    addKey(pos,'2',"2",1.0f,1.0f);
    addKey(pos,'3',"3",1.0f,1.0f);
    addKey(pos,'4',"4",1.0f,1.0f);
    addKey(pos,'5',"5",1.0f,1.0f);
    addKey(pos,'6',"6",1.0f,1.0f);
    addKey(pos,'7',"7",1.0f,1.0f);
    addKey(pos,'8',"8",1.0f,1.0f);
    addKey(pos,'9',"9",1.0f,1.0f);
    addKey(pos,'0',"0",1.0f,1.0f);
    addKey(pos,'-',"-",1.0f,1.0f);
    addKey(pos,'=',"=",1.0f,1.0f);
    addKey(pos,osgGA::GUIEventAdapter::KEY_BackSpace,"Backspace",3.0f,0.5f);

    pos.x() = 0.0f;
    pos.z() += 1.0f;
    
    float F_height = 0.5f;
    addKey(pos,osgGA::GUIEventAdapter::KEY_Escape,"Esc",2.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F1,"F1",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F2,"F2",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F3,"F3",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F4,"F4",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F5,"F5",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F6,"F6",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F7,"F7",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F8,"F8",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F9,"F9",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F10,"F10",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F11,"F11",1.0f,F_height);
    addKey(pos,osgGA::GUIEventAdapter::KEY_F12,"F12",1.0f,F_height);


    
    float cursorMoveHeight=0.5f;

    pos = middle;    
    addKey(pos,osgGA::GUIEventAdapter::KEY_Left,"Left",2.0f,cursorMoveHeight);
    osg::Vec3 down = pos;
    addKey(pos,osgGA::GUIEventAdapter::KEY_Down,"Down",2.0f,cursorMoveHeight);
    addKey(pos,osgGA::GUIEventAdapter::KEY_Right,"Right",2.0f,cursorMoveHeight);
    
    pos = down;
    pos.z() += 1.0f;
    
    addKey(pos,osgGA::GUIEventAdapter::KEY_Up,"Up",2.0f,cursorMoveHeight);
    


}





class KeyboardEventHandler : public osgGA::GUIEventHandler
{
public:
    
        KeyboardEventHandler(KeyboardModel* keyboardModel):
            _keyboardModel(keyboardModel) {}
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    _keyboardModel->keyChange(ea.getKey(),1);
                    return true;
                }
                case(osgGA::GUIEventAdapter::KEYUP):
                {
                    _keyboardModel->keyChange(ea.getKey(),0);
                    return true;
                }

                default:
                    return false;
            }
        }

        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
        
        osg::ref_ptr<KeyboardModel> _keyboardModel;
        
};

int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getProgramName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-c","Mannually create occluders");
   
    // initialize the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);


    osg::ref_ptr<KeyboardModel> keyboardModel = new KeyboardModel;

    KeyboardEventHandler* keh = new KeyboardEventHandler(keyboardModel.get());
    viewer.getEventHandlerList().push_front(keh);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

     
    // attach the scene graph.
    viewer.setSceneData( keyboardModel->getScene() );

    osgDB::writeNodeFile(*keyboardModel->getScene(),"test.osg");


    // create the windows and run the threads.
    viewer.realize(Producer::CameraGroup::ThreadPerCamera);


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
   
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}
