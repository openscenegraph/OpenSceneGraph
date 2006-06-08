//C++ header - Open Scene Graph Simulation - Copyright (C) 1998-2006 Robert Osfield
// Distributed under the terms of the GNU General Public License (GPL)
// as published by the Free Software Foundation.
//
// All software using osgSim must be GPL'd or excempted via the 
// purchase of the Open Scene Graph Professional License (OSGPL)
// for further information contact robert@openscenegraph.com.

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
        virtual void drawImplementation(osg::State& state) const;


    protected:
    
        virtual ~LightPointSpriteDrawable() {}

        osg::ref_ptr<osg::PointSprite>        _sprite;

};

}

#endif

