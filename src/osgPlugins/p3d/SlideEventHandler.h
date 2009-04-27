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

#ifndef SLIDEEVENTHANDLER
#define SLIDEEVENTHANDLER 1

#include <osg/Switch>
#include <osg/Timer>

#include <osgGA/GUIEventHandler>
#include <osgViewer/Viewer>

#include "CompileSlideCallback.h"
#include "SlideShowConstructor.h"


struct dereference_less
{
    template<class T, class U>
    inline bool operator() (const T& lhs,const U& rhs) const
    {
        return *lhs < *rhs;
    }
};

struct ObjectOperator : public osg::Referenced
{
    inline bool operator < (const ObjectOperator& rhs) const { return ptr() < rhs.ptr(); }

    virtual void* ptr() const = 0;

    virtual void enter() = 0;
    virtual void maintain() = 0;
    virtual void leave() = 0;
    virtual void setPause(bool pause) = 0;
    virtual void reset() = 0;

    virtual ~ObjectOperator() {}
};

class ActiveOperators
{
public:
    ActiveOperators();
    ~ActiveOperators();
    
    void collect(osg::Node* incommingNode, osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    void process();
    
    void setPause(bool pause);
    bool getPause() const { return _pause; }
    
    void reset();

    typedef std::set< osg::ref_ptr<ObjectOperator>, dereference_less >  OperatorList;

protected:

    void processOutgoing();
    void processIncomming();
    void processMaintained();

    bool            _pause;

    OperatorList    _previous;
    OperatorList    _current;

    OperatorList    _outgoing;
    OperatorList    _incomming;
    OperatorList    _maintained;

};

class SlideEventHandler : public osgGA::GUIEventHandler
{
public:

    SlideEventHandler(osgViewer::Viewer* viewer=0);
    
    static SlideEventHandler* instance();
    
    META_Object(osgslideshowApp,SlideEventHandler);

    void set(osg::Node* model);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;
    
    osgViewer::Viewer* getViewer() { return _viewer.get(); }

    enum WhichPosition
    {
        FIRST_POSITION = 0,
        LAST_POSITION = 0xffffffff
    };

    void compileSlide(unsigned int slideNum);
    void releaseSlide(unsigned int slideNum);

    unsigned int getNumSlides();
    
    unsigned int getActiveSlide() const { return _activeSlide; }
    unsigned int getActiveLayer() const { return _activeLayer; }

    bool selectSlide(unsigned int slideNum,unsigned int layerNum=FIRST_POSITION);
    bool selectLayer(unsigned int layerNum);

    bool nextLayerOrSlide();
    bool previousLayerOrSlide();

    bool nextSlide();
    bool previousSlide();

    bool nextLayer();
    bool previousLayer();
    
    bool home();

    void setAutoSteppingActive(bool flag = true) { _autoSteppingActive = flag; }
    bool getAutoSteppingActive() const { return _autoSteppingActive; }
    
    void setTimeDelayBetweenSlides(double dt) { _timePerSlide = dt; }
    double getTimeDelayBetweenSlides() const { return _timePerSlide; }
    
    double getDuration(const osg::Node* node) const;

    double getCurrentTimeDelayBetweenSlides() const;

    void setReleaseAndCompileOnEachNewSlide(bool flag) { _releaseAndCompileOnEachNewSlide = flag; }
    bool getReleaseAndCompileOnEachNewSlide() const { return _releaseAndCompileOnEachNewSlide; }

    void setTimeDelayOnNewSlideWithMovies(float t) { _timeDelayOnNewSlideWithMovies = t; }
    float getTimeDelayOnNewSlideWithMovies() const { return _timeDelayOnNewSlideWithMovies; }

    void setLoopPresentation(bool loop) { _loopPresentation = loop; }
    bool getLoopPresentation() const { return _loopPresentation; }
    
    void dispatchEvent(const SlideShowConstructor::KeyPosition& keyPosition);

    enum ObjectMask
    {
        MOVIE = 1<<0,
        OBJECTS = 1<<1,
        ALL_OBJECTS = MOVIE | OBJECTS
    };

protected:

    ~SlideEventHandler() {}
    SlideEventHandler(const SlideEventHandler&,const osg::CopyOp&) {}

    bool home(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    void updateAlpha(bool, bool, float x, float y);
    void updateLight(float x, float y);
    

    osg::observer_ptr<osgViewer::Viewer>    _viewer;
    
    osg::observer_ptr<osg::Switch>          _showSwitch;
    unsigned int                            _activePresentation;
    
    osg::observer_ptr<osg::Switch>          _presentationSwitch;
    unsigned int                            _activeSlide;

    osg::observer_ptr<osg::Switch>          _slideSwitch;
    unsigned int                            _activeLayer;

    bool                                    _firstTraversal;
    double                                  _previousTime;
    double                                  _timePerSlide;
    bool                                    _autoSteppingActive;
    bool                                    _loopPresentation;
    bool                                    _pause;
    bool                                    _hold;
    
    bool                                    _updateLightActive;
    bool                                    _updateOpacityActive;
    float                                   _previousX, _previousY;
    
    bool                                    _cursorOn;

    bool                                    _releaseAndCompileOnEachNewSlide;

    bool                                    _firstSlideOrLayerChange;
    osg::Timer_t                            _tickAtFirstSlideOrLayerChange;
    osg::Timer_t                            _tickAtLastSlideOrLayerChange;

    float                                   _timeDelayOnNewSlideWithMovies;
    
    double                                  _minimumTimeBetweenKeyPresses;
    double                                  _timeLastKeyPresses;
    
    ActiveOperators                         _activeOperators;
    
    osg::ref_ptr<ss3d::CompileSlideCallback>  _compileSlideCallback;

    void updateOperators();
        
};

#endif
