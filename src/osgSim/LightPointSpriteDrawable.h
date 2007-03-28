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

#ifndef OSGSIM_LIGHTPOINTSPRITEDRAWABLE
#define OSGSIM_LIGHTPOINTSPRITEDRAWABLE 1

#include <osgSim/Export>

#include <osg/Drawable>
#include "LightPointDrawable.h"
#include <osg/PointSprite>

namespace osgSim {


class OSGSIM_EXPORT LightPointSpriteDrawable : public osgSim::LightPointDrawable
{
    public :

        LightPointSpriteDrawable();
        
        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        LightPointSpriteDrawable(const LightPointSpriteDrawable&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        virtual osg::Object* cloneType() const { return new LightPointSpriteDrawable(); }
        virtual osg::Object* clone(const osg::CopyOp&) const { return new LightPointSpriteDrawable(); }
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const LightPointSpriteDrawable*>(obj)!=NULL; }
        virtual const char* className() const { return "LightPointSpriteDrawable"; }

        
        /** draw LightPoints. */
        virtual void drawImplementation(osg::RenderInfo& renderInfo) const;


    protected:
    
        virtual ~LightPointSpriteDrawable() {}

        osg::ref_ptr<osg::PointSprite>        _sprite;

};

}

#endif

