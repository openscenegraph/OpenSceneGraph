#include <osgGA/KeySwitchMatrixManipulator>
#include <osg/Notify>

using namespace osgGA;

void KeySwitchMatrixManipulator::addMatrixManipulator(int key, std::string name, MatrixManipulator *cm)
{
    if(!cm) return;

    _manips[key]=std::make_pair(name,osg::ref_ptr<MatrixManipulator>(cm));
    
    if(!_current)
    {
        _current=cm;
        _current->setHomePosition(_homeEye,_homeCenter,_homeUp,_autoComputeHomePosition);
        _current->setNode(0);
        _current->setCoordinateFrameCallback(getCoordinateFrameCallback());
        _current->setByMatrix(getMatrix());
    }
}

void KeySwitchMatrixManipulator::addNumberedMatrixManipulator(MatrixManipulator *cm)
{
    if(!cm) return;
    addMatrixManipulator('1'+_manips.size(),cm->className(),cm);
}

void KeySwitchMatrixManipulator::selectMatrixManipulator(unsigned int num)
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
        itr->second.second->setHomePosition(_homeEye,_homeCenter,_homeUp,_autoComputeHomePosition);

        if (_current.valid())
        {
            if ( !itr->second.second->getCoordinateFrameCallback() )
            {
                itr->second.second->setCoordinateFrameCallback(_current->getCoordinateFrameCallback());
            }
        
            if ( !itr->second.second->getNode() )
            {
                itr->second.second->setNode(_current->getNode());
            }
            itr->second.second->setByMatrix(_current->getMatrix());
        }
        _current = itr->second.second;
    }
}

void KeySwitchMatrixManipulator::setNode(osg::Node* node)
{
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->setNode(node);
    }
}

void KeySwitchMatrixManipulator::setHomePosition(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up, bool autoComputeHomePosition)
{
    MatrixManipulator::setHomePosition(eye, center, up, autoComputeHomePosition);
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->setHomePosition(eye, center, up, autoComputeHomePosition);
    }
}

void KeySwitchMatrixManipulator::setAutoComputeHomePosition(bool flag)
{
    _autoComputeHomePosition = flag;
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->setAutoComputeHomePosition(flag);
    }
}

void KeySwitchMatrixManipulator::setMinimumDistance(float minimumDistance)
{
    _minimumDistance = minimumDistance;
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->setMinimumDistance(minimumDistance);
    }
}

void KeySwitchMatrixManipulator::computeHomePosition()
{
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->computeHomePosition();
    }
}

void KeySwitchMatrixManipulator::setCoordinateFrameCallback(CoordinateFrameCallback* cb)
{
    _coordinateFrameCallback = cb;
    for(KeyManipMap::iterator itr=_manips.begin();
        itr!=_manips.end();
        ++itr)
    {
        itr->second.second->setCoordinateFrameCallback(cb);
    }
}

MatrixManipulator* KeySwitchMatrixManipulator::getMatrixManipulatorWithIndex(unsigned int index)
{
    unsigned i=0;
    for(KeyManipMap::iterator itr = _manips.begin();
         itr != _manips.end();
         ++itr, ++i)
    {
        if (i==index) return itr->second.second.get(); 
    }
    return 0;
}

const MatrixManipulator* KeySwitchMatrixManipulator::getMatrixManipulatorWithIndex(unsigned int index) const
{
    unsigned i=0;
    for(KeyManipMap::const_iterator itr = _manips.begin();
         itr != _manips.end();
         ++itr, ++i)
    {
        if (i==index) return itr->second.second.get(); 
    }
    return 0;
}

MatrixManipulator* KeySwitchMatrixManipulator::getMatrixManipulatorWithKey(unsigned int key)
{
    KeyManipMap::iterator itr = _manips.find(key); 
    if (itr!=_manips.end()) return itr->second.second.get(); 
    else return 0;
}

const MatrixManipulator* KeySwitchMatrixManipulator::getMatrixManipulatorWithKey(unsigned int key) const
{
    KeyManipMap::const_iterator itr = _manips.find(key); 
    if (itr!=_manips.end()) return itr->second.second.get(); 
    else return 0;
}

bool KeySwitchMatrixManipulator::handle(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    if (!_current) return false;

    bool handled = false;

    if (!ea.getHandled() && ea.getEventType()==GUIEventAdapter::KEYDOWN)
    {

        KeyManipMap::iterator it=_manips.find(ea.getKey());
        if(it != _manips.end())
        {
            osg::notify(osg::INFO)<<"Switching to manipulator: "<<(*it).second.first<<std::endl;
            if ( !it->second.second->getNode() )
            {
                it->second.second->setNode(_current->getNode());
            }
            it->second.second->setByMatrix(_current->getMatrix());
            it->second.second->init(ea,aa);
            _current = it->second.second;

            //_cameraManipChangeCallbacks.notify(this);
            
            handled = true;
        }
    }

    return _current->handle(ea,aa) || handled;
}

void KeySwitchMatrixManipulator::getUsage(osg::ApplicationUsage& usage) const
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
