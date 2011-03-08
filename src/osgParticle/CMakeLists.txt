
IF(DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGPARTICLE_LIBRARY)
ELSE()
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF()

SET(LIB_NAME osgParticle)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(TARGET_H
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
    
    ${HEADER_PATH}/CompositePlacer
    ${HEADER_PATH}/AngularDampingOperator
    ${HEADER_PATH}/DampingOperator
    ${HEADER_PATH}/ExplosionOperator
    ${HEADER_PATH}/OrbitOperator
    ${HEADER_PATH}/DomainOperator
    ${HEADER_PATH}/BounceOperator
    ${HEADER_PATH}/SinkOperator
)

# FIXME: For OS X, need flag for Framework or dylib
SET(TARGET_SRC
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
    
    DomainOperator.cpp
    BounceOperator.cpp
    SinkOperator.cpp
    ${OPENSCENEGRAPH_VERSIONINFO_RC}
)

SET(TARGET_LIBRARIES   
    osgUtil
    osgDB
    osg
    OpenThreads
)

SETUP_LIBRARY(${LIB_NAME})
