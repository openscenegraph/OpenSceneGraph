// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: osgwidgetinput.cpp 50 2008-05-06 05:06:36Z cubicool $

#include <osgDB/WriteFile>

#include <osgWidget/Util>
#include <osgWidget/WindowManager>
#include <osgWidget/Box>
#include <osgWidget/Table>
#include <osgWidget/Frame>
#include <osgWidget/Label>
#include <osgWidget/Input>

const unsigned int MASK_2D = 0xF0000000;

const char* INFO =
    "Use the Input Wigets below to enter the X, Y, and Z position of a\n"
    "sphere to be inserted into the scene. Once you've done this, use\n"
    "the button below to add it!"
;

void setupLabel(osgWidget::Label* label) {
    label->setFontSize(16);
    label->setFontColor(1.0f, 1.0f, 1.0f, 1.0f);
    label->setFont("fonts/Vera.ttf");
    label->setPadding(2.0f);
    label->setHeight(18.0f);
    label->setCanFill(true);
}

osgWidget::Input* createTableRow(
    osgWidget::Table*  table,
    unsigned int       rowNum,
    const std::string& valName
) {
    std::stringstream ssLabel;
    std::stringstream ssInput;

    ssLabel << "Label_Row" << rowNum;
    ssInput << "Input_Row" << rowNum;

    osgWidget::Label* label = new osgWidget::Label(ssLabel.str(), valName);
    osgWidget::Input* input = new osgWidget::Input(ssInput.str(), "", 20);

    setupLabel(label);
    setupLabel(input);

    label->setWidth(50.0f);
    label->setColor(0.1f, 0.1f, 0.1f, 1.0f);

    input->setWidth(150.0f);
    input->setColor(0.4f, 0.4f, 0.4f, 1.0f);

    table->addWidget(label, rowNum, 0);
    table->addWidget(input, rowNum, 1);

    return input;
}

osgWidget::Label* createLabel(const std::string& text) {
    osgWidget::Label* label = new osgWidget::Label("", text);

    setupLabel(label);

    return label;
}

class Button: public osgWidget::Label {
public:
    typedef std::vector<osgWidget::Input*> Inputs;

private:
    Inputs _xyz;

public:
    Button(const std::string& text, const Inputs& inputs):
    osgWidget::Label("", text),
    _xyz(inputs) {
        setupLabel(this);

        setEventMask(osgWidget::EVENT_MASK_MOUSE_CLICK);
        setShadow(0.1f);
        addHeight(4.0f);
    }

    bool mousePush(double, double, osgWidget::WindowManager*) {
        osgWidget::warn()
            << "x: " << _xyz[0]->getLabel() << std::endl
            << "y: " << _xyz[1]->getLabel() << std::endl
            << "z: " << _xyz[2]->getLabel() << std::endl
        ;

        return false;
    }
};

// TODO: Testing our _parent/EmbeddedWindow stuff.
bool info(osgWidget::Event& ev) {
    osgWidget::warn() << "MousePush @ Window: " << ev.getWindow()->getName() << std::endl;

    return true;
}

int main(int argc, char** argv) {
    osgViewer::Viewer viewer;

    osgWidget::WindowManager* wm = new osgWidget::WindowManager(
        &viewer,
        1280.0f,
        1024.0f,
        MASK_2D,
        osgWidget::WindowManager::WM_PICK_DEBUG
    );
    
    osgWidget::Box*   box   = new osgWidget::Box("vbox", osgWidget::Box::VERTICAL);
    osgWidget::Table* table = new osgWidget::Table("table", 3, 2);
    osgWidget::Box*   lbox1 = new osgWidget::Box("lbox1", osgWidget::Box::HORIZONTAL);
    osgWidget::Box*   lbox2 = new osgWidget::Box("lbox2", osgWidget::Box::HORIZONTAL);
    osgWidget::Frame* frame = osgWidget::Frame::createSimpleFrameWithSingleTexture(
        "frame",
        "osgWidget/theme.png",
        64.0f,
        64.0f,
        16.0f,
        16.0f,
        100.0f,
        100.0f
    );

    osgWidget::Input* x = createTableRow(table, 0, "X Position");
    osgWidget::Input* y = createTableRow(table, 1, "Y Position");
    osgWidget::Input* z = createTableRow(table, 2, "Z Position");
    
    Button::Inputs inputs;

    inputs.push_back(x);
    inputs.push_back(y);
    inputs.push_back(z);

    table->addCallback(new osgWidget::Callback(&info, osgWidget::EVENT_MOUSE_PUSH));

    lbox1->addWidget(createLabel(INFO));
    lbox2->addWidget(new Button("Add To Scene...", inputs));

    box->addWidget(lbox1->embed());
    box->addWidget(table->embed());
    box->addWidget(lbox2->embed());
    box->addCallback(new osgWidget::Callback(&info, osgWidget::EVENT_MOUSE_PUSH));

    frame->setWindow(box);
    frame->getEmbeddedWindow()->setSize(box->getWidth(), box->getHeight());
    frame->getBackground()->setColor(0.0f, 0.0f, 0.0f, 0.0f);
    frame->attachTabFocusCallback();

    for(osgWidget::Frame::Iterator i = frame->begin(); i != frame->end(); i++) {
        if(i->valid()) i->get()->setColor(0.5f, 0.7f, 1.0f, 1.0f);
    }

    wm->addChild(frame);

    /*
    // Print out our focus list, it should just have 3 widgets.
    osgWidget::WidgetList wl;

    box->getFocusList(wl);

    for(osgWidget::WidgetList::iterator i = wl.begin(); i != wl.end(); i++) {
        osgWidget::warn() << i->get()->getName() << std::endl;
    }
    */
    
    lbox1->getBackground()->setColor(1.0f, 0.0f, 0.0f, 1.0f, osgWidget::Widget::UPPER_LEFT);
    lbox1->getBackground()->setColor(0.0f, 1.0f, 0.0f, 1.0f, osgWidget::Widget::LOWER_LEFT);
    lbox1->getBackground()->setColor(0.0f, 0.0f, 1.0f, 1.0f, osgWidget::Widget::LOWER_RIGHT);
    lbox1->getBackground()->setColor(1.0f, 1.0f, 1.0f, 1.0f, osgWidget::Widget::UPPER_RIGHT);
    lbox1->setVisibilityMode(osgWidget::Window::VM_ENTIRE);
    lbox1->update();

    int r = osgWidget::createExample(viewer, wm);

    // osgWidget::writeWindowManagerNode(wm);
    // osgDB::writeNodeFile(*box, "osgWidget.osg");

    return r;
}
