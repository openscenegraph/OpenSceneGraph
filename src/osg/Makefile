TOPDIR = ../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
	Billboard.cpp\
	BoundingBox.cpp\
	BoundingSphere.cpp\
	Camera.cpp\
	ClipPlane.cpp \
	ColorMask.cpp \
	ColorMatrix.cpp \
	CopyOp.cpp \
	CullFace.cpp\
	Depth.cpp \
	DisplaySettings.cpp\
	Drawable.cpp\
	EarthSky.cpp\
	Fog.cpp\
	FrameStamp.cpp\
	FrontFace.cpp\
	Geode.cpp\
	GeoSet.cpp\
	GeoSet_ogl.cpp\
	GLExtensions.cpp\
	Group.cpp\
	Image.cpp\
	Impostor.cpp\
	ImpostorSprite.cpp\
	Light.cpp\
	LightModel.cpp\
	LightSource.cpp\
	LineSegment.cpp\
	LineStipple.cpp\
	LineWidth.cpp\
	LOD.cpp\
	Material.cpp\
	Matrix.cpp\
	Node.cpp\
	NodeCallback.cpp\
	NodeVisitor.cpp\
	Notify.cpp\
	Object.cpp\
	Point.cpp\
	PolygonMode.cpp\
	PolygonOffset.cpp\
	PositionAttitudeTransform.cpp\
	Quat.cpp\
	ShadeModel.cpp\
	State.cpp\
	StateSet.cpp\
	Stencil.cpp \
	Switch.cpp\
	TexEnv.cpp\
	TexGen.cpp\
	TexMat.cpp\
	Texture.cpp\
	Timer.cpp\
	Transform.cpp\
	Transparency.cpp\
	Version.cpp\
	Viewport.cpp\


DEF     += -DSG_LIBRARY

LIBS    += $(GL_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules 
