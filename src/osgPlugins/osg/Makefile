TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        AlphaFunc.cpp\
        Billboard.cpp\
        ClipPlane.cpp\
        ColorMask.cpp\
        ConvexPlanerOccluder.cpp\
        CullFace.cpp\
        Depth.cpp\
        Drawable.cpp\
        EarthSky.cpp\
        Fog.cpp\
        FrontFace.cpp\
        Geode.cpp\
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
        Node.cpp\
        Object.cpp\
        OccluderNode.cpp\
        Point.cpp\
        PolygonMode.cpp\
        PolygonOffset.cpp\
        Projection.cpp\
        ReaderWriterOSG.cpp\
        ShadeModel.cpp\
        StateSet.cpp\
        Stencil.cpp\
        Switch.cpp\
        TexEnv.cpp\
        TexGen.cpp\
        TexMat.cpp\
        Texture.cpp\
        Transform.cpp\
        Transparency.cpp\

LIBS   += $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osg
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
