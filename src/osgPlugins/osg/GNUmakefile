TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        AlphaFunc.cpp\
        AnimationPath.cpp\
        AutoTransform.cpp\
        Billboard.cpp\
        BlendFunc.cpp\
        CameraNode.cpp\
        CameraView.cpp\
        ClearNode.cpp\
        ClipNode.cpp\
        ClipPlane.cpp\
        ClusterCullingCallback.cpp\
        ColorMask.cpp\
        ConvexPlanarOccluder.cpp\
        CoordinateSystemNode.cpp\
        CullFace.cpp\
        Depth.cpp\
        Drawable.cpp\
        EllipsoidModel.cpp\
        Fog.cpp\
        FragmentProgram.cpp\
        FrontFace.cpp\
        Geode.cpp\
        Geometry.cpp\
        Group.cpp\
        Image.cpp\
        Light.cpp\
        LightModel.cpp\
        LightSource.cpp\
        LineStipple.cpp\
        LineWidth.cpp\
        LOD.cpp\
        Material.cpp\
        Matrix.cpp\
        MatrixTransform.cpp\
        NodeCallback.cpp\
        Node.cpp\
        Object.cpp\
        OccluderNode.cpp\
        PagedLOD.cpp\
        Point.cpp\
        PointSprite.cpp\
        PolygonMode.cpp\
        PolygonOffset.cpp\
        PositionAttitudeTransform.cpp\
        Program.cpp\
        Projection.cpp\
        ProxyNode.cpp\
        ReaderWriterOSG.cpp\
        Scissor.cpp\
        Sequence.cpp\
        ShadeModel.cpp\
        Shader.cpp\
        Shape.cpp\
        ShapeDrawable.cpp\
        StateSet.cpp\
        Stencil.cpp\
        Switch.cpp\
	TessellationHints.cpp\
        TexEnvCombine.cpp\
        TexEnv.cpp\
        TexEnvFilter.cpp\
        TexGen.cpp\
        TexGenNode.cpp\
        TexMat.cpp\
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


LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
