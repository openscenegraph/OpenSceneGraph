
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGPARTICLE_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgParticle)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/AccelOperator
    ${HEADER_PATH}/AngularAccelOperator
    ${HEADER_PATH}/BoxPlacer
    ${HEADER_PATH}/CenteredPlacer
    ${HEADER_PATH}/ConnectedParticleSystem
    ${HEADER_PATH}/ConstantRateCounter
    ${HEADER_PATH}/Counter
    ${HEADER_PATH}/Emitter
    ${HEADER_PATH}/ExplosionDebrisEffect
    ${HEADER_PATH}/ExplosionEffect
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/FireEffect
    ${HEADER_PATH}/FluidFrictionOperator
    ${HEADER_PATH}/FluidProgram
    ${HEADER_PATH}/ForceOperator
    ${HEADER_PATH}/Interpolator
    ${HEADER_PATH}/LinearInterpolator
    ${HEADER_PATH}/ModularEmitter
    ${HEADER_PATH}/ModularProgram
    ${HEADER_PATH}/MultiSegmentPlacer
    ${HEADER_PATH}/Operator
    ${HEADER_PATH}/Particle
    ${HEADER_PATH}/ParticleEffect
    ${HEADER_PATH}/ParticleProcessor
    ${HEADER_PATH}/ParticleSystem
    ${HEADER_PATH}/ParticleSystemUpdater
    ${HEADER_PATH}/Placer
    ${HEADER_PATH}/PointPlacer
    ${HEADER_PATH}/PrecipitationEffect
    ${HEADER_PATH}/Program
    ${HEADER_PATH}/RadialShooter
    ${HEADER_PATH}/RandomRateCounter
    ${HEADER_PATH}/range
    ${HEADER_PATH}/SectorPlacer
    ${HEADER_PATH}/SegmentPlacer
    ${HEADER_PATH}/Shooter
    ${HEADER_PATH}/SmokeEffect
    ${HEADER_PATH}/SmokeTrailEffect
    ${HEADER_PATH}/VariableRateCounter
    ${HEADER_PATH}/Version
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ConnectedParticleSystem.cpp
    Emitter.cpp
    ExplosionDebrisEffect.cpp
    ExplosionEffect.cpp
    FireEffect.cpp
    FluidFrictionOperator.cpp
    FluidProgram.cpp
    ModularEmitter.cpp
    ModularProgram.cpp
    MultiSegmentPlacer.cpp
    Particle.cpp
    ParticleEffect.cpp
    ParticleProcessor.cpp
    ParticleSystem.cpp
    ParticleSystemUpdater.cpp
    PrecipitationEffect.cpp
    Program.cpp
    SmokeEffect.cpp
    SmokeTrailEffect.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}     
    osgUtil
    osgDB
    osg
    OpenThreads
)
LINK_CORELIB_DEFAULT(${LIB_NAME})

INCLUDE(ModuleInstall OPTIONAL)
