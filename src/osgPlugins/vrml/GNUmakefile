#
# $Id$
#

TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
        ReaderWriterVRML2.cpp\

OPENVRML_HOME ? = /usr/local

LIBS   += $(OSG_LIBS) $(OTHER_LIBS) -L$(OPENVRML_HOME)/lib -lopenvrml

INC    += -I$(OPENVRML_HOME)/include/openvrml

TARGET_BASENAME = vrml
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules
