/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#ifndef KEYEVENTHANDLER
#define KEYEVENTHANDLER 1

#include <osg/StateSet>
#include <osg/Point>

#include <osgGA/GUIEventHandler>

#include <osgPresentation/SlideEventHandler>

namespace osgPresentation
{

class OSGPRESENTATION_EXPORT KeyEventHandler : public osgGA::GUIEventHandler
{
    public:

        KeyEventHandler(int key, osgPresentation::Operation operation, const JumpData& jumpData=JumpData());
        KeyEventHandler(int key, const std::string& str, osgPresentation::Operation operation, const JumpData& jumpData=JumpData());
        KeyEventHandler(int key, const osgPresentation::KeyPosition& keyPos, const JumpData& jumpData=JumpData());

        void setKey(int key) { _key = key; }
        int getKey() const { return _key; }

        void setOperation(osgPresentation::Operation operation) { _operation = operation; }
        osgPresentation::Operation getOperation() const { return _operation; }

        void setCommand(const std::string& str) { _command = str; }
        const std::string& getCommand() const { return _command; }

        void setKeyPosition(const osgPresentation::KeyPosition& keyPos) { _keyPos = keyPos; }
        const osgPresentation::KeyPosition&  getKeyPosition() const { return _keyPos; }

        void setJumpData(const JumpData& jumpData) { _jumpData = jumpData; }
        const JumpData& getJumpData() const { return _jumpData; }

        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa, osg::Object* object, osg::NodeVisitor* nv);

        virtual void getUsage(osg::ApplicationUsage& usage) const;

        void doOperation();

        int                                 _key;

        std::string                         _command;
        osgPresentation::KeyPosition        _keyPos;
        osgPresentation::Operation          _operation;

        JumpData                            _jumpData;
};

}

#endif
