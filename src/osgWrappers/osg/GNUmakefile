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
	CullingSet.cpp\
	CullSettings.cpp\
	CullStack.cpp\
	Depth.cpp\
	DisplaySettings.cpp\
	Drawable.cpp\
	DrawPixels.cpp\
	Fog.cpp\
	FragmentProgram.cpp\
	FrameStamp.cpp\
	FrontFace.cpp\
	Geode.cpp\
	Geometry.cpp\
	Group.cpp\
	Image.cpp\
	ImageStream.cpp\
	Impostor.cpp\
	ImpostorSprite.cpp\
	Light.cpp\
	LightModel.cpp\
	LightSource.cpp\
	LineSegment.cpp\
	LineStipple.cpp\
	LineWidth.cpp\
	LOD.cpp\
	LogicOp.cpp\
	Material.cpp\
	Matrix.cpp\
	Matrixd.cpp\
	Matrixf.cpp\
	MatrixTransform.cpp\
	Multisample.cpp\
	NodeCallback.cpp\
	Node.cpp\
	NodeVisitor.cpp\
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
	RefNodePath.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	Shader.cpp\
	ShadowVolumeOccluder.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	StateAttribute.cpp\
	State.cpp\
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
	UByte4.cpp\
	Uniform.cpp\
	Vec2.cpp\
	Vec2d.cpp\
	Vec2f.cpp\
	Vec3.cpp\
	Vec3d.cpp\
	Vec3f.cpp\
	Vec4.cpp\
	Vec4d.cpp\
	Vec4f.cpp\
	VertexProgram.cpp\
	Viewport.cpp\

LIBS   += -losg -losgIntrospection $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
