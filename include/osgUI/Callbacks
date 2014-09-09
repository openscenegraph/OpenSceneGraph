/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#ifndef OSGUI_CALLBACKS
#define OSGUI_CALLBACKS

#include <osg/Callback>
#include <osgGA/EventVisitor>

#include <osgUI/Export>
#include <osgUI/Widget>

namespace osgUI
{


class OSGUI_EXPORT CloseCallback : public osg::CallbackObject
{
public:
    CloseCallback(const std::string& callbackName=std::string("close"), osgUI::Widget* closeWidget=0);
    CloseCallback(const CloseCallback& hc, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, CloseCallback);

    void setCloseWidget(osgUI::Widget* widget) { _closeWidget = widget; }
    osgUI::Widget* getCloseWidget() { return _closeWidget.get(); }
    const osgUI::Widget* getCloseWidget() const { return _closeWidget.get(); }

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

protected:
    virtual ~CloseCallback() {}
    osg::observer_ptr<osgUI::Widget> _closeWidget;
};

class OSGUI_EXPORT HandleCallback : public osg::CallbackObject
{
public:
    HandleCallback();
    HandleCallback(const HandleCallback& hc, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, HandleCallback);

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;
    virtual bool handle(osgGA::EventVisitor* ev, osgGA::Event* event) const;

protected:
    virtual ~HandleCallback() {}
};

class OSGUI_EXPORT DragCallback : public HandleCallback
{
public:
    DragCallback();
    DragCallback(const DragCallback& dc, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, DragCallback);

    void setDragging(bool v) { _dragging = v; }
    bool getDragging() const { return _dragging; }

    void setPreviousPosition(const osg::Vec3d& position) { _previousPosition = position; }
    const osg::Vec3d& getPreviousPosition() const { return _previousPosition; }

    virtual bool handle(osgGA::EventVisitor* ev, osgGA::Event* event) const;

protected:
    virtual ~DragCallback() {}

    bool        _dragging;
    osg::Vec3d  _previousPosition;

};

}

#endif
