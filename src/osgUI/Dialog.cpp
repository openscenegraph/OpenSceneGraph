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


#include <osgUI/Dialog>
#include <osgUI/PushButton>
#include <osgUI/Label>
#include <osgUI/Callbacks>

#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>

using namespace osgUI;

Dialog::Dialog()
{
}

Dialog::Dialog(const osgUI::Dialog& dialog, const osg::CopyOp& copyop):
    Widget(dialog, copyop),
    _title(dialog._title)
{
}

bool Dialog::handleImplementation(osgGA::EventVisitor* /*ev*/, osgGA::Event* event)
{
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        //case(osgGA::GUIEventAdapter::KEYDOWN):
        case(osgGA::GUIEventAdapter::KEYUP):
            OSG_NOTICE<<"Key pressed : "<<ea->getKey()<<std::endl;

            break;
        default:
            break;
    }

    return false;
}

void Dialog::createGraphicsImplementation()
{
    _group = new osg::Group;

    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    float titleHeight = 10.0;
    osg::BoundingBox titleBarExtents(_extents.xMin(), _extents.yMax(), _extents.zMin(), _extents.xMax()-titleHeight, _extents.yMax()+titleHeight, _extents.zMin());
    osg::BoundingBox closeButtonExtents(_extents.xMax()-titleHeight, _extents.yMax(), _extents.zMin(), _extents.xMax(), _extents.yMax()+titleHeight, _extents.zMin());

    osg::Vec4 dialogBackgroundColor(0.84,0.82,0.82,1.0);
    osg::Vec4 dialogTitleBackgroundColor(0.5,0.5,1.0,1.0);

    _group->addChild( style->createPanel(_extents, dialogBackgroundColor) );

    _group->addChild( style->createPanel(titleBarExtents, dialogTitleBackgroundColor) );

    osg::BoundingBox dialogWithTitleExtents(_extents);
    dialogWithTitleExtents.expandBy(titleBarExtents);
    dialogWithTitleExtents.expandBy(closeButtonExtents);

    bool requiresFrame = (getFrameSettings() && getFrameSettings()->getShape()!=osgUI::FrameSettings::NO_FRAME);
    if (requiresFrame)
    {
        _group->addChild(style->createFrame(dialogWithTitleExtents, getFrameSettings(), dialogBackgroundColor));

        titleBarExtents.xMin() += getFrameSettings()->getLineWidth();
        titleBarExtents.yMax() -= getFrameSettings()->getLineWidth();
        closeButtonExtents.xMax() -= getFrameSettings()->getLineWidth();
        closeButtonExtents.yMax() -= getFrameSettings()->getLineWidth();
    }

    OSG_NOTICE<<"Dialog::_extents ("<<_extents.xMin()<<", "<<_extents.yMin()<<", "<<_extents.zMin()<<"), ("<<_extents.xMax()<<", "<<_extents.yMax()<<", "<<_extents.zMax()<<")"<<std::endl;
    OSG_NOTICE<<"Dialog::titleBarExtents ("<<titleBarExtents.xMin()<<", "<<titleBarExtents.yMin()<<", "<<titleBarExtents.zMin()<<"), ("<<titleBarExtents.xMax()<<", "<<titleBarExtents.yMax()<<", "<<titleBarExtents.zMax()<<")"<<std::endl;
    OSG_NOTICE<<"Dialog::dialogWithTitleExtents ("<<dialogWithTitleExtents.xMin()<<", "<<dialogWithTitleExtents.yMin()<<", "<<dialogWithTitleExtents.zMin()<<"), ("<<dialogWithTitleExtents.xMax()<<", "<<dialogWithTitleExtents.yMax()<<", "<<dialogWithTitleExtents.zMax()<<")"<<std::endl;

#if 0
    osg::ref_ptr<Node> node = style->createText(titleBarExtents, getAlignmentSettings(), getTextSettings(), _title);
    _titleDrawable = dynamic_cast<osgText::Text*>(node.get());
    _titleDrawable->setDataVariance(osg::Object::DYNAMIC);
    _group->addChild(_titleDrawable.get());
#endif

    osg::ref_ptr<PushButton> closeButton = new osgUI::PushButton;
    closeButton->setExtents(closeButtonExtents);
    closeButton->setText("x");
    closeButton->setAlignmentSettings(getAlignmentSettings());
    closeButton->setTextSettings(getTextSettings());
    //closeButton->setFrameSettings(getFrameSettings());
    closeButton->getOrCreateUserDataContainer()->addUserObject(new osgUI::CloseCallback("released", this));

    osg::ref_ptr<Label> titleLabel = new osgUI::Label;
    titleLabel->setExtents(titleBarExtents);
    titleLabel->setText(_title);
    titleLabel->setAlignmentSettings(getAlignmentSettings());
    titleLabel->setTextSettings(getTextSettings());
    titleLabel->setFrameSettings(getFrameSettings());
    titleLabel->getOrCreateUserDataContainer()->addUserObject(new osgUI::DragCallback);

    _group->addChild(closeButton.get());
    _group->addChild(titleLabel.get());

    style->setupDialogStateSet(getOrCreateWidgetStateSet(), 5);
    style->setupClipStateSet(dialogWithTitleExtents, getOrCreateWidgetStateSet());

    // render before the subgraph
    setGraphicsSubgraph(-1, _group.get());

    // render after the subgraph
    setGraphicsSubgraph(1, style->createDepthSetPanel(dialogWithTitleExtents));
}
