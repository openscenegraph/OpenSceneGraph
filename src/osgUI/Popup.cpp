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


#include <osgUI/Popup>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>

using namespace osgUI;

Popup::Popup()
{
}

Popup::Popup(const osgUI::Popup& dialog, const osg::CopyOp& copyop):
    Widget(dialog, copyop),
    _title(dialog._title)
{
}

bool Popup::handleImplementation(osgGA::EventVisitor* /*ev*/, osgGA::Event* event)
{
//    OSG_NOTICE<<"Popup::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        //case(osgGA::GUIEventAdapter::KEYDOWN):
        case(osgGA::GUIEventAdapter::KEYUP):
            OSG_NOTICE<<"Key pressed : "<<(char)ea->getKey()<<std::endl;

            if (ea->getKey()=='c')
            {
                setVisible(false);
                ea->setHandled(true);

                return true;
            }

            break;
        default:
            break;
    }

    return false;
}

void Popup::leaveImplementation()
{
//    setVisible(false);
}

void Popup::createGraphicsImplementation()
{
   _transform = new osg::PositionAttitudeTransform;

    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    osg::Vec4 dialogBackgroundColor(0.9,0.9,0.9,1.0);

    _transform->addChild( style->createPanel(_extents, dialogBackgroundColor) );

    bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);
    if (requiresFrame) { _transform->addChild(style->createFrame(_extents, getFrameSettings(), dialogBackgroundColor)); }
#if 1
    style->setupDialogStateSet(getOrCreateWidgetStateSet(),6);
#else
    style->setupPopupStateSet(getOrCreateWidgetStateSet(),6);
#endif
    style->setupClipStateSet(_extents, getOrCreateWidgetStateSet());


    // render before the subgraph
    setGraphicsSubgraph(-1, _transform.get());

    // render after the subgraph
    setGraphicsSubgraph(1, style->createDepthSetPanel(_extents));
}
