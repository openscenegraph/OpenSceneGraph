// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <ctype.h>
#include <osgWidget/Box>

namespace osgWidget {

Box::Box(const std::string& name, BoxType bt, bool uniform):
Window   (name),
_boxType (bt),
_uniform (uniform),
_lastAdd (0) {
}

Box::Box(const Box& box, const osg::CopyOp& co):
Window   (box, co),
_boxType (box._boxType),
_uniform (box._uniform),
_lastAdd (box._lastAdd) {
}

// TODO: Here's something to consider! If we resize the box by 1 every time, only the
// first resizable Widget will continue to get larger. This is really silly.
void Box::_resizeImplementation(point_type w, point_type h) {
    // Get the number of Widgets that agree to fill. Also perfom some casting to integers
    // in case we're being request to resize with pixel perfection.
    point_type numFill  = _getNumFill();
    int        iw       = static_cast<int>(w);
    int        ih       = static_cast<int>(h);
    int        inumFill = static_cast<int>(numFill);
    int        wrem     = 0;
    int        hrem     = 0;

    // If we have some widgets that fill, use these variables to keep a running count
    // of what needs to be added.
    if(inumFill) {
        wrem = iw % inumFill;
        hrem = ih % inumFill;
    }

    // If we have any widgets that agree to fill and there has been an honest resize
    // request, handle it here. The first case handles resizes where we have AT LEAST
    // as many pixels to fill as we have objects.
    if(numFill > 0.0f && (w != 0.0f || h != 0.0f)) {
        unsigned int cur = 0;

        for(Iterator i = begin(); i != end(); i++) if(i->valid() && i->get()->canFill()) {
            point_type addWidth  = 0.0f;
            point_type addHeight = 0.0f;

            // If our last added-to Widget was the last one, reset it to 0.
            if(_lastAdd >= size()) _lastAdd = 0;

            // We EVENLY give any remaining space to all fillable Widgets. In the
            // future we may want to be able to specify a fill "percent", which
            // would be some portion of the total available space.
            if(_boxType == HORIZONTAL) {
                if(w) {
                    addWidth += static_cast<point_type>(iw / inumFill);

                    if(cur >= _lastAdd && wrem) {
                        _lastAdd++;
                        addWidth++;
                        wrem--;
                    }
                }

                if(h) addHeight += h;
            }

            else {
                if(w) addWidth += w;

                if(h) {
                    addHeight += static_cast<point_type>(ih / inumFill);

                    if(cur >= _lastAdd && hrem) {
                        _lastAdd++;
                        addHeight++;
                        hrem--;
                    }
                }
            }

            if(addWidth != 0.0f) i->get()->addWidth(addWidth);

            if(addHeight != 0.0f) i->get()->addHeight(addHeight);

            cur++;
        }
    }

    // Get the width and height of our largest widgets; these values take
    // into account the padding, and will be affected by any resizing that occured above.
    point_type maxWidth  = _getMaxWidgetWidthTotal();
    point_type maxHeight = _getMaxWidgetHeightTotal();

    // Create counters for the various offsets as we position Widgets.
    point_type xoff = 0.0f;
    point_type yoff = 0.0f;
    point_type xadd = 0.0f;
    point_type yadd = 0.0f;

    for(Iterator i = begin(); i != end(); i++) {
        Widget* widget = i->get();

        // This positioning works by setting each Widget's unmodified origin and then
        // letting Window::_positionWidget calculate the padding/fill.
        if(_boxType == HORIZONTAL) {
            // First, lets set it to the proper x offset, ignoring any padding.
            widget->setOrigin(xoff, 0.0f);

            // Immediately reset our xoff for the next iteration.
            if(_uniform) {
                _positionWidget(widget, maxWidth, maxHeight);

                xadd = maxWidth;
            }

            else {
                _positionWidget(widget, widget->getWidthTotal(), maxHeight);

                xadd = widget->getWidthTotal();
            }
        }

        else {
            widget->setOrigin(0.0f, yoff);

            if(_uniform) {
                _positionWidget(widget, maxWidth, maxHeight);

                yadd = maxHeight;
            }

            else {
                _positionWidget(widget, maxWidth, widget->getHeightTotal());

                yadd = widget->getHeightTotal();
            }
        }

        xoff += xadd;
        yoff += yadd;
    }
}

Window::Sizes Box::_getWidthImplementation() const {
    // The width of a horizontal box is all of the widgets added together.
    if(_boxType == HORIZONTAL) {
        // If we're a uniformly sized box, our width is our largest width plus our
        // largest padding, multiplied times the number of widgets. Our minimum width
        // is the size of the largest minWidth times the number of widgets.
        if(_uniform) return Sizes(
            _getMaxWidgetWidthTotal() * size(),
            _getMaxWidgetMinWidthTotal() * size()
        );

        // Othweriwse, our width is all of the widths added together, and our minWidth
        // is all of the minWidths added together.
        else return Sizes(
            _accumulate<Plus>(&Widget::getWidthTotal),
            _accumulate<Plus>(&Widget::getMinWidthTotal)
        );
    }

    // If we're a vertical Box, our width is the width of the larget Widget in the group.
    // Our minWidth is the largest minWidth of the Widgets in the group.
    else return Sizes(
        _getMaxWidgetWidthTotal(),
        _getMaxWidgetMinWidthTotal()
    );
}

Window::Sizes Box::_getHeightImplementation() const {
    if(_boxType == VERTICAL) {
        if(_uniform) return Sizes(
            _getMaxWidgetHeightTotal() * size(),
            _getMaxWidgetMinHeightTotal() * size()
        );

        else return Sizes(
            _accumulate<Plus>(&Widget::getHeightTotal),
            _accumulate<Plus>(&Widget::getMinHeightTotal)
        );
    }

    else return Sizes(
        _getMaxWidgetHeightTotal(),
        _getMaxWidgetMinHeightTotal()
    );
}

}
