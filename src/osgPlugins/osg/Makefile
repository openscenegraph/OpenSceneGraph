#!gmake
include $(OSGHOME)/Make/makedefs

C++FILES = \
        AlphaFunc.cpp\
        Billboard.cpp\
        ClipPlane.cpp\
        ColorMask.cpp\
        ColorMatrix.cpp\
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
        Point.cpp\
        PolygonMode.cpp\
        PolygonOffset.cpp\
        ReaderWriterOSG.cpp\
        ShadeModel.cpp\
        StateSet.cpp\
        Stencil.cpp\
        Switch.cpp\
        TexEnv.cpp\
        TexGen.cpp\
        TexMat.cpp\
        Texture.cpp\
        TextureCubeMap.cpp\
        Transform.cpp\
        Transparency.cpp\

TARGET_BASENAME = osgdb_osg

LOADABLE = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(DL_EXT)
LIB=

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(DL_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB
MACOSXLIBS = -L../../../lib -losg -losgUtil -losgDB -ldl -lm -lstdc++


C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules



