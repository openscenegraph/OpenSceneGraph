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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.

#include <osgManipulator/Scale2DDragger>
#include <osgManipulator/Command>

#include <osg/ShapeDrawable>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Material>

using namespace osgManipulator;

namespace
{

osg::Vec2d computeScale(const osg::Vec3d& startProjectedPoint,
                        const osg::Vec3d& projectedPoint,
                        const osg::Vec2d& scaleCenter)
{
    osg::Vec2d scale(1.0,1.0);
    if ((startProjectedPoint[0] - scaleCenter[0]) != 0.0)
        scale[0] = (projectedPoint[0] - scaleCenter[0])/(startProjectedPoint[0] - scaleCenter[0]);
    if ((startProjectedPoint[2] - scaleCenter[1]) != 0.0)
        scale[1] = (projectedPoint[2] - scaleCenter[1])/(startProjectedPoint[2] - scaleCenter[1]);
    return scale;
}

}

Scale2DDragger::Scale2DDragger(ScaleMode scaleMode) : Dragger(), _minScale(0.001,0.001), _scaleMode(scaleMode)
{
    _projector = new PlaneProjector(osg::Plane(0.0,1.0,0.0,0.0));
    setColor(osg::Vec4(0.0, 1.0, 0.0, 1.0));
    setPickColor(osg::Vec4(1.0, 1.0, 0.0, 1.0));

    _topLeftHandlePosition.set    (-0.5,0.5);
    _bottomLeftHandlePosition.set (-0.5,-0.5);
    _bottomRightHandlePosition.set(0.5,-0.5);
    _topRightHandlePosition.set   (0.5,0.5);
}

Scale2DDragger::~Scale2DDragger()
{
}

bool Scale2DDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    // Check if the dragger node is in the nodepath.
    if (!pointer.contains(this)) return false;

    switch (ea.getEventType())
    {
        // Pick start.
        case (osgGA::GUIEventAdapter::PUSH):
            {
                // Get the LocalToWorld matrix for this node and set it for the projector.
                osg::NodePath nodePathToRoot;
                computeNodePathToRoot(*this,nodePathToRoot);
                osg::Matrix localToWorld = osg::computeLocalToWorld(nodePathToRoot);
                _projector->setLocalToWorld(localToWorld);

                if (_projector->project(pointer, _startProjectedPoint))
                {
                    _scaleCenter.set(0.0,0.0);

                    if (pointer.contains(_topLeftHandleNode.get()))
                    {
                        _referencePoint = _topLeftHandlePosition;
                        if (_scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT)
                            _scaleCenter = _bottomRightHandlePosition;
                    }
                    else if (pointer.contains(_bottomLeftHandleNode.get()))
                    {
                        _referencePoint = _bottomLeftHandlePosition;
                        if (_scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT)
                            _scaleCenter = _topRightHandlePosition;
                    }
                    else if (pointer.contains(_bottomRightHandleNode.get()))
                    {
                        _referencePoint = _bottomRightHandlePosition;
                        if (_scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT)
                            _scaleCenter = _topLeftHandlePosition;
                    }
                    else if (pointer.contains(_topRightHandleNode.get()))
                    {
                        _referencePoint = _topRightHandlePosition;
                        if (_scaleMode == SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT)
                            _scaleCenter = _bottomLeftHandlePosition;

                    }

                    // Generate the motion command.
                    osg::ref_ptr<Scale2DCommand> cmd = new Scale2DCommand();
                    cmd->setStage(MotionCommand::START);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
                    cmd->setReferencePoint(_referencePoint);

                    // Dispatch command.
                    dispatch(*cmd);

                    // Set color to pick color.
                    setMaterialColor(_pickColor,*this);

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick move.
        case (osgGA::GUIEventAdapter::DRAG):
            {
                osg::Vec3d projectedPoint;
                if (_projector->project(pointer, projectedPoint))
                {
                    // Compute scale.
                    osg::Vec2d scale = computeScale(_startProjectedPoint,projectedPoint,_scaleCenter);

                    if (scale[0] < getMinScale()[0]) scale[0] = getMinScale()[0];
                    if (scale[1] < getMinScale()[1]) scale[1] = getMinScale()[1];

                    // Generate the motion command.
                    osg::ref_ptr<Scale2DCommand> cmd = new Scale2DCommand();
                    cmd->setStage(MotionCommand::MOVE);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
                    cmd->setScale(scale);
                    cmd->setScaleCenter(_scaleCenter);
                    cmd->setReferencePoint(_referencePoint);
                    cmd->setMinScale(getMinScale());

                    // Dispatch command.
                    dispatch(*cmd);

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case (osgGA::GUIEventAdapter::RELEASE):
            {
                osg::ref_ptr<Scale2DCommand> cmd = new Scale2DCommand();

                cmd->setStage(MotionCommand::FINISH);
                cmd->setReferencePoint(_referencePoint);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                // Dispatch command.
                dispatch(*cmd);

                // Reset color.
                setMaterialColor(_color,*this);

                aa.requestRedraw();

                return true;
            }
        default:
            return false;
    }
}

void Scale2DDragger::setupDefaultGeometry()
{
    osg::Geode* lineGeode = new osg::Geode;
    // Create a line.
    {
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(4);
        (*vertices)[0].set(_topLeftHandlePosition[0],0.0,_topLeftHandlePosition[1]);
        (*vertices)[1].set(_bottomLeftHandlePosition[0],0.0,_bottomLeftHandlePosition[1]);
        (*vertices)[2].set(_bottomRightHandlePosition[0],0.0,_bottomRightHandlePosition[1]);
        (*vertices)[3].set(_topRightHandlePosition[0],0.0,_topRightHandlePosition[1]);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,vertices->size()));

        lineGeode->addDrawable(geometry);
    }

    // Turn of lighting for line and set line width.
    lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(2.0);
    lineGeode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);

    // Add line and cones to the scene.
    addChild(lineGeode);

    // Create a top left box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(_topLeftHandlePosition[0],
                                                                         0.0,_topLeftHandlePosition[1]), 0.05)));
        addChild(geode);
        setTopLeftHandleNode(*geode);
    }

    // Create a bottom left box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(_bottomLeftHandlePosition[0],
                                                                         0.0,_bottomLeftHandlePosition[1]), 0.05)));
        addChild(geode);
        setBottomLeftHandleNode(*geode);
    }

    // Create a bottom right box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(_bottomRightHandlePosition[0],
                                                                         0.0,_bottomRightHandlePosition[1]), 0.05)));
        addChild(geode);
        setBottomRightHandleNode(*geode);
    }

    // Create a top right box.
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Box(osg::Vec3(_topRightHandlePosition[0],
                                                                         0.0,_topRightHandlePosition[1]), 0.05)));
        addChild(geode);
        setTopRightHandleNode(*geode);
    }
}
