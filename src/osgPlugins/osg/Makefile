#!smake
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
        LightSource.cpp\
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
        Transform.cpp\
        Transparency.cpp\

TARGET_BASENAME = osgdb_osg

TARGET_LIB_FILES = lib$(TARGET_BASENAME).$(SO_EXT)

LIB = $(OSGHOME)/lib/osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

TARGET_LOADER_FILES = osgPlugins/$(TARGET_BASENAME).$(SO_EXT)

LIBS    =  $(GL_LIBS) -losg -losgDB

C++FLAGS += -I. -I$(OSGHOME)/include 
LDFLAGS += -L$(OSGHOME)/lib

include $(OSGHOME)/Make/makerules



