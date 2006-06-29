// ***************************************************************************
//
//   Generated automatically by genwrapper.
//   Please DO NOT EDIT this file!
//
// ***************************************************************************

#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Billboard>
#include <osg/BoundingBox>
#include <osg/CameraNode>
#include <osg/ClearNode>
#include <osg/ClipNode>
#include <osg/Drawable>
#include <osg/Geode>
#include <osg/Group>
#include <osg/LOD>
#include <osg/LightSource>
#include <osg/Matrix>
#include <osg/Matrixd>
#include <osg/Matrixf>
#include <osg/Node>
#include <osg/OccluderNode>
#include <osg/Polytope>
#include <osg/Projection>
#include <osg/State>
#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/TexGenNode>
#include <osg/Transform>
#include <osg/Vec3>
#include <osgUtil/CullVisitor>
#include <osgUtil/RenderBin>
#include <osgUtil/RenderStage>
#include <osgUtil/StateGraph>

// Must undefine IN and OUT macros defined in Windows headers
#ifdef IN
#undef IN
#endif
#ifdef OUT
#undef OUT
#endif

TYPE_NAME_ALIAS(osg::Matrix::value_type, osgUtil::CullVisitor::value_type);

BEGIN_OBJECT_REFLECTOR(osgUtil::CullVisitor)
	I_BaseType(osg::NodeVisitor);
	I_BaseType(osg::CullStack);
	I_Constructor0();
	I_Method0(osgUtil::CullVisitor *, cloneType);
	I_Method0(void, reset);
	I_Method0(osg::Vec3, getEyePoint);
	I_Method2(float, getDistanceToEyePoint, IN, const osg::Vec3 &, pos, IN, bool, withLODScale);
	I_Method2(float, getDistanceFromEyePoint, IN, const osg::Vec3 &, pos, IN, bool, withLODScale);
	I_Method1(void, apply, IN, osg::Node &, x);
	I_Method1(void, apply, IN, osg::Geode &, node);
	I_Method1(void, apply, IN, osg::Billboard &, node);
	I_Method1(void, apply, IN, osg::LightSource &, node);
	I_Method1(void, apply, IN, osg::ClipNode &, node);
	I_Method1(void, apply, IN, osg::TexGenNode &, node);
	I_Method1(void, apply, IN, osg::Group &, node);
	I_Method1(void, apply, IN, osg::Transform &, node);
	I_Method1(void, apply, IN, osg::Projection &, node);
	I_Method1(void, apply, IN, osg::Switch &, node);
	I_Method1(void, apply, IN, osg::LOD &, node);
	I_Method1(void, apply, IN, osg::ClearNode &, node);
	I_Method1(void, apply, IN, osg::CameraNode &, node);
	I_Method1(void, apply, IN, osg::OccluderNode &, node);
	I_Method1(void, setClearNode, IN, const osg::ClearNode *, earthSky);
	I_Method0(const osg::ClearNode *, getClearNode);
	I_Method1(void, pushStateSet, IN, const osg::StateSet *, ss);
	I_Method0(void, popStateSet);
	I_Method1(void, setStateGraph, IN, osgUtil::StateGraph *, rg);
	I_Method0(osgUtil::StateGraph *, getRootStateGraph);
	I_Method0(osgUtil::StateGraph *, getCurrentStateGraph);
	I_Method1(void, setRenderStage, IN, osgUtil::RenderStage *, rg);
	I_Method0(osgUtil::RenderStage *, getRenderStage);
	I_Method0(osgUtil::RenderBin *, getCurrentRenderBin);
	I_Method1(void, setCurrentRenderBin, IN, osgUtil::RenderBin *, rb);
	I_Method0(osgUtil::CullVisitor::value_type, getCalculatedNearPlane);
	I_Method0(osgUtil::CullVisitor::value_type, getCalculatedFarPlane);
	I_Method3(osgUtil::CullVisitor::value_type, computeNearestPointInFrustum, IN, const osg::Matrix &, matrix, IN, const osg::Polytope::PlaneList &, planes, IN, const osg::Drawable &, drawable);
	I_Method2(bool, updateCalculatedNearFar, IN, const osg::Matrix &, matrix, IN, const osg::BoundingBox &, bb);
	I_MethodWithDefaults3(bool, updateCalculatedNearFar, IN, const osg::Matrix &, matrix, , IN, const osg::Drawable &, drawable, , IN, bool, isBillboard, false);
	I_Method1(void, updateCalculatedNearFar, IN, const osg::Vec3 &, pos);
	I_Method2(void, addDrawable, IN, osg::Drawable *, drawable, IN, osg::RefMatrix *, matrix);
	I_Method3(void, addDrawableAndDepth, IN, osg::Drawable *, drawable, IN, osg::RefMatrix *, matrix, IN, float, depth);
	I_Method2(void, addPositionedAttribute, IN, osg::RefMatrix *, matrix, IN, const osg::StateAttribute *, attr);
	I_Method3(void, addPositionedTextureAttribute, IN, unsigned int, textureUnit, IN, osg::RefMatrix *, matrix, IN, const osg::StateAttribute *, attr);
	I_Method0(void, popProjectionMatrix);
	I_Method3(bool, clampProjectionMatrixImplementation, IN, osg::Matrixf &, projection, IN, double &, znear, IN, double &, zfar);
	I_Method3(bool, clampProjectionMatrixImplementation, IN, osg::Matrixd &, projection, IN, double &, znear, IN, double &, zfar);
	I_Method3(bool, clampProjectionMatrix, IN, osg::Matrixf &, projection, IN, osgUtil::CullVisitor::value_type &, znear, IN, osgUtil::CullVisitor::value_type &, zfar);
	I_Method3(bool, clampProjectionMatrix, IN, osg::Matrixd &, projection, IN, osgUtil::CullVisitor::value_type &, znear, IN, osgUtil::CullVisitor::value_type &, zfar);
	I_Method1(void, setState, IN, osg::State *, state);
	I_Method0(osg::State *, getState);
	I_Method0(const osg::State *, getState);
	I_ReadOnlyProperty(osgUtil::CullVisitor::value_type, CalculatedFarPlane);
	I_ReadOnlyProperty(osgUtil::CullVisitor::value_type, CalculatedNearPlane);
	I_Property(const osg::ClearNode *, ClearNode);
	I_Property(osgUtil::RenderBin *, CurrentRenderBin);
	I_ReadOnlyProperty(osgUtil::StateGraph *, CurrentStateGraph);
	I_ReadOnlyProperty(osg::Vec3, EyePoint);
	I_Property(osgUtil::RenderStage *, RenderStage);
	I_ReadOnlyProperty(osgUtil::StateGraph *, RootStateGraph);
	I_Property(osg::State *, State);
	I_WriteOnlyProperty(osgUtil::StateGraph *, StateGraph);
END_REFLECTOR

