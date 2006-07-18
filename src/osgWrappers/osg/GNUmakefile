TOPDIR = ../../..
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
	ClampColor.cpp\
	ClearNode.cpp\
	ClipNode.cpp\
	ClipPlane.cpp\
	ClusterCullingCallback.cpp\
	CollectOccludersVisitor.cpp\
	ColorMask.cpp\
	ColorMatrix.cpp\
	ConvexPlanarOccluder.cpp\
	ConvexPlanarPolygon.cpp\
	CoordinateSystemNode.cpp\
	CopyOp.cpp\
	CullFace.cpp\
	CullSettings.cpp\
	CullStack.cpp\
	CullingSet.cpp\
	Depth.cpp\
	DisplaySettings.cpp\
	DrawPixels.cpp\
	Drawable.cpp\
	Endian.cpp\
	Fog.cpp\
	FragmentProgram.cpp\
	FrameBufferObject.cpp\
	FrameStamp.cpp\
	FrontFace.cpp\
	GL2Extensions.cpp\
	Geode.cpp\
	Geometry.cpp\
	GraphicsContext.cpp\
	GraphicsThread.cpp\
	Group.cpp\
	Image.cpp\
	ImageStream.cpp\
	LOD.cpp\
	Light.cpp\
	LightModel.cpp\
	LightSource.cpp\
	LineSegment.cpp\
	LineStipple.cpp\
	LineWidth.cpp\
	LogicOp.cpp\
	Material.cpp\
	Matrix.cpp\
	MatrixTransform.cpp\
	Matrixd.cpp\
	Matrixf.cpp\
	Multisample.cpp\
	Node.cpp\
	NodeCallback.cpp\
	NodeTrackerCallback.cpp\
	NodeVisitor.cpp\
	Notify.cpp\
	Object.cpp\
	OccluderNode.cpp\
	PagedLOD.cpp\
	Plane.cpp\
	Point.cpp\
	PointSprite.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PolygonStipple.cpp\
	Polytope.cpp\
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
	TexEnv.cpp\
	TexEnvCombine.cpp\
	TexEnvFilter.cpp\
	TexGen.cpp\
	TexGenNode.cpp\
	TexMat.cpp\
	Texture.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	TextureCubeMap.cpp\
	TextureRectangle.cpp\
	Timer.cpp\
	Transform.cpp\
	Uniform.cpp\
	Vec2.cpp\
	Vec2b.cpp\
	Vec2d.cpp\
	Vec2f.cpp\
	Vec2s.cpp\
	Vec3.cpp\
	Vec3b.cpp\
	Vec3d.cpp\
	Vec3f.cpp\
	Vec3s.cpp\
	Vec4.cpp\
	Vec4b.cpp\
	Vec4d.cpp\
	Vec4f.cpp\
	Vec4s.cpp\
	Vec4ub.cpp\
	VertexProgram.cpp\
	Viewport.cpp\
	observer_ptr.cpp\

LIBS   += -losg -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
