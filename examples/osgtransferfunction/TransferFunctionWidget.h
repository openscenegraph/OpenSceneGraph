/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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

#ifndef OSGUI_TRANSFERFUNCTIONWIDGET
#define OSGUI_TRANSFERFUNCTIONWIDGET

#include <osg/Group>
#include <osg/TransferFunction>

#include <osgUI/Widget>

namespace osgUI
{

class TransferFunctionWidget : public osgUI::Widget
{
public:
    TransferFunctionWidget(osg::TransferFunction1D* tf=0);
    TransferFunctionWidget(const TransferFunctionWidget& tfw, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Node(osgUI, TransferFunctionWidget);

    virtual void traverseImplementation(osg::NodeVisitor& nv);

    virtual bool handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event);

    void setTransferFunction(const osg::TransferFunction1D* tf);
    osg::TransferFunction1D* getTransferFunction() { return _transferFunction.get(); }
    const osg::TransferFunction1D* getTransferFunction() const { return _transferFunction.get(); }

    void resetVisibleRange();
    void setVisibleRange(float left, float right);
    void translateVisibleRange(float delta);
    void scaleVisibleRange(float center, float delta);

    virtual void createGraphicsImplementation();

protected:
    virtual ~TransferFunctionWidget() {}

    osg::ref_ptr<osg::TransferFunction1D> _transferFunction;

    osg::ref_ptr<osg::Geode>            _geode;
    osg::ref_ptr<osg::Geometry>         _geometry;
    osg::ref_ptr<osg::Vec3Array>        _vertices;
    osg::ref_ptr<osg::Vec4Array>        _colours;
    osg::ref_ptr<osg::DrawElementsUShort> _background_primitives;
    osg::ref_ptr<osg::DrawElementsUShort> _historgram_primitives;
    osg::ref_ptr<osg::DrawElementsUShort> _outline_primitives;

    float       _min;
    float       _max;
    float       _left;
    float       _right;

    bool        _startedDrag;
    float       _previousDragPosition;
};

}

#endif
