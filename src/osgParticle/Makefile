TOPDIR = ../..
include $(TOPDIR)/Make/makedefs

CXXFILES =\
    Emitter.cpp\
    FluidFrictionOperator.cpp\
    IO_AccelOperator.cpp\
    IO_CenteredPlacer.cpp\
    IO_Emitter.cpp\
    IO_FluidFrictionOperator.cpp\
    IO_ForceOperator.cpp\
    IO_LinearInterpolator.cpp\
    IO_ModularEmitter.cpp\
    IO_ModularProgram.cpp\
    IO_MultiSegmentPlacer.cpp\
    IO_Particle.cpp\
    IO_ParticleProcessor.cpp\
    IO_ParticleSystem.cpp\
    IO_ParticleSystemUpdater.cpp\
    IO_PointPlacer.cpp\
    IO_Program.cpp\
    IO_RadialShooter.cpp\
    IO_RandomRateCounter.cpp\
    IO_SectorPlacer.cpp\
    IO_SegmentPlacer.cpp\
    IO_VariableRateCounter.cpp\
    ModularEmitter.cpp\
    ModularProgram.cpp\
    MultiSegmentPlacer.cpp\
    Particle.cpp\
    ParticleProcessor.cpp\
    ParticleSystem.cpp\
    ParticleSystemUpdater.cpp\
    Program.cpp\
    Version.cpp\



LIBS += -losg $(OTHER_LIBS) $(DYNAMICLIBRARYLIB)
DEF  += -DOSGPARTICLE_LIBRARY

TARGET_BASENAME = osgParticle
LIB = $(LIB_PREFIX)$(TARGET_BASENAME).$(LIB_EXT)

include $(TOPDIR)/Make/makerules
