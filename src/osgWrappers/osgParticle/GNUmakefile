TOPDIR = ../../..
include $(TOPDIR)/Make/makedefs 

CXXFILES =\
	AccelOperator.cpp\
	AngularAccelOperator.cpp\
	BoxPlacer.cpp\
	CenteredPlacer.cpp\
	ConnectedParticleSystem.cpp\
	ConstantRateCounter.cpp\
	Counter.cpp\
	Emitter.cpp\
	ExplosionDebrisEffect.cpp\
	ExplosionEffect.cpp\
	FireEffect.cpp\
	FluidFrictionOperator.cpp\
	FluidProgram.cpp\
	ForceOperator.cpp\
	Interpolator.cpp\
	LinearInterpolator.cpp\
	ModularEmitter.cpp\
	ModularProgram.cpp\
	MultiSegmentPlacer.cpp\
	Operator.cpp\
	Particle.cpp\
	ParticleProcessor.cpp\
	ParticleSystem.cpp\
	ParticleSystemUpdater.cpp\
	PointPlacer.cpp\
	PrecipitationEffect.cpp\
	Program.cpp\
	RadialShooter.cpp\
	RandomRateCounter.cpp\
	SegmentPlacer.cpp\

LIBS   += -losgParticle -losg -losgIntrospection -losgUtil -losgDB  $(GL_LIBS) $(OTHER_LIBS)

OPTF = -O

TARGET_BASENAME = osgParticle
include $(TOPDIR)/Make/cygwin_wrapper_def
WRAPPER = $(WRAPPER_PREFIX)$(TARGET_BASENAME).$(PLUGIN_EXT)

include $(TOPDIR)/Make/makerules 
