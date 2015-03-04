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

#ifndef OSGPRESENTATION_TIMOUTOUT
#define OSGPRESENTATION_TIMOUTOUT 1

#include <osg/Transform>

#include <osgPresentation/SlideEventHandler>

namespace osgPresentation {

class OSGPRESENTATION_EXPORT HUDSettings : public osg::Referenced
{
    public:
        HUDSettings(double slideDistance, float eyeOffset, unsigned int leftMask, unsigned int rightMask);

        virtual bool getModelViewMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;

        virtual bool getInverseModelViewMatrix(osg::Matrix& matrix, osg::NodeVisitor* nv) const;

        double          _slideDistance;
        double          _eyeOffset;
        unsigned int    _leftMask;
        unsigned int    _rightMask;

protected:

        virtual ~HUDSettings();
};

class OSGPRESENTATION_EXPORT Timeout : public osg::Transform
{
    public:

        Timeout(HUDSettings* hudSettings=0);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        Timeout(const Timeout& timeout,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osgPresentation, Timeout);


        void setIdleDurationBeforeTimeoutDisplay(double t) { _idleDurationBeforeTimeoutDisplay = t; }
        double getIdleDurationBeforeTimeoutDisplay() const { return _idleDurationBeforeTimeoutDisplay; }

        void setIdleDurationBeforeTimeoutAction(double t) { _idleDurationBeforeTimeoutAction = t; }
        double getIdleDurationBeforeTimeoutAction() const { return _idleDurationBeforeTimeoutAction; }


        void setKeyStartsTimoutDisplay(int key) { _keyStartsTimoutDisplay = key; }
        int getKeyStartsTimoutDisplay() const { return _keyStartsTimoutDisplay; }

        void setKeyDismissTimoutDisplay(int key) { _keyDismissTimoutDisplay = key; }
        int getKeyDismissTimoutDisplay() const { return _keyDismissTimoutDisplay; }

        void setKeyRunTimoutAction(int key) { _keyRunTimeoutAction = key; }
        int getKeyRunTimoutAction() const { return _keyRunTimeoutAction; }


        void setDisplayBroadcastKeyPosition(const osgPresentation::KeyPosition& keyPos) { _displayBroadcastKeyPos = keyPos; }
        const osgPresentation::KeyPosition&  getDisplayBroadcastKeyPosition() const { return _displayBroadcastKeyPos; }

        void setDismissBroadcastKeyPosition(const osgPresentation::KeyPosition& keyPos) { _dismissBroadcastKeyPos = keyPos; }
        const osgPresentation::KeyPosition&  getDismissBroadcastKeyPosition() const { return _dismissBroadcastKeyPos; }

        void setActionKeyPosition(const osgPresentation::KeyPosition& keyPos) { _actionKeyPos = keyPos; }
        const osgPresentation::KeyPosition&  getActionKeyPosition() const { return _actionKeyPos; }

        void setActionBroadcastKeyPosition(const osgPresentation::KeyPosition& keyPos) { _actionBroadcastKeyPos = keyPos; }
        const osgPresentation::KeyPosition&  getActionBroadcastKeyPosition() const { return _actionBroadcastKeyPos; }

        void setActionJumpData(const JumpData& jumpData) { _actionJumpData = jumpData; }
        const JumpData& getActionJumpData() const { return _actionJumpData; }

        virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const;

        virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor*) const;

        virtual void traverse(osg::NodeVisitor& nv);

    protected:

        virtual ~Timeout();

        void broadcastEvent(osgViewer::Viewer* viewer, const osgPresentation::KeyPosition& keyPos);

        osg::ref_ptr<HUDSettings> _hudSettings;

        int     _previousFrameNumber;
        double  _timeOfLastEvent;
        bool    _displayTimeout;

        double  _idleDurationBeforeTimeoutDisplay;
        double  _idleDurationBeforeTimeoutAction;

        int _keyStartsTimoutDisplay;
        int _keyDismissTimoutDisplay;
        int _keyRunTimeoutAction;

        osgPresentation::KeyPosition _displayBroadcastKeyPos;
        osgPresentation::KeyPosition _dismissBroadcastKeyPos;

        osgPresentation::KeyPosition _actionKeyPos;
        osgPresentation::KeyPosition _actionBroadcastKeyPos;
        JumpData _actionJumpData;
};

}

#endif
