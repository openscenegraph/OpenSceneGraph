TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPathCallback.cpp\
	AnimationPath.cpp\
	AzimElevationSector.cpp\
	AzimSector.cpp\
	AutoTransform.cpp\
	Billboard.cpp\
	BlendFunc.cpp\
	BlinkSequence.cpp\
	ClipNode.cpp\
	ClipPlane.cpp\
        ClusterCullingCallback.cpp\
        CameraNode.cpp\
        CameraView.cpp\
	ConeSector.cpp\
        ConvexPlanarOccluder.cpp\
        ConvexPlanarPolygon.cpp\
        CoordinateSystemNode.cpp\
	CullFace.cpp\
	DataInputStream.cpp\
	DataOutputStream.cpp\
	Depth.cpp\
	DirectionalSector.cpp\
	DOFTransform.cpp\
	Drawable.cpp\
	DrawArrayLengths.cpp\
	DrawArrays.cpp\
	DrawElementsUByte.cpp\
	DrawElementsUInt.cpp\
	DrawElementsUShort.cpp\
	ElevationSector.cpp\
	EllipsoidModel.cpp\
	Exception.cpp\
	FragmentProgram.cpp\
	FrontFace.cpp\
	Geode.cpp\
	Geometry.cpp\
	Group.cpp\
	Image.cpp\
	Impostor.cpp\
	Light.cpp\
	LightModel.cpp\
	LightPoint.cpp\
	LightPointNode.cpp\
	LightSource.cpp\
	LineWidth.cpp\
	LOD.cpp\
	Material.cpp\
	MatrixTransform.cpp\
	MultiSwitch.cpp\
	MultiTextureControl.cpp\
	Node.cpp\
	Object.cpp\
	OccluderNode.cpp\
	PagedLOD.cpp\
	Point.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PositionAttitudeTransform.cpp\
	PrimitiveSet.cpp\
	Program.cpp\
	ProxyNode.cpp\
	ReaderWriterIVE.cpp\
	Sequence.cpp\
	Scissor.cpp\
	ShadeModel.cpp\
	Shader.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	StateSet.cpp\
	Switch.cpp\
	TexEnvCombine.cpp\
	TexEnv.cpp\
	TexGen.cpp\
	TexGenNode.cpp\
	TexMat.cpp\
	Text.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	Texture.cpp\
	TextureCubeMap.cpp\
	TextureRectangle.cpp\
	Transform.cpp\
	Uniform.cpp\
	VertexProgram.cpp\
	Viewport.cpp\
	VisibilityGroup.cpp\

LIBS   += -losgFX -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = ive
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
