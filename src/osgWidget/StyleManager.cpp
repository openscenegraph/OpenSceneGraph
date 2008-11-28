// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <sstream>
#include <osg/io_utils>
#include <osgWidget/StyleManager>

namespace osgWidget {

Style::Style(const std::string& name, const std::string& style):
_style(style) {
    setName(name);
}

Style::Style(const Style& style, const osg::CopyOp& co):
osg::Object (style, co),
_style      (style._style) {
}

bool Style::applyStyle(Widget* widget, Reader r) {
    std::string str;
    osg::Vec2   vec2;
    osg::Vec3   vec3;
    osg::Vec4   vec4;
    float       f;

    if(_match("pos %i %i", r) || _match("pos %f %f", r)) {
        r.readSequence(vec2);
    
        widget->setOrigin(vec2);
    }

    else if(_match("pos-x %i", r) || _match("pos-x %f", r)) {
        r.readSequence(f);
    
        widget->setX(f);
    }

    else if(_match("pos-y %i", r) || _match("pos-y %f", r)) {
        r.readSequence(f);
    
        widget->setY(f);
    }

    else if(_match("size %i %i", r) || _match("size %f %f", r)) {
        r.readSequence(vec2);
    
        widget->setSize(vec2);
    }

    else if(_match("width %i", r) || _match("width %f", r)) {
        r.readSequence(f);
    
        widget->setWidth(f);
    }

    else if(_match("height %i", r) || _match("height %f", r)) {
        r.readSequence(f);
    
        widget->setHeight(f);
    }

    // Color using 4x 0-255 integers.
    else if(_match("color %i %i %i %i", r)) {
        r.readSequence(vec4);
        
        widget->setColor(vec4 / 255.0f);
    }

    // Color using 3x 0-255 integers with a default alpha of 255.
    else if(_match("color %i %i %i", r)) {
        r.readSequence(vec3);
        
        widget->setColor(osg::Vec4(vec3[0], vec3[1], vec3[2], 255.0f) / 255.0f);
    }
        
    // Color using 4x 0.0f-1.0f floats.
    else if(_match("color %f %f %f %f", r)) {
        r.readSequence(vec4);
        
        widget->setColor(vec4);
    }

    // Color using 3x 0.0f-1.0f floats with a default alpha of 1.0f.
    else if(_match("color %f %f %f", r)) {
        r.readSequence(vec3);
        
        widget->setColor(osg::Vec4(vec3[0], vec3[1], vec3[2], 1.0f));
    }

    // Set padding uniformly.
    else if(_match("padding %i", r)) {
        r.readSequence(f);
        
        widget->setPadding(f);
    }

    // Set left padding.
    else if(_match("padding-left %i", r)) {
        r.readSequence(f);
        
        widget->setPadLeft(f);
    }

    // Set right padding.
    else if(_match("padding-right %i", r)) {
        r.readSequence(f);
        
        widget->setPadRight(f);
    }

    // Set top padding.
    else if(_match("padding-top %i", r)) {
        r.readSequence(f);
        
        widget->setPadTop(f);
    }

    // Set bottom padding.
    else if(_match("padding-bottom %i", r)) {
        r.readSequence(f);
        
        widget->setPadBottom(f);
    }

    else if(_match("layer %w", r)) {
        r.readSequence(str);

        widget->setLayer(strToLayer(str));
    }

    else if(_match("valign %w", r)) {
        r.readSequence(str);

        widget->setAlignVertical(strToVAlign(str));
    }

    else if(_match("halign %w", r)) {
        r.readSequence(str);
        
        widget->setAlignHorizontal(strToHAlign(str));
    }

    else if(_match("coordmode %w", r)) {
        r.readSequence(str);

        widget->setCoordinateMode(strToCoordMode(str));
    }

    else if(_match("fill %w", r)) {
        r.readSequence(str);

        widget->setCanFill(strToFill(str));
    }

    else if(_match("image %s", r)) {
        r.readSequence(str);

        widget->setImage(str, true);
    }

    // Otherwise, increment the stream pointer.
    else return false;

    return true;
}

bool Style::applyStyle(Label* label, Reader r) {
    return false;
}

bool Style::applyStyle(Input* input, Reader r) {
    return false;
}

bool Style::applyStyle(Window* window, Reader r) {
    osg::Vec2 vec2;
    float     f;

    if(_match("pos %i %i", r) || _match("pos %f %f", r)) {
        r.readSequence(vec2);
    
        window->setOrigin(vec2.x(), vec2.y());
    }

    else if(_match("pos-x %i", r) || _match("pos-x %f", r)) {
        r.readSequence(f);
    
        window->setX(f);
    }

    else if(_match("pos-y %i", r) || _match("pos-y %f", r)) {
        r.readSequence(f);
    
        window->setY(f);
    }

    else if(_match("size %i %i", r) || _match("size %f %f", r)) {
        r.readSequence(vec2);

        window->resize(vec2.x(), vec2.y());
    }

    else if(_match("width %i", r) || _match("width %f", r)) {
        r.readSequence(f);
    
        window->resize(f);
    }

    else if(_match("height %i", r) || _match("height %f", r)) {
        r.readSequence(f);
    
        window->resize(0.0f, f);
    }

    else return false;

    return true;
}

bool Style::applyStyle(Canvas* label, Reader r) {
    return false;
}


bool Style::applyStyle(Window::EmbeddedWindow*, Reader r) {
    return false;
}

bool Style::applyStyle(Box* box, Reader r) {
    if(applyStyle(static_cast<Window*>(box), r)) return true;

    return false;
}

bool Style::applyStyle(Frame::Corner*, Reader r) {
    return false;
}

bool Style::applyStyle(Frame::Border*, Reader r) {
    return false;
}

Widget::Layer Style::strToLayer(const std::string& layer)
{
    std::string l = lowerCase(layer);

    if(l == "top") return Widget::LAYER_TOP;

    else if(l == "high") return Widget::LAYER_HIGH;

    else if(l == "middle") return Widget::LAYER_MIDDLE;

    else if(l == "low") return Widget::LAYER_LOW;
    
    else if(l == "bg") return Widget::LAYER_BG;

    else {
        warn() << "Unkown Layer name [" << layer << "]; using LAYER_MIDDLE." << std::endl;

        return Widget::LAYER_MIDDLE;
    }
}

Widget::VerticalAlignment Style::strToVAlign(const std::string& valign) {
    std::string va = lowerCase(valign);

    if(va == "center") return Widget::VA_CENTER;

    else if(va == "top") return Widget::VA_TOP;

    else if(va == "bottom") return Widget::VA_BOTTOM;

    else {
        warn() << "Unkown VAlign name [" << valign << "]; using VA_CENTER." << std::endl;

        return Widget::VA_CENTER;
    }
}

Widget::HorizontalAlignment Style::strToHAlign(const std::string& halign) {
    std::string ha = lowerCase(halign);

    if(ha == "center") return Widget::HA_CENTER;

    else if(ha == "left") return Widget::HA_LEFT;

    else if(ha == "right") return Widget::HA_RIGHT;

    else {
        warn() << "Unkown HAlign name [" << halign << "]; using HA_CENTER." << std::endl;

        return Widget::HA_CENTER;
    }
}

Widget::CoordinateMode Style::strToCoordMode(const std::string& coordmode) {
    std::string cm = lowerCase(coordmode);

    if(cm == "absolute") return Widget::CM_ABSOLUTE;

    else if(cm == "relative") return Widget::CM_RELATIVE;

    else {
        warn()
            << "Unkown CoordMode name [" << coordmode
            << "]; using CM_ABSOLUTE." << std::endl
        ;

        return Widget::CM_ABSOLUTE;
    }
}

bool Style::strToFill(const std::string& fill) {
    std::string cm = lowerCase(fill);
    
    if(cm == "true") return true;

    else if(cm == "false") return false;

    else {
        warn()
            << "Unkown Fill name [" << fill
            << "]; using false." << std::endl
        ;

        return false;
    }
}

StyleManager::StyleManager() {
}

StyleManager::StyleManager(const StyleManager& manager, const osg::CopyOp& co):
osg::Object(manager, co) {
    for(ConstIterator i = _styles.begin(); i != _styles.end(); i++) if(i->second.valid()) {
        _styles[i->first] = new Style(*i->second.get(), osg::CopyOp::DEEP_COPY_ALL);
    }
}

bool StyleManager::_applyStyleToObject(osg::Object* obj, const std::string& style) {
    std::string c = obj->className();

    if(!std::string("Widget").compare(c)) return _coerceAndApply<Widget>(
        obj,
        style,
        c
    );

    else if(!std::string("Label").compare(c)) return _coerceAndApply<Label>(
        obj,
        style,
        c
    );

    else if(!std::string("Box").compare(c)) return _coerceAndApply<Box>(
        obj,
        style,
        c
    );
    else if(!std::string("Canvas").compare(c)) return _coerceAndApply<Canvas>(
        obj,
        style,
        c
    );


    else warn()
        << "StyleManager does not support coercion of objects of type "
        << c << "." << std::endl
    ;

    return false;
}

bool StyleManager::addStyle(Style* style) {
    if(!style || style->getName().empty()) {
        warn() << "Cannot add a NULL or nameless Style object." << std::endl;

        return false;
    }

    _styles[style->getName()] = style;

    return true;
}

}
