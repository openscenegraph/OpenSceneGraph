TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	IO_AccelOperator.cpp\
	IO_AngularAccelOperator.cpp\
	IO_BoxPlacer.cpp\
	IO_CenteredPlacer.cpp\
	IO_ConnectedParticleSystem.cpp\
        IO_ConstantRateCounter.cpp\
	IO_Emitter.cpp\
	IO_FluidFrictionOperator.cpp\
	IO_ForceOperator.cpp\
	IO_LinearInterpolator.cpp\
	IO_ModularEmitter.cpp\
	IO_ModularProgram.cpp\
	IO_MultiSegmentPlacer.cpp\
	IO_Particle.cpp\
	IO_ParticleEffect.cpp\
	IO_ParticleProcessor.cpp\
	IO_ParticleSystem.cpp\
	IO_ParticleSystemUpdater.cpp\
	IO_PointPlacer.cpp\
	IO_Program.cpp\
	IO_RadialShooter.cpp\
	IO_RandomRateCounter.cpp\
	IO_SectorPlacer.cpp\
	IO_SegmentPlacer.cpp\
	IO_SmokeEffect.cpp\
	IO_SmokeTrailEffect.cpp\
	IO_FireEffect.cpp\
	IO_ExplosionEffect.cpp\
	IO_ExplosionDebrisEffect.cpp\
	IO_VariableRateCounter.cpp\


LIBS   += -losgParticle $(OSG_LIBS) $(OTHER_LIBS)

TARGET_BASENAME = osgParticle
include $(TOPDIR)/Make/cygwin_plugin_def
PLUGIN = $(PLUGIN_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
