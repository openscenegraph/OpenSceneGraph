TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
        ApplicationUsage.cpp\
        ArgumentParser.cpp\
        AutoTransform.cpp\
	Array.cpp\
	Billboard.cpp\
	BoundingBox.cpp\
	BoundingSphere.cpp\
	BlendFunc.cpp\
	Camera.cpp\
	ClipNode.cpp\
	ClipPlane.cpp\
	ColorMask.cpp\
	ColorMatrix.cpp\
	CollectOccludersVisitor.cpp\
	ConvexPlanarPolygon.cpp\
	ConvexPlanarOccluder.cpp\
	CopyOp.cpp\
	CullFace.cpp\
	CullingSet.cpp\
	CullStack.cpp\
	Depth.cpp\
	DisplaySettings.cpp\
	DOFTransform.cpp\
	Drawable.cpp\
	DrawPixels.cpp\
	ClearNode.cpp\
	Fog.cpp\
	FrameStamp.cpp\
	FrontFace.cpp\
	GLExtensions.cpp\
	Geometry.cpp\
	GeoSet.cpp\
	GeoSet_ogl.cpp\
	Geode.cpp\
	Group.cpp\
	Image.cpp\
	Impostor.cpp\
	ImpostorSprite.cpp\
	LOD.cpp\
	Light.cpp\
	LightModel.cpp\
	LightSource.cpp\
	LineSegment.cpp\
	LineStipple.cpp\
	LineWidth.cpp\
	Material.cpp\
	Matrix.cpp\
	MatrixTransform.cpp\
	Node.cpp\
	NodeCallback.cpp\
	NodeVisitor.cpp\
	Notify.cpp\
	Object.cpp\
	OccluderNode.cpp\
	Point.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PolygonStipple.cpp\
	PositionAttitudeTransform.cpp\
	PrimitiveSet.cpp\
	Projection.cpp\
	Quat.cpp\
	Referenced.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	ShadowVolumeOccluder.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	State.cpp\
	StateSet.cpp\
	Stencil.cpp\
	Switch.cpp\
	TexEnv.cpp\
	TexEnvCombine.cpp\
	TexGen.cpp\
	TexMat.cpp\
	Texture.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	TextureCubeMap.cpp\
	TextureRectangle.cpp\
	Timer.cpp\
	Transform.cpp\
	UnitTestFramework.cpp\
	Version.cpp\
	Vec3.cpp\
	VertexProgram.cpp\
	Viewport.cpp\

DEF     += -DSG_LIBRARY

LIBS    += $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
