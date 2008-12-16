// -*-c++-*- osgWidget - Copyright Cedric Pinson 2008


#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Frame>
#include <osgWidget/Box>
#include <osgWidget/Widget>
#include <osgWidget/Types>
#include <osgDB/ReadFile>
#include <osgAnimation/EaseMotion>
#include <osg/io_utils>
#include <iostream>

const unsigned int MASK_2D = 0xF0000000;

class MessageBox
{
protected:
        
    osgWidget::Frame* createButtonOk(const std::string& theme, const std::string& text, const std::string& font, int fontSize);
    osgWidget::Label* createLabel(const std::string& string, const std::string& font, int size, const osgWidget::Color& color);
        
    osg::ref_ptr<osgWidget::Frame> _window;
    osg::ref_ptr<osgWidget::Frame> _button;

public:
        
    osgWidget::Frame* getButton();
    osgWidget::Frame* getWindow();

    bool create(const std::string& themeMessage, 
                const std::string& themeButton,
                const std::string& titleText,
                const std::string& messageText,
                const std::string& buttonText,
                const std::string& font,
                int fontSize);

};
osgWidget::Frame* MessageBox::getButton() { return _button.get(); }
osgWidget::Frame* MessageBox::getWindow() { return _window.get(); }

struct AlphaSetterVisitor : public osg::NodeVisitor
{
    float _alpha;
    AlphaSetterVisitor( float alpha = 1.0):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { _alpha = alpha;}

    void apply(osg::MatrixTransform& node)
    {
        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(&node);

        if (win) {
//             osgWidget::warn() << "I am in Window: " << win->getName() << std::endl;

            for (osgWidget::Window::Iterator it = win->begin(); it != win->end(); it++)
            {
//                 osgWidget::warn() << "   I am operating on Widget: " << it->get()->getName() << std::endl;
                
                osgWidget::Color color = it->get()->getColor();
                color[3] = color[3] *_alpha;
                it->get()->setColor(color);
            }
            {
                osgWidget::Color color = win->getBackground()->getColor();
                color[3] = color[3] *_alpha;
                win->getBackground()->setColor(color);
            }
        }
        traverse(node);
    }
};


struct ColorSetterVisitor : public osg::NodeVisitor
{
    osgWidget::Color _color;
    ColorSetterVisitor( const osgWidget::Color& color):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { _color = color;}

    void apply(osg::MatrixTransform& node)
    {
        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(&node);

        if (win) {
//            osgWidget::warn() << "I am in Window: " << win->getName() << std::endl;

            for (osgWidget::Window::Iterator it = win->begin(); it != win->end(); it++)
            {
//                osgWidget::warn() << "   I am operating on Widget: " << it->get()->getName() << std::endl;
                
//                 osgWidget::Color color = it->get()->getColor();
//                 color[3] = color[3] *_alpha;
                it->get()->setColor(_color);
            }
            {
//                 osgWidget::Color color = win->getBackground()->getColor();
//                 color[3] = color[3] *_alpha;
                win->getBackground()->setColor(osgWidget::Color(0,0,0,0));
            }
        }
        traverse(node);
    }
};



struct EventOK : public osgWidget::Callback, osg::NodeCallback
{
    typedef osgAnimation::OutCubicMotion WidgetMotion;
//    typedef osgAnimation::OutQuartMotion WidgetMotion;
    WidgetMotion _motionOver;
    WidgetMotion _motionLeave;
    
    double _lastUpdate;
    osgWidget::Color _defaultColor;
    osgWidget::Color _overColor;
    bool _over;
    osg::ref_ptr<osgWidget::Frame> _frame;
    float _width;
    float _height;
    osg::Matrix _matrix;
    EventOK(osgWidget::Frame* frame) : osgWidget::Callback(osgWidget::EVENT_ALL), _frame(frame) 
    {
        _motionOver = WidgetMotion(0.0, 0.4);
        _motionLeave = WidgetMotion(0.0, 0.5);
        _defaultColor = _frame->getEmbeddedWindow()->getColor();
        _overColor = osgWidget::Color(229.0/255.0,
                                      103.0/255.0,
                                      17.0/255,
                                      _defaultColor[3]);
        _over  = false;
    }

    bool operator()(osgWidget::Event& ev)
    {
        if (ev.type == osgWidget::EVENT_MOUSE_ENTER)
        {
            _over = true;
            _width = _frame->getWidth();
            _height = _frame->getHeight();
            _motionOver.reset();
            _matrix = _frame->getMatrix();
            //_frame->setMatrix(osg::Matrix::scale(2, 2, 1) * _frame->getMatrix());
            _frame->setScale(1.1f); //osg::Matrix::scale(2, 2, 1) * _frame->getMatrix());
            _frame->update(); //osg::Matrix::scale(2, 2, 1) * _frame->getMatrix());
            std::cout << "enter" << std::endl;
            return true;
        }
        else if (ev.type == osgWidget::EVENT_MOUSE_LEAVE)
        {
            _over = false;
            _motionLeave.reset();
            //_frame->setMatrix(_matrix);
            _frame->setScale(1.0f);
            _frame->update();
            std::cout << "leave" << std::endl;
            return true;
        }
        return false;
    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();
            double dt = fs->getSimulationTime() - _lastUpdate;
            _lastUpdate = fs->getSimulationTime();

            if (_frame.valid())
            {
                float value;
                if (_over)
                {
                    _motionOver.update(dt);
                    value = _motionOver.getValue();
                }
                else
                {
                    _motionLeave.update(dt);
                    value = 1.0 - _motionLeave.getValue();
                }

                osgWidget::Color c = _defaultColor + ((_overColor - _defaultColor) * value);
                ColorSetterVisitor colorSetter(c);
                _frame->accept(colorSetter);
            }
        }
        node->traverse(*nv);
    }
};



osgWidget::Label* MessageBox::createLabel(const std::string& string, const std::string& font, int size, const osgWidget::Color& color)
{
    osgWidget::Label* label = new osgWidget::Label("", "");
    label->setFont(font);
    label->setFontSize(size);
    label->setFontColor(color);
    label->setColor(osgWidget::Color(0,0,0,0));
    label->setLabel(string);
    label->setCanFill(true);
    return label;
}

osgWidget::Frame* MessageBox::createButtonOk(const std::string& theme, 
                                                 const std::string& text, 
                                                 const std::string& font, 
                                                 int fontSize)
{
    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "ButtonOK",
        osgDB::readImageFile(theme),
        300.0f, 
        50.0f,
        osgWidget::Frame::FRAME_TEXTURE
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* label = createLabel(text, font, fontSize, osgWidget::Color(0,0,0,1));

    osgWidget::Box* box = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    box->addWidget(label);
    box->resize();
    osgWidget::Color colorBack = frame->getEmbeddedWindow()->getColor();
    box->getBackground()->setColor(colorBack);
    frame->getEmbeddedWindow()->setWindow(box);
    box->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
    box->setEventMask(osgWidget::EVENT_NONE);
    frame->setVisibilityMode(osgWidget::Window::VM_ENTIRE);

    frame->resizeFrame(box->getWidth(), box->getHeight());
    frame->resizeAdd(0, 0);

    EventOK* event = new EventOK(frame.get());
    frame->setUpdateCallback(event);
    frame->addCallback(event);

    return frame.release();
}

bool MessageBox::create(const std::string& themeMessage, 
                        const std::string& themeButton,
                        const std::string& titleText,
                        const std::string& messageText,
                        const std::string& buttonText,
                        const std::string& font,
                        int fontSize)
{

    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "error",
        osgDB::readImageFile(themeMessage),
        300.0f,
        50.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* labelText = createLabel(messageText, font, fontSize, osgWidget::Color(0,0,0,1));
    osgWidget::Label* labelTitle = createLabel(titleText, font, fontSize+5, osgWidget::Color(0.4,0,0,1));

    osgWidget::Box*   box   = new osgWidget::Box("VBOX", osgWidget::Box::VERTICAL);

    _button = createButtonOk(themeButton, buttonText, font, fontSize);
    osgWidget::Widget* buttonOK = _button->embed();
    _button->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
    buttonOK->setColor(osgWidget::Color(0,0,0,0));
    buttonOK->setCanFill(false);

    labelTitle->setPadBottom(30.0f);
    labelText->setPadBottom(30.0f);

    box->addWidget(buttonOK);
    box->addWidget(labelText);
    box->addWidget(labelTitle);

    osgWidget::Color colorBack = frame->getEmbeddedWindow()->getColor();
    box->getBackground()->setColor(colorBack);

    frame->setWindow(box);

    box->resize();
    frame->resizeFrame(box->getWidth(), box->getHeight());
    _window = frame;
    return true;
}







const char* LABEL1 =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in..."
;

int main(int argc, char** argv) 
{

    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    int fontSize = 20;
    std::string font="fonts/arial.ttf";
    std::string buttonTheme = "osgWidget/theme-8-shadow.png";
    std::string borderTheme = "osgWidget/theme-8.png";

    MessageBox message;
    message.create(borderTheme, 
                   buttonTheme,
                   "Error - Critical",
                   LABEL1,
                   "Quit",
                   font,
                   fontSize);

    AlphaSetterVisitor alpha(.8f);
    message.getWindow()->accept(alpha);

    wm->addChild(message.getWindow());

    // center
    osgWidget::point_type w = wm->getWidth();
    osgWidget::point_type h = wm->getHeight();
    osgWidget::point_type ww = message.getWindow()->getWidth();
    osgWidget::point_type hw = message.getWindow()->getHeight();
    osgWidget::point_type ox = (w - ww) / 2;
    osgWidget::point_type oy = (h - hw) / 2;
    message.getWindow()->setPosition(osgWidget::Point(
        osg::round(ox), osg::round(oy), message.getWindow()->getPosition()[2])
    );
//    frame->resizeAdd(30, 30);

//    AlphaSetterVisitor alpha(.8f);
//    frame->accept(alpha);
    return osgWidget::createExample(viewer, wm); //osgDB::readNodeFile("cow.osg"));

}

























































#if 0
struct AlphaSetterVisitor : public osg::NodeVisitor
{
    float _alpha;
    AlphaSetterVisitor( float alpha = 1.0):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { _alpha = alpha;}

    void apply(osg::MatrixTransform& node)
    {
        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(&node);

        if (win) {
//             osgWidget::warn() << "I am in Window: " << win->getName() << std::endl;

            for (osgWidget::Window::Iterator it = win->begin(); it != win->end(); it++)
            {
//                 osgWidget::warn() << "   I am operating on Widget: " << it->get()->getName() << std::endl;
                
                osgWidget::Color color = it->get()->getColor();
                color[3] = color[3] *_alpha;
                it->get()->setColor(color);
            }
            {
                osgWidget::Color color = win->getBackground()->getColor();
                color[3] = color[3] *_alpha;
                win->getBackground()->setColor(color);
            }
        }
        traverse(node);
    }
};


struct ColorSetterVisitor : public osg::NodeVisitor
{
    osgWidget::Color _color;
    ColorSetterVisitor( const osgWidget::Color& color):osg::NodeVisitor(TRAVERSE_ALL_CHILDREN) { _color = color;}

    void apply(osg::MatrixTransform& node)
    {
        osgWidget::Window* win = dynamic_cast<osgWidget::Window*>(&node);

        if (win) {
//            osgWidget::warn() << "I am in Window: " << win->getName() << std::endl;

            for (osgWidget::Window::Iterator it = win->begin(); it != win->end(); it++)
            {
//                osgWidget::warn() << "   I am operating on Widget: " << it->get()->getName() << std::endl;
                
//                 osgWidget::Color color = it->get()->getColor();
//                 color[3] = color[3] *_alpha;
                it->get()->setColor(_color);
            }
            {
//                 osgWidget::Color color = win->getBackground()->getColor();
//                 color[3] = color[3] *_alpha;
                win->getBackground()->setColor(osgWidget::Color(0,0,0,0));
            }
        }
        traverse(node);
    }
};


struct EventOK : public osgWidget::Callback, osg::NodeCallback
{
    typedef osgAnimation::OutQuartMotion WidgetMotion;
    WidgetMotion _motionOver;
    WidgetMotion _motionLeave;
    
    double _lastUpdate;
    osgWidget::Color _defaultColor;
    osgWidget::Color _overColor;
    bool _over;
    osg::ref_ptr<osgWidget::Frame> _frame;
    float _width;
    float _height;
    EventOK(osgWidget::Frame* frame) : osgWidget::Callback(osgWidget::EVENT_ALL), _frame(frame) 
    {
        _motionOver = WidgetMotion(0.0, 0.4);
        _motionLeave = WidgetMotion(0.0, 0.5);
        _defaultColor = _frame->getEmbeddedWindow()->getColor();
        _overColor = osgWidget::Color(229.0/255.0,
                                      103.0/255.0,
                                      17.0/255,
                                      _defaultColor[3]);
        _over  = false;
    }

    bool operator()(osgWidget::Event& ev)
    {
        if (ev.type == osgWidget::EVENT_MOUSE_ENTER)
        {
            _over = true;
//            std::cout << "Enter" << std::endl;
            _width = _frame->getWidth();
            _height = _frame->getHeight();
            _motionOver.reset();

//             _frame->resize(_width * 1.2, _height * 1.2);
            return true;
        }
        else if (ev.type == osgWidget::EVENT_MOUSE_LEAVE) 
        {
            _over = false;
//            std::cout << "Leave" << std::endl;
//             _frame->resize(_width, _height);
            _motionLeave.reset();
            return true;
        }
        return false;
    }

    void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        if (nv->getVisitorType() == osg::NodeVisitor::UPDATE_VISITOR)
        {
            const osg::FrameStamp* fs = nv->getFrameStamp();
            double dt = fs->getSimulationTime() - _lastUpdate;
            _lastUpdate = fs->getSimulationTime();

            if (_frame.valid())
            {
                float value;
                if (_over)
                {
                    _motionOver.update(dt);
                    value = _motionOver.getValue();
                }
                else
                {
                    _motionLeave.update(dt);
                    value = 1.0 - _motionLeave.getValue();
                }

                osgWidget::Color c = _defaultColor + ((_overColor - _defaultColor) * value);
                ColorSetterVisitor colorSetter(c);
                _frame->accept(colorSetter);
            }
        }
        node->traverse(*nv);
    }
};



osgWidget::Label* createLabel(const std::string& string, const std::string& font, int size, const osgWidget::Color& color)
{
    osgWidget::Label* label = new osgWidget::Label("", "");
    label->setFont(font);
    label->setFontSize(size);
    label->setFontColor(color);
    label->setColor(osgWidget::Color(0,0,0,0));
    label->setLabel(string);
    label->setCanFill(true);
    return label;
}

osgWidget::Window* createButtonOk(const std::string& theme, const std::string& text, int fontSize)
{
    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "ButtonOK",
        osgDB::readImageFile(theme),
        300.0f, 
        50.0f,
        osgWidget::Frame::FRAME_TEXTURE
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* label = createLabel(text, "fonts/Vera.ttf", fontSize, osgWidget::Color(0,0,0,1));

    osgWidget::Box* box = new osgWidget::Box("HBOX", osgWidget::Box::HORIZONTAL);
    box->addWidget(label);
    box->resize();
    osgWidget::Color colorBack = frame->getEmbeddedWindow()->getColor();
    box->getBackground()->setColor(colorBack);
    frame->getEmbeddedWindow()->setWindow(box);
    box->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
    box->setEventMask(osgWidget::EVENT_NONE);

    frame->resizeFrame(box->getWidth(), box->getHeight());
    frame->resizeAdd(0, 0);

    EventOK* event = new EventOK(frame);
    frame->setUpdateCallback(event);
    frame->addCallback(event);


    return frame.release();
}

osgWidget::Frame* createErrorMessage(const std::string& themeMessage, 
                              const std::string& themeButton,
                              const std::string& titleText,
                              const std::string& messageText,
                              const std::string& buttonText,
                              const std::string& font,
                              int fontSize)
{

    osg::ref_ptr<osgWidget::Frame> frame = osgWidget::Frame::createSimpleFrameFromTheme(
        "error",
        osgDB::readImageFile(themeMessage),
        300.0f,
        50.0f,
        osgWidget::Frame::FRAME_ALL
        );
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);

    osgWidget::Label* labelText = createLabel(messageText, font, fontSize, osgWidget::Color(0,0,0,1));
    osgWidget::Label* labelTitle = createLabel(titleText, font, fontSize+5, osgWidget::Color(0.4,0,0,1));

    osgWidget::Box*   box   = new osgWidget::Box("VBOX", osgWidget::Box::VERTICAL);

    osgWidget::Widget* buttonOK = createButtonOk(themeButton, buttonText, fontSize)->embed();
    buttonOK->setColor(osgWidget::Color(0,0,0,0));
    buttonOK->setCanFill(false);

    box->addWidget(buttonOK);
    box->addWidget(labelText);
    box->addWidget(labelTitle);

    osgWidget::Color colorBack = frame->getEmbeddedWindow()->getColor();
    box->getBackground()->setColor(colorBack);

    frame->setWindow(box);

    box->resize();
    frame->resizeFrame(box->getWidth(), box->getHeight());
    return frame.release();
}


const char* LABEL1 =
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed\n"
    "do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris\n"
    "nisi ut aliquip ex ea commodo consequat. Duis aute irure dolor in..."
;

int main(int argc, char** argv) 
{
    std::string theme = "osgWidget/theme-1.png";
    if (argc > 1)
        theme = std::string(argv[1]);

    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );

    osgWidget::Frame* frame = createErrorMessage(theme,
                                          "osgWidget/theme-8-shadow.png",
                                          "Error - Critical",
                                          LABEL1,
                                          "Ok",
                                          "fonts/Vera.ttf",
                                          20);
    // Add everything to the WindowManager.
    wm->addChild(frame);
    frame->resizeAdd(30, 30);

    AlphaSetterVisitor alpha(.8f);
    frame->accept(alpha);
    return osgWidget::createExample(viewer, wm, osgDB::readNodeFile("cow.osg"));
}
#endif
