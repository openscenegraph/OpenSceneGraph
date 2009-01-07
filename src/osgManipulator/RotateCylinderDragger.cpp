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

#include <osgManipulator/RotateCylinderDragger>
#include <osgManipulator/Command>
#include <osgManipulator/CommandManager>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Material>


using namespace osgManipulator;

RotateCylinderDragger::RotateCylinderDragger()
{
    _projector = new CylinderPlaneProjector();
    setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    setPickColor(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
}

RotateCylinderDragger::~RotateCylinderDragger()
{
}

bool RotateCylinderDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
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

                _startLocalToWorld = _projector->getLocalToWorld();
                _startWorldToLocal = _projector->getWorldToLocal();

                if (_projector->isPointInFront(pointer, _startLocalToWorld))
                    _projector->setFront(true);
                else
                    _projector->setFront(false);

                osg::Vec3d projectedPoint;
                if (_projector->project(pointer, projectedPoint))
                {
                    // Generate the motion command.
                    osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
                    cmd->setStage(MotionCommand::START);
                    cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);

                    // Dispatch command.
                    if (_commandManager)
                    {
                        _commandManager->addSelectionsToCommand(*cmd, *getParentDragger());
                        _commandManager->dispatch(*cmd);
                    }

                    // Set color to pick color.
                    setMaterialColor(_pickColor,*this);

                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevRotation = osg::Quat();
                    _prevPtOnCylinder = _projector->isProjectionOnCylinder();

                    aa.requestRedraw();
                }
                return true; 
            }
            
        // Pick move.
        case (osgGA::GUIEventAdapter::DRAG):
            {
                // Get the LocalToWorld matrix for this node and set it for the projector.
                osg::Matrix localToWorld = osg::Matrix(_prevRotation) * _startLocalToWorld;
                _projector->setLocalToWorld(localToWorld);

                osg::Vec3d projectedPoint;
                if (_projector->project(pointer, projectedPoint))
                {
                    osg::Vec3d prevProjectedPoint = _prevWorldProjPt * _projector->getWorldToLocal();
                    osg::Quat  deltaRotation = _projector->getRotation(prevProjectedPoint, _prevPtOnCylinder,
                                                                      projectedPoint, _projector->isProjectionOnCylinder());
                    osg::Quat rotation = deltaRotation * _prevRotation;

                    // Generate the motion command.
                    osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
                    cmd->setStage(MotionCommand::MOVE);
                    cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);
                    cmd->setRotation(rotation);

                    // Dispatch command.
                    if (_commandManager)
                    {
                        _commandManager->addSelectionsToCommand(*cmd, *getParentDragger());
                        _commandManager->dispatch(*cmd);
                    }

                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevRotation = rotation;
                    _prevPtOnCylinder = _projector->isProjectionOnCylinder();
                    aa.requestRedraw();
                }
                return true; 
            }
            
        // Pick finish.
        case (osgGA::GUIEventAdapter::RELEASE):
            {
                osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();

                cmd->setStage(MotionCommand::FINISH);
                cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);
                    
                // Dispatch command.
                if (_commandManager)
                {
                    _commandManager->addSelectionsToCommand(*cmd, *getParentDragger());
                    _commandManager->dispatch(*cmd);
                }

                // Reset color.
                setMaterialColor(_color,*this);
                
                aa.requestRedraw();

                return true;
            }
        default:
            return false;
    }
}

void RotateCylinderDragger::setupDefaultGeometry()
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(new osg::ShapeDrawable(const_cast<osg::Cylinder*>(_projector->getCylinder())));
    addChild(geode);
}
