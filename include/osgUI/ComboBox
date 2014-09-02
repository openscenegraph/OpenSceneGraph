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

#ifndef OSGUI_COMBOBOX
#define OSGUI_COMBOBOX

#include <osgUI/Popup>
#include <osg/Switch>
#include <osgText/Text>

namespace osgUI
{

class OSGUI_EXPORT Item : public osg::Object
{
public:

    Item() : _color(1.0f,1.0f,1.0f,0.0f) {}
    Item(const std::string& str) : _text(str), _color(1.0f,1.0f,1.0f,0.0f) {}
    Item(const std::string& str, const osg::Vec4& col) : _text(str), _color(col) {}
    Item(const osg::Vec4& col) : _color(col) {}

    Item(const Item& item, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) : osg::Object(item,copyop), _text(item._text), _color(item._color) {}

    META_Object(osgUI, Item);

    void setText(const std::string& text) { _text = text; }
    std::string& getText() { return _text; }
    const std::string& getText() const { return _text; }

    void setColor(const osg::Vec4f& color) { _color = color; }
    osg::Vec4f& getColor() { return _color; }
    const osg::Vec4f& getColor() const { return _color; }

protected:
    virtual ~Item() {}

    std::string _text;
    osg::Vec4   _color;
};

class OSGUI_EXPORT ComboBox : public osgUI::Widget
{
public:
    ComboBox();
    ComboBox(const ComboBox& combobox, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, ComboBox);

    void addItem(Item* item) { _items.push_back(item); dirty(); }

    void setItem(unsigned int i, Item* item) { _items[i] = item; dirty(); }
    Item* getItem(unsigned int i) { return _items[i].get(); }
    const Item* getItem(unsigned int i) const { return _items[i].get(); }

    void clear() { _items.clear(); dirty(); }
    void removeItem(unsigned int i) { _items.erase(_items.begin()+i); dirty(); }
    unsigned int getNumItems() { return static_cast<unsigned int>(_items.size()); }

    typedef std::vector< osg::ref_ptr<Item> > Items;

    void setItems(const Items& items) { _items = items; }
    Items& getItems() { return _items; }
    const Items& getItems() const { return _items; }

    void setCurrentIndex(unsigned int i);
    unsigned int getCurrentIndex() const { return _currentIndex; }

    virtual void currrentIndexChanged(unsigned int i);
    virtual void currentIndexChangedImplementation(unsigned int i);


    virtual bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);
    virtual void createGraphicsImplementation();
    virtual void enterImplementation();
    virtual void leaveImplementation();

protected:
    virtual ~ComboBox() {}

    Items                       _items;
    unsigned int                _currentIndex;
    osg::Vec3d                  _popupItemOrigin;
    osg::Vec3d                  _popupItemSize;

    osg::ref_ptr<osg::Switch> _buttonSwitch;
    osg::ref_ptr<osg::Switch> _backgroundSwitch;
    osg::ref_ptr<osgUI::Popup> _popup;
};

}

#endif
