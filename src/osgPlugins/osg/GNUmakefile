TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        AlphaFunc.cpp\
        AnimationPath.cpp\
        Billboard.cpp\
        BlendFunc.cpp\
        ClipPlane.cpp\
        ColorMask.cpp\
        ConvexPlanarOccluder.cpp\
        CullFace.cpp\
        ClipNode.cpp\
        Depth.cpp\
        Drawable.cpp\
        ClearNode.cpp\
        Fog.cpp\
        FragmentProgram.cpp\
        FrontFace.cpp\
        Geode.cpp\
        Geometry.cpp\
        Group.cpp\
        Image.cpp\
        Impostor.cpp\
        Light.cpp\
        LightModel.cpp\
        LightSource.cpp\
        LineStipple.cpp\
        LineWidth.cpp\
        LOD.cpp\
        Material.cpp\
        Matrix.cpp\
        MatrixTransform.cpp\
        Node.cpp\
        Object.cpp\
        OccluderNode.cpp\
        PagedLOD.cpp\
        Point.cpp\
        PolygonMode.cpp\
        PolygonOffset.cpp\
        PositionAttitudeTransform.cpp\
        Projection.cpp\
        ReaderWriterOSG.cpp\
        ShadeModel.cpp\
        Shape.cpp\
        ShapeDrawable.cpp\
        StateSet.cpp\
        Sequence.cpp\
        Stencil.cpp\
        Switch.cpp\
	TessellationHints.cpp\
        TexEnv.cpp\
        TexEnvCombine.cpp\
        TexGen.cpp\
        TexMat.cpp\
        Texture.cpp\
        Texture1D.cpp\
        Texture2D.cpp\
        Texture3D.cpp\
        TextureRectangle.cpp\
        TextureCubeMap.cpp\
        Transform.cpp\
        VertexProgram.cpp\


LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
