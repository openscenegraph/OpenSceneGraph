TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPathCallback.cpp\
	AnimationPath.cpp\
	AzimElevationSector.cpp\
	AzimSector.cpp\
	Billboard.cpp\
	BlendFunc.cpp\
	BlinkSequence.cpp\
	ClipNode.cpp\
	ClipPlane.cpp\
        ClusterCullingCallback.cpp\
	ConeSector.cpp\
        ConvexPlanarOccluder.cpp\
        ConvexPlanarPolygon.cpp\
        CoordinateSystemNode.cpp\
	CullFace.cpp\
	DataInputStream.cpp\
	DataOutputStream.cpp\
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
	Node.cpp\
	Object.cpp\
	OccluderNode.cpp\
	PagedLOD.cpp\
	Point.cpp\
	PolygonOffset.cpp\
	PositionAttitudeTransform.cpp\
	PrimitiveSet.cpp\
	ReaderWriterIVE.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	StateSet.cpp\
	Switch.cpp\
	TexEnvCombine.cpp\
	TexEnv.cpp\
	TexGen.cpp\
	TexGenNode.cpp\
	TexMat.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	Texture.cpp\
	TextureCubeMap.cpp\
	Transform.cpp\
	VertexProgram.cpp\
	VisibilityGroup.cpp\

LIBS   += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = ive
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
