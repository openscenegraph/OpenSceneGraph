#include <osgGA/KeySwitchCameraManipulator>
#include <osg/Notify>

using namespace osgGA;

void KeySwitchCameraManipulator::addCameraManipulator(int key, std::string name, CameraManipulator *cm)
{
    if(!cm) return;

    _manips[key]=std::make_pair(name,osg::ref_ptr<CameraManipulator>(cm));
    if(!_current.valid()){
        _current=cm;
        _current->setNode(_current->getNode());
        _current->setCamera(_current->getCamera());
    }
}

void KeySwitchCameraManipulator::addNumberedCameraManipulator(CameraManipulator *cm)
{
    if(!cm) return;
    addCameraManipulator('1'+_manips.size(),cm->className(),cm);
}

void KeySwitchCameraManipulator::selectCameraManipulator(unsigned int num)
{
    unsigned int manipNo = 0;
    KeyManipMap::iterator itr;
    for(itr=_manips.begin();
        manipNo!=num && itr!=_manips.end();
        ++itr,++manipNo)
    {
    }
    
    if (itr!=_manips.end())
    {
        if (_current.valid())
        {
            itr->second.second->setNode(_current->getNode());
            itr->second.second->setCamera(_current->getCamera());
        }
        _current = itr->second.second;
    }
}

CameraManipulator* KeySwitchCameraManipulator::getCameraManipulator(unsigned int num)
{
    KeyManipMap::iterator itr = _manips.find(num); 
    if (itr!=_manips.end()) return itr->second.second.get(); 
    else return 0;
}

const CameraManipulator* KeySwitchCameraManipulator::getCameraManipulator(unsigned int num) const
{
    KeyManipMap::const_iterator itr = _manips.find(num); 
    if (itr!=_manips.end()) return itr->second.second.get(); 
    else return 0;
}

bool KeySwitchCameraManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    if(ea.getEventType()==GUIEventAdapter::KEYDOWN){

        KeyManipMap::iterator it=_manips.find(ea.getKey());
        if(it != _manips.end()){
            osg::notify(osg::INFO)<<"Switching to manipulator: "<<(*it).second.first<<std::endl;
            it->second.second->setNode(_current->getNode());
            it->second.second->setCamera(_current->getCamera());
            it->second.second->init(ea,aa);
            _current = it->second.second;

            //_cameraManipChangeCallbacks.notify(this);

        }
    }

    return _current->handle(ea,aa);
}

void KeySwitchCameraManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    for(KeyManipMap::const_iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        std::string key; key += (char)(itr->first);
        std::string explanation(std::string("Select '")+itr->second.first+std::string("' camera manipulator"));
        if (_current==itr->second.second) explanation += " (default)";

        usage.addKeyboardMouseBinding(key,explanation);
        itr->second.second->getUsage(usage);
    }
}
