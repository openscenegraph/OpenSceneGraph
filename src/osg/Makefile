TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
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
	ConvexPlanerPolygon.cpp\
	ConvexPlanerOccluder.cpp\
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
	MemoryManager.cpp\
	Node.cpp\
	NodeCallback.cpp\
	NodeVisitor.cpp\
	Notify.cpp\
	Object.cpp\
	OccluderNode.cpp\
	Point.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PositionAttitudeTransform.cpp\
	Primitive.cpp\
	Projection.cpp\
	Quat.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	ShadowVolumeOccluder.cpp\
	State.cpp\
	StateSet.cpp\
	Stencil.cpp\
	Switch.cpp\
	TexEnv.cpp\
	TexEnvCombine.cpp\
	TexGen.cpp\
	TexMat.cpp\
	Texture.cpp\
	TextureBase.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	TextureCubeMap.cpp\
	Timer.cpp\
	Transform.cpp\
	Version.cpp\
	Viewport.cpp\

DEF     += -DSG_LIBRARY

LIBS    += $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
