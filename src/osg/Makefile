#!smake
include ../../Make/makedefs

C++FILES = \
	AlphaFunc.cpp\
	Billboard.cpp\
	BoundingBox.cpp\
	BoundingSphere.cpp\
	Camera.cpp\
	ClipPlane.cpp \
	ColorMask.cpp \
	CullFace.cpp\
	Depth.cpp \
	Drawable.cpp\
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
	LOD.cpp\
	Material.cpp\
	Matrix.cpp\
	Node.cpp\
	NodeVisitor.cpp\
        Notify.cpp\
	Object.cpp\
        Point.cpp\
	PolygonMode.cpp\
        PolygonOffset.cpp\
        Quat.cpp\
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

TARGET_LIB_FILES = lib$(TARGET_BASENAME).so

TARGET_INCLUDE_FILES = \
	osg/AlphaFunc\
	osg/Billboard\
	osg/BoundingBox\
	osg/BoundingSphere\
	osg/BoundsChecking\
	osg/Camera\
	osg/ClipPlane\
	osg/ClippingVolume\
	osg/ColorMask\
	osg/CullFace\
	osg/Depth\
	osg/Drawable\
	osg/Export\
	osg/Fog\
	osg/FrameStamp\
	osg/FrontFace\
	osg/GL\
	osg/GLExtensions\
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
	osg/Material\
	osg/Matrix\
	osg/MemoryAdapter\
	osg/Node\
	osg/NodeCallback\
	osg/NodeVisitor\
	osg/Notify\
	osg/Object\
	osg/Point\
	osg/PolygonMode\
	osg/PolygonOffset\
	osg/Plane\
	osg/Quat\
	osg/Referenced\
	osg/State\
	osg/StateAttribute\
	osg/StateSet\
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


TARGET_DATA_FILES = \
	cessna.osg\
	cow.osg\
	dumptruck.osg\
	e-s-bike.osg\
	flight_park.fly\
	glider.osg\
	ncc1701d.osg\
	paraglider.osg\
	torus.osg\
	turtle.osg\
	Test/CullFace.osg\
	Test/Point.osg\
	Test/PolygonOffset.osg\
	Images/lz.rgb\
	Images/reflect.rgb\
	Images/tank.rgb\
	Images/tree0.rgba\
	Images/water.rgb\
	Images/white.rgb\


LIBS    =  -lGLU -lGL -lm

LIB	= ../../lib/lib$(TARGET_BASENAME).so
#LIB	= ../../lib/lib$(TARGET_BASENAME).a

C++FLAGS += -I ../../include

include ../../Make/makerules

