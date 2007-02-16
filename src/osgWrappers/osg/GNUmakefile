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
	PagedLOD.cpp\
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
	Sequence.cpp\
	ShadeModel.cpp\
	Shader.cpp\
	ShadowVolumeOccluder.cpp\
	Shape.cpp\
	ShapeDrawable.cpp\
	State.cpp\
	StateSet.cpp\
	Stats.cpp\
	StencilTwoSided.cpp\
	Switch.cpp\
	TexEnvCombine.cpp\
	TexEnvFilter.cpp\
	Texture.cpp\
	Texture1D.cpp\
	Texture2D.cpp\
	Texture3D.cpp\
	TextureCubeMap.cpp\
	TextureRectangle.cpp\
	Transform.cpp\
	Uniform.cpp\
	Vec2d.cpp\
	Vec3d.cpp\
	Vec4b.cpp\
	Vec4s.cpp\
	VertexProgram.cpp\
	View.cpp\

LIBS   += -losg -losg -losgIntrospection  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
