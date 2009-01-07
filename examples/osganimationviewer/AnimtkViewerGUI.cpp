/*  -*-c++-*- 
 *  Copyright (C) 2008 Cedric Pinson <mornifle@plopbyte.net>
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
 *
 * Authors:
 * Cedric Pinson <mornifle@plopbyte.net>
 * jeremy Moles <jeremy@emperorlinux.com>
*/

#include "AnimtkViewer"
#include "AnimtkViewerGUI"

#include <osg/Version>
#include <osgWidget/WindowManager>
#include <osgAnimation/EaseMotion>

const std::string IMAGE_PATH = "osgWidget/";

template <class T>
struct Sampler: public osg::Drawable::UpdateCallback 
{
    T _motion;
    Sampler() {
    }
};

typedef Sampler<osgAnimation::OutQuadMotion> WidgetSampler;

struct ButtonFunctor: public WidgetSampler 
{
    float _direction;
    float _previous;

    const float _speed;
    
    ButtonFunctor(): _speed(5) { _direction = -_speed; _previous = 0;}

    bool enter(osgWidget::Event&)
    {
        _direction = _speed; 
        return true;
    }

    bool leave(osgWidget::Event&)
    {
        _direction = -_speed; 
        return true;
    }

    void update(osg::NodeVisitor* nv , osg::Drawable* geom) 
    {
        const osg::FrameStamp* f = nv->getFrameStamp();
        float dt = f->getSimulationTime() - _previous;
        _previous = f->getSimulationTime();
        update(dt,dynamic_cast<osgWidget::Widget*>(geom));
    }

    void update(float t, osgWidget::Widget* w)
    {
        if (!w) return;
        _motion.update(t*_direction); 
        float val = _motion.getValue()*0.5;
        val += 0.5;
        if (val >= 1.0)
            val = 1.0;
        w->setColor(osg::Vec4(val, val, val, 1));
    }
};

struct LabelFunctor: public WidgetSampler 
{
    float _previous;
    bool  _active;

    const float _fadeOutTime;

    osgAnimation::OutCubicMotion _scaleSampler;

    LabelFunctor():
        _fadeOutTime(1.5f) 
{
        _previous = 0.0f;
        _active   = false;

        _scaleSampler = osgAnimation::OutCubicMotion(0.5, 1.0, 1.0);
    }

    void setActive(bool active) 
{
        _active = active;

        if(active) _motion.reset();

        _scaleSampler.reset();
    }

    void update(osg::NodeVisitor* nv , osg::Drawable* geom) 
{
        const osg::FrameStamp* f = nv->getFrameStamp();

        float st = f->getSimulationTime();
        float dt = st - _previous;

        _previous = st;

        if(!_active) return;

        update(dt, dynamic_cast<osgWidget::Label*>(geom));
        updateScale(dt, dynamic_cast<osgWidget::Label*>(geom));
    }

    void update(float t, osgWidget::Label* w) 
{
        if(!w) return;

        _motion.update(t / _fadeOutTime);

        float val = _motion.getValue();

        if(val >= 1.0f) {
            _motion.reset();
            _active = false;
        }

        w->setFontColor(osg::Vec4(0.0f, 0.0f, 0.0f, (1.0f - val) * 0.7f));
    }

    void updateScale(float t, osgWidget::Label* w) 
{
        _scaleSampler.update(t);
        float val = _scaleSampler.getValue();
        osgWidget::Window* win = w->getParent();
        win->setScale(val);
        win->update();
    }

};


struct ListFunctor: public osg::NodeCallback 
{
    float _previous;
    int   _direction;

    osgAnimation::InQuadMotion _transformSampler;

    ListFunctor() 
{
        _direction = 1;
        _previous  = 0;

        _transformSampler.update(1.0f);
    }

    void toggleShown() 
{
        if(_direction == 1) _direction = -1;

        else _direction = 1;
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) 
{
        const osg::FrameStamp* f = nv->getFrameStamp();

        float st = f->getSimulationTime();
        float dt = st - _previous;

        _previous = st;

        _transformSampler.update((dt * _direction) / 0.5f);

        float val = _transformSampler.getValue();

        if(val > 1.0f || val < 0.0f) return;

        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(node);

        float w   = win->getWidth();
        float wmw = win->getWindowManager()->getWidth();

        win->setX((wmw - w) + (val * w));
        win->update();
    }

};

// This is a temporary hack to "prevent" dragging on Widgets and Windows.
bool eatDrag(osgWidget::Event&) 
{
    return true;
}

AnimtkViewerGUI::AnimtkViewerGUI(osgViewer::View* view, float w, float h, unsigned int mask):
    osgWidget::WindowManager(view, w, h, mask, 0) 
{
    _createButtonBox();
    _createLabelBox();
    _createListBox();

    _labelBox->setAnchorHorizontal(osgWidget::Window::HA_LEFT);
    _labelBox->setY(74.0f);
    _labelBox->setVisibilityMode(osgWidget::Window::VM_ENTIRE);

    _listBox->setOrigin(getWidth(), 74.0f);

    addChild(_buttonBox.get());
    addChild(_labelBox.get());
    addChild(_listBox.get());

    resizeAllWindows();

    // Remember, you can't call resizePercent until AFTER the box is parented
    // by a WindowManager; how could it possibly resize itself if it doesn't know
    // how large it's viewable area is?
    _buttonBox->resizePercent(100.0f);
    _buttonBox->resizeAdd(0.0f, 10.0f);
}

osgWidget::Widget* AnimtkViewerGUI::_createButton(const std::string& name) 
{
    osgWidget::Widget* b = new osgWidget::Widget(name, 64.0f, 64.0f);
    
    if(!b) return 0;

    b->setImage(IMAGE_PATH + name + ".png", true);
    b->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);

    ButtonFunctor* bt = new ButtonFunctor();
    b->setUpdateCallback(bt);
    
    b->addCallback(new osgWidget::Callback(&ButtonFunctor::enter, bt, osgWidget::EVENT_MOUSE_ENTER));
    b->addCallback(new osgWidget::Callback(&ButtonFunctor::leave, bt, osgWidget::EVENT_MOUSE_LEAVE));
    b->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_buttonPush, this, osgWidget::EVENT_MOUSE_PUSH));
    b->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));

    return b;
}

bool AnimtkViewerGUI::_listMouseHover(osgWidget::Event& ev) 
{
    osgWidget::Label* l = dynamic_cast<osgWidget::Label*>(ev.getWidget());

    if(!l) return false;

    if(ev.type == osgWidget::EVENT_MOUSE_ENTER) l->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);

    else if(ev.type == osgWidget::EVENT_MOUSE_LEAVE) l->setFontColor(1.0f, 1.0f, 1.0f, 0.3f);

    else if(ev.type == osgWidget::EVENT_MOUSE_PUSH) {
        AnimtkViewerModelController::instance().playByName(ev.getWidget()->getName());
    }
    
    else return false;

    return true;
}

bool AnimtkViewerGUI::_buttonPush(osgWidget::Event& ev) 
{
    if(!ev.getWidget()) return false;

    osgWidget::Label* l = static_cast<osgWidget::Label*>(_labelBox->getByName("label"));

    if(!l) return false;

    LabelFunctor* lf = dynamic_cast<LabelFunctor*>(l->getUpdateCallback());

    if(!lf) return false;

    // We're safe at this point, so begin processing.
    AnimtkViewerModelController& mc   = AnimtkViewerModelController::instance();
    std::string                  name = ev.getWidget()->getName();

    if(name == "play") mc.play();

    else if(name == "stop") mc.stop();

    else if(name == "next") 
{
        mc.next();

        l->setFontColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.7f));
        l->setLabel(mc.getCurrentAnimationName());
        lf->setActive(true);
    }
    
    else if(name == "back") 
{
        mc.previous();
        
        l->setFontColor(osg::Vec4(0.0f, 0.0f, 0.0f, 0.7f));
        l->setLabel(mc.getCurrentAnimationName());
        lf->setActive(true);
    }

    else if(name == "pause") 
{
    }

    else if(name == "open") 
{
        ListFunctor* lsf = dynamic_cast<ListFunctor*>(_listBox->getUpdateCallback());

        if(!lsf) return false;

        lsf->toggleShown();
    }

    else return false;

    return true;
}

void AnimtkViewerGUI::_createButtonBox() 
{
    _buttonBox = new osgWidget::Box("buttonBox", osgWidget::Box::HORIZONTAL);

    osgWidget::Widget* space = new osgWidget::Widget("nullSpace", 0.0f, 0.0f);
    osgWidget::Widget* back  = _createButton("back");
    osgWidget::Widget* next  = _createButton("next");
    osgWidget::Widget* play  = _createButton("play");
    osgWidget::Widget* pause = _createButton("pause");
    osgWidget::Widget* stop  = _createButton("stop");
    osgWidget::Widget* open  = _createButton("open");

    space->setCanFill(true);
    space->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    _buttonBox->addWidget(space);
    _buttonBox->addWidget(back);
    _buttonBox->addWidget(next);
    _buttonBox->addWidget(play);
    _buttonBox->addWidget(pause);
    _buttonBox->addWidget(stop);
    _buttonBox->addWidget(open);
    _buttonBox->addWidget(osg::clone(space, "space1", osg::CopyOp::DEEP_COPY_ALL));
    _buttonBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.7f);
    
    _buttonBox->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
    _buttonBox->addCallback(new osgWidget::Callback(&eatDrag, osgWidget::EVENT_MOUSE_DRAG));
}

void AnimtkViewerGUI::_createListBox() 
{
    _listBox = new osgWidget::Box("listBox", osgWidget::Box::VERTICAL);

    const AnimtkViewerModelController::AnimationMapVector& amv =
        AnimtkViewerModelController::instance().getAnimationMap()
        ;

    for(
        AnimtkViewerModelController::AnimationMapVector::const_iterator i = amv.begin();
        i != amv.end();
        i++
        ) 
{
        osgWidget::Label* label = new osgWidget::Label(*i);

        label->setCanFill(true);
        label->setFont("fonts/Vera.ttf");
        label->setFontSize(15);
        label->setFontColor(1.0f, 1.0f, 1.0f, 0.3f);
        label->setPadding(5.0f);
        label->setAlignHorizontal(osgWidget::Widget::HA_RIGHT);
        label->setLabel(*i);
        label->setEventMask(osgWidget::EVENT_MASK_MOUSE_DRAG);
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_ENTER));
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_LEAVE));
        label->addCallback(new osgWidget::Callback(&AnimtkViewerGUI::_listMouseHover, this, osgWidget::EVENT_MOUSE_PUSH));

        _listBox->addWidget(label);
    }

    ListFunctor* lf = new ListFunctor();

    _listBox->setUpdateCallback(lf);
    _listBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.7f);
}

void AnimtkViewerGUI::_createLabelBox() 
{
    _labelBox = new osgWidget::Box("labelBox", osgWidget::Box::VERTICAL);

    osgWidget::Label* label = new osgWidget::Label("label");
    
    label->setFont("fonts/Vera.ttf");
    label->setFontSize(50);
    label->setFontColor(0.0f, 0.0f, 0.0f, 0.7f);
    label->setAlignHorizontal(osgWidget::Widget::HA_LEFT);
    label->setPadding(10.0f);

    LabelFunctor* lf = new LabelFunctor();
    label->setUpdateCallback(lf);

    _labelBox->addWidget(label);
    _labelBox->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
}
