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

#include <osgManipulator/Translate2DDragger>
#include <osgManipulator/Command>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Material>

using namespace osgManipulator;

Translate2DDragger::Translate2DDragger()
{
    _projector = new PlaneProjector(osg::Plane(0.0,1.0,0.0,0.0));
    _polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
    setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    setPickColor(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
}

Translate2DDragger::Translate2DDragger(const osg::Plane& plane)
{
    _projector = new PlaneProjector(plane);
    _polygonOffset = new osg::PolygonOffset(-1.0f,-1.0f);
    setColor(osg::Vec4(0.0f, 1.0f, 0.0f, 1.0f));
    setPickColor(osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
}

Translate2DDragger::~Translate2DDragger()
{
}

bool Translate2DDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
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
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

                    cmd->setStage(MotionCommand::START);
                    cmd->setReferencePoint(_startProjectedPoint);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                    // Dispatch command.
                    dispatch(*cmd);

                    // Set color to pick color.
                    setMaterialColor(_pickColor,*this);
                    getOrCreateStateSet()->setAttributeAndModes(_polygonOffset.get(), osg::StateAttribute::ON);

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
                    // Generate the motion command.
                    osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

                    cmd->setStage(MotionCommand::MOVE);
                    cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());
                    cmd->setTranslation(projectedPoint - _startProjectedPoint);
                    cmd->setReferencePoint(_startProjectedPoint);

                    // Dispatch command.
                    dispatch(*cmd);

                    aa.requestRedraw();
                }
                return true;
            }

        // Pick finish.
        case (osgGA::GUIEventAdapter::RELEASE):
            {
                osg::ref_ptr<TranslateInPlaneCommand> cmd = new TranslateInPlaneCommand(_projector->getPlane());

                    cmd->setStage(MotionCommand::FINISH);
                cmd->setReferencePoint(_startProjectedPoint);
                cmd->setLocalToWorldAndWorldToLocal(_projector->getLocalToWorld(),_projector->getWorldToLocal());

                // Dispatch command.
                dispatch(*cmd);

                // Reset color.
                setMaterialColor(_color,*this);
                getOrCreateStateSet()->removeAttribute(_polygonOffset.get());

                aa.requestRedraw();

                return true;
            }
        default:
            return false;
    }
}

void Translate2DDragger::setupDefaultGeometry()
{
    // Create a line.
    osg::Geode* lineGeode = new osg::Geode;
    {
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(2);
        (*vertices)[0] = osg::Vec3(0.0f,0.0f,-0.5f);
        (*vertices)[1] = osg::Vec3(0.0f,0.0f,0.5f);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINES,0,2));

        lineGeode->addDrawable(geometry);
    }

    // Turn of lighting for line and set line width.
    osg::LineWidth* linewidth = new osg::LineWidth();
    linewidth->setWidth(2.0f);
    lineGeode->getOrCreateStateSet()->setAttributeAndModes(linewidth, osg::StateAttribute::ON);
    lineGeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::Geode* geode = new osg::Geode;

    // Create left cone.
    {
        osg::Cone* cone = new osg::Cone (osg::Vec3(0.0f, 0.0f, -0.5f), 0.025f, 0.10f);
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f,0.0f,-1.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
        cone->setRotation(rotation);
        geode->addDrawable(new osg::ShapeDrawable(cone));
    }

    // Create right cone.
    {
        osg::Cone* cone = new osg::Cone (osg::Vec3(0.0f, 0.0f, 0.5f), 0.025f, 0.10f);
        geode->addDrawable(new osg::ShapeDrawable(cone));
    }

    // Create an invisible cylinder for picking the line.
    {
        osg::Cylinder* cylinder = new osg::Cylinder (osg::Vec3(0.0f,0.0f,0.0f), 0.015f, 1.0f);
        osg::Drawable* drawable = new osg::ShapeDrawable(cylinder);
        setDrawableToAlwaysCull(*drawable);
        geode->addDrawable(drawable);
    }

    // MatrixTransform to rotate the geometry according to the normal of the plane.
    osg::MatrixTransform* xform = new osg::MatrixTransform;

    // Create an arrow in the X axis.
    {
        osg::MatrixTransform* arrow = new osg::MatrixTransform;
        arrow->addChild(lineGeode);
        arrow->addChild(geode);

        // Rotate X-axis arrow appropriately.
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(1.0f, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f));
        arrow->setMatrix(osg::Matrix(rotation));

        xform->addChild(arrow);
    }

    // Create an arrow in the Z axis.
    {
        osg::Group* arrow = new osg::Group;
        arrow->addChild(lineGeode);
        arrow->addChild(geode);

        xform->addChild(arrow);
    }

    // Rotate the xform so that the geometry lies on the plane.
    {
        osg::Vec3 normal = _projector->getPlane().getNormal(); normal.normalize();
        osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 1.0f, 0.0f), normal);
        xform->setMatrix(osg::Matrix(rotation));
    }

    addChild(xform);
}
