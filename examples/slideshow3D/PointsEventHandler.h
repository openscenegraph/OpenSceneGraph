/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the GNU Public License (GPL) version 1.0 or 
 * (at your option) any later version. 
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#ifndef POINTSEVENTHANDLER
#define POINTSEVENTHANDLER 1

#include <osg/StateSet>
#include <osg/Point>

#include <osgGA/GUIEventHandler>

class PointsEventHandler : public osgGA::GUIEventHandler
{
    public:
        PointsEventHandler();
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);
        
        virtual void accept(osgGA::GUIEventHandlerVisitor& v);
        
        void getUsage(osg::ApplicationUsage& usage) const;
        
        void setStateSet(osg::StateSet* stateset) { _stateset=stateset; }
        
        osg::StateSet* getStateSet() { return _stateset.get(); }
        
        const osg::StateSet* getStateSet() const { return _stateset.get(); }
        
        void setPointSize(float psize);

        float getPointSize() const;
        
        void changePointSize(float delta);
        
        osg::ref_ptr<osg::StateSet> _stateset;
        osg::ref_ptr<osg::Point>    _point;

};

#endif
