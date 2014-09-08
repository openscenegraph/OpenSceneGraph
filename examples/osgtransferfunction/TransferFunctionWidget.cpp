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

#include "TransferFunctionWidget.h"

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/TexGen>
#include <osg/AlphaFunc>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/io_utils>

#include <osgGA/EventVisitor>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgViewer/View>

using namespace osgUI;

TransferFunctionWidget::TransferFunctionWidget(osg::TransferFunction1D* tf):
    _min(FLT_MAX),
    _max(-FLT_MAX),
    _left(FLT_MAX),
    _right(-FLT_MAX),
    _startedDrag(false),
    _previousDragPosition(0.0f)
{
    setNumChildrenRequiringEventTraversal(1);
    setExtents(osg::BoundingBox(0.0,0.0,0.0,1.0,1.0,0.0));
    setTransferFunction(tf);
}

TransferFunctionWidget::TransferFunctionWidget(const TransferFunctionWidget& tfw, const osg::CopyOp& copyop):
    Widget(tfw,copyop)
{
    setExtents(tfw.getExtents());
    setTransferFunction(tfw.getTransferFunction());
}

void TransferFunctionWidget::setTransferFunction(const osg::TransferFunction1D* tf)
{
    if (_transferFunction==tf) return;

    _transferFunction = const_cast<osg::TransferFunction1D*>(tf);

    if (_transferFunction.valid())
    {
        osg::TransferFunction1D::ColorMap& colorMap = _transferFunction->getColorMap();
        if (colorMap.empty())
        {
            _min = FLT_MAX;
            _max = -FLT_MAX;
        }
        else
        {
            _min = colorMap.begin()->first;
            _max = colorMap.rbegin()->first;
        }
    }

    resetVisibleRange();
}

void TransferFunctionWidget::resetVisibleRange()
{
    setVisibleRange(_min, _max);
}

void TransferFunctionWidget::setVisibleRange(float left, float right)
{
    if (left<_min) left = _min;
    if (right>_max) right = _max;

    _left = left;
    _right = right;
//    OSG_NOTICE<<"setVisibleRange("<<_left<<", "<<_right<<")"<<std::endl;
    createGraphics();
}

void TransferFunctionWidget::translateVisibleRange(float delta)
{
    float new_left = _left+(_right-_left)*delta;
    float new_right = _right+(_right-_left)*delta;
    if (delta<0.0)
    {
        if (new_left<_min)
        {
            new_right += (_min-new_left);
            new_left = _min;
        }
    }
    else
    {
        if (new_right>_max)
        {
            new_left += (_max-new_right);
            new_right = _max;
        }
    }

    setVisibleRange(new_left, new_right);
}

void TransferFunctionWidget::scaleVisibleRange(float center, float delta)
{
    float scale = powf(2.0, delta);
    setVisibleRange(center+(_left-center)*scale,
                    center+(_right-center)*scale);
}


void TransferFunctionWidget::traverseImplementation(osg::NodeVisitor& nv)
{
    Widget::traverseImplementation(nv);
}

bool TransferFunctionWidget::handleImplementation(osgGA::EventVisitor* ev, osgGA::Event* event)
{
    osgGA::GUIEventAdapter* ea = event->asGUIEventAdapter();
    if (!ea) return false;

    switch(ea->getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
            // OSG_NOTICE<<"Pressed button "<<ea->getButton()<<std::endl;
            _startedDrag = false;
            if (ea->getButtonMask()==osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
            {
                osg::Vec3d position;
                if (computePositionInLocalCoordinates(ev, ea, position))
                {
                    _startedDrag = true;
                    _previousDragPosition = position.x();
                }
            }
            break;
        case(osgGA::GUIEventAdapter::RELEASE):
            // OSG_NOTICE<<"Released button "<<ea->getButton()<<std::endl;
            _startedDrag = false;
            break;
        case(osgGA::GUIEventAdapter::DRAG):
            // OSG_NOTICE<<"Dragged "<<std::endl;
            if (_startedDrag)
            {
                osg::Vec3d position;
                if (computePositionInLocalCoordinates(ev, ea, position))
                {
                    float delta = -(position.x()-_previousDragPosition);

                    _previousDragPosition = position.x();

                    translateVisibleRange(delta);
                }
            }
            break;
        case(osgGA::GUIEventAdapter::SCROLL):
        {
            osg::Vec3d position;
            if (computePositionInLocalCoordinates(ev, ea, position))
            {
                float translation = 0.0;
                float increment = 0.1;
                float scale = 1.0;
                float ratio = (1.0f+increment);
                switch(ea->getScrollingMotion())
                {
                    case(osgGA::GUIEventAdapter::SCROLL_NONE):
                        break;
                    case(osgGA::GUIEventAdapter::SCROLL_LEFT):
                        translation -= increment;
                        break;
                    case(osgGA::GUIEventAdapter::SCROLL_RIGHT):
                        translation += increment;
                        break;
                    case(osgGA::GUIEventAdapter::SCROLL_UP):
                        scale /= ratio;
                        break;
                    case(osgGA::GUIEventAdapter::SCROLL_DOWN):
                        scale *= ratio;
                        break;
                    case(osgGA::GUIEventAdapter::SCROLL_2D):
                        translation = increment*ea->getScrollingDeltaX();
                        scale = powf(ratio, increment*ea->getScrollingDeltaY());
                        break;
                }
                float center = _left+(_right-_left)*position.x();
                // OSG_NOTICE<<"translation = "<<translation<<", scale = "<<scale<<", x="<<position.x()<<", center="<<center<<std::endl;
                setVisibleRange(translation+center+(_left-center)*scale,
                                translation+center+(_right-center)*scale);

            }
            break;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            // OSG_NOTICE<<"Pressed key"<<ea->getKey()<<std::endl;
            float delta = 0.02;
            if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Left) translateVisibleRange(-delta);
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Right) translateVisibleRange(delta);
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Up) scaleVisibleRange((_left+_right)*0.5f, -delta);
            else if (ea->getKey()==osgGA::GUIEventAdapter::KEY_Down) scaleVisibleRange((_left+_right)*0.5f, delta);
            break;
        }
        case(osgGA::GUIEventAdapter::KEYUP):
            // OSG_NOTICE<<"Released key"<<ea->getKey()<<std::endl;
            if (ea->getKey()==' ' ||ea->getKey()==osgGA::GUIEventAdapter::KEY_Home) resetVisibleRange();
            break;
        default:
            break;
    }
    return false;
}

void TransferFunctionWidget::createGraphicsImplementation()
{
//    OSG_NOTICE<<"Create graphics"<<std::endl;

    typedef osg::TransferFunction1D::ColorMap ColorMap;
    ColorMap& colorMap = _transferFunction->getColorMap();
    if (colorMap.empty()) return;

    float depth = 0.0f;
    float yMax = 0.0f;

    // find yMax
    for(ColorMap::iterator itr = colorMap.begin();
        itr != colorMap.end();
        ++itr)
    {
        float y = itr->second[3];
        if (y>yMax) yMax = y;
    }

    float xScale = 1.0f/(_right-_left);
    float xOffset = -_left;
    float yScale = 1.0f/yMax;

    if (!_geode)
    {
        _geode = new osg::Geode;
        addChild(_geode.get());
    }

    {

        if (!_geometry)
        {
            _geometry = new osg::Geometry;
            _geometry->setDataVariance(osg::Geometry::DYNAMIC);
            _geometry->setUseDisplayList(false);
            _geometry->setUseVertexBufferObjects(false);

            _geode->addDrawable(_geometry.get());

            osg::ref_ptr<osg::StateSet> stateset = _geometry->getOrCreateStateSet();

            stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF | osg::StateAttribute::PROTECTED);
            stateset->setMode(GL_BLEND, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

            osg::ref_ptr<osg::AlphaFunc> alphaFunc = new osg::AlphaFunc(osg::AlphaFunc::GREATER, 0.0f);
            stateset->setAttributeAndModes(alphaFunc.get(), osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);

            osg::ref_ptr<osg::Image> image = new osg::Image;
            image->allocateImage(1,1,1, GL_RGBA, GL_UNSIGNED_BYTE);
            unsigned char* data = image->data();
            data[0] = 255;
            data[1] = 255;
            data[2] = 255;
            data[3] = 255;

            osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;
            texture->setImage(image.get());
            texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
            texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
            texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_BORDER);
            texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_BORDER);
            texture->setBorderColor(osg::Vec4(1.0f,1.0f,0.0f,0.0f));

            stateset->setTextureAttribute(0, texture.get());

            osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen;
            texgen->setMode(osg::TexGen::OBJECT_LINEAR);
            texgen->setPlane(osg::TexGen::S, osg::Plane(1.0,0.0,0.0,0.0));
            texgen->setPlane(osg::TexGen::T, osg::Plane(0.0,1.0,0.0,0.0));

            stateset->setTextureAttribute(0, texgen.get());
            stateset->setTextureMode(0, GL_TEXTURE_GEN_S, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            stateset->setTextureMode(0, GL_TEXTURE_GEN_T, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
            stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::ON | osg::StateAttribute::PROTECTED);
        }


        if (!_vertices)
        {
            _vertices = new osg::Vec3Array;
            _geometry->setVertexArray(_vertices.get());
        }

        if (!_colours)
        {
            _colours = new osg::Vec4Array;
            _geometry->setColorArray(_colours.get(), osg::Array::BIND_PER_VERTEX);
        }

        osg::Vec4 background_color(1.0f, 1.0f, 1.0f, 0.1f);

        unsigned numColumnsRequired = colorMap.size();
        _vertices->resize(0);
        _vertices->reserve(numColumnsRequired*3);
        for(ColorMap::iterator itr = colorMap.begin();
            itr != colorMap.end();
            ++itr)
        {
            float x = itr->first;
            osg::Vec4 color = itr->second;

            float y = itr->second[3];
            color[3] = 1.0f;

            _vertices->push_back(osg::Vec3((x+xOffset)*xScale, 0.0f, depth));
            _colours->push_back(color);

            _vertices->push_back(osg::Vec3((x+xOffset)*xScale, y*yScale, depth));
            _colours->push_back(color);

            _vertices->push_back(osg::Vec3((x+xOffset)*xScale, y*yScale, depth));
            _colours->push_back(background_color);

            _vertices->push_back(osg::Vec3((x+xOffset)*xScale, yMax*yScale, depth));
            _colours->push_back(background_color);
        }

        if (!_background_primitives)
        {
            _background_primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
            _geometry->addPrimitiveSet(_background_primitives.get());
        }

        if (!_historgram_primitives)
        {
            _historgram_primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
            _geometry->addPrimitiveSet(_historgram_primitives.get());
        }

        if (!_outline_primitives)
        {
            _outline_primitives = new osg::DrawElementsUShort(GL_LINE_STRIP);
            _geometry->addPrimitiveSet(_outline_primitives.get());
        }

        _background_primitives->resize(0);
        _historgram_primitives->resize(0);
        _outline_primitives->resize(0);

        for(unsigned int i=0; i<numColumnsRequired; ++i)
        {
            int iv = i*4;

            _background_primitives->push_back(iv+3);
            _background_primitives->push_back(iv+2);

            _historgram_primitives->push_back(iv+1);
            _historgram_primitives->push_back(iv+0);

            _outline_primitives->push_back(iv+1);
        }

    }

#if 0
    static bool first = true;
    if (first)
    {
        osgDB::writeNodeFile(*_geode, "test.osgt");
        first = false;
    }
#endif

    _geometry->dirtyBound();

    // make sure the general widget geometry/state is created and _graphicsInitialized reset to false
    Widget::createGraphicsImplementation();
}
