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
#include <osg/io_utils>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ExplosionDebriEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/FireEffect>

typedef std::vector<std::string> FileList;
typedef std::map<std::string, osg::ref_ptr<osg::Node> >  ObjectMap;

static ObjectMap    s_objectMap;

class Character : public osg::Referenced
{
public:
    Character();
    
    void setCharacter(const std::string& filename, const std::string& name, const osg::Vec3& orgin, const osg::Vec3& width, const osg::Vec3& catchPos, float positionRatio);
    
    void setLives(const std::string& filename, const osg::Vec3& orgin, const osg::Vec3& delta, unsigned int numLives);
    
    void setCatches(const std::string& filename, const osg::Vec3& orgin, const osg::Vec3& delta, unsigned int numLives);

    void moveLeft();
    void moveRight();
    void moveTo(float positionRatio);

    void reset();

    bool addCatch();
    bool looseLife();

    osg::Vec3 getCurrentCenterOfBasket() const { return _character->getPosition()+_centerBasket; }
    float getCurrentRadiusOfBasket() const { return _radiusBasket; }

    osg::Vec3 getLowerLeft() const { return _character->getPosition(); }
    osg::Vec3 getUpperRight() const { return _character->getPosition(); }

    osg::Vec3 _origin;
    osg::Vec3 _width;

    float                                        _positionRatio;
    osg::ref_ptr<osg::PositionAttitudeTransform> _character;

    unsigned int                                 _numLives;
    osg::ref_ptr<osg::Switch>                    _livesSwitch;

    unsigned int                                 _numCatches;
    osg::ref_ptr<osg::Switch>                    _catchSwitch;
    
    osg::ref_ptr<osg::Group>                     _objectsGroup;
    
    osg::Vec3                                    _centerBasket;
    float                                        _radiusBasket;
    
};

Character::Character():
    _positionRatio(0.5f),
    _numLives(3),
    _numCatches(0)
{
}


void Character::setCharacter(const std::string& filename, const std::string& name, const osg::Vec3& origin, const osg::Vec3& width, const osg::Vec3& catchPos, float positionRatio)
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
        osg::Vec3 pos(-0.5f*_characterSize,0.0f,0.0f);
        osg::Vec3 width(_characterSize*((float)image->s())/(float)(image->t()),0.0f,0.0);
        osg::Vec3 height(0.0f,0.0f,_characterSize);

        osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos,width,height);
        osg::StateSet* stateset = geometry->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geometry);

        _character = new osg::PositionAttitudeTransform;
        _character->setName(name);
        _character->addChild(geode);
        
        moveTo(positionRatio);

        _centerBasket = width*catchPos.x() + height*catchPos.y() + pos;
        _radiusBasket = width.length()*catchPos.z();

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
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        for(unsigned int i=0; i<numLives; ++i)
        {
            osg::Vec3 pos = origin + delta*(float)i + osg::Vec3(0.0f,0.0f,0.0f);
            osg::Vec3 width(characterSize*((float)image->s())/(float)(image->t()),0.0f,0.0);
            osg::Vec3 height(0.0f,0.0f,characterSize);

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
        stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

        for(unsigned int i=0; i<numCatches; ++i)
        {
            osg::Vec3 pos = origin + delta*(float)i + osg::Vec3(0.0f,0.0f,0.0f);
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


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CatchableObject  : public osg::Referenced
{
    public:
        CatchableObject();

        void setObject(const std::string& filename, const std::string& name, const osg::Vec3& center, float size, const osg::Vec3& direction);

        bool anyInside(const osg::Vec3& lower_left, const osg::Vec3& top_right);

        bool centerInside(const osg::Vec3& center, float radius);
        
        void explode();
        
        bool dangerous() { return _dangerous; }

        void stop() { _stopped = true; }
        
        bool stopped() { return _stopped; }
        
        void setTimeToRemove(double time) { _timeToRemove=time; }
        
        double getTimeToRemove() { return _timeToRemove; }
        
        bool needToRemove(double time) { return  _timeToRemove>=0.0 && time>_timeToRemove; }
        
        osg::ref_ptr<osg::PositionAttitudeTransform> _object;
        osg::Vec3                                    _velocity;
        float                                        _mass;
        float                                        _radius;

        bool                                         _stopped;
        bool                                         _dangerous;

        double                                       _timeToRemove;

        static void setUpCatchablesMap(const FileList& fileList);

    public:
    
        // update position and velocity
        void update(double dt);

        /// Set the viscosity of the fluid.
        inline void setFluidViscosity(float v)
        {
            _viscosity = v;
            _viscosityCoefficient = 6 * osg::PI * _viscosity;
        }
       
        /// Get the viscosity of the fluid.
        inline float getFluidViscosity() const { return _viscosity; }

        /// Set the density of the fluid.
        inline void setFluidDensity(float d)
        {
            _density = d;
            _densityCoefficeint = 0.2f * osg::PI * _density;
        }

        /// Get the density of the fluid.
        inline float getFluidDensity() const { return _density; }
        
        
        /// Set the wind vector.
        inline void setWind(const osg::Vec3& wind) { _wind = wind; }
        
        /// Get the wind vector.
        inline const osg::Vec3& getWind() const { return _wind; }
        
        /// Set the acceleration vector.
        inline void setAcceleration(const osg::Vec3& v) { _acceleration = v; }
        
        /// Get the acceleration vector.
        inline const osg::Vec3& getAcceleration() const { return _acceleration; }

        /** Set the acceleration vector to the gravity on earth (0, 0, -9.81).
            The acceleration will be multiplied by the <CODE>scale</CODE> parameter.
        */
        inline void setToGravity(float scale = 1.0f) { _acceleration.set(0, 0, -9.81f*scale); }

        /// Set the fluid parameters as for air (20°C temperature).
        inline void setFluidToAir()
        {
            setToGravity(1.0f);
            setFluidDensity(1.2929f);
            setFluidViscosity(1.8e-5f);
        }
        
        /// Set the fluid parameters as for pure water (20°C temperature).
        inline void setFluidToWater()
        {
            setToGravity(1.0f);
            setFluidDensity(1.0f);
            setFluidViscosity(1.002e-3f);
        }
            

    protected:

        osg::Vec3   _acceleration;
        float       _viscosity;
        float       _density;
        osg::Vec3   _wind;

        float       _viscosityCoefficient;
        float       _densityCoefficeint;
        
 
};

CatchableObject::CatchableObject()
{
    _stopped = false;
    _dangerous = false;
    
    _timeToRemove = -1.0; // do not remove.
    setFluidToAir();
}

void CatchableObject::setUpCatchablesMap(const FileList& fileList)
{
    for(FileList::const_iterator itr=fileList.begin();
        itr!=fileList.end();
        ++itr)
    {
        const std::string& filename = *itr;
        osg::Image* image = osgDB::readImageFile(filename);
        if (image)
        {
            osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
            stateset->setTextureAttributeAndModes(0,new osg::Texture2D(image),osg::StateAttribute::ON);
            stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
            stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            
            osg::Vec3 width((float)(image->s())/(float)(image->t()),0.0f,0.0);
            osg::Vec3 height(0.0f,0.0f,1.0f);
            osg::Vec3 pos = (width+height)*-0.5f;

            osg::Geometry* geometry = osg::createTexturedQuadGeometry(pos,width,height);
            geometry->setStateSet(stateset.get());

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(geometry);

            s_objectMap[filename] = geode;
        }
    }
}

void CatchableObject::setObject(const std::string& filename, const std::string& name, const osg::Vec3& center, float characterSize, const osg::Vec3& velocity)
{
    _radius = 0.5f*characterSize;
    float Area = osg::PI*_radius*_radius;
    float Volume = Area*_radius*4.0f/3.0f;

    _velocity = velocity;
    _mass = 1000.0*Volume;

    if (s_objectMap.count(filename)!=0)
    {
        osg::PositionAttitudeTransform* scaleTransform = new osg::PositionAttitudeTransform;
        scaleTransform->setScale(osg::Vec3(characterSize,characterSize,characterSize));
        scaleTransform->addChild(s_objectMap[filename].get());

        _object = new osg::PositionAttitudeTransform;
        _object->setName(name);
        _object->setPosition(center);
        _object->addChild(scaleTransform);
    }
    else
    {
        osg::notify(osg::NOTICE)<<"CatchableObject::setObject("<<filename<<") not able to create catchable object."<<std::endl;
    }
}

void CatchableObject::update(double dt)
{
    if (_stopped) return;

    float Area = osg::PI*_radius*_radius;
    float Volume = Area*_radius*4.0f/3.0f;

    // compute force due to gravity + boyancy of displacing the fluid that the particle is emersed in.
    osg::Vec3 force = _acceleration * (_mass - _density*Volume);

    // compute force due to friction
    osg::Vec3 relative_wind = _velocity-_wind;            
    force -= relative_wind * Area * (_viscosityCoefficient + _densityCoefficeint*relative_wind.length());            

    // divide force by mass to get acceleration.
    _velocity += force*(dt/_mass);
    _object->setPosition(_object->getPosition()+_velocity*dt);
}

bool CatchableObject::anyInside(const osg::Vec3& lower_left, const osg::Vec3& upper_right)
{
    osg::Vec3 pos = _object->getPosition();
    
    if (pos.x()+_radius < lower_left.x()) return false;
    if (pos.x()-_radius > upper_right.x()) return false;
    if (pos.z()+_radius < lower_left.z()) return false;
    if (pos.z()-_radius > upper_right.z()) return false;

    return true;
}

bool CatchableObject::centerInside(const osg::Vec3& center, float radius)
{
    osg::Vec3 delta = _object->getPosition() - center;
    return (delta.length()<radius);
}


void CatchableObject::explode()
{
    osg::Vec3 position(0.0f,0.0f,0.0f);
    osgParticle::ExplosionEffect* explosion = new osgParticle::ExplosionEffect(position, _radius);
    osgParticle::ExplosionDebriEffect* explosionDebri = new osgParticle::ExplosionDebriEffect(position, _radius);
    osgParticle::SmokeEffect* smoke = new osgParticle::SmokeEffect(position, _radius);
    osgParticle::FireEffect* fire = new osgParticle::FireEffect(position, _radius);

    explosion->setWind(_wind);
    explosionDebri->setWind(_wind);
    smoke->setWind(_wind);
    fire->setWind(_wind);

    _object->addChild(explosion);
    _object->addChild(explosionDebri);
    _object->addChild(smoke);
    _object->addChild(fire);

    _dangerous = true;

}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class GameEventHandler : public osgGA::GUIEventHandler
{
public:

    GameEventHandler();
    
    META_Object(osgStereImageApp,GameEventHandler);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;
    
    osg::Matrix getCameraPosition();
    
    osg::Node* createScene();
    
    void setFOVY(float fovy) { _fovy = fovy; }
    float getFOVY() const { return _fovy; }
    
    void setBackground(const std::string& background) { _backgroundImageFile = background; }
    
    void createNewCatchable();

    enum Players
    {
        PLAYER_GIRL,
        PLAYER_BOY
    };

    void addPlayer(Players player)
    {
        osg::Vec3 livesPosition;
        osg::Vec3 catchesPosition;
        if (_numberOfPlayers==0)
        {
            livesPosition = _originBaseLine+osg::Vec3(0.0f,-0.5f,0.0f);
            catchesPosition = _originBaseLine+osg::Vec3(200.0f,-0.5f,0.0f);
        }
        else
        {
            livesPosition = _originBaseLine+osg::Vec3(900.0f,-0.5f,000.0f);
            catchesPosition = _originBaseLine+osg::Vec3(1100.0f,-0.5f,0.0f);
        }
        
        switch(player)
        {
            case PLAYER_GIRL:
            {
                std::string player_one = "Catch/girl.png"; 
                osg::Vec3 catchPos(0.2, 0.57, 0.34);

                _players[_numberOfPlayers].setCharacter(player_one,"girl", _originBaseLine + osg::Vec3(0.0f,-1.0f,0.0f), _widthBaseLine, catchPos, 0.5f);
                _players[_numberOfPlayers].setLives(player_one,livesPosition, osg::Vec3(0.0f,0.0f,100.0f),3);
                _players[_numberOfPlayers].setCatches("Catch/broach.png",catchesPosition, osg::Vec3(0.0f,0.0f,100.0f),10);

                ++_numberOfPlayers;
                break;
            }
            case PLAYER_BOY:
            {
                std::string player_two = "Catch/boy.png"; 
                osg::Vec3 catchPos(0.8, 0.57, 0.34);

                _players[_numberOfPlayers].setCharacter(player_two,"boy", _originBaseLine + osg::Vec3(0.0f,-2.0f,0.0f), _widthBaseLine, catchPos, 0.5f);
                _players[_numberOfPlayers].setLives(player_two,livesPosition, osg::Vec3(0.0f,0.0f,100.0f),3);
                _players[_numberOfPlayers].setCatches("Catch/broach.png",catchesPosition, osg::Vec3(0.0f,0.0f,100.0f),10);

                 ++_numberOfPlayers;
               break;
            }
        }                
    }
        
        

protected:

    ~GameEventHandler() {}
    GameEventHandler(const GameEventHandler&,const osg::CopyOp&) {}

    osg::Vec3 _origin;
    osg::Vec3 _width;
    osg::Vec3 _height;
    osg::Vec3 _originBaseLine;
    osg::Vec3 _widthBaseLine;
    float     _characterSize;
    
    float _fovy;
    
    std::string _backgroundImageFile;

    osg::ref_ptr<osg::Group> _group;
    
    unsigned int _numberOfPlayers;
    Character _players[2];

    typedef std::list< osg::ref_ptr<CatchableObject> > CatchableObjectList;
    CatchableObjectList _catchableObjects;
    
    FileList _backgroundFiles;
    FileList _benignCatachables;

    bool _leftKeyPressed;
    bool _rightKeyPressed;    
        
};




GameEventHandler::GameEventHandler()
{
    _origin.set(0.0f,0.0f,0.0f);
    _width.set(1280.0f,0.0f,0.0f);
    _height.set(0.0f,0.0f,1024.0f);
    _widthBaseLine = _width*0.9f;
    _originBaseLine = _origin+_width*0.5-_widthBaseLine*0.5f;
    _characterSize = _width.length()*0.2f;

    _backgroundImageFile = "Catch/farm.JPG";
    
    _numberOfPlayers = 0;

    _leftKeyPressed=false;
    _rightKeyPressed=false;

    _backgroundFiles.push_back("Catch/farm.JPG");
    _backgroundFiles.push_back("Catch/sky1.JPG");
    _backgroundFiles.push_back("Catch/sky2.JPG");
    _backgroundFiles.push_back("Catch/sky3.JPG");

    _benignCatachables.push_back("Catch/a.png");
    _benignCatachables.push_back("Catch/b.png");
    _benignCatachables.push_back("Catch/c.png");
    _benignCatachables.push_back("Catch/m.png");
    _benignCatachables.push_back("Catch/n.png");
    _benignCatachables.push_back("Catch/s.png");
    _benignCatachables.push_back("Catch/t.png");
    _benignCatachables.push_back("Catch/u.png");
    _benignCatachables.push_back("Catch/ball.png");
    
    CatchableObject::setUpCatchablesMap(_benignCatachables);
    
    

}

bool GameEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::FRAME):
        {
            // move characters
            if (_leftKeyPressed)
            {
                if (_numberOfPlayers>=2) _players[1].moveLeft();
            }
            
            if (_rightKeyPressed)
            {
                if (_numberOfPlayers>=2) _players[1].moveRight();
            }

            static double previous_time = ea.time();
            double dt = ea.time()-previous_time;
            previous_time = ea.time();
            
            // move objects
            for(CatchableObjectList::iterator itr=_catchableObjects.begin();
                itr!=_catchableObjects.end();
                ++itr)
            {
                (*itr)->update(dt);
                
                bool removeEntry = false;

                for(unsigned int i=0;i<_numberOfPlayers;++i)
                {
                    if ((*itr)->dangerous())
                    {
                        if ((*itr)->anyInside(_players[i].getLowerLeft(),_players[i].getUpperRight()))
                        {
                            _players[i].looseLife();
                            removeEntry = true;
                        }
                    }
                    else
                    {
                        if ((*itr)->centerInside(_players[i].getCurrentCenterOfBasket(),_players[i].getCurrentRadiusOfBasket()))
                        {
                            _players[i].addCatch();
                            removeEntry = true;
                        }
                    }
                }

                if (!(*itr)->anyInside(_origin, _origin+_width+_height) || 
                    (*itr)->needToRemove(ea.time()) ||
                    removeEntry)
                {
                    // need to remove
                    // remove child from parents.
                    osg::ref_ptr<osg::PositionAttitudeTransform> child = (*itr)->_object;
                    osg::Node::ParentList parents = child->getParents();
                    for(osg::Node::ParentList::iterator pitr=parents.begin();
                        pitr!=parents.end();
                        ++pitr)
                    {
                        (*pitr)->removeChild(child.get());
                    }

                    // remove child from catchable list
                    itr = _catchableObjects.erase(itr);

                    createNewCatchable();

                }
                else if ((*itr)->anyInside(_origin, _origin+_width) && !(*itr)->stopped())
                {
                    // hit base line
                    (*itr)->explode();
                    (*itr)->stop();
                    (*itr)->setTimeToRemove(ea.time()+3.0);
                }
                
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
            else if (ea.getKey()==' ')
            {
                for(unsigned int i=0;i<_numberOfPlayers;++i)
                {
                    _players[i].reset();
                }
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

            if (_numberOfPlayers>=1) _players[0].moveTo(px);

            return true;
        }

        default:
            return false;
    }
}

void GameEventHandler::getUsage(osg::ApplicationUsage&) const
{
}

osg::Matrix GameEventHandler::getCameraPosition()
{
    osg::Matrix cameraPosition;
    osg::Vec3 center = _origin+(_width+_height)*0.5f;
    
    float distance = _height.length()/(2.0f*tanf(_fovy*0.5f));
    
    cameraPosition.makeLookAt(center-osg::Vec3(0.0f,distance,0.0f),center,osg::Vec3(0.0f,0.0f,1.0f));
    return cameraPosition;
}

osg::Node* GameEventHandler::createScene()
{
    _group = new osg::Group;

    if (_numberOfPlayers==0)
    {
        addPlayer(PLAYER_GIRL);
    }

    for(unsigned int i=0;i<_numberOfPlayers;++i)
    {
        _group->addChild(_players[i]._character.get());
        _group->addChild(_players[i]._livesSwitch.get());
        _group->addChild(_players[i]._catchSwitch.get());
    }    
    
    createNewCatchable();
    createNewCatchable();
    createNewCatchable();
    createNewCatchable();

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

            _group->addChild(geode);
            
        }
    }

    return _group.get();
}

void GameEventHandler::createNewCatchable()
{
    if (_benignCatachables.empty()) return;

    unsigned int catachableIndex = (unsigned int)((float)_benignCatachables.size()*(float)rand()/(float)RAND_MAX);
    if (catachableIndex>=_benignCatachables.size()) catachableIndex = _benignCatachables.size()-1;
    
    const std::string& filename = _benignCatachables[catachableIndex];

    float ratio = ((float)rand() / (float)RAND_MAX);
    float size = 100.0f*((float)rand() / (float)RAND_MAX);
    float angle = osg::PI*0.25f + 0.5f*osg::PI*((float)rand() / (float)RAND_MAX);
    float speed = 200.0f*((float)rand() / (float)RAND_MAX);

    CatchableObject* catchableObject = new CatchableObject;
    osg::Vec3 position = _origin+_height+_width*ratio + osg::Vec3(0.0f,-0.7f,0.0f);
    osg::Vec3 velocity(-cosf(angle)*speed,0.0f,-sinf(angle)*speed);
    //std::cout<<"angle = "<<angle<<" velocity="<<velocity<<std::endl;
    catchableObject->setObject(filename,"boy",position,size,velocity);
    _catchableObjects.push_back(catchableObject);

    // catchableObject->explode();

    _group->addChild(catchableObject->_object.get());
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
    GameEventHandler* seh = new GameEventHandler();
    viewer.getEventHandlerList().push_front(seh);

    std::string filename;
    if (arguments.read("-b",filename))
    {
        seh->setBackground(filename);
    }


    while (arguments.read("--boy")) seh->addPlayer(GameEventHandler::PLAYER_BOY);
    while (arguments.read("--girl")) seh->addPlayer(GameEventHandler::PLAYER_GIRL);
    
    

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

    //osgDB::writeNodeFile(*rootNode,"test.osg");

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

