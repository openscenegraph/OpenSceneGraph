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

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Material>


using namespace osgManipulator;

namespace
{

//------------------------------------------------------------------------------
osg::Geometry* createDiskGeometry(float radius, float offset, float z, unsigned int numSegments
   )
{
   const float angleDelta = 2.0f*osg::PI/float(numSegments);
   const unsigned int numPoints = (numSegments+1) * 2;
   float angle = 0.0f;
   osg::Vec3Array* vertexArray = new osg::Vec3Array(numPoints);
   osg::Vec3Array* normalArray = new osg::Vec3Array(numPoints);
   unsigned int p = 0;
   for(unsigned int i = 0; i < numSegments; ++i,angle+=angleDelta)
   {
      float c = cosf(angle);
      float s = sinf(angle);
      // Outer point
      (*vertexArray)[p].set(radius*c, radius*s, z);
      (*normalArray)[p].set(0.0, 0.0, -1.0);
      ++p;
      // Inner point
      (*vertexArray)[p].set((radius-offset)*c, (radius-offset)*s, z);
      (*normalArray)[p].set(0.0, 0.0, -1.0);
      ++p;
   }
   // do last points by hand to ensure no round off errors.
   (*vertexArray)[p] = (*vertexArray)[0];
   (*normalArray)[p] = (*normalArray)[0];
   ++p;
   (*vertexArray)[p] = (*vertexArray)[1];
   (*normalArray)[p] = (*normalArray)[1];

   osg::Geometry* geometry = new osg::Geometry;
   geometry->setVertexArray(vertexArray);
   geometry->setNormalArray(normalArray, osg::Array::BIND_PER_VERTEX);
   geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP, 0, vertexArray->size()));
   return geometry;
}

}

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
                    dispatch(*cmd);

                    // Set color to pick color.
                    setMaterialColor(_pickColor,*this);

                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevRotation = osg::Quat();

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
                    osg::Quat  deltaRotation = _projector->getRotation(prevProjectedPoint,
                                                                      projectedPoint);
                    osg::Quat rotation = deltaRotation * _prevRotation;

                    // Generate the motion command.
                    osg::ref_ptr<Rotate3DCommand> cmd = new Rotate3DCommand();
                    cmd->setStage(MotionCommand::MOVE);
                    cmd->setLocalToWorldAndWorldToLocal(_startLocalToWorld,_startWorldToLocal);
                    cmd->setRotation(rotation);

                    // Dispatch command.
                    dispatch(*cmd);

                    _prevWorldProjPt = projectedPoint * _projector->getLocalToWorld();
                    _prevRotation = rotation;
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

void RotateCylinderDragger::setupDefaultGeometry()
{
    osg::Geode* geode = new osg::Geode;
    {
        osg::TessellationHints* hints = new osg::TessellationHints;
        hints->setCreateTop(false);
        hints->setCreateBottom(false);
        hints->setCreateBackFace(false);

        float radius    = 1.0f;
        float height    = 0.1f;
        float thickness = 0.1f;

        // outer cylinder
        osg::Cylinder* cylinder = new osg::Cylinder;
        cylinder->setHeight(height);
        cylinder->setRadius(radius);
        osg::ShapeDrawable* cylinderDrawable = new osg::ShapeDrawable(cylinder, hints);
        geode->addDrawable(cylinderDrawable);

        // inner cylinder
        osg::Cylinder* cylinder1 = const_cast<osg::Cylinder*>(_projector->getCylinder());
        cylinder1->setHeight(height);
        cylinder1->setRadius(radius-thickness);
        osg::ShapeDrawable* cylinderDrawable1 = new osg::ShapeDrawable(cylinder1, hints);
        geode->addDrawable(cylinderDrawable1);

        // top
        geode->addDrawable(createDiskGeometry(radius, thickness,  height/2, 100));
        // bottom
        geode->addDrawable(createDiskGeometry(radius, thickness, -height/2, 100));
    }
    addChild(geode);
}
