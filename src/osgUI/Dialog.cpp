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

bool Dialog::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    OSG_NOTICE<<"Dialog::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::KEYDOWN):
            OSG_NOTICE<<"Key pressed : "<<ea->getKey()<<std::endl;

            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Escape)
            {
                close();
                dirty();
                return true;
            }

            break;
        default:
            break;
    }

    return false;
}

void Dialog::close()
{
    if (_transform.valid()) _transform->setNodeMask(0x0);
}

void Dialog::open()
{
    if (_transform.valid()) _transform->setNodeMask(0xffffffff);
}

void Dialog::createGraphicsImplementation()
{

    if (_titleDrawable.valid())
    {
        OSG_NOTICE<<"Dialog::createGraphicsImplementation() updating existing TextDrawable"<<std::endl;
        _titleDrawable->setText(_title);
        _graphicsInitialized = true;
    }
    else
    {
        OSG_NOTICE<<"Dialog::createGraphicsImplementation()"<<std::endl;

        Widget::createGraphicsImplementation();

        _transform = new osg::PositionAttitudeTransform;
        addChild(_transform.get());

        Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

        float titleHeight = 1.0;
        osg::BoundingBox titleBarExents(_extents.xMin(), _extents.yMax(), _extents.zMin(), _extents.xMax(), _extents.yMax()+titleHeight, _extents.zMin());

        osg::ref_ptr<Node> node = style->createText(titleBarExents, getAlignmentSettings(), getTextSettings(), _title);
        _titleDrawable = dynamic_cast<osgText::Text*>(node.get());
        _titleDrawable->setDataVariance(osg::Object::DYNAMIC);
        _transform->addChild(_titleDrawable.get());

        osg::Vec4 dialogBackgroundColor(0.8,0.8,0.8,1.0);
        osg::Vec4 dialogTitleBackgroundColor(0.5,0.5,1.0,1.0);

        _transform->addChild( style->createPanel(_extents, dialogBackgroundColor) );
        _transform->addChild( style->createPanel(titleBarExents, dialogTitleBackgroundColor) );
    }
}
