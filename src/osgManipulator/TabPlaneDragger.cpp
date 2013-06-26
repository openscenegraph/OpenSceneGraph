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

#include <osgManipulator/TabPlaneDragger>
#include <osgManipulator/AntiSquish>

#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/LineWidth>
#include <osg/Quat>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/AutoTransform>

using namespace osgManipulator;

namespace
{

osg::Node* createHandleNode(Scale2DDragger* cornerScaleDragger, float handleScaleFactor, bool twosided)
{
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = osg::Vec3(cornerScaleDragger->getTopLeftHandlePosition()[0],0.0,cornerScaleDragger->getTopLeftHandlePosition()[1]) * handleScaleFactor;
    (*vertices)[1] = osg::Vec3(cornerScaleDragger->getBottomLeftHandlePosition()[0],0.0,cornerScaleDragger->getBottomLeftHandlePosition()[1]) * handleScaleFactor;
    (*vertices)[2] = osg::Vec3(cornerScaleDragger->getBottomRightHandlePosition()[0],0.0,cornerScaleDragger->getBottomRightHandlePosition()[1]) * handleScaleFactor;
    (*vertices)[3] = osg::Vec3(cornerScaleDragger->getTopRightHandlePosition()[0],0.0,cornerScaleDragger->getTopRightHandlePosition()[1]) * handleScaleFactor;

    osg::Geometry* geometry = new osg::Geometry();
    geometry->setVertexArray(vertices);
    geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,vertices->size()));

    osg::Vec3Array* normals = new osg::Vec3Array;
    normals->push_back(osg::Vec3(0.0,1.0,0.0));
    geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

    osg::Geode* geode = new osg::Geode;
    geode->setName("Dragger Handle");
    geode->addDrawable(geometry);

    if (!twosided)
    {
        osg::CullFace* cullface = new osg::CullFace;
        cullface->setMode(osg::CullFace::FRONT);
        geode->getOrCreateStateSet()->setAttribute(cullface, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
        geode->getOrCreateStateSet()->setMode(GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    }

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    return geode;
}

osg::Node* createHandleScene(const osg::Vec3& pos, osg::Node* handleNode, float handleScaleFactor)
{
    osg::AutoTransform *at = new osg::AutoTransform;
    at->setPosition(pos);
    at->setPivotPoint(pos * handleScaleFactor);
    at->setAutoScaleToScreen(true);
    at->addChild(handleNode);

    AntiSquish* as = new AntiSquish;
    as->setPivot(pos);
    as->addChild(at);

    return as;
}

void createCornerScaleDraggerGeometry(Scale2DDragger* cornerScaleDragger, osg::Node* handleNode, float handleScaleFactor)
{
    // Create a top left box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(cornerScaleDragger->getTopLeftHandlePosition()[0],
                                                             0.0,cornerScaleDragger->getTopLeftHandlePosition()[1]),
                                                   handleNode, handleScaleFactor);
        cornerScaleDragger->addChild(handleScene);
        cornerScaleDragger->setTopLeftHandleNode(*handleScene);
    }

    // Create a bottom left box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(cornerScaleDragger->getBottomLeftHandlePosition()[0],
                                                             0.0,cornerScaleDragger->getBottomLeftHandlePosition()[1]),
                                                   handleNode, handleScaleFactor);
        cornerScaleDragger->addChild(handleScene);
        cornerScaleDragger->setBottomLeftHandleNode(*handleScene);
    }

    // Create a bottom right box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(cornerScaleDragger->getBottomRightHandlePosition()[0],
                                                             0.0,cornerScaleDragger->getBottomRightHandlePosition()[1]),
                                                   handleNode, handleScaleFactor);
        cornerScaleDragger->addChild(handleScene);
        cornerScaleDragger->setBottomRightHandleNode(*handleScene);
    }

    // Create a top right box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(cornerScaleDragger->getTopRightHandlePosition()[0],
                                                             0.0,cornerScaleDragger->getTopRightHandlePosition()[1]),
                                                    handleNode, handleScaleFactor);
        cornerScaleDragger->addChild(handleScene);
        cornerScaleDragger->setTopRightHandleNode(*handleScene);
    }
}

void createEdgeScaleDraggerGeometry(Scale1DDragger* horzEdgeScaleDragger, Scale1DDragger* vertEdgeScaleDragger,
                                    osg::Node* handleNode, float handleScaleFactor)
{
    // Create a left box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(horzEdgeScaleDragger->getLeftHandlePosition(),0.0,0.0),
                                                   handleNode, handleScaleFactor);
        horzEdgeScaleDragger->addChild(handleScene);
        horzEdgeScaleDragger->setLeftHandleNode(*handleScene);
    }

    // Create a right box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(horzEdgeScaleDragger->getRightHandlePosition(),0.0,0.0),
                                                   handleNode, handleScaleFactor);
        horzEdgeScaleDragger->addChild(handleScene);
        horzEdgeScaleDragger->setRightHandleNode(*handleScene);
    }

    // Create a top box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(vertEdgeScaleDragger->getLeftHandlePosition(),0.0,0.0),
                                                   handleNode, handleScaleFactor);
        vertEdgeScaleDragger->addChild(handleScene);
        vertEdgeScaleDragger->setLeftHandleNode(*handleScene);
    }

    // Create a bottom box.
    {
        osg::Node* handleScene = createHandleScene(osg::Vec3(vertEdgeScaleDragger->getRightHandlePosition(),0.0,0.0),
                                                   handleNode, handleScaleFactor);
        vertEdgeScaleDragger->addChild(handleScene);
        vertEdgeScaleDragger->setRightHandleNode(*handleScene);
    }

    osg::Quat rotation; rotation.makeRotate(osg::Vec3(0.0f, 0.0f, 1.0f), osg::Vec3(1.0f, 0.0f, 0.0f));
    vertEdgeScaleDragger->setMatrix(osg::Matrix(rotation));
}

void createTranslateDraggerGeometry(Scale2DDragger* cornerScaleDragger, TranslatePlaneDragger* translateDragger)
{
    // Create a polygon.
    {
        osg::Geode* geode = new osg::Geode;
        osg::Geometry* geometry = new osg::Geometry();

        osg::Vec3Array* vertices = new osg::Vec3Array(4);
        (*vertices)[0] = osg::Vec3(cornerScaleDragger->getTopLeftHandlePosition()[0],0.0,cornerScaleDragger->getTopLeftHandlePosition()[1]);
        (*vertices)[1] = osg::Vec3(cornerScaleDragger->getBottomLeftHandlePosition()[0],0.0,cornerScaleDragger->getBottomLeftHandlePosition()[1]);
        (*vertices)[2] = osg::Vec3(cornerScaleDragger->getBottomRightHandlePosition()[0],0.0,cornerScaleDragger->getBottomRightHandlePosition()[1]);
        (*vertices)[3] = osg::Vec3(cornerScaleDragger->getTopRightHandlePosition()[0],0.0,cornerScaleDragger->getTopRightHandlePosition()[1]);

        geometry->setVertexArray(vertices);
        geometry->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,vertices->size()));

        osg::Vec3Array* normals = new osg::Vec3Array;
        normals->push_back(osg::Vec3(0.0,1.0,0.0));
        geometry->setNormalArray(normals, osg::Array::BIND_OVERALL);

        geode->addDrawable(geometry);

        osg::PolygonMode* polymode = new osg::PolygonMode;
        polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
        geode->getOrCreateStateSet()->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

        geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        translateDragger->getTranslate2DDragger()->addChild(geode);
    }

}

}

TabPlaneDragger::TabPlaneDragger( float handleScaleFactor )
          :_handleScaleFactor( handleScaleFactor )
{
    _cornerScaleDragger = new Scale2DDragger(Scale2DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT);
    addChild(_cornerScaleDragger.get());
    addDragger(_cornerScaleDragger.get());

    _horzEdgeScaleDragger = new Scale1DDragger(Scale1DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT);
    addChild(_horzEdgeScaleDragger.get());
    addDragger(_horzEdgeScaleDragger.get());

    _vertEdgeScaleDragger = new Scale1DDragger(Scale1DDragger::SCALE_WITH_OPPOSITE_HANDLE_AS_PIVOT);
    addChild(_vertEdgeScaleDragger.get());
    addDragger(_vertEdgeScaleDragger.get());

    _translateDragger = new TranslatePlaneDragger();
    _translateDragger->setColor(osg::Vec4(0.7f, 0.7f, 0.7f, 1.0f));
    addChild(_translateDragger.get());
    addDragger(_translateDragger.get());

    setParentDragger(getParentDragger());
}

TabPlaneDragger::~TabPlaneDragger()
{
}

bool TabPlaneDragger::handle(const PointerInfo& pointer, const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    if (ea.getButtonMask() & osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON) return false;

    // Check if the dragger node is in the nodepath.
    if (!pointer.contains(this)) return false;

    // Since the translate plane and the handleNode lie on the same plane the hit could've been on either one. But we
    // need to handle the scaling draggers before the translation. Check if the node path has the scaling nodes else
    // check for the scaling nodes in next hit.
    if (_cornerScaleDragger->handle(pointer, ea, aa))
        return true;
    if (_horzEdgeScaleDragger->handle(pointer, ea, aa))
        return true;
    if (_vertEdgeScaleDragger->handle(pointer, ea, aa))
        return true;

    PointerInfo nextPointer(pointer);
    nextPointer.next();

    while (!nextPointer.completed())
    {
        if (_cornerScaleDragger->handle(nextPointer, ea, aa))
            return true;
        if (_horzEdgeScaleDragger->handle(nextPointer, ea, aa))
            return true;
        if (_vertEdgeScaleDragger->handle(nextPointer, ea, aa))
            return true;

        nextPointer.next();
    }

    if (_translateDragger->handle(pointer, ea, aa))
        return true;

    return false;
}

void TabPlaneDragger::setupDefaultGeometry(bool twoSidedHandle)
{
    osg::ref_ptr<osg::Node> handleNode = createHandleNode(_cornerScaleDragger.get(), _handleScaleFactor, twoSidedHandle);

    createCornerScaleDraggerGeometry(_cornerScaleDragger.get(), handleNode.get(), _handleScaleFactor);
    createEdgeScaleDraggerGeometry(_horzEdgeScaleDragger.get(),_vertEdgeScaleDragger.get(),handleNode.get(),_handleScaleFactor);
    createTranslateDraggerGeometry(_cornerScaleDragger.get(), _translateDragger.get());
}


