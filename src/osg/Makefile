#!smake
include $(OSGHOME)/Make/makedefs

C++FILES = \
	AlphaFunc.cpp\
	AutoTransform.cpp\
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
	LightSource.cpp\
	LineSegment.cpp\
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
        

TARGET_BASENAME = osg

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

TARGET_INCLUDE_FILES = \
	osg/Notify\
	osg/AlphaFunc\
	osg/AutoTransform\
	osg/Billboard\
	osg/BoundingBox\
	osg/BoundingSphere\
	osg/BoundsChecking\
	osg/Camera\
	osg/ClipPlane\
	osg/ClippingVolume\
	osg/ColorMask\
	osg/ColorMatrix\
	osg/CopyOp\
	osg/CullFace\
	osg/Depth\
	osg/DisplaySettings\
	osg/Drawable\
	osg/EarthSky\
	osg/Export\
	osg/Fog\
	osg/FrameStamp\
	osg/FrontFace\
	osg/GL\
	osg/GLExtensions\
	osg/GLU\
	osg/GeoSet\
	osg/Geode\
	osg/Group\
	osg/Image\
	osg/Impostor\
	osg/ImpostorSprite\
	osg/LOD\
	osg/Light\
	osg/LightSource\
	osg/LineSegment\
	osg/LineWidth\
	osg/Material\
	osg/Math\
	osg/Matrix\
	osg/MemoryAdapter\
	osg/Node\
	osg/NodeCallback\
	osg/NodeVisitor\
	osg/Object\
	osg/Point\
	osg/PolygonMode\
	osg/PolygonOffset\
	osg/PositionAttitudeTransform\
	osg/Plane\
	osg/Quat\
	osg/Referenced\
	osg/ShadeModel\
	osg/State\
	osg/StateAttribute\
	osg/StateSet\
	osg/Statistics\
	osg/Stencil\
	osg/Switch\
	osg/TexEnv\
	osg/TexGen\
	osg/TexMat\
	osg/Texture\
	osg/Timer\
	osg/Transform\
	osg/Transparency\
	osg/Types\
	osg/Vec2\
	osg/Vec3\
	osg/Vec4\
	osg/Version\
	osg/Viewport\
	osg/mem_ptr\
	osg/ref_ptr\


LIBS    =  $(GL_LIBS)
MACOSXLIBS =  -lm -ldl -lstdc++

LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).$(SO_EXT)
#LIB	= $(OSGHOME)/lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I $(OSGHOME)/include -DSG_LIBRARY

#LDFLAGS += -Wl,--out-implib,./../lib/lib$(TARGET_BASENAME).a

include $(OSGHOME)/Make/makerules

