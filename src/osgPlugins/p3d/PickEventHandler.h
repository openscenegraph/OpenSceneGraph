/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#ifndef PICKEVENTHANDLER
#define PICKEVENTHANDLER 1

#include <osg/StateSet>
#include <osg/Point>

#include <osgGA/GUIEventHandler>

#include "SlideShowConstructor.h"

class PickEventHandler : public osgGA::GUIEventHandler
{
    public:

        PickEventHandler(SlideShowConstructor::Operation operation, bool relativeJump=true, int slideNum=0, int layerNum=0);
        PickEventHandler(const std::string& str, SlideShowConstructor::Operation operation, bool relativeJump=true, int slideNum=0, int layerNum=0);
        PickEventHandler(const SlideShowConstructor::KeyPosition& keyPos, bool relativeJump=true, int slideNum=0, int layerNum=0);
        
        void setOperation(SlideShowConstructor::Operation operation) { _operation = operation; }
        SlideShowConstructor::Operation getOperation() const { return _operation; }
        
        void setCommand(const std::string& str) { _command = str; }
        const std::string& getCommand() const { return _command; }
    
        void setKeyPosition(const SlideShowConstructor::KeyPosition& keyPos) { _keyPos = keyPos; }
        const SlideShowConstructor::KeyPosition&  getKeyPosition() const { return _keyPos; }
        
        void setRelativeJump(int slideDelta, int layerDelta);
        void setAbsoluteJump(int slideNum, int layerNum);
        
        bool getRelativeJump() const { return _relativeJump; }
        int getSlideNum() const { return _slideNum; }
        int getLayerNum() const { return _layerNum; }
        
        bool requiresJump() const { return _relativeJump ? (_slideNum!=0 || _layerNum!=0) : true; }

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv);
                
        virtual void accept(osgGA::GUIEventHandlerVisitor& v);
        
        virtual void getUsage(osg::ApplicationUsage& usage) const;
        
        void doOperation();
        
        std::string                         _command;
        SlideShowConstructor::KeyPosition   _keyPos;
        SlideShowConstructor::Operation     _operation;
        
        bool                                _relativeJump;
        int                                 _slideNum;
        int                                 _layerNum;
};

#endif
