/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/SlideEventHandler>
#include <osgPresentation/SlideShowConstructor>
#include <osgPresentation/AnimationMaterial>

#include <osg/AnimationPath>
#include <osg/Transform>
#include <osg/TexEnvCombine>
#include <osg/LightSource>
#include <osg/AlphaFunc>
#include <osg/Timer>
#include <osg/io_utils>
#include <osg/os_utils>

#include <osgUtil/TransformCallback>
#include <osgUtil/GLObjectsVisitor>

#include <osgDB/WriteFile>

#include <osgGA/AnimationPathManipulator>


#include <iostream>

using namespace osgPresentation;

static osg::observer_ptr<SlideEventHandler> s_seh;

SlideEventHandler* SlideEventHandler::instance() { return s_seh.get(); }

bool JumpData::jump(SlideEventHandler* seh) const
{
        OSG_INFO<<"Requires jump"<<seh<<", "<<relativeJump<<", "<<slideNum<<", "<<layerNum<<", "<<slideName<<", "<<layerName<<std::endl;

        int slideNumToUse = slideNum;
        int layerNumToUse = layerNum;

        if (!slideName.empty())
        {
            osg::Switch* presentation = seh->getPresentationSwitch();
            if (presentation)
            {
                for(unsigned int i=0; i<presentation->getNumChildren(); ++i)
                {
                    osg::Node* node = seh->getSlide(i);
                    std::string name;
                    if (node->getUserValue("name",name) && slideName==name)
                    {
                        slideNumToUse = i;
                        break;
                    }
                }
            }
        }
        else if (relativeJump)
        {
            slideNumToUse = seh->getActiveSlide() + slideNum;
        }


        if (!layerName.empty())
        {
            osg::Switch* slide = seh->getSlide(slideNumToUse);
            if (slide)
            {
                unsigned int i;
                for(i=0; i<slide->getNumChildren(); ++i)
                {
                    osg::Node* node = slide->getChild(i);
                    std::string name;
                    if (node->getUserValue("name",name))
                    {
                        if (layerName==name)
                        {
                            layerNumToUse = i;
                            break;
                        }
                    }
                }
                if (i==slide->getNumChildren())
                {
                    OSG_INFO<<"Could not find layer with "<<layerName<<std::endl;
                }
            }
            else
            {
                OSG_INFO<<"No appropriate Slide found."<<std::endl;
            }
        }
        else if (relativeJump)
        {
            layerNumToUse = seh->getActiveLayer() + layerNum;
        }

        if (slideNumToUse<0) slideNumToUse = 0;
        if (layerNumToUse<0) layerNumToUse = 0;

        OSG_INFO<<"   jump to "<<slideNumToUse<<", "<<layerNumToUse<<std::endl;
        return seh->selectSlide(slideNumToUse,layerNumToUse);
}

void LayerAttributes::callEnterCallbacks(osg::Node* node)
{
    OSG_INFO<<"LayerAttributes::callEnterCallbacks("<<node<<")"<<std::endl;
    for(LayerCallbacks::iterator itr = _enterLayerCallbacks.begin();
        itr != _enterLayerCallbacks.end();
        ++itr)
    {
        (*(*itr))(node);
    }
}

void LayerAttributes::callLeaveCallbacks(osg::Node* node)
{
    OSG_INFO<<"LayerAttributes::callLeaveCallbacks("<<node<<")"<<std::endl;
    for(LayerCallbacks::iterator itr = _leaveLayerCallbacks.begin();
        itr != _leaveLayerCallbacks.end();
        ++itr)
    {
        (*(*itr))(node);
    }
}


struct InteractiveImageSequenceOperator : public ObjectOperator
{
    InteractiveImageSequenceOperator(osg::ImageSequence* imageSequence):
        _imageSequence(imageSequence) {}

    virtual void* ptr() const { return _imageSequence.get(); }

    virtual void enter(SlideEventHandler* seh)
    {
        set(seh);
        // need to pause till the load has been completed.
    }

    virtual void maintain(SlideEventHandler* /*seh*/)
    {
    }

    virtual void leave(SlideEventHandler* /*seh*/)
    {
    }

    virtual void setPause(SlideEventHandler* /*seh*/, bool /*pause*/)
    {
    }

    virtual void reset(SlideEventHandler* seh)
    {
        set(seh);
    }

    void set(SlideEventHandler* /*seh*/)
    {
        //OSG_NOTICE<<"InteractiveImageSequenceOperator::set(..)"<<std::endl;
    }

    osg::ref_ptr<osg::ImageSequence>  _imageSequence;
};

struct ImageStreamOperator : public ObjectOperator
{
    ImageStreamOperator(osg::ImageStream* imageStream):
        _imageStream(imageStream),
        _delayTime(0.0),
        _startTime(0.0),
        _stopTime(-1.0),
        _timeOfLastReset(0.0),
        _started(false),
        _stopped(false)
    {
        _imageStream->getUserValue("delay",_delayTime);
        _imageStream->getUserValue("start",_startTime);
        _imageStream->getUserValue("stop",_stopTime);
    }

    virtual void* ptr() const { return _imageStream.get(); }


    virtual void enter(SlideEventHandler* seh)
    {
        OSG_NOTICE<<"enter() : _imageStream->rewind() + play"<<std::endl;

        reset(seh);
    }

    virtual void frame(SlideEventHandler* seh)
    {
        if (_delayTime!=0.0 && !_started && !_stopped)
        {
            double timeSinceLastRest = seh->getReferenceTime()-_timeOfLastReset;
            if (timeSinceLastRest>_delayTime)
            {
                OSG_NOTICE<<"ImageStreamOperator::frame("<<seh->getReferenceTime()<<") calling start"<<std::endl;
                start(seh);
            }
        }
        if (_stopTime>0.0 && _started && !_stopped)
        {
            double timeSinceLastReset = seh->getReferenceTime()-_timeOfLastReset;
            double timeSinceStart = (timeSinceLastReset-_delayTime);
            if ((timeSinceStart+_startTime)>_stopTime)
            {
                OSG_NOTICE<<"ImageStreamOperator::frame("<<seh->getReferenceTime()<<") calling stop"<<std::endl;
                stop(seh);
            }
        }
    }

    virtual void maintain(SlideEventHandler*)
    {
        OSG_NOTICE<<"ImageStreamOperator::maintain()"<<std::endl;
    }

    virtual void leave(SlideEventHandler*)
    {
       OSG_NOTICE<<"leave() : _imageStream->pause()"<<std::endl;

        _imageStream->pause();
    }

    virtual void setPause(SlideEventHandler*, bool pause)
    {
       OSG_INFO<<"_imageStream->setPause("<<pause<<")"<<std::endl;

        if (_started)
        {
            if (pause) _imageStream->pause();
            else _imageStream->play();
        }
    }

    virtual void reset(SlideEventHandler* seh)
    {
        OSG_NOTICE<<"ImageStreamOperator::reset()"<<std::endl;

        _timeOfLastReset = seh->getReferenceTime();
        _stopped = false;

        if (_delayTime==0.0)
        {
            start(seh);
        }
    }

    void start(SlideEventHandler*)
    {
        if (_started) return;

        _started = true;
        _stopped = false;

        if (_startTime!=0.0) _imageStream->seek(_startTime);
        else _imageStream->rewind();

        //_imageStream->setVolume(previousVolume);

        _imageStream->play();

        // add a delay so that movie thread has a chance to do the rewind
        float microSecondsToDelay = SlideEventHandler::instance()->getTimeDelayOnNewSlideWithMovies() * 1000000.0f;
        OpenThreads::Thread::microSleep(static_cast<unsigned int>(microSecondsToDelay));
    }

    void stop(SlideEventHandler* /*seh*/)
    {
        if (!_started) return;

        _started = false;
        _stopped = true;

        _imageStream->pause();
    }

    osg::ref_ptr<osg::ImageStream>  _imageStream;
    double      _delayTime;
    double      _startTime;
    double      _stopTime;
    double      _timeOfLastReset;
    bool        _started;
    bool        _stopped;
};

struct CallbackOperator : public ObjectOperator
{
    CallbackOperator(osg::Node* node, osg::Referenced* callback):
        _node(node),
        _callback(callback) {}

    virtual void* ptr() const { return _callback.get(); }

    virtual void enter(SlideEventHandler* seh)
    {
        reset(seh);
    }

    virtual void maintain(SlideEventHandler*)
    {
    }

    virtual void leave(SlideEventHandler*)
    {
    }

    virtual void setPause(SlideEventHandler*, bool pause)
    {
        osg::NodeCallback* nc = dynamic_cast<osg::NodeCallback*>(_callback.get());
        osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(_callback.get());
        osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(_callback.get());
        AnimationMaterialCallback* amc = dynamic_cast<AnimationMaterialCallback*>(_callback.get());
        PropertyAnimation* pa = dynamic_cast<PropertyAnimation*>(_callback.get());
        if (apc)
        {
            OSG_INFO<<"apc->setPause("<<pause<<")"<<std::endl;
            apc->setPause(pause);
        }
        else if (tc)
        {
            OSG_INFO<<"tc->setPause("<<pause<<")"<<std::endl;
            tc->setPause(pause);
        }
        else if (amc)
        {
            OSG_INFO<<"amc->setPause("<<pause<<")"<<std::endl;
            amc->setPause(pause);
        }
        else if (pa)
        {
            pa->setPause(pause);
        }
        else if (nc)
        {
            OSG_INFO<<"Need to pause callback : "<<nc->className()<<std::endl;
        }

    }

    virtual void reset(SlideEventHandler*)
    {
        osg::NodeCallback* nc = dynamic_cast<osg::NodeCallback*>(_callback.get());
        osg::AnimationPathCallback* apc = dynamic_cast<osg::AnimationPathCallback*>(_callback.get());
        osgUtil::TransformCallback* tc = dynamic_cast<osgUtil::TransformCallback*>(_callback.get());
        AnimationMaterialCallback* amc = dynamic_cast<AnimationMaterialCallback*>(_callback.get());
        PropertyAnimation* pa = dynamic_cast<PropertyAnimation*>(_callback.get());
        if (apc)
        {
            apc->reset();
            apc->update(*_node);
        }
        else if (tc)
        {
        }
        else if (amc)
        {
            amc->reset();
            amc->update(*_node);
        }
        else if (pa)
        {
            pa->reset();
            pa->update(*_node);
        }
        else
        {
            OSG_INFO<<"Need to reset callback : "<<nc->className()<<std::endl;
        }
    }


    osg::ref_ptr<osg::Node>         _node;
    osg::ref_ptr<osg::Referenced>   _callback;
};

struct LayerAttributesOperator : public ObjectOperator
{
    LayerAttributesOperator(osg::Node* node, LayerAttributes* la):
        _node(node),
        _layerAttribute(la)
    {
    }

    virtual void* ptr() const { return _layerAttribute.get(); }

    virtual void enter(SlideEventHandler*)
    {
        _layerAttribute->callEnterCallbacks(_node.get());

        if (!_layerAttribute->_keys.empty())
        {
            OSG_INFO<<"applyKeys {"<<std::endl;

            for(LayerAttributes::Keys::iterator itr = _layerAttribute->_keys.begin();
                itr != _layerAttribute->_keys.end();
                ++itr)
            {
                SlideEventHandler::instance()->dispatchEvent(*itr);
            }

            OSG_INFO<<"}"<<std::endl;
        }
        if (!_layerAttribute->_runStrings.empty())
        {
            for(LayerAttributes::RunStrings::iterator itr = _layerAttribute->_runStrings.begin();
                itr != _layerAttribute->_runStrings.end();
                ++itr)
            {

                OSG_NOTICE<<"Run "<<itr->c_str()<<std::endl;
                osg::Timer_t startTick = osg::Timer::instance()->tick();

                int result = osg_system(itr->c_str());

                OSG_INFO<<"system("<<*itr<<") result "<<result<<std::endl;

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

    virtual void maintain(SlideEventHandler*)
    {
    }

    virtual void leave(SlideEventHandler*)
    {
        OSG_INFO<<"LayerAttribute leave"<<std::endl;

         _layerAttribute->callLeaveCallbacks(_node.get());
    }

    virtual void setPause(SlideEventHandler*, bool /*pause*/)
    {
    }

    virtual void reset(SlideEventHandler*)
    {
    }


    osg::ref_ptr<osg::Node>                             _node;
    osg::ref_ptr<LayerAttributes> _layerAttribute;
};


class FindOperatorsVisitor : public osg::NodeVisitor
{
public:
    FindOperatorsVisitor(ActiveOperators::OperatorList& operatorList, osg::NodeVisitor::TraversalMode tm):
        osg::NodeVisitor(tm),
        _operatorList(operatorList) {}

    META_NodeVisitor(osgPresentation, FindOperatorsVisitor)

    void apply(osg::Node& node)
    {
        if (node.getStateSet()) process(node.getStateSet());

        if (node.getUpdateCallback())
        {
            _operatorList.insert(new CallbackOperator(&node, node.getUpdateCallback()));
        }

        LayerAttributes* la = dynamic_cast<LayerAttributes*>(node.getUserData());
        if (la)
        {
            if ((_objectsHandled[la]++)==0)
            {
                OSG_INFO<<"LayerAttributeOperator for "<<la<<" required, assigning one."<<std::endl;
                _operatorList.insert(new LayerAttributesOperator(&node, la));
            }
            else
            {
                OSG_INFO<<"LayerAttributeOperator for "<<la<<" not required, as one already assigned."<<std::endl;
            }
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
            osg::ImageSequence* imageSequence = dynamic_cast<osg::ImageSequence*>(image);
            osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(image);
            if (imageSequence && imageSequence->getName()=="USE_MOUSE_X_POSITION")
            {
                if ((_objectsHandled[image]++)==0)
                {
                    OSG_INFO<<"ImageSequenceOperator for"<<imageSequence<<" required, assigning one, name = '"<<image->getName()<<"'"<<std::endl;
                    _operatorList.insert(new InteractiveImageSequenceOperator(imageSequence));
                }
                else
                {
                    OSG_INFO<<"ImageSequenceOperator for"<<imageSequence<<" not required, as one already assigned"<<std::endl;
                }
            }
            else if (imageStream)
            {
                if ((_objectsHandled[image]++)==0)
                {
                    OSG_INFO<<"ImageStreamOperator for"<<imageStream<<" required, assigning one"<<std::endl;
                    _operatorList.insert(new ImageStreamOperator(imageStream));
                }
                else
                {
                    OSG_INFO<<"ImageStreamOperator for"<<imageStream<<" not required, as one already assigned"<<std::endl;
                }
            }
        }
    }

    typedef std::map<osg::Referenced*,unsigned int> ObjectsHandled;
    ObjectsHandled _objectsHandled;

    ActiveOperators::OperatorList& _operatorList;
};


ActiveOperators::ActiveOperators():
    _pause(false)
{
}

ActiveOperators::~ActiveOperators()
{
}

void ActiveOperators::collect(osg::Node* incomingNode, osg::NodeVisitor::TraversalMode tm)
{
    _previous.swap(_current);

    _current.clear();

    FindOperatorsVisitor fov(_current, tm);

    if (incomingNode)
    {
        incomingNode->accept(fov);
    }
    else
    {
        OSG_NOTICE<<"ActiveOperators::collect() incomingNode="<<incomingNode<<std::endl;
    }

    OSG_INFO<<"ActiveOperators::collect("<<incomingNode<<")"<<std::endl;
    OSG_INFO<<"  _previous.size()="<<_previous.size()<<std::endl;
    OSG_INFO<<"  _current.size()="<<_current.size()<<std::endl;

    _outgoing.clear();
    _incoming.clear();
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
        if (_previous.count(curr)==0) _incoming.insert(curr);
    }
}

void ActiveOperators::frame(SlideEventHandler* seh)
{
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        (*itr)->frame(seh);
    }
}

void ActiveOperators::setPause(SlideEventHandler* seh, bool pause)
{
    _pause = pause;
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        (*itr)->setPause(seh, _pause);
    }
}


void ActiveOperators::reset(SlideEventHandler* seh)
{
    for(OperatorList::iterator itr = _current.begin();
        itr != _current.end();
        ++itr)
    {
        (*itr)->reset(seh);
    }
}

void ActiveOperators::process(SlideEventHandler* seh)
{
    processOutgoing(seh);
    processMaintained(seh);
    processIncoming(seh);
}

void ActiveOperators::processOutgoing(SlideEventHandler* seh)
{
    OSG_INFO<<"  outgoing.size()="<<_outgoing.size()<<std::endl;
    for(OperatorList::iterator itr = _outgoing.begin();
        itr != _outgoing.end();
        ++itr)
    {
        (*itr)->leave(seh);
    }
}

void ActiveOperators::processMaintained(SlideEventHandler* seh)
{
    OSG_INFO<<"  maintained.size()="<<_maintained.size()<<std::endl;
    for(OperatorList::iterator itr = _maintained.begin();
        itr != _maintained.end();
        ++itr)
    {
        (*itr)->maintain(seh);
    }
}

void ActiveOperators::processIncoming(SlideEventHandler* seh)
{
    OSG_INFO<<"  incoming.size()="<<_incoming.size()<<std::endl;
    for(OperatorList::iterator itr = _incoming.begin();
        itr != _incoming.end();
        ++itr)
    {
        (*itr)->enter(seh);
        (*itr)->setPause(seh, _pause);
    }
}




class FindHomePositionVisitor : public osg::NodeVisitor
{
public:

    FindHomePositionVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN) {}

    void apply(osg::Node& node)
    {
        HomePosition* homePosition = dynamic_cast<HomePosition*>(node.getUserData());
        if (homePosition)
        {
            _homePosition = homePosition;
        }

        traverse(node);
    }

    osg::ref_ptr<HomePosition> _homePosition;

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
        FilePathData* fdd = dynamic_cast<FilePathData*>(node.getUserData());
        if (fdd)
        {
            OSG_INFO<<"Recorded FilePathData"<<std::endl;
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
        traverse(node);
    }

    void apply(osg::LightSource& lightsource)
    {
        if (lightsource.getLight())
        {
            if (lightsource.getReferenceFrame()==osg::LightSource::RELATIVE_RF)
            {
                apply( osg::Matrix::identity(), lightsource.getLight());
            }
            else
            {
                apply(osg::computeEyeToLocal(_viewMatrix,_nodePath), lightsource.getLight());
            }
        }

        traverse(lightsource);
    }

    void apply(const osg::Matrixd& matrix, osg::Light* light)
    {
        // compute direction of light based on a projecting onto a hemi-sphere.
        float sum_x2_y2 = _currentX*_currentX + _currentY*_currentY;
        osg::Vec3 direction;
        if (sum_x2_y2<1.0) direction.set(_currentX, _currentY, sqrtf(1.0-sum_x2_y2));
        else direction.set(_currentX, _currentY, 0.0);

        direction.normalize();

        direction = osg::Matrixd::transform3x3(matrix, direction);
        direction.normalize();

        light->setPosition(osg::Vec4(direction,0.0f));
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
                OSG_INFO<<"Adjusting alpha func"<<std::endl;

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
                OSG_INFO<<"Adjusting material func"<<std::endl;
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
    _activePresentation(0),
    _presentationSwitch(0),
    _activeSlide(0),
    _slideSwitch(0),
    _activeLayer(0),
    _firstTraversal(true),
    _referenceTime(-1.0),
    _previousTime(-1.0),
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
    _timeLastKeyPresses(-1.0),
    _requestReload(false)
{
    s_seh = this;
}

SlideEventHandler::SlideEventHandler(const SlideEventHandler& seh,const osg::CopyOp& copyop):
    osg::Object(seh, copyop),
    osg::Callback(seh, copyop),
    osgGA::GUIEventHandler(seh,copyop),
    _viewer(seh._viewer),
    _activePresentation(seh._activePresentation),
    _presentationSwitch(seh._presentationSwitch),
    _activeSlide(seh._activeSlide),
    _slideSwitch(seh._slideSwitch),
    _activeLayer(seh._activeLayer),
    _firstTraversal(true),
    _referenceTime(seh._referenceTime),
    _previousTime(seh._previousTime),
    _timePerSlide(seh._timePerSlide),
    _autoSteppingActive(seh._autoSteppingActive),
    _loopPresentation(seh._loopPresentation),
    _pause(seh._pause),
    _hold(seh._hold),
    _updateLightActive(seh._updateLightActive),
    _updateOpacityActive(seh._updateOpacityActive),
    _previousX(seh._previousX), _previousY(seh._previousY),
    _cursorOn(seh._cursorOn),
    _releaseAndCompileOnEachNewSlide(seh._releaseAndCompileOnEachNewSlide),
    _firstSlideOrLayerChange(seh._firstSlideOrLayerChange),
    _tickAtFirstSlideOrLayerChange(seh._tickAtFirstSlideOrLayerChange),
    _tickAtLastSlideOrLayerChange(seh._tickAtLastSlideOrLayerChange),
    _timeDelayOnNewSlideWithMovies(seh._timeDelayOnNewSlideWithMovies),
    _minimumTimeBetweenKeyPresses(seh._minimumTimeBetweenKeyPresses),
    _timeLastKeyPresses(seh._timeLastKeyPresses),
    _requestReload(false)
{
    s_seh = this;
}

double SlideEventHandler::getDuration(const osg::Node* node) const
{
    const LayerAttributes* la = dynamic_cast<const LayerAttributes*>(node->getUserData());
    return la ? la->_duration : -1.0;
}

void SlideEventHandler::set(osg::Node* model)
{
#if 0
    // pause all slides, then just re-enable the current slide.
    ActivityUpdateCallbacksVisitor aucv(ALL_OBJECTS, true);
    aucv.setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    model->accept(aucv);
#endif
    _firstSlideOrLayerChange = true;
    _tickAtFirstSlideOrLayerChange = 0;
    _tickAtLastSlideOrLayerChange = 0;
    _timeLastKeyPresses = -1;

    ActiveOperators operators;
    operators.collect(model, osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);
    operators.setPause(this, true);

    FindNamedSwitchVisitor findPresentation("Presentation");
    model->accept(findPresentation);

    std::string fullpath;
    model->getUserValue("fullpath", fullpath);
    if (!fullpath.empty()) setUserValue("fullpath", fullpath);

    if (findPresentation._switch)
    {
        OSG_INFO<<"Presentation '"<<model->getName()<<"'"<<std::endl;
        _presentationSwitch = findPresentation._switch;

        double duration = getDuration(_presentationSwitch.get());
        if (duration>=0.0)
        {
            OSG_INFO<<"Presentation time set to "<<duration<<std::endl;
            _timePerSlide = duration;
        }

        //selectSlide(0);
    }
    else
    {
        OSG_INFO<<"No presentation present in scene."<<std::endl;

        _presentationSwitch = 0;
        _activeSlide = 0;

        FindNamedSwitchVisitor findSlide("Slide");
        model->accept(findSlide);

        if (findSlide._switch)
        {
            OSG_INFO<<"Found presentation slide"<<findSlide._switch->getName()<<std::endl;

            _slideSwitch = findSlide._switch;
            //selectLayer(0);
        }
        else
        {
            OSG_INFO<<"No slides present in scene, unable to operate as a slideshow."<<std::endl;
        }

    }
}

double SlideEventHandler::getCurrentTimeDelayBetweenSlides() const
{
    if (_slideSwitch.valid())
    {
        double duration = -1.0;
        if (_activeLayer<static_cast<int>(_slideSwitch->getNumChildren()))
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

bool SlideEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{

    if (!_viewer)
    {
        _viewer = dynamic_cast<osgViewer::Viewer*>(&aa);
        selectSlide(0);
        home();
        OSG_NOTICE<<"Assigned viewer. to SlideEventHandler"<<std::endl;
    }
    //else  OSG_NOTICE<<"SlideEventHandler::handle() "<<ea.getTime()<<std::endl;

    if (ea.getHandled()) return false;

    _referenceTime = ea.getTime();

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
                        aa.requestRedraw();
                    }
                    else
                    {
                        // we're holding of the move to next layer to slide, but we need slip the time forward accordingly
                        // componensate for the extra time that this frame is receiving.
                        _previousTime = time-getCurrentTimeDelayBetweenSlides();
                    }
                }
            }
            _activeOperators.frame(this);

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

            if (ea.getKey()=='g')
            {
                if (!_autoSteppingActive)
                {
                    _autoSteppingActive = true;
                    _previousTime = ea.time();
                }
                return true;
            }
            else if (ea.getKey()=='h')
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
                    _activeOperators.setPause(this, _pause);
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
                    _activeOperators.setPause(this, _pause);
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
                _activeOperators.reset(this);
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
                updateAlpha(true,false,ea.getXnormalized(),ea.getYnormalized());
                return true;
            }
            else if (ea.getKey()=='i')
            {
                updateAlpha(false,true,ea.getXnormalized(),ea.getYnormalized());
                return true;
            }
            else if (ea.getKey()=='k')
            {
                updateLight(ea.getXnormalized(),ea.getYnormalized());
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
            else if (ea.getKey()=='R')
            {
                // reload presentation to reflect changes from editor
                setRequestReload(true);
                return true;
            }
            else if (ea.getKey()=='E')
            {
                char* editor = getenv("P3D_EDITOR");
                if (!editor) editor = getenv("EDITOR");

                std::string filename;
                if (editor && getUserValue("fullpath", filename) && !filename.empty())
                {
                    std::stringstream command;
                    command<<editor<<" "<<filename<<" &"<<std::endl;

                    int result = osg_system(command.str().c_str());

                    OSG_INFO<<"system("<<command.str()<<") result "<<result<<std::endl;

                }
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

osg::Switch* SlideEventHandler::getSlide(int slideNum)
{
    if (slideNum<0 || slideNum>static_cast<int>(_presentationSwitch->getNumChildren())) return 0;

    FindNamedSwitchVisitor findSlide("Slide");
    _presentationSwitch->getChild(slideNum)->accept(findSlide);
    return findSlide._switch;
}

osg::Node* SlideEventHandler::getLayer(int slideNum, int layerNum)
{
    osg::Switch* slide = getSlide(slideNum);
    return (slide && (layerNum>=0 && layerNum<static_cast<int>(slide->getNumChildren()))) ? slide->getChild(layerNum) : 0;
}


bool SlideEventHandler::selectSlide(int slideNum,int layerNum)
{
    if (!_presentationSwitch || _presentationSwitch->getNumChildren()==0) return false;

    OSG_INFO<<"selectSlide("<<slideNum<<","<<layerNum<<")"<<std::endl;

    if (slideNum<0 || slideNum>=static_cast<int>(_presentationSwitch->getNumChildren()))
    {
        slideNum = _presentationSwitch->getNumChildren()-1;
    }

    osg::Timer_t tick = osg::Timer::instance()->tick();

    if (_firstSlideOrLayerChange)
    {
        _firstSlideOrLayerChange = false;
        _tickAtFirstSlideOrLayerChange = tick;
        _tickAtLastSlideOrLayerChange = tick;
    }

    OSG_INFO<<"selectSlide("<<slideNum<<","<<layerNum<<") at time "<<osg::Timer::instance()->delta_s(_tickAtFirstSlideOrLayerChange, tick)<<" seconds, length ="<<osg::Timer::instance()->delta_s(_tickAtLastSlideOrLayerChange, tick)<<" seconds"<<std::endl;

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

    //OSG_INFO<<"Selected slide '"<<_presentationSwitch->getChild(_activeSlide)->getName()<<"'"<<std::endl;


    FindNamedSwitchVisitor findSlide("Slide");
    _presentationSwitch->getChild(_activeSlide)->accept(findSlide);

    bool result = false;
    if (findSlide._switch)
    {
        //OSG_INFO<<"Found slide '"<<findSlide._switch->getName()<<"'"<<std::endl;
        _slideSwitch = findSlide._switch;

        result = selectLayer(layerNum);


    }
    else
    {
        //OSG_INFO<<"Not found slide"<<std::endl;
        updateOperators();
    }


    // refersh the viewer.
    //_viewer->getKeySwitchMatrixManipulator()->setMinimumDistance(0.001);

    if (_viewer->getCameraManipulator())
    {
        _viewer->getCameraManipulator()->setNode(_slideSwitch.get());

        _viewer->computeActiveCoordinateSystemNodePath();
    }

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

bool SlideEventHandler::selectLayer(int layerNum)
{
    if (!_slideSwitch || _slideSwitch->getNumChildren()==0) return false;

    bool withinSlide = true;

    if (layerNum>=static_cast<int>(_slideSwitch->getNumChildren()))
    {
        withinSlide = false;
        layerNum = LAST_POSITION;
    }

    if (layerNum<0)
    {
        layerNum = _slideSwitch->getNumChildren()-1;
    }

    if (layerNum>=static_cast<int>(_slideSwitch->getNumChildren())) return false;

    _activeLayer = layerNum;
    _slideSwitch->setSingleChildOn(_activeLayer);

    updateOperators();

    OSG_INFO<<"Selected layer '"<<_slideSwitch->getChild(_activeLayer)->getName()<<"' num="<<_activeLayer<< std::endl;

    return withinSlide;
}

bool SlideEventHandler::nextLayerOrSlide()
{
    if (nextLayer())
    {
        return true;
    }
    else
    {
        return nextSlide();
    }
}

bool SlideEventHandler::previousLayerOrSlide()
{
    OSG_INFO<<"previousLayerOrSlide()"<<std::endl;
    if (previousLayer()) return true;
    else return previousSlide();
}

bool SlideEventHandler::nextSlide()
{
    OSG_INFO<<"nextSlide()"<<std::endl;
    LayerAttributes* la = _slideSwitch.valid() ? dynamic_cast<LayerAttributes*>(_slideSwitch->getUserData()) : 0;
    if (la && la->getJumpData().requiresJump())
    {
        return la->getJumpData().jump(this);
    }

    if (selectSlide(_activeSlide+1)) return true;
    else if (_loopPresentation) return selectSlide(0);
    else return false;
}

bool SlideEventHandler::previousSlide()
{
    OSG_INFO<<"previousSlide()"<<std::endl;
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
    LayerAttributes* la = (_slideSwitch.valid() && _activeLayer<static_cast<int>(_slideSwitch->getNumChildren())) ? dynamic_cast<LayerAttributes*>(_slideSwitch->getChild(_activeLayer)->getUserData()) : 0;
    if (la)
    {
        la->callLeaveCallbacks(_slideSwitch->getChild(_activeLayer));

        if (la->getJumpData().requiresJump())
        {
            return la->getJumpData().jump(this);
        }
    }

    OSG_INFO<<"nextLayer() calling selectLayer("<<_activeLayer+1<<")"<<std::endl;
    return selectLayer(_activeLayer+1);
}

bool SlideEventHandler::previousLayer()
{
    OSG_INFO<<"previousLayer()"<<std::endl;
    if (_activeLayer>0) return selectLayer(_activeLayer-1);
    else return false;
}


void SlideEventHandler::updateOperators()
{
    _activeOperators.collect(_slideSwitch.get());
    _activeOperators.process(this);

    if (_viewer.valid())
    {
        updateLight(0.0f,0.0f);
    }
}

bool SlideEventHandler::home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    FindHomePositionVisitor fhpv;
    osg::Node* node = _viewer->getSceneData();
    if (node) node->accept(fhpv);

    if (_viewer->getCameraManipulator())
    {
        if (fhpv._homePosition.valid())
        {
            OSG_INFO<<"Doing home for stored home position."<<std::endl;

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
    }

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
    OSG_INFO<<"updateAlpha("<<x<<","<<y<<")"<<std::endl;

    UpdateAlphaVisitor uav(modAlphaFunc, modMaterial, x,y);
    if (_presentationSwitch.valid()) _presentationSwitch->accept(uav);
    else if (_viewer->getSceneData()) _viewer->getSceneData()->accept(uav);
}


void SlideEventHandler::updateLight(float x, float y)
{
    OSG_INFO<<"updateLight("<<x<<", "<<y<<")"<<std::endl;

    UpdateLightVisitor uav(_viewer->getCamera()->getViewMatrix(),x,y);
    _viewer->getSceneData()->accept(uav);

    if (_viewer->getLightingMode()!= osg::View::NO_LIGHT && _viewer->getLight())
    {
        if (_viewer->getLightingMode()== osg::View::SKY_LIGHT)
        {
            uav.apply(_viewer->getCamera()->getViewMatrix(), _viewer->getLight());
        }
        else if (_viewer->getLightingMode()== osg::View::HEADLIGHT)
        {
            uav.apply(osg::Matrix::identity(), _viewer->getLight());
        }
    }

}

void SlideEventHandler::compileSlide(unsigned int slideNum)
{
    if (!_compileSlideCallback)
    {
        _compileSlideCallback = new CompileSlideCallback();

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

void SlideEventHandler::forwardEventToDevices(osgGA::Event* event)
{
    if (!event) return;

    // dispatch cloned event to devices
    if (!_viewer)
    {
        OSG_NOTICE<<"Warning: SlideEventHandler::forwardEventToDevices(Event*) error, no Viewer to dispatch to."<<std::endl;
        return;
    }

    osgViewer::View::Devices& devices = _viewer->getDevices();
    for(osgViewer::View::Devices::iterator i = devices.begin(); i != devices.end(); ++i)
    {
        if((*i)->getCapabilities() & osgGA::Device::SEND_EVENTS)
        {
            (*i)->sendEvent(*event);
        }
    }
}

void SlideEventHandler::dispatchEvent(osgGA::Event* event)
{
    if (!event) return;

    // dispatch cloned event to devices
    if (!_viewer)
    {
        OSG_NOTICE<<"Warning: SlideEventHandler::forwardEventToDevices(Event*) error, no Viewer to dispatch to."<<std::endl;
        return;
    }

    osgGA::EventQueue* eq = _viewer!=0 ? _viewer->getEventQueue() : 0;
    if (!eq)
    {
        OSG_NOTICE<<"Warning: SlideEventHandler::dispatchEvent(KeyPosition&) error, no EventQueue to dispatch to."<<std::endl;
        return;
    }

    eq->addEvent(event);
}

void SlideEventHandler::dispatchEvent(const KeyPosition& keyPosition)
{
    if (!_viewer)
    {
        OSG_NOTICE<<"Warning: SlideEventHandler::dispatchEvent(KeyPosition*) error, no Viewer to dispatch to."<<std::endl;
        return;
    }

    if (keyPosition._forwardToDevices)
    {
        osg::ref_ptr<osgGA::GUIEventAdapter> event = new osgGA::GUIEventAdapter();
        event->setKey(keyPosition._key);
        event->setTime(_viewer->getEventQueue()->getTime());

        // forward key-down
        event->setEventType(osgGA::GUIEventAdapter::KEYDOWN);
        forwardEventToDevices(event.get());

        // forward key-up
        event->setEventType(osgGA::GUIEventAdapter::KEYUP);
        forwardEventToDevices(event.get());

        // ignore local event-queue
        return;
    }

    osgGA::EventQueue* eq = _viewer!=0 ? _viewer->getEventQueue() : 0;
    if (!eq)
    {
        OSG_NOTICE<<"Warning: SlideEventHandler::dispatchEvent(KeyPosition&) error, no EventQueue to dispatch to."<<std::endl;
        return;
    }

    // reset the time of the last key press to ensure that the event is disgarded as a key repeat.
    _timeLastKeyPresses = -1.0;

    if (keyPosition._x!=FLT_MAX)
    {
        float xRescaled = eq->getCurrentEventState()->getXmin() + (keyPosition._x+1.0f)*0.5f*(eq->getCurrentEventState()->getXmax()-eq->getCurrentEventState()->getXmin());
        eq->getCurrentEventState()->setX(xRescaled);
    }

    if (keyPosition._y!=FLT_MAX)
    {
        float y = (eq->getCurrentEventState()->getMouseYOrientation()==osgGA::GUIEventAdapter::Y_INCREASING_UPWARDS) ?
                   keyPosition._y : -keyPosition._y;

        float yRescaled = eq->getCurrentEventState()->getYmin() + (y+1.0f)*0.5f*(eq->getCurrentEventState()->getYmax()-eq->getCurrentEventState()->getYmin());
        eq->getCurrentEventState()->setY(yRescaled);
    }

    eq->keyPress(keyPosition._key);
    eq->keyRelease(keyPosition._key);
}


void SlideEventHandler::setRequestReload(bool flag)
{
    _requestReload = flag;
}

bool SlideEventHandler::checkNeedToDoFrame()
{
    if (_viewer.valid())
    {
        if (_viewer->getRequestRedraw()) return true;
        if (_viewer->getRequestContinousUpdate()) return true;

        // check if the database pager needs to update the scene
        if (_viewer->getDatabasePager()->requiresUpdateSceneGraph()) return true;

        // check if the image pager needs to update the scene
        if (_viewer->getImagePager()->requiresUpdateSceneGraph()) return true;

        // if there update callbacks then we need to do frame.
        if (_viewer->getCamera()->getUpdateCallback()) return true;

        if (!_pause)
        {
            if (_slideSwitch.valid() && _activeLayer<static_cast<int>(_slideSwitch->getNumChildren()))
            {
                if (_slideSwitch->getChild(_activeLayer)->getNumChildrenRequiringUpdateTraversal()>0) return true;
            }
            else if (_viewer->getSceneData()!=0 && (_viewer->getSceneData()->getUpdateCallback() || (_viewer->getSceneData()->getNumChildrenRequiringUpdateTraversal()>0))) return true;

            if (_autoSteppingActive)
            {
                if (_firstTraversal) return true;
                else
                {
                    osg::Timer_t tick = osg::Timer::instance()->tick();
                    double currentTime = osg::Timer::instance()->delta_s(_viewer->getStartTick(), tick);
                    if ((currentTime-_previousTime)>=getCurrentTimeDelayBetweenSlides()) return true;
                }
            }
        }

        // check if events are available and need processing
        if (_viewer->checkEvents()) return true;

        // now check if any of the event handles have prompted a redraw.
        if (_viewer->getRequestRedraw()) return true;
        if (_viewer->getRequestContinousUpdate()) return true;

        return false;
    }
    else
    {
        return true;
    }
}
