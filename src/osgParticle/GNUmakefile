TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
	Emitter.cpp\
	ExplosionEffect.cpp\
	ExplosionDebrisEffect.cpp\
	SmokeEffect.cpp\
	SmokeTrailEffect.cpp\
	FireEffect.cpp\
	FluidFrictionOperator.cpp\
	FluidProgram.cpp\
	ModularEmitter.cpp\
	ModularProgram.cpp\
	MultiSegmentPlacer.cpp\
	Particle.cpp\
	ParticleEffect.cpp\
	ParticleProcessor.cpp\
	ParticleSystem.cpp\
	PrecipitationEffect.cpp\
	ConnectedParticleSystem.cpp\
	ParticleSystemUpdater.cpp\
	Program.cpp\
	Version.cpp\


LIBS += -losg -losgDB -losgUtil $(GL_LIBS) $(OTHER_LIBS) $(DYNAMICLIBRARYLIB)
DEF  += -DOSGPARTICLE_LIBRARY

TARGET_BASENAME = osgParticle
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules
