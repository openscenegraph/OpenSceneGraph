/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the GNU Public License (GPL) version 1.0 or 
 * (at your option) any later version. 
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/


#ifndef SLIDEEVENTHANDLER
#define SLIDEEVENTHANDLER 1

#include <osg/Switch>

#include <osgGA/GUIEventHandler>

class SlideEventHandler : public osgGA::GUIEventHandler
{
public:

    SlideEventHandler();
    
    META_Object(osgslideshowApp,SlideEventHandler);

    void set(osg::Node* model);

    virtual void accept(osgGA::GUIEventHandlerVisitor& v) { v.visit(*this); }

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
    
    virtual void getUsage(osg::ApplicationUsage& usage) const;

    enum WhichPosition
    {
        FIRST_POSITION = 0,
        LAST_POSITION = 0xffffffff,
    };

    bool selectSlide(unsigned int slideNum,unsigned int layerNum=FIRST_POSITION);
    bool selectLayer(unsigned int layerNum);

    bool nextLayerOrSlide();
    bool previousLayerOrSlide();

    bool nextSlide();
    bool previousSlide();

    bool nextLayer();
    bool previousLayer();
    
    void setAutoSteppingActive(bool flag) { _autoSteppingActive = true; }
    bool getAutoSteppingActive() const { return _autoSteppingActive; }
    
    void setTimeDelayBetweenSlides(double dt) { _timePerSlide = dt; }
    double getTimeDelayBetweenSlides() const { return _timePerSlide; }
    
protected:

    ~SlideEventHandler() {}
    SlideEventHandler(const SlideEventHandler&,const osg::CopyOp&) {}

    osg::ref_ptr<osg::Switch>   _presentationSwitch;
    unsigned int                _activeSlide;

    osg::ref_ptr<osg::Switch>   _slideSwitch;
    unsigned int                _activeLayer;

    bool                        _firstTraversal;
    double                      _previousTime;
    double                      _timePerSlide;
    bool                        _autoSteppingActive;
    bool                        _loopPresentation;
    bool                        _pause;
        
};

#endif
