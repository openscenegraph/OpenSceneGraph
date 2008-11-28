// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osg/Math>
#include <osgWidget/WindowManager>
#include <osgWidget/Label>

namespace osgWidget {

Label::Label(const std::string& name, const std::string& label):
Widget     (name, 0, 0),
_text      (new osgText::Text()), 
_textIndex (0) {
    _text->setAlignment(osgText::Text::LEFT_BOTTOM);
    _text->setDataVariance(osg::Object::DYNAMIC);

    if(label.size()) {
        _text->setText(label);

        _calculateSize(getTextSize());
    }

    // TODO: Make a patch for this!
    // If you're wondering why we don't use this let me explain...
    //
    // _text->setAlignment(osgText::Text::CENTER_CENTER);
    //
    // When you set the position of an osgText::Text object which has a CENTER_CENTER
    // alignment, the internal implementation of osgText may give it values that have
    // a "decimal" portion, which is NO GOOD on orthographic 2D displays where we
    // want "pixel perfect" ratios. Thus, until I can remedy this internally with
    // osgText::Text, I will need to calculate the center myself.

    setColor(0.0f, 0.0f, 0.0f, 0.0f);
}

Label::Label(const Label& label, const osg::CopyOp& co):
Widget     (label, co),
_textIndex (label._textIndex) {
    _text = new osgText::Text(*label._text, co);
}

void Label::_calculateSize(const XYCoord& size) {
    // if(size.x() && size.y()) setMinimumSize(size.x(), size.y());

    if(getWidth() < size.x()) setWidth(size.x());
    
    if(getHeight() < size.y()) setHeight(size.y());
}

// TODO: This will almost certainly get out of sync. :(
void Label::parented(Window* parent) {
    osg::Geode* geode = parent->getGeode();

    // If we've been cloned, use the index of the old text Drawable if it's already there.
    // However, we have a problem here: imagine a Label gets cloned AFTER being added to
    // a Window; it'll have a _textIndex, but that _textIndex won't apply to the
    // currently cloned object. In this case, we'll need to check to be SURE.
    osgText::Text* text = dynamic_cast<osgText::Text*>(geode->getDrawable(_textIndex));
    
    if(text) parent->getGeode()->setDrawable(_textIndex, _text.get());

    // Otherwise, add it as new.
    else _textIndex = parent->addDrawableAndGetIndex(_text.get());
}

void Label::unparented(Window* parent) {
    if(_textIndex) parent->getGeode()->removeDrawable(_text.get());

    _textIndex = 0;
}

void Label::positioned() {
    XYCoord    size = getTextSize();
    point_type x    = osg::round(((getWidth() - size.x()) / 2.0f) + getX());
    point_type y    = osg::round(((getHeight() - size.y()) / 2.0f) + getY());
    point_type z    = _calculateZ(getLayer() + 1);

    // These values are permisable with CENTER_CENTER mode is active.
    // point_type x  = round(getX() + (getWidth() / 2.0f));
    // point_type y  = round(getY() + (getHeight() / 2.0f));

    /*
    warn() << "Label widget size : " << getWidth() << " x " << getHeight() << std::endl;
    warn() << "Label widget tsize: " << getWidthTotal() << " x " << getHeightTotal() << std::endl;
    warn() << "Label XY coords   : " << getX() << " x " << getY() << std::endl;
    warn() << "Label BB in size  : " << size.x() << " x " << size.y() << std::endl;
    warn() << "Label xy position : " << x << " y " << y << std::endl;
    warn() << "------------------------------------" << std::endl;
    */

    const WindowManager* wm = _getWindowManager();

    if(wm && wm->isUsingRenderBins()) {
        _text->getOrCreateStateSet()->setRenderBinDetails(
            static_cast<int>(z * OSGWIDGET_RENDERBIN_MOD),
            "RenderBin"
        );

        z = 0.0f;
    }

    _text->setPosition(osg::Vec3(x, y, z));
}

void Label::setLabel(const std::string& label) {
    _text->setText(label);

    _calculateSize(getTextSize());
}

void Label::setFont(const std::string& font) {
    _text->setFont(font);
    
    _calculateSize(getTextSize());
}

void Label::setFontSize(unsigned int size) {
    _text->setCharacterSize(size);
    _text->setFontResolution(size, size);
    
    _calculateSize(getTextSize());
}

void Label::setFontColor(const Color& c) {
    _text->setColor(c);
}

void Label::setShadow(point_type offset) {
    _text->setBackdropType(osgText::Text::DROP_SHADOW_BOTTOM_RIGHT);
    _text->setBackdropImplementation(osgText::Text::NO_DEPTH_BUFFER);
    _text->setBackdropOffset(offset);

    _calculateSize(getTextSize());
}

XYCoord Label::getTextSize() const {
    osg::BoundingBox bb = _text->getBound();

    return XYCoord(
        osg::round(bb.xMax() - bb.xMin()),
        osg::round(bb.yMax() - bb.yMin())
    );
}

}
