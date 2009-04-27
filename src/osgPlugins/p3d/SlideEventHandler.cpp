/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#include "SlideEventHandler.h"
#include "SlideShowConstructor.h"

#include <osg/AnimationPath>
#include <osg/Transform>
#include <osg/TexEnvCombine>
#include <osg/LightSource>
#include <osg/AlphaFunc>
#include <osg/io_utils>

#include <osgUtil/TransformCallback>
#include <osgUtil/GLObjectsVisitor>

#include <osgGA/AnimationPathManipulator>

#include "AnimationMaterial.h"

#include <iostream>


static osg::observer_ptr<SlideEventHandler> s_seh;

SlideEventHandler* SlideEventHandler::instance() { return s_seh.get(); }

struct ImageStreamOperator : public ObjectOperator
{
    ImageStreamOperator(osg::ImageStream* imageStream):
        _imageStream(imageStream) {}

    virtual void* ptr() const { return _imageStream.get(); }

    virtual void enter()
    {
        osg::notify(osg::INFO)<<"enter() : _imageStream->rewind() + play"<<std::endl;

        reset();
    }
    
    virtual void maintain()
    {
    }
    
    virtual void leave()
    {
       osg::notify(osg::INFO)<<"leave() : _imageStream->pause()"<<std::endl;

        _imageStream->pause();
    }

    virtual void setPause(bool pause)
    {
       osg::notify(osg::INFO)<<"_imageStream->setPause("<<pause<<")"<<std::endl;

        if (pause) _imageStream->pause();
        else _imageStream->play();
    }
    
    virtual void reset()
    {
        osg::ImageStream::StreamStatus previousStatus = _imageStream->getStatus();

        _imageStream->rewind();


        //_imageStream->setVolume(previousVolume);
        
        if(previousStatus==osg::ImageStream::PLAYING)
        {
            _imageStream->play();
        }

        // add a delay so that movie thread has a chance to do the rewind    
        float microSecondsToDelay = SlideEventHandler::instance()->getTimeDelayOnNewSlideWithMovies() * 1000000.0f;
        OpenThreads::Thread::microSleep(static_cast<unsigned int>(microSecondsToDelay));
    }

    osg::ref_ptr<osg::ImageStream>  _imageStream;
};

struct CallbackOperator : public ObjectOperator
{
    CallbackOperator(osg::Node* node, osg::Referenced* callback):
        _node(node),
        _callback(callback) {}

    virtual void* ptr() const { return _callback.get(); }

    virtual void enter()
    {
        reset();
    }
    
    virtual void maintain()
    {
    }
    
    virtual void leave()
    {
    }

    virtual void setPause(bool pause)
    {
	osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(_callback.get());
	osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(_callback.get());
	ss3d::AnimationMaterialCallback* amc = dynamic_cast<ss3d::AnimationMaterialCallback*>(_callback.get());
	if (apc)
	{
            osg::notify(osg::INFO)<<"apc->setPause("<<pause<<")"<<std::endl;
            apc->setPause(pause);
	}
	if (tc)
	{
            osg::notify(osg::INFO)<<"tc->setPause("<<pause<<")"<<std::endl;
            tc->setPause(pause);
	}
	if (amc)
	{
            osg::notify(osg::INFO)<<"amc->setPause("<<pause<<")"<<std::endl;
            amc->setPause(pause);
	}
    }

    virtual void reset()
    {
	osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(_callback.get());
	osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(_callback.get());
	ss3d::AnimationMaterialCallback* amc = dynamic_cast<ss3d::AnimationMaterialCallback*>(_callback.get());
	if (apc)
	{
            apc->reset();
            apc->update(*_node);
	}
	if (tc)
	{
	}
	if (amc)
	{
            amc->reset();
            amc->update(*_node);
	}
    }


    osg::ref_ptr<osg::Node>         _node;
    osg::ref_ptr<osg::Referenced>   _callback;
};

struct LayerAttributesOperator : public ObjectOperator
{
    LayerAttributesOperator(osg::Node* node, SlideShowConstructor::LayerAttributes* la):
        _node(node),
        _layerAttribute(la)
    {
    }

    virtual void* ptr() const { return _layerAttribute.get(); }

    virtual void enter()
    {
        _layerAttribute->callEnterCallbacks(_node.get());

        if (!_layerAttribute->_keys.empty())
        {
            osg::notify(osg::INFO)<<"applyKeys {"<<std::endl;

            for(SlideShowConstructor::LayerAttributes::Keys::iterator itr = _layerAttribute->_keys.begin();
                itr != _layerAttribute->_keys.end();
                ++itr)
            {
                SlideEventHandler::instance()->dispatchEvent(*itr);
            }

            osg::notify(osg::INFO)<<"}"<<std::endl;
        }   
        if (!_layerAttribute->_runStrings.empty())
        {
            for(SlideShowConstructor::LayerAttributes::RunStrings::iterator itr = _layerAttribute->_runStrings.begin();
                itr != _layerAttribute->_runStrings.end();
                ++itr)
            {

                osg::notify(osg::NOTICE)<<"Run "<<itr->c_str()<<std::endl;
                osg::Timer_t startTick = osg::Timer::instance()->tick();

                int result = system(itr->c_str());

	        osg::notify(osg::INFO)<<"system("<<*itr<<") result "<<result<<std::endl;

                double timeForRun = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());

                osgGA::EventQueue* eq = SlideEventHandler::instance()->getViewer()->getEventQueue();
                if (eq)
                {
                    osg::Timer_t new_startTick = eq->getStartTick() + osg::Timer_t(timeForRun / osg::Timer::instance()->getSecondsPerTick());
                    eq->setStartTick(new_startTick);
                }
            }
        }

    }
    
    virtual void maintain()
    {
    }
    
    virtual void leave()
    {
        osg::notify(osg::INFO)<<"LayerAttribute leave"<<std::endl;

         _layerAttribute->callLeaveCallbacks(_node.get());
    }

    virtual void setPause(bool pause)
    {
    }

    virtual void reset()
    {
    }


    osg::ref_ptr<osg::Node>                             _node;
    osg::ref_ptr<SlideShowConstructor::LayerAttributes> _layerAttribute;
};


class FindOperatorsVisitor : public osg::NodeVisitor
{
public:
    FindOperatorsVisitor(ActiveOperators::OperatorList& operatorList, osg::NodeVisitor::TraversalMode tm):
        osg::NodeVisitor(tm),
        _operatorList(operatorList) {}
		
    void apply(osg::Node& node)    
    {
    	if (node.getStateSet()) process(node.getStateSet());

	if (node.getUpdateCallback())
	{
            _operatorList.insert(new CallbackOperator(&node, node.getUpdateCallback()));
	}

        SlideShowConstructor::LayerAttributes* la = dynamic_cast<SlideShowConstructor::LayerAttributes*>(node.getUserData());
        if (la)
        {
            _operatorList.insert(new LayerAttributesOperator(&node, la));
        }
        
        traverse(node);
    }

    void apply(osg::Geode& node)    
    {
        apply((osg::Node&)node);
        
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable->getStateSet()) process(drawable->getStateSet());
        }
    }

    virtual void process(osg::StateSet* ss)
    {
        for(unsigned int i=0;i<ss->getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            osg::Image* image = texture ? texture->getImage(0) : 0;
            osg::ImageStream* imageStream = image ? dynamic_cast<osg::ImageStream*>(image) : 0;
            if (imageStream)
            {
                _operatorList.insert(new ImageStreamOperator(imageStream));
            }
        }
    }	
    
    ActiveOperators::OperatorList& _operatorList;
};


ActiveOperators::ActiveOperators():
    _pause(false)
{
}

ActiveOperators::~ActiveOperators()
{
}

void ActiveOperators::collect(osg::Node* incommingNode, osg::NodeVisitor::TraversalMode tm)
{
    _previous.swap(_current);
    
    _current.clear();
    
    FindOperatorsVisitor fov(_current, tm);
    incommingNode->accept(fov);
    
    osg::notify(osg::INFO)<<"ActiveOperators::collect("<<incommingNode<<")"<<std::endl;
    osg::notify(osg::INFO)<<"  _previous.size()="<<_previous.size()<<std::endl;
    osg::notify(osg::INFO)<<"  _current.size()="<<_current.size()<<std::endl;
    
    _outgoing.clear();
    _incomming.clear();
    _maintained.clear();
    
    for(OperatorList::iterator itr = _previous.begin();
        itr != _previous.end();
        ++itr)
    {
        ObjectOperator* prev = itr->get();
        if (_current.count(prev)==0) _outgoing.insert(prev);
        else _maintained.insert(prev);
    }
    
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        ObjectOperator* curr = itr->get();
        if (_previous.count(curr)==0) _incomming.insert(curr);
    }
}

void ActiveOperators::setPause(bool pause)
{
    _pause = pause;
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        (*itr)->setPause(_pause);
    }
}


void ActiveOperators::reset()
{
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        (*itr)->reset();
    }
}

void ActiveOperators::process()
{
    processOutgoing();
    processMaintained();
    processIncomming();
}

void ActiveOperators::processOutgoing()
{
    osg::notify(osg::INFO)<<"  outgoing.size()="<<_outgoing.size()<<std::endl;    
    for(OperatorList::iterator itr = _outgoing.begin();
        itr != _outgoing.end();
        ++itr)
    {
        (*itr)->leave();
    }
}

void ActiveOperators::processMaintained()
{
    osg::notify(osg::INFO)<<"  maintained.size()="<<_maintained.size()<<std::endl;
    for(OperatorList::iterator itr = _maintained.begin();
        itr != _maintained.end();
        ++itr)
    {
        (*itr)->maintain();
    }
}

void ActiveOperators::processIncomming()
{
    osg::notify(osg::INFO)<<"  incomming.size()="<<_incomming.size()<<std::endl;
    for(OperatorList::iterator itr = _incomming.begin();
        itr != _incomming.end();
        ++itr)
    {
        (*itr)->enter();
        (*itr)->setPause(_pause);
    }
}




class FindHomePositionVisitor : public osg::NodeVisitor
{
public:

    FindHomePositionVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}
        
    void apply(osg::Node& node)
    {
        SlideShowConstructor::HomePosition* homePosition = dynamic_cast<SlideShowConstructor::HomePosition*>(node.getUserData());
        if (homePosition)
        {
            _homePosition = homePosition;
        }
        
        traverse(node);
    }
        
    osg::ref_ptr<SlideShowConstructor::HomePosition> _homePosition;
        
};

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


class FindFilePathDataVisitor : public osg::NodeVisitor
{
public:

    FindFilePathDataVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}
        
    void apply(osg::Node& node)
    {
        SlideShowConstructor::FilePathData* fdd = dynamic_cast<SlideShowConstructor::FilePathData*>(node.getUserData());
        if (fdd) 
        {
            osg::notify(osg::INFO)<<"Recorded FilePathData"<<std::endl;
            osgDB::setDataFilePathList(fdd->filePathList);
        }
        
        traverse(node);
    }
        
};

class UpdateLightVisitor : public osg::NodeVisitor
{
public:

    UpdateLightVisitor(const osg::Matrixd& viewMatrix, float currentX, float currentY):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _viewMatrix(viewMatrix),
        _currentX(currentX), _currentY(currentY) {}
        
    void apply(osg::Node& node)
    {
        if (node.getStateSet()) 
        {
            apply(*node.getStateSet());
        }
        
        traverse(node);
    }
    
    void apply(osg::LightSource& lightsource)
    {
        if (lightsource.getStateSet()) 
        {
            apply(*lightsource.getStateSet());
        }
    
        if (lightsource.getLight())
        {
            osg::notify(osg::INFO)<<"Adjusting light"<<std::endl;

            osg::Light* light = lightsource.getLight();

            float azim = _currentX*osg::PI;
            float elevation = _currentY*osg::PI_2;
            osg::Vec3 direction(sin(azim)*cos(elevation),sin(elevation),cos(azim)*cos(elevation));
            
            if (lightsource.getReferenceFrame()==osg::LightSource::RELATIVE_RF)
            {
                osg::notify(osg::INFO)<<"Relative to absolute"<<std::endl;
            }
            else
            {
                osg::Matrix matrix(osg::computeEyeToLocal(_viewMatrix,_nodePath));
                osg::notify(osg::INFO)<<"ModelView"<<matrix<<std::endl;

                //direction = osg::Matrixd::transform3x3(matrix,direction);
                //direction.normalize();

                //direction = direction*matrix;
                //direction.normalize();

            }

            light->setPosition(osg::Vec4(direction,0.0f));

        }
        
        traverse(lightsource);
    }

    void apply(osg::StateSet& stateset)
    {
        osg::TexEnvCombine* texenvcombine = dynamic_cast<osg::TexEnvCombine*>(stateset.getTextureAttribute(0,osg::StateAttribute::TEXENV));
        if (texenvcombine)
        {
            apply(*texenvcombine);
        }
    }
       
    void apply(osg::TexEnvCombine& texenv)
    {
        osg::notify(osg::INFO)<<"Adjusting tex env combine"<<std::endl;
        
        osg::Matrix matrix(osg::computeEyeToLocal(_viewMatrix,_nodePath));
        
        osg::notify(osg::INFO)<<"ModelView"<<matrix<<std::endl;

        float azim = _currentX*osg::PI;
        float elevation = _currentY*osg::PI_2;
        osg::Vec3 direction(sin(azim)*cos(elevation),sin(elevation),cos(azim)*cos(elevation));

        direction = osg::Matrixd::transform3x3(matrix,direction);
        direction.normalize();

        texenv.setConstantColor(osg::Vec4((direction.x()+1.0f)*0.5f,(direction.y()+1.0f)*0.5f,(direction.z()+1.0f)*0.5f,1.0f));
    }


    osg::Matrixd    _viewMatrix;
    float           _currentX, _currentY;
        
};

class UpdateAlphaVisitor : public osg::NodeVisitor
{
public:

    UpdateAlphaVisitor(bool modAlphaFunc, bool modMaterial, float currentX, float currentY):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _modAlphaFunc(modAlphaFunc), _modMaterial(modMaterial),
        _currentX(currentX), _currentY(currentY) {}
        
    void apply(osg::Node& node)
    {
        if (node.getStateSet()) apply(*node.getStateSet());
        traverse(node);
    }
    
    void apply(osg::StateSet& stateset)
    {
        if (_modAlphaFunc)
        {
            osg::AlphaFunc* alphaFunc = dynamic_cast<osg::AlphaFunc*>(stateset.getAttribute(osg::StateAttribute::ALPHAFUNC));
            if (alphaFunc)
            {
                osg::notify(osg::INFO)<<"Adjusting alpha func"<<std::endl;

                float alpha = alphaFunc->getReferenceValue();
                alpha = osg::clampBetween((1.0f-_currentY)*0.5f,0.0f,1.0f);

                alphaFunc->setReferenceValue(alpha);
            }
        }
        
        if (_modMaterial)
        {        
            osg::Material* material = dynamic_cast<osg::Material*>(stateset.getAttribute(osg::StateAttribute::MATERIAL));
            if (material)
            {
                osg::notify(osg::INFO)<<"Adjusting material func"<<std::endl;
                float alpha = osg::clampBetween((_currentY+1.0f)*0.5f,0.0f,1.0f);
                material->setAlpha(osg::Material::FRONT_AND_BACK,alpha);
            }
        }
    }

    bool _modAlphaFunc, _modMaterial;
    float   _currentX, _currentY;
        
};


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SlideEventHandler
//
SlideEventHandler::SlideEventHandler(osgViewer::Viewer* viewer):
    _viewer(viewer),
    _presentationSwitch(0),
    _activeSlide(0),
    _slideSwitch(0),
    _activeLayer(0),
    _firstTraversal(true),
    _previousTime(-1.0f),
    _timePerSlide(1.0),
    _autoSteppingActive(false),
    _loopPresentation(false),
    _pause(false),
    _hold(false),
    _updateLightActive(false),
    _updateOpacityActive(false),
    _previousX(0), _previousY(0),
    _cursorOn(true),
    _releaseAndCompileOnEachNewSlide(false),
    _firstSlideOrLayerChange(true),
    _tickAtFirstSlideOrLayerChange(0),
    _tickAtLastSlideOrLayerChange(0),
    _timeDelayOnNewSlideWithMovies(0.25f),
    _minimumTimeBetweenKeyPresses(0.25),
    _timeLastKeyPresses(-1.0)
{
    s_seh = this;
}

double SlideEventHandler::getDuration(const osg::Node* node) const
{
    const SlideShowConstructor::LayerAttributes* la = dynamic_cast<const SlideShowConstructor::LayerAttributes*>(node->getUserData());
    return la ? la->_duration : -1.0;
}

void SlideEventHandler::set(osg::Node* model)
{
#if 0
    // pause all slides, then just reenable the current slide.
    ActivityUpdateCallbacksVisitor aucv(ALL_OBJECTS, true);
    aucv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    model->accept(aucv);
#endif

    ActiveOperators operators;
    operators.collect(model, osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    operators.setPause(true);

    FindNamedSwitchVisitor findPresentation("Presentation");
    model->accept(findPresentation);
    
    if (findPresentation._switch)
    {
        osg::notify(osg::INFO)<<"Presentation '"<<model->getName()<<"'"<<std::endl;
        _presentationSwitch = findPresentation._switch;
        
        double duration = getDuration(_presentationSwitch.get());
        if (duration>=0.0)
        {
            osg::notify(osg::INFO)<<"Presentation time set to "<<duration<<std::endl;
            _timePerSlide = duration;
        }
        
        //selectSlide(0);
    }
    else
    {
        osg::notify(osg::INFO)<<"No presentation present in scene."<<std::endl;

        _presentationSwitch = 0;
        _activeSlide = 0;
        
        FindNamedSwitchVisitor findSlide("Slide");
        model->accept(findSlide);
        
        if (findSlide._switch)
        {
            osg::notify(osg::INFO)<<"Found presentation slide"<<findSlide._switch->getName()<<std::endl;

            _slideSwitch = findSlide._switch;
            //selectLayer(0);
        }
        else
        {
            osg::notify(osg::INFO)<<"No slides present in scene, unable to operate as a slideshow."<<std::endl;
        }
    
    }
}

double SlideEventHandler::getCurrentTimeDelayBetweenSlides() const
{
    if (_slideSwitch.valid())
    {
        double duration = -1.0;
        if (_activeLayer<_slideSwitch->getNumChildren())
        {
            duration = getDuration(_slideSwitch->getChild(_activeLayer));
        }

        if (duration < 0.0)
        {
            duration = getDuration(_slideSwitch.get());
        }

        if (duration >=0 )
        {
            return duration;
        }        
    }
        
    return _timePerSlide;
}


void SlideEventHandler::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
    if (ev)
    {
        if (node->getNumChildrenRequiringEventTraversal()>0) traverse(node,nv);

        if (ev->getActionAdapter() && !ev->getEvents().empty())
        {
            for(osgGA::EventQueue::Events::iterator itr = ev->getEvents().begin();
                itr != ev->getEvents().end();
                ++itr)
            {
                handleWithCheckAgainstIgnoreHandledEventsMask(*(*itr), *(ev->getActionAdapter()), node, nv);
            }
        }
    }
}

bool SlideEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{

    if (!_viewer)
    {
        _viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        selectSlide(0);
        home();
        osg::notify(osg::NOTICE)<<"Assigned viewer. to SlideEventHandler"<<std::endl;
    }
    // else  osg::notify(osg::NOTICE)<<"SlideEventHandler::handle()"<<std::endl;


    if (ea.getHandled()) return false;

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
                else if (time-_previousTime>=getCurrentTimeDelayBetweenSlides())
                {
                    // _previousTime = time;
                    
                    if (!_hold)
                    {
                        // increment the previous by the required time delay, note relative to the current
                        // to keep the time relative to an absolute time signal, thus avoid drift of timing.
                        _previousTime += getCurrentTimeDelayBetweenSlides();

                        nextLayerOrSlide();
                    }
                    else
                    {
                        // we're holding of the move to next layer to slide, but we need slip the time forward accordingly
                        // componensate for the extra time that this frame is recieving.
                        _previousTime = time-getCurrentTimeDelayBetweenSlides();
                    }
                }
            }
            return false;
        }

        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            double time = ea.time();
            double deltaTime = time - _timeLastKeyPresses;
            if (deltaTime < _minimumTimeBetweenKeyPresses)
            {
                 break;
            } 
            
            _timeLastKeyPresses = time;

            if (ea.getKey()=='a')
            {
                if (!_autoSteppingActive)
                {
                    _autoSteppingActive = true;
                    _previousTime = ea.time();
                }
                return true;
            }
            else if (ea.getKey()=='q')
            {
                if (_autoSteppingActive)
                {
                    _autoSteppingActive = false;
                    _previousTime = ea.time();
                }
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Home ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Home)
            {
                _autoSteppingActive = false;
                selectSlide(0);
                home(ea,aa);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_End ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_End)
            {
                _autoSteppingActive = false;
                selectSlide(LAST_POSITION,LAST_POSITION);
                home(ea,aa);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Down)
            {
                _autoSteppingActive = false;
                nextLayer();
                return true;
            }
            else if (ea.getKey()=='n')
            {
                _autoSteppingActive = false;
                nextLayerOrSlide();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Up)
            {
                _autoSteppingActive = false;
                previousLayer();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Down || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Down)
            {
                _autoSteppingActive = false;
                nextLayerOrSlide();
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Page_Up ||
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Up)
            {
                _autoSteppingActive = false;
                previousLayerOrSlide();
                return true;
            }
            else if (ea.getKey()=='N' || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_Right || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Right)
            {
                _autoSteppingActive = false;
                nextSlide();
                home(ea,aa);
                return true;
            }
            else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left || 
                     ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Left)
            {
                _autoSteppingActive = false;
                previousSlide();
                home(ea,aa);
                return true;
            }
            else if (ea.getKey()=='p')
            {
                if (!_pause)
                {
                    _pause = true;
#if 0                    
                    resetUpdateCallbackActivity(ALL_OBJECTS);
#endif
                    _activeOperators.setPause(_pause);
                }
                
                return true;
                
            }
            else if (ea.getKey()=='o')
            {
                if (_pause)
                {
                    _pause = false;
#if 0                    
                    resetUpdateCallbackActivity(ALL_OBJECTS);
#endif
                    _activeOperators.setPause(_pause);
                }
                return true;
                
            }
            else if (ea.getKey()=='h')
            {
                _hold = true;
                return true;
            }
            else if (ea.getKey()=='r')
            {
#if 0
                resetUpdateCallbacks(ALL_OBJECTS);
#endif
                _activeOperators.reset();
                return true;
            }
/*            
            else if (ea.getKey()=='c')
            {
                _cursorOn = !_cursorOn;
            
                for( unsigned int i = 0; i < _viewer->getCameraConfig()->getNumberOfCameras(); i++ )
                {
                    Producer::Camera* cam = _viewer->getCameraConfig()->getCamera(i);
                    Producer::RenderSurface* rs = cam->getRenderSurface();
                    rs->useCursor(_cursorOn);
                }

                return true;
            }
*/
            else if (ea.getKey()=='u')
            {
                updateAlpha(true,false,ea.getX(),ea.getY());
                return true;
            }
            else if (ea.getKey()=='i')
            {
                updateAlpha(false,true,ea.getX(),ea.getY());
                return true;
            }
            else if (ea.getKey()=='k')
            {
                updateLight(ea.getX(),ea.getY());
                return true;
            }

            return false;
        }
        case(osgGA::GUIEventAdapter::KEYUP):
        {
            if (ea.getKey()=='h')
            {
                _hold = false;
                return true;
            }
            return false;
        }
        default:
            return false;
    }
    return false;
}

void SlideEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("a","Toggle on/off the automatic advancement for image to image");
    usage.addKeyboardMouseBinding("n","Advance to next layer or slide");
    usage.addKeyboardMouseBinding("p","Move to previous layer or slide");
}

unsigned int SlideEventHandler::getNumSlides()
{
    if (_presentationSwitch.valid()) return _presentationSwitch->getNumChildren();
    else return 0;
}


bool SlideEventHandler::selectSlide(unsigned int slideNum,unsigned int layerNum)
{
    if (!_presentationSwitch) return false;

    osg::notify(osg::INFO)<<"selectSlide("<<slideNum<<","<<layerNum<<")"<<std::endl;
    
    if (slideNum==LAST_POSITION && _presentationSwitch->getNumChildren()>0)
    {
        slideNum = _presentationSwitch->getNumChildren()-1;
    }
    
    if (slideNum>=_presentationSwitch->getNumChildren()) return false;


    osg::Timer_t tick = osg::Timer::instance()->tick();

    if (_firstSlideOrLayerChange)
    {
        _firstSlideOrLayerChange = false;
        _tickAtFirstSlideOrLayerChange = tick;
        _tickAtLastSlideOrLayerChange = tick;
    }

    osg::notify(osg::INFO)<<"selectSlide("<<slideNum<<","<<layerNum<<") at time "<<osg::Timer::instance()->delta_s(_tickAtFirstSlideOrLayerChange, tick)<<" seconds, length ="<<osg::Timer::instance()->delta_s(_tickAtLastSlideOrLayerChange, tick)<<" seconds"<<std::endl;
    
    _tickAtLastSlideOrLayerChange = tick;
    
    // dectivate movies etc on current active slide.
    bool newSlide = _activeSlide!=slideNum;
    if (newSlide)
    {
        if (_releaseAndCompileOnEachNewSlide)
        {
            releaseSlide(_activeSlide);
        }
    }

    _activeSlide = slideNum;
    _presentationSwitch->setSingleChildOn(_activeSlide);

    //osg::notify(osg::INFO)<<"Selected slide '"<<_presentationSwitch->getChild(_activeSlide)->getName()<<"'"<<std::endl;

    
    FindNamedSwitchVisitor findSlide("Slide");
    _presentationSwitch->getChild(_activeSlide)->accept(findSlide);

    bool result = false;
    if (findSlide._switch)
    {
        //osg::notify(osg::INFO)<<"Found slide '"<<findSlide._switch->getName()<<"'"<<std::endl;
        _slideSwitch = findSlide._switch;

        result = selectLayer(layerNum);

        
    }
    else
    {
        //osg::notify(osg::INFO)<<"Not found slide"<<std::endl;
        updateOperators();
    }
    

    // refersh the viewer.
    //_viewer->getKeySwitchMatrixManipulator()->setMinimumDistance(0.001);
    
    _viewer->getCameraManipulator()->setNode(_slideSwitch.get());
    
    _viewer->computeActiveCoordinateSystemNodePath();
    
    // resetUpdateCallbacks(ALL_OBJECTS);
    
    bool _useSlideFilePaths = false;
    if (_useSlideFilePaths)
    {
        // set up the file paths
        FindFilePathDataVisitor ffpdv;
        _presentationSwitch->accept(ffpdv);
    }
    
    if (newSlide && _releaseAndCompileOnEachNewSlide)
    {
        compileSlide(slideNum);
    }

    return result;
    
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

    updateOperators();

    osg::notify(osg::INFO)<<"Selected layer '"<<_slideSwitch->getChild(_activeLayer)->getName()<<"' num="<<_activeLayer<< std::endl;

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
    SlideShowConstructor::LayerAttributes* la = _slideSwitch.valid() ? dynamic_cast<SlideShowConstructor::LayerAttributes*>(_slideSwitch->getUserData()) : 0;
    if (la && la->requiresJump())
    {
        if (la->getRelativeJump())
        {
            int previousSlide = getActiveSlide();
            int previousLayer = getActiveLayer();
            int newSlide = previousSlide + la->getSlideNum();
            int newLayer = previousLayer + la->getLayerNum();
            if (newLayer<0) 
            {
                newLayer = 0;
            }

            return selectSlide(newSlide, newLayer);
        }
        else
        {
            return selectSlide(la->getSlideNum(),la->getLayerNum());
        }
    }

    if (selectSlide(_activeSlide+1)) return true;
    else if (_loopPresentation) return selectSlide(0);
    else return false;
}

bool SlideEventHandler::previousSlide()
{
#if 1
    // start position when doing previous slide set to top of slide
    if (_activeSlide>0) return selectSlide(_activeSlide-1);
    else if (_loopPresentation && _presentationSwitch.valid()) return selectSlide(_presentationSwitch->getNumChildren()-1);
    else return false;
#else
    // start position when doing previous slide set to end of slide
    if (_activeSlide>0) return selectSlide(_activeSlide-1,LAST_POSITION);
    else if (_loopPresentation && _presentationSwitch.valid()) return selectSlide(_presentationSwitch->getNumChildren()-1,LAST_POSITION);
    else return false;
#endif
}

bool SlideEventHandler::nextLayer()
{
    SlideShowConstructor::LayerAttributes* la = (_slideSwitch.valid() && _activeLayer>=0) ? dynamic_cast<SlideShowConstructor::LayerAttributes*>(_slideSwitch->getChild(_activeLayer)->getUserData()) : 0;
    if (la)
    {
        la->callLeaveCallbacks(_slideSwitch->getChild(_activeLayer));
    
        if (la->requiresJump())
        {
            if (la->getRelativeJump())
            {
                int previousSlide = getActiveSlide();
                int previousLayer = getActiveLayer();
                int newSlide = previousSlide + la->getSlideNum();
                int newLayer = previousLayer + la->getLayerNum();
                if (newLayer<0) 
                {
                    newLayer = 0;
                }

                return selectSlide(newSlide, newLayer);
            }
            else
            {
                return selectSlide(la->getSlideNum(),la->getLayerNum());
            }
        }
    }

    return selectLayer(_activeLayer+1);
}

bool SlideEventHandler::previousLayer()
{
    if (_activeLayer>0) return selectLayer(_activeLayer-1);
    else return false;
}


void SlideEventHandler::updateOperators()
{
    _activeOperators.collect(_slideSwitch.get());
    _activeOperators.process();

    if (_viewer.valid())
    {
        UpdateLightVisitor uav(_viewer->getCamera()->getViewMatrix(),0.0f,0.0f);
        _viewer->getSceneData()->accept(uav);
    }
}

bool SlideEventHandler::home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    FindHomePositionVisitor fhpv;
    osg::Node* node = _viewer->getSceneData();
    if (node) node->accept(fhpv);

    if (fhpv._homePosition.valid())
    {
        osg::notify(osg::INFO)<<"Doing home for stored home position."<<std::endl;

        _viewer->getCameraManipulator()->setAutoComputeHomePosition(false);
        _viewer->getCameraManipulator()->setHomePosition(
                                             fhpv._homePosition->eye,
                                             fhpv._homePosition->center,
                                             fhpv._homePosition->up);
    }
    else
    {
        _viewer->getCameraManipulator()->setAutoComputeHomePosition(true);
    }
    _viewer->getCameraManipulator()->home(ea,aa);

    return true;
}

bool SlideEventHandler::home()
{
    osg::ref_ptr<osgGA::GUIEventAdapter> ea =  new osgGA::GUIEventAdapter;
    ea->setEventType(osgGA::GUIEventAdapter::FRAME);
    ea->setTime(_viewer->getEventQueue()->getTime());

    home(*ea,*_viewer);
    return true;
}

void SlideEventHandler::updateAlpha(bool modAlphaFunc, bool modMaterial, float x, float y)
{
    osg::notify(osg::INFO)<<"updateAlpha("<<x<<","<<y<<")"<<std::endl;
    
    UpdateAlphaVisitor uav(modAlphaFunc, modMaterial, x,y);
    if (_presentationSwitch.valid()) _presentationSwitch->accept(uav);
    else if (_viewer->getSceneData()) _viewer->getSceneData()->accept(uav);
}


void SlideEventHandler::updateLight(float x, float y)
{
    osg::notify(osg::INFO)<<"updateLight("<<x<<", "<<y<<")"<<std::endl;

    UpdateLightVisitor uav(_viewer->getCamera()->getViewMatrix(),x,y);
    _viewer->getSceneData()->accept(uav);
}

void SlideEventHandler::compileSlide(unsigned int slideNum)
{
    if (!_compileSlideCallback)
    {
        _compileSlideCallback = new ss3d::CompileSlideCallback();

        osgViewer::Viewer::Cameras cameras;
        _viewer->getCameras(cameras);

        for(osgViewer::Viewer::Cameras::iterator itr = cameras.begin();
            itr != cameras.end();
            ++itr)
        {
            (*itr)->setPreDrawCallback(_compileSlideCallback.get());
        }

    }            
    
    _compileSlideCallback->needCompile(_presentationSwitch->getChild(slideNum));
    
}

void SlideEventHandler::releaseSlide(unsigned int slideNum)
{
    osgUtil::GLObjectsVisitor globjVisitor(osgUtil::GLObjectsVisitor::RELEASE_DISPLAY_LISTS|
                                           osgUtil::GLObjectsVisitor::RELEASE_STATE_ATTRIBUTES);
    globjVisitor.setNodeMaskOverride(0xffffffff);
    
    _presentationSwitch->getChild(slideNum)->accept(globjVisitor);
}

void SlideEventHandler::dispatchEvent(const SlideShowConstructor::KeyPosition& keyPosition)
{
    osg::notify(osg::INFO)<<" keyPosition._key "<<keyPosition._key<<" "<<keyPosition._x<<" "<<keyPosition._y<<std::endl;

    osgGA::EventQueue* eq = _viewer->getEventQueue();

    // reset the time of the last key press to ensure thatthe event is disgarded as a key repeat.
    _timeLastKeyPresses = -1.0;
    
    if (keyPosition._x!=FLT_MAX)
    {
        float xRescaled = eq->getCurrentEventState()->getXmin() + (keyPosition._x+1.0f)*0.5f*(eq->getCurrentEventState()->getXmax()-eq->getCurrentEventState()->getXmin());
        eq->getCurrentEventState()->setX(xRescaled);
    }
    
    if (keyPosition._y!=FLT_MAX)
    {
        float yRescaled = eq->getCurrentEventState()->getYmin() + (keyPosition._y+1.0f)*0.5f*(eq->getCurrentEventState()->getXmax()-eq->getCurrentEventState()->getYmin());
        eq->getCurrentEventState()->setY(yRescaled);
    }

    eq->keyPress(keyPosition._key);
    eq->keyRelease(keyPosition._key);
}



