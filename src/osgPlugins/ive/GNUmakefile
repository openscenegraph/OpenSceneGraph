TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AnimationPath.cpp\
	AnimationPathCallback.cpp\
	Billboard.cpp\
	BlendFunc.cpp\
	CullFace.cpp\
	DataInputStream.cpp\
	DataOutputStream.cpp\
	DrawArrayLengths.cpp\
	DrawArrays.cpp\
	DrawElementsUShort.cpp\
	Drawable.cpp\
	Exception.cpp\
	Geode.cpp\
	Geometry.cpp\
	Group.cpp\
	Image.cpp\
	LOD.cpp\
	Light.cpp\
	LightSource.cpp\
	Material.cpp\
	MatrixTransform.cpp\
	Node.cpp\
	Object.cpp\
	PositionAttitudeTransform.cpp\
	PrimitiveSet.cpp\
	Sequence.cpp\
	StateSet.cpp\
	TexEnv.cpp\
	TexEnvCombine.cpp\
	TexGen.cpp\
	Texture.cpp\
	Texture2D.cpp\
	TextureCubeMap.cpp\
	Transform.cpp\
	ReaderWriterIVE.cpp\

LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = ive
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
