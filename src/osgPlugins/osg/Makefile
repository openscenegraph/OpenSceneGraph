TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        AlphaFunc.cpp\
        Billboard.cpp\
        BlendFunc.cpp\
        ClipPlane.cpp\
        ColorMask.cpp\
        ConvexPlanerOccluder.cpp\
        CullFace.cpp\
        Depth.cpp\
        DOFTransform.cpp\
        Drawable.cpp\
        ClearNode.cpp\
        Fog.cpp\
        FrontFace.cpp\
        Geode.cpp\
        Geometry.cpp\
        GeoSet.cpp\
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
        Point.cpp\
        PolygonMode.cpp\
        PolygonOffset.cpp\
        PositionAttitudeTransform.cpp\
        Projection.cpp\
        ReaderWriterOSG.cpp\
        ShadeModel.cpp\
        StateSet.cpp\
        Sequence.cpp\
        Stencil.cpp\
        Switch.cpp\
        TexEnv.cpp\
        TexGen.cpp\
        TexMat.cpp\
        Texture.cpp\
        Transform.cpp\
        TextureCubeMap.cpp\


LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
