TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
        ApplicationUsage.cpp\
        ArgumentParser.cpp\
	Array.cpp\
        AutoTransform.cpp\
	Billboard.cpp\
	BlendColor.cpp\
	BlendEquation.cpp\
	BlendFunc.cpp\
	BoundingBox.cpp\
	BoundingSphere.cpp\
	BufferObject.cpp\
	CameraNode.cpp\
	CameraView.cpp\
	ClearNode.cpp\
	ClipNode.cpp\
	ClipPlane.cpp\
	ClampColor.cpp\
	ClusterCullingCallback.cpp\
	CollectOccludersVisitor.cpp\
	ColorMask.cpp\
	ColorMatrix.cpp\
	ConvexPlanarOccluder.cpp\
	ConvexPlanarPolygon.cpp\
	CoordinateSystemNode.cpp\
	CopyOp.cpp\
	CullFace.cpp\
	CullingSet.cpp\
	CullSettings.cpp\
	CullStack.cpp\
	Depth.cpp\
	DisplaySettings.cpp\
	Drawable.cpp\
	DrawPixels.cpp\
        dxtctool.cpp\
	Fog.cpp\
	FragmentProgram.cpp\
	FrameStamp.cpp\
	FrameBufferObject.cpp\
	FrontFace.cpp\
	Geode.cpp\
	Geometry.cpp\
	GLExtensions.cpp\
	GraphicsContext.cpp\
	GraphicsThread.cpp\
	Group.cpp\
	Image.cpp\
	ImageStream.cpp\
	Light.cpp\
	LightModel.cpp\
	LightSource.cpp\
	LineSegment.cpp\
	LineStipple.cpp\
	LineWidth.cpp\
	LOD.cpp\
	LogicOp.cpp\
	Material.cpp\
	Matrixd.cpp\
	Matrixf.cpp\
	MatrixTransform.cpp\
	Multisample.cpp\
	NodeCallback.cpp\
	NodeTrackerCallback.cpp\
	Node.cpp\
	NodeVisitor.cpp\
	Notify.cpp\
	Object.cpp\
	OccluderNode.cpp\
	PagedLOD.cpp\
	Point.cpp\
	PointSprite.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PolygonStipple.cpp\
	PositionAttitudeTransform.cpp\
	PrimitiveSet.cpp\
	Program.cpp\
	Projection.cpp\
	ProxyNode.cpp\
	Quat.cpp\
	Referenced.cpp\
	Scissor.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	Shader.cpp\
	ShadowVolumeOccluder.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	State.cpp\
	StateAttribute.cpp\
	StateSet.cpp\
	Stencil.cpp\
	Switch.cpp\
	TexEnvCombine.cpp\
	TexEnv.cpp\
	TexEnvFilter.cpp\
	TexGen.cpp\
	TexGenNode.cpp\
	TexMat.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	Texture.cpp\
	TextureCubeMap.cpp\
	TextureRectangle.cpp\
	Timer.cpp\
	Transform.cpp\
	Uniform.cpp\
	UnitTestFramework.cpp\
	Vec3.cpp\
	Version.cpp\
	VertexProgram.cpp\
	Viewport.cpp\

DEF     += -DOSG_LIBRARY

LIBS    += $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
