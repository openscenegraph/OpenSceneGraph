TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
	AnimationPathCallback.cpp\
	Billboard.cpp\
	BlendFunc.cpp\
        ConvexPlanarOccluder.cpp\
        ConvexPlanarPolygon.cpp\
        CoordinateSystemNode.cpp\
        ClusterCullingCallback.cpp\
	CullFace.cpp\
	DataInputStream.cpp\
	DataOutputStream.cpp\
	DOFTransform.cpp\
	DrawArrayLengths.cpp\
	DrawArrays.cpp\
	DrawElementsUByte.cpp\
	DrawElementsUShort.cpp\
	DrawElementsUInt.cpp\
	Drawable.cpp\
	EllipsoidModel.cpp\
	Exception.cpp\
	Geode.cpp\
	Geometry.cpp\
	Group.cpp\
	Image.cpp\
	Impostor.cpp\
	LOD.cpp\
	Light.cpp\
	LightSource.cpp\
	Material.cpp\
	MatrixTransform.cpp\
	MultiSwitch.cpp\
	Node.cpp\
	LineWidth.cpp\
	Object.cpp\
	OccluderNode.cpp\
	PagedLOD.cpp\
	PositionAttitudeTransform.cpp\
	PolygonOffset.cpp\
	Point.cpp\
	PrimitiveSet.cpp\
	Sequence.cpp\
	ShadeModel.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	Switch.cpp\
	StateSet.cpp\
	TexEnv.cpp\
	TexEnvCombine.cpp\
	TexGen.cpp\
	TexMat.cpp\
	Texture.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	FragmentProgram.cpp\
	VertexProgram.cpp\
	TextureCubeMap.cpp\
	Transform.cpp\
	ReaderWriterIVE.cpp\
	AzimElevationSector.cpp\
	AzimSector.cpp\
	BlinkSequence.cpp\
	ConeSector.cpp\
	DirectionalSector.cpp\
	ElevationSector.cpp\
	LightPoint.cpp\
	LightPointNode.cpp\
	VisibilityGroup.cpp\

LIBS   += -losgSim -losgText $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = ive
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
