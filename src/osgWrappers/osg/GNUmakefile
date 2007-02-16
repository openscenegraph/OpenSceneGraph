TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AlphaFunc.cpp\
	AnimationPath.cpp\
	ApplicationUsage.cpp\
	ArgumentParser.cpp\
	Array.cpp\
	BufferObject.cpp\
	Camera.cpp\
	ComputeBoundsVisitor.cpp\
	DrawPixels.cpp\
	Drawable.cpp\
	Endian.cpp\
	Fog.cpp\
	FragmentProgram.cpp\
	FrameBufferObject.cpp\
	FrameStamp.cpp\
	FrontFace.cpp\
	Geode.cpp\
	Notify.cpp\
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
	ShadeModel.cpp\
	ShadowVolumeOccluder.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	State.cpp\
	StateSet.cpp\
	Stats.cpp\
	TexEnv.cpp\
	TexGen.cpp\
	TexGenNode.cpp\
	TexMat.cpp\
	Texture.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	TextureCubeMap.cpp\
	Transform.cpp\
	Uniform.cpp\
	Vec2f.cpp\
	Vec2s.cpp\
	Vec3d.cpp\
	Vec4b.cpp\
	Vec4d.cpp\
	Vec4s.cpp\
	VertexProgram.cpp\

LIBS   += -losg -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
