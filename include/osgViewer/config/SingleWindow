/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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

#ifndef OSGVIEWER_SingleWindow
#define OSGVIEWER_SingleWindow 1

#include <osgViewer/View>

namespace osgViewer {

/** single camera on a single window.*/
class OSGVIEWER_EXPORT SingleWindow : public ViewConfig
{
    public:
        
        SingleWindow():_x(0),_y(0),_width(-1),_height(-1),_screenNum(0),_windowDecoration(true),_overrideRedirect(false) {}
        SingleWindow(int x, int y, int width, int height, unsigned int screenNum=0):_x(x),_y(y),_width(width),_height(height),_screenNum(screenNum),_windowDecoration(true),_overrideRedirect(false) {}
        SingleWindow(const SingleWindow& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):ViewConfig(rhs,copyop), _x(rhs._x),_y(rhs._y),_width(rhs._width),_height(rhs._height),_screenNum(rhs._screenNum),_windowDecoration(rhs._windowDecoration), _overrideRedirect(rhs._overrideRedirect) {}
        
        META_Object(osgViewer,SingleWindow);
        
        virtual void configure(osgViewer::View& view) const;
        
        void setX(int x) { _x = x; }
        int getX() const { return _x; }
        
        void setY(int y) { _y = y; }
        int getY() const { return _y; }
        
        void setWidth(int w) { _width = w; }
        int getWidth() const { return _width; }

        void setHeight(int h) { _height = h; }
        int getHeight() const { return _height; }

        void setScreenNum(unsigned int sn) { _screenNum = sn; }
        unsigned int getScreenNum() const { return _screenNum; }
        
        void setWindowDecoration(bool wd) { _windowDecoration = wd; }
        bool getWindowDecoration() const { return _windowDecoration; }
        
        void setOverrideRedirect(bool override) { _overrideRedirect = override; }
        bool getOverrideRedirect() const { return _overrideRedirect; }
        
    protected:
        
        int _x, _y, _width, _height;
        unsigned int _screenNum;
        bool _windowDecoration;
        bool _overrideRedirect;
};

}

#endif
