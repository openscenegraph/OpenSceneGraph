#include "SlideEventHandler.h"
#include "SlideShowConstructor.h"

#include <osg/AnimationPath>
#include <osgUtil/TransformCallback>

class FindNamedSwitchVisitor : public osg::NodeVisitor
{
public:

    FindNamedSwitchVisitor(const std::string& name):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _name(name),
        _switch(0) {}
        
    void apply(osg::Switch& sw)
    {
        if (sw.getName().find(_name)!=std::string::npos)
        {
            _switch = &sw;
            return; // note, no need to do traverse now we've located the relevant switch
        }
        
        traverse(sw);
    }
        
    std::string     _name;
    osg::Switch*    _switch;
        
};


SlideEventHandler::SlideEventHandler():
    _presentationSwitch(0),
    _activeSlide(0),
    _slideSwitch(0),
    _activeLayer(0),
    _firstTraversal(true),
    _previousTime(-1.0f),
    _timePerSlide(1.0),
    _autoSteppingActive(false),
    _loopPresentation(true),
    _pause(false)
{
}

void SlideEventHandler::set(osg::Node* model)
{
    FindNamedSwitchVisitor findPresentation("Presentation");
    model->accept(findPresentation);
    
    if (findPresentation._switch)
    {
        std::cout<<"Presentation '"<<model->getName()<<"'"<<std::endl;
        _presentationSwitch = findPresentation._switch;
        
        SlideShowConstructor::Duration* durationData = dynamic_cast<SlideShowConstructor::Duration*>(_presentationSwitch->getUserData());
        if (durationData)
        {
            std::cout<<"Presentation time set to "<<durationData->duration<<std::endl;
            _timePerSlide = durationData->duration;
        }
        
        selectSlide(0);
    }
    else
    {
        std::cout<<"No presentation present in scene."<<std::endl;

        _presentationSwitch = 0;
        _activeSlide = 0;
        
        FindNamedSwitchVisitor findSlide("Slide");
        model->accept(findSlide);
        
        if (findSlide._switch)
        {
            std::cout<<"Found presentation slide"<<findSlide._switch->getName()<<std::endl;

            _slideSwitch = findSlide._switch;
            selectLayer(0);
        }
        else
        {
            std::cout<<"No slides present in scene, unable to operate as a slideshow."<<std::endl;
        }
    
    }
}

double SlideEventHandler::getCurrentTimeDelayBetweenSlides() const
{
    const SlideShowConstructor::Duration* durationData = dynamic_cast<const SlideShowConstructor::Duration*>(_slideSwitch->getChild(_activeLayer)->getUserData());
    if (durationData)
    {
        return durationData->duration;
    }
    
    durationData = dynamic_cast<const SlideShowConstructor::Duration*>(_slideSwitch->getUserData());
    if (durationData)
    {
        return durationData->duration;
    }
    
    return _timePerSlide;
}


bool SlideEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::FRAME):
        {
            if (_autoSteppingActive && !_pause)
            {
                double time = ea.time();

                if (_firstTraversal)
                {
                    _firstTraversal = false;
                    _previousTime = time;
                }
                else if (time-_previousTime>getCurrentTimeDelayBetweenSlides())
                {
                    _previousTime = time;

                    nextLayerOrSlide();
                }

            }
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='a')
            {
                _autoSteppingActive = !_autoSteppingActive;
                _previousTime = ea.time();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Home ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Home)
            {
                _autoSteppingActive = false;
                selectSlide(0);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_End ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_End)
            {
                _autoSteppingActive = false;
                selectSlide(LAST_POSITION,LAST_POSITION);
                return true;
            }
            else if (ea.getKey()=='n' || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down)
            {
                _autoSteppingActive = false;
                nextLayerOrSlide();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Up)
            {
                _autoSteppingActive = false;
                previousLayerOrSlide();
                return true;
            }
            else if (ea.getKey()=='N' || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Right || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Down)
            {
                _autoSteppingActive = false;
                nextSlide();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Up)
            {
                _autoSteppingActive = false;
                previousSlide();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Pause ||
                     ea.getKey()=='p')
            {
                _pause = !_pause;
                
                resetUpdateCallbackActivity();
                
            }
            else if (ea.getKey()=='r')
            {
                resetUpdateCallbacks();
            }

            return false;
        }

        default:
            return false;
    }
}

void SlideEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("a","Toggle on/off the automatic advancement for image to image");
    usage.addKeyboardMouseBinding("n","Advance to next layer or slide");
    usage.addKeyboardMouseBinding("p","Move to previous layer or slide");
}

bool SlideEventHandler::selectSlide(unsigned int slideNum,unsigned int layerNum)
{
    if (!_presentationSwitch) return false;
    
    if (slideNum==LAST_POSITION && _presentationSwitch->getNumChildren()>0)
    {
        slideNum = _presentationSwitch->getNumChildren()-1;
    }
    
    if (slideNum>=_presentationSwitch->getNumChildren()) return false;

    
    _activeSlide = slideNum;
    _presentationSwitch->setSingleChildOn(_activeSlide);

    //std::cout<<"Selected slide '"<<_presentationSwitch->getChild(_activeSlide)->getName()<<"'"<<std::endl;

    
    FindNamedSwitchVisitor findSlide("Slide");
    _presentationSwitch->getChild(_activeSlide)->accept(findSlide);

    if (findSlide._switch)
    {
        //std::cout<<"Found slide '"<<findSlide._switch->getName()<<"'"<<std::endl;
        _slideSwitch = findSlide._switch;
        return selectLayer(layerNum);
    }
    else
    {
        //std::cout<<"Not found slide"<<std::endl;
    }
    
    return false;
    
}

bool SlideEventHandler::selectLayer(unsigned int layerNum)
{
    if (!_slideSwitch) return false;

    if (layerNum==LAST_POSITION && _slideSwitch->getNumChildren()>0)
    {
        layerNum = _slideSwitch->getNumChildren()-1;
    }

    if (layerNum>=_slideSwitch->getNumChildren()) return false;
    
    _activeLayer = layerNum;
    _slideSwitch->setSingleChildOn(_activeLayer);
    
    //std::cout<<"Selected layer '"<<_slideSwitch->getChild(_activeLayer)->getName()<<"' num="<<_activeLayer<< std::endl;

    return true;
}

bool SlideEventHandler::nextLayerOrSlide()
{
    if (nextLayer()) return true;
    else return nextSlide();
}

bool SlideEventHandler::previousLayerOrSlide()
{
    if (previousLayer()) return true;
    else return previousSlide();
}

bool SlideEventHandler::nextSlide()
{
    if (selectSlide(_activeSlide+1)) return true;
    else if (_loopPresentation) return selectSlide(0);
    else return false;
}

bool SlideEventHandler::previousSlide()
{
    if (_activeSlide>0) return selectSlide(_activeSlide-1,LAST_POSITION);
    else if (_loopPresentation && _presentationSwitch.valid()) return selectSlide(_presentationSwitch->getNumChildren()-1,LAST_POSITION);
    else return false;
}

bool SlideEventHandler::nextLayer()
{
    return selectLayer(_activeLayer+1);
}

bool SlideEventHandler::previousLayer()
{
    if (_activeLayer>0) return selectLayer(_activeLayer-1);
    else return false;
}


class ResetUpdateCallbacksVisitor : public osg::NodeVisitor
{
public:
    ResetUpdateCallbacksVisitor(bool pause): 
//        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _pause(pause) {}
        

    void apply(osg::Node& node)    
    {
        osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback());
        osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(node.getUpdateCallback());
        if (apc)
        {
            apc->reset();
            apc->update(node);
        }
        if (tc)
        {
            //tc->reset();
        }
        traverse(node);
    }
    
    bool _pause;
};

void SlideEventHandler::resetUpdateCallbacks()
{
    ResetUpdateCallbacksVisitor rucv(_pause);
    _presentationSwitch->accept(rucv);
}

class ActivityUpdateCallbacksVisitor : public osg::NodeVisitor
{
public:
    ActivityUpdateCallbacksVisitor(bool pause): 
//        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _pause(pause) {}
        

    void apply(osg::Node& node)    
    {
        osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(node.getUpdateCallback());
        osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(node.getUpdateCallback());
        if (apc)
        {
            apc->setPause(_pause);
        }
        if (tc)
        {
            tc->setPause(_pause);
        }
        traverse(node);
    }
    
    bool _pause;
};

void SlideEventHandler::resetUpdateCallbackActivity()
{
    ActivityUpdateCallbacksVisitor aucv(_pause);
    _presentationSwitch->accept(aucv);
}
