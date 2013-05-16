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

#ifndef OSGVIEWER_SingleScreen
#define OSGVIEWER_SingleScreen 1

#include <osgViewer/View>

namespace osgViewer {

/** single camera associated with a single full screen GraphicsWindow.*/
class OSGVIEWER_EXPORT SingleScreen : public ViewConfig
{
    public:
        
        SingleScreen(unsigned int screenNum=0) : _screenNum(screenNum) {}
        SingleScreen(const SingleScreen& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) : ViewConfig(rhs,copyop), _screenNum(rhs._screenNum) {}
        
        META_Object(osgViewer, SingleScreen);
        
        virtual void configure(osgViewer::View& view) const;

        void setScreenNum(unsigned int sn) { _screenNum = sn; }
        unsigned int getScreenNum() const { return _screenNum; }
        
    protected:
        
        unsigned int _screenNum;
};

}

#endif
