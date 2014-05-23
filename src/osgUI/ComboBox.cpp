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


#include <osgUI/ComboBox>
#include <osgText/String>
#include <osgText/Font>
#include <osgText/Text>
#include <osg/Notify>
#include <osg/io_utils>

using namespace osgUI;

ComboBox::ComboBox()
{
}

ComboBox::ComboBox(const osgUI::ComboBox& combobox, const osg::CopyOp& copyop):
    Widget(combobox, copyop),
    _items(combobox._items)
{
}

bool ComboBox::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    // OSG_NOTICE<<"ComboBox::handleImplementation"<<std::endl;

    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::SCROLL):
            if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_DOWN)
            {
                if (getCurrentItem()<getNumItems()-1) setCurrentItem(getCurrentItem()+1);
                return true;
            }
            else if (ea->getScrollingMotion()==osgGA::GUIEventAdapter::SCROLL_UP)
            {
                if (getCurrentItem()>0) setCurrentItem(getCurrentItem()-1);
                return true;
            }
            break;
        case(osgGA::GUIEventAdapter::KEYDOWN):
            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Down)
            {
                if (getCurrentItem()<getNumItems()-1) setCurrentItem(getCurrentItem()+1);
                return true;
            }
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Up)
            {
                if (getCurrentItem()>0) setCurrentItem(getCurrentItem()-1);
                return true;
            }

            break;
        case(osgGA::GUIEventAdapter::PUSH):
            OSG_NOTICE<<"Button pressed "<<std::endl;
            break;
        case(osgGA::GUIEventAdapter::RELEASE):
            OSG_NOTICE<<"Button release "<<std::endl;
            break;
        default:
            break;
    }

    return false;
}

void ComboBox::enterImplementation()
{
    OSG_NOTICE<<"ComboBox enter"<<std::endl;
}


void ComboBox::leaveImplementation()
{
    OSG_NOTICE<<"ComboBox leave"<<std::endl;
}

void ComboBox::setCurrentItem(unsigned int i)
{
    OSG_NOTICE << "ComboBox::setCurrentItem("<<i<<")"<<std::endl;
    _currentItem = i;
    if (_switch.valid()) _switch->setSingleChildOn(_currentItem);
}


void ComboBox::createGraphicsImplementation()
{
    Style* style = (getStyle()!=0) ? getStyle() : Style::instance().get();

    _switch = new osg::Switch;

    if (!_items.empty())
    {
        for(Items::iterator itr = _items.begin();
            itr != _items.end();
            ++itr)
        {
            Item* item = itr->get();
            OSG_NOTICE<<"Creating item "<<item->getText()<<", "<<item->getColor()<<std::endl;
            osg::ref_ptr<osg::Group> group = new osg::Group;
            if (item->getColor().a()!=0.0f) group->addChild( style->createPanel(_extents, item->getColor()) );
            if (!item->getText().empty()) group->addChild( style->createText(_extents, getAlignmentSettings(), getTextSettings(), item->getText()) );
            _switch->addChild(group.get());
        }
    }
    else
    {
        _switch->addChild( style->createPanel(_extents, osg::Vec4(1.0f,1.0f,1.0f,1.0f)) );
    }

    _switch->setSingleChildOn(_currentItem);

    style->setupClipStateSet(_extents, getOrCreateStateSet());
    setGraphicsSubgraph(_switch.get());
}
