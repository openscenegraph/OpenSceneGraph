TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
        atmosphere.cpp \
        chunk.cpp \
        lib3ds_float.cpp \
        matrix.cpp \
        quat.cpp \
        tcb.cpp \
        viewport.cpp \
        background.cpp \
        ease.cpp \
        light.cpp \
        mesh.cpp \
        readwrite.cpp \
        tracks.cpp \
        camera.cpp \
        file.cpp \
        material.cpp \
        node.cpp \
        shadow.cpp \
        vector.cpp \
        ReaderWriter3DS.cpp\


INC += -I$(THISDIR)

LIBS  += $(OSG_LIBS) $(OTHER_LIBS)

PLUGIN = osgdb_3ds.$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
