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

#include <osgManipulator/Constraint>
#include <osgManipulator/Command>
#include <osg/Vec2d>

#include <math.h>

using namespace osgManipulator;

namespace
{

double round_to_nearest_int(double x) { return floor(x+0.5); }

osg::Vec3d snap_point_to_grid(const osg::Vec3d& point, const osg::Vec3d& origin, const osg::Vec3d& spacing)
{
    osg::Vec3d scale;
    scale[0] = spacing[0] ? round_to_nearest_int((point[0] - origin[0]) / spacing[0]) : 1.0;
    scale[1] = spacing[1] ? round_to_nearest_int((point[1] - origin[1]) / spacing[1]) : 1.0;
    scale[2] = spacing[2] ? round_to_nearest_int((point[2] - origin[2]) / spacing[2]) : 1.0;
    osg::Vec3d snappedPoint = origin;
    snappedPoint += osg::Vec3(scale[0]*spacing[0],scale[1]*spacing[1],scale[2]*spacing[2]);
    return snappedPoint;
}

}


void Constraint::computeLocalToWorldAndWorldToLocal() const
{
    osg::NodePath pathToRoot;
    computeNodePathToRoot(const_cast<osg::Node&>(getReferenceNode()),pathToRoot);
    _localToWorld = osg::computeLocalToWorld(pathToRoot);
    _worldToLocal = osg::computeWorldToLocal(pathToRoot);
}

GridConstraint::GridConstraint(osg::Node& refNode, const osg::Vec3d& origin, const osg::Vec3d& spacing)
    : Constraint(refNode), _origin(origin), _spacing(spacing)
{
}

bool GridConstraint::constrain(TranslateInLineCommand& command) const
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
        computeLocalToWorldAndWorldToLocal();
    else if (command.getStage() == osgManipulator::MotionCommand::FINISH)
        return true;

    osg::Vec3d translatedPoint = command.getLineStart() + command.getTranslation();
    osg::Vec3d localTranslatedPoint = (osg::Vec3d(translatedPoint)
                                       * command.getLocalToWorld() * getWorldToLocal());
    osg::Vec3d newLocalTranslatedPoint = snap_point_to_grid(localTranslatedPoint,
                                                            _origin,
                                                            _spacing);
    command.setTranslation(newLocalTranslatedPoint * getLocalToWorld() * command.getWorldToLocal() - command.getLineStart());

    return true;
}

bool GridConstraint::constrain(TranslateInPlaneCommand& command) const
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
        computeLocalToWorldAndWorldToLocal();
    else if (command.getStage() == osgManipulator::MotionCommand::FINISH)
        return true;

    osg::Matrix commandToConstraint = command.getLocalToWorld() * getWorldToLocal();
    osg::Matrix constraintToCommand = getLocalToWorld() * command.getWorldToLocal();

    // Snap the reference point to grid.
    osg::Vec3d localRefPoint = command.getReferencePoint() * commandToConstraint;
    osg::Vec3d snappedLocalRefPoint = snap_point_to_grid(localRefPoint, _origin, _spacing);
    osg::Vec3d snappedCmdRefPoint = snappedLocalRefPoint * constraintToCommand;

    // Snap the translated point to grid.
    osg::Vec3d translatedPoint = snappedCmdRefPoint + command.getTranslation();
    osg::Vec3d localTranslatedPoint = osg::Vec3d(translatedPoint) * commandToConstraint;
    osg::Vec3d newLocalTranslatedPoint = snap_point_to_grid(localTranslatedPoint, _origin, _spacing);

    // Set the snapped translation.
    command.setTranslation(newLocalTranslatedPoint * constraintToCommand - snappedCmdRefPoint);

    return true;
}

bool GridConstraint::constrain(Scale1DCommand& command) const
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
        computeLocalToWorldAndWorldToLocal();
    else if (command.getStage() == osgManipulator::MotionCommand::FINISH)
        return true;

    double scaledPoint = (command.getReferencePoint() - command.getScaleCenter()) * command.getScale() + command.getScaleCenter();

    osg::Matrix constraintToCommand = getLocalToWorld() * command.getWorldToLocal();
    osg::Vec3d commandOrigin = _origin * constraintToCommand;
    osg::Vec3d commandSpacing = (_origin + _spacing) * constraintToCommand - commandOrigin;

    double spacingFactor = commandSpacing[0] ? round_to_nearest_int((scaledPoint-commandOrigin[0])/commandSpacing[0]) : 1.0;

    double snappedScaledPoint = commandOrigin[0] + commandSpacing[0] * spacingFactor;

    double denom = (command.getReferencePoint() - command.getScaleCenter());
    double snappedScale = (denom) ? (snappedScaledPoint - command.getScaleCenter()) / denom : 1.0;
    if (snappedScale < command.getMinScale()) snappedScale = command.getMinScale();

    command.setScale(snappedScale);
    return true;
}

bool GridConstraint::constrain(Scale2DCommand& command) const
{
    if (command.getStage() == osgManipulator::MotionCommand::START)
        computeLocalToWorldAndWorldToLocal();
    else if (command.getStage() == osgManipulator::MotionCommand::FINISH)
        return true;

    osg::Vec2d scaledPoint = command.getReferencePoint() - command.getScaleCenter();
    scaledPoint[0] *= command.getScale()[0];
    scaledPoint[1] *= command.getScale()[1];
    scaledPoint += command.getScaleCenter();

    osg::Matrix constraintToCommand = getLocalToWorld() * command.getWorldToLocal();
    osg::Vec3d commandOrigin = _origin * constraintToCommand;
    osg::Vec3d commandSpacing = (_origin + _spacing) * constraintToCommand - commandOrigin;

    osg::Vec2d spacingFactor;
    spacingFactor[0] = commandSpacing[0] ? round_to_nearest_int((scaledPoint[0] - commandOrigin[0])/commandSpacing[0]) : 1.0;
    spacingFactor[1] = commandSpacing[2] ? round_to_nearest_int((scaledPoint[1] - commandOrigin[2])/commandSpacing[2]) : 1.0;

    osg::Vec2d snappedScaledPoint = (osg::Vec2d(commandOrigin[0],commandOrigin[2])
                                     + osg::Vec2d(commandSpacing[0]*spacingFactor[0],
                                                  commandSpacing[2]*spacingFactor[1]));

    osg::Vec2d denom = command.getReferencePoint() - command.getScaleCenter();
    osg::Vec2d snappedScale;
    snappedScale[0] = denom[0] ? (snappedScaledPoint[0] - command.getScaleCenter()[0]) / denom[0] : 1.0;
    snappedScale[1] = denom[1] ? (snappedScaledPoint[1] - command.getScaleCenter()[1]) / denom[1] : 1.0;

    if (snappedScale[0] < command.getMinScale()[0]) snappedScale[0] = command.getMinScale()[0];
    if (snappedScale[1] < command.getMinScale()[1]) snappedScale[1] = command.getMinScale()[1];

    command.setScale(snappedScale);
    return true;
}

bool GridConstraint::constrain(ScaleUniformCommand&) const
{
    // Can you correctly snap a ScaleUniformCommand using a Grid constraint that has
    // different spacings in the three axis??
    return false;
}
