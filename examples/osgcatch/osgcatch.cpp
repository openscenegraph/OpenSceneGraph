/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgProducer/Viewer>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgUtil/Optimizer>

#include <osg/Geode>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/Switch>
#include <osg/TexMat>
#include <osg/Texture2D>

typedef std::vector<std::string> FileList;

struct Character
{
    Character();
    
    void setCharacter(const std::string& filename, const std::string& name, const osg::Vec3& orgin, const osg::Vec3& width, float positionRatio);
    
    void setLives(const std::string& filename, const osg::Vec3& orgin, const osg::Vec3& delta, unsigned int numLives);
    
    void setCatches(const std::string& filename, const osg::Vec3& orgin, const osg::Vec3& delta, unsigned int numLives);

    void moveLeft();
    void moveRight();
    void moveTo(float positionRatio);

    void reset();

    bool addCatch();
    bool looseLife();

    osg::Vec3 _origin;
    osg::Vec3 _width;

    float                                        _positionRatio;
    osg::ref_ptr<osg::PositionAttitudeTransform> _character;

    unsigned int                                 _numLives;
    osg::ref_ptr<osg::Switch>                    _livesSwitch;

    unsigned int                                 _numCatches;
    osg::ref_ptr<osg::Switch>                    _catchSwitch;
};

Character::Character():
    _positionRatio(0.5f),
    _numLives(3),
    _numCatches(0)
{
}


void Character::setCharacter(const std::string& filename, const std::string& name, const osg::Vec3& origin, const osg::Vec3& width, float positionRatio)
{
    _origin = origin;
    _width = width;
    _positionRatio = positionRatio;
    _numLives = 3;
    _numCatches = 0;

    float _characterSize = _width.length()*0.2f;

    osg::Image* image = osgDB::readImageFile(filename);
    if (image)
    {
        osg::Vec3 pos(-0.5f*_characterSize,0.0f,0.0);
        osg::Vec3 width(_characterSize,0.0f,0.0);
        osg::Vec3 height(0.0f,0.0f,_characterSize*((float)image->t())/(float)(image->s()));

        osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos,width,height);
        osg::StateSet* stateset = geometry->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geometry);

        _character = new osg::PositionAttitudeTransform;
        _character->setName(name);
        _character->addChild(geode);
        
        moveTo(positionRatio);
    }
}

void Character::setLives(const std::string& filename, const osg::Vec3& origin, const osg::Vec3& delta, unsigned int numLives)
{
    float characterSize = delta.length();

    _numLives = numLives;
    _livesSwitch = new osg::Switch;

    osg::Image* image = osgDB::readImageFile(filename);
    if (image)
    {
        osg::StateSet* stateset = _livesSwitch->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);

        for(unsigned int i=0; i<numLives; ++i)
        {
            osg::Vec3 pos = origin + delta*(float)i;
            osg::Vec3 width(characterSize,0.0f,0.0);
            osg::Vec3 height(0.0f,0.0f,characterSize*((float)image->t())/(float)(image->s()));

            osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos,width,height);

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);

            _livesSwitch->addChild(geode,true);

        }
    }

}

void Character::setCatches(const std::string& filename, const osg::Vec3& origin, const osg::Vec3& delta, unsigned int numCatches)
{
    float characterSize = delta.length();

    _numCatches = 0;
    _catchSwitch = new osg::Switch;

    osg::Image* image = osgDB::readImageFile(filename);
    if (image)
    {
        osg::StateSet* stateset = _catchSwitch->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);

        for(unsigned int i=0; i<numCatches; ++i)
        {
            osg::Vec3 pos = origin + delta*(float)i;
            osg::Vec3 width(characterSize,0.0f,0.0);
            osg::Vec3 height(0.0f,0.0f,characterSize*((float)image->t())/(float)(image->s()));

            osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos,width,height);

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);

            _catchSwitch->addChild(geode,false);

        }
    }

}

void Character::moveLeft()
{
    moveTo(_positionRatio - 0.01f);
}

void Character::moveRight()
{
    moveTo(_positionRatio + 0.01f);
}

void Character::moveTo(float positionRatio)
{
    if (positionRatio<0.0f) positionRatio = 0.0f;
    if (positionRatio>1.0f) positionRatio = 1.0f;

    _positionRatio = positionRatio;
    _character->setPosition(_origin+_width*+positionRatio);
}

void Character::reset()
{
    _numCatches = 0;
    _numLives = _livesSwitch->getNumChildren();

    _livesSwitch->setAllChildrenOn();
    _catchSwitch->setAllChildrenOff();
}

bool Character::addCatch()
{
    if (!_catchSwitch || _numCatches>=_catchSwitch->getNumChildren()) return false;
    
    _catchSwitch->setValue(_numCatches,true);
    ++_numCatches;
    
    return true;
}

bool Character::looseLife()
{
    if (!_livesSwitch || _numLives==0) return true;
    
    --_numLives;
    _livesSwitch->setValue(_numLives,false);
    
    return (_numLives==0);
}


class SlideEventHandler : public osgGA::GUIEventHandler
{
public:

    SlideEventHandler();
    
    META_Object(osgStereImageApp,SlideEventHandler);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;
    
    osg::Matrix getCameraPosition();
    
    osg::Node* createScene();
    
    void setFOVY(float fovy) { _fovy = fovy; }
    float getFOVY() const { return _fovy; }
    
    void setBackground(const std::string& background) { _backgroundImageFile = background; }

protected:

    ~SlideEventHandler() {}
    SlideEventHandler(const SlideEventHandler&,const osg::CopyOp&) {}

    osg::Vec3 _origin;
    osg::Vec3 _width;
    osg::Vec3 _height;
    osg::Vec3 _originBaseLine;
    osg::Vec3 _widthBaseLine;
    float     _characterSize;
    
    float _fovy;
    
    std::string _backgroundImageFile;


    
    Character _player1;
    Character _player2;


    bool _leftKeyPressed;
    bool _rightKeyPressed;    
        
};




SlideEventHandler::SlideEventHandler()
{
    _origin.set(0.0f,0.0f,0.0f);
    _width.set(1280.0f,0.0f,0.0f);
    _height.set(0.0f,0.0f,1024.0f);
    _widthBaseLine = _width*0.9f;
    _originBaseLine = _origin+_width*0.5-_widthBaseLine*0.5f;
    _characterSize = _width.length()*0.2f;

    _backgroundImageFile = "Images/land_shallow_topo_2048.jpg";

    _leftKeyPressed=false;
    _rightKeyPressed=false;
}

bool SlideEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::FRAME):
        {
            if (_leftKeyPressed)
            {
                _player2.moveLeft();
            }
            
            if (_rightKeyPressed)
            {
                _player2.moveRight();
            }
            
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
            {
                _leftKeyPressed=true;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
            {
                _rightKeyPressed=true;
                return true;
            }
            else if (ea.getKey()=='1')
            {
                _player1.looseLife();
                _player1.addCatch();
                return true;
            }
            else if (ea.getKey()=='2')
            {
                _player2.looseLife();
                _player2.addCatch();
                return true;
            }
            else if (ea.getKey()==' ')
            {
                _player1.reset();
                _player2.reset();
                return true;
            }
        }
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
            {
                _leftKeyPressed=false;
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
            {
                _rightKeyPressed=false;
                return true;
            }
        }
        case(osgGA::GUIEventAdapter::DRAG):
        case(osgGA::GUIEventAdapter::MOVE):
        {
            float px = (ea.getXnormalized()+1.0f)*0.5f;

            _player1.moveTo(px);

            return true;
        }

        default:
            return false;
    }
}

void SlideEventHandler::getUsage(osg::ApplicationUsage&) const
{
}

osg::Matrix SlideEventHandler::getCameraPosition()
{
    osg::Matrix cameraPosition;
    osg::Vec3 center = _origin+(_width+_height)*0.5f;
    
    float distance = _height.length()/(2.0f*tanf(_fovy*0.5f));
    
    cameraPosition.makeLookAt(center-osg::Vec3(0.0f,distance,0.0f),center,osg::Vec3(0.0f,0.0f,1.0f));
    return cameraPosition;
}

osg::Node* SlideEventHandler::createScene()
{


    osg::Group* group = new osg::Group;
    
    _player1.setCharacter("Catch/girl.png","girl", _originBaseLine, _widthBaseLine, 0.4f);
    _player1.setLives("Catch/girl.png",_originBaseLine, osg::Vec3(0.0f,0.0f,100.0f),3);
    _player1.setCatches("Catch/a.JPG",_originBaseLine+osg::Vec3(200.0f,0.0f,0.0f), osg::Vec3(0.0f,0.0f,100.0f),10);
    group->addChild(_player1._character.get());
    group->addChild(_player1._livesSwitch.get());
    group->addChild(_player1._catchSwitch.get());

    _player2.setCharacter("Catch/boy.png","boy", _originBaseLine, _widthBaseLine, 0.4f);
    _player2.setLives("Catch/boy.png",_originBaseLine+osg::Vec3(900.0f,0.0f,000.0f), osg::Vec3(0.0f,0.0f,100.0f),3);
    _player2.setCatches("Catch/b.JPG",_originBaseLine+osg::Vec3(1100.0f,0.0f,0.0f), osg::Vec3(0.0f,0.0f,100.0f),10);
    group->addChild(_player2._character.get());
    group->addChild(_player2._livesSwitch.get());
    group->addChild(_player2._catchSwitch.get());

    
    // background
    {
        osg::Image* image = osgDB::readImageFile(_backgroundImageFile);
        if (image)
        {
            osg::Geometry* geometry = osg::createTexturedQuadGeometry(_origin,_width,_height);
            osg::StateSet* stateset = geometry->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);
            
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);

            group->addChild(geode);
            
        }
    }


    return group;
}


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use node masks to create stereo images.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-d <float>","Time delay in sceonds between the display of successive image pairs when in auto advance mode.");
    arguments.getApplicationUsage()->addCommandLineOption("-a","Enter auto advance of image pairs on start up.");
    arguments.getApplicationUsage()->addCommandLineOption("-x <float>","Horizontal offset of left and right images.");
    arguments.getApplicationUsage()->addCommandLineOption("-y <float>","Vertical offset of left and right images.");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::ESCAPE_SETS_DONE);

    // register the handler to add keyboard and mosue handling.
    SlideEventHandler* seh = new SlideEventHandler();
    viewer.getEventHandlerList().push_front(seh);

    std::string filename;
    if (arguments.read("-b",filename))
    {
        seh->setBackground(filename);
    }


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
    
    // now the windows have been realized we switch off the cursor to prevent it
    // distracting the people seeing the stereo images.
    float fovy = 1.0f;
    for( unsigned int i = 0; i < viewer.getCameraConfig()->getNumberOfCameras(); i++ )
    {
        Producer::Camera* cam = viewer.getCameraConfig()->getCamera(i);
        Producer::RenderSurface* rs = cam->getRenderSurface();
        rs->useCursor(false);
        fovy = osg::DegreesToRadians(cam->getLensVerticalFov());
    }
    
    seh->setFOVY(fovy);
    
    // creat the scene from the file list.
    osg::ref_ptr<osg::Node> rootNode = seh->createScene();

    osgDB::writeNodeFile(*rootNode,"test.osg");

    // set the scene to render
    viewer.setSceneData(rootNode.get());

    // create the windows and run the threads.
    viewer.realize();
    
    viewer.requestWarpPointer(0.5f,0.5f);
        

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        viewer.setView(seh->getCameraPosition());

        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete before exit.
    viewer.sync();
    
    return 0;
}

