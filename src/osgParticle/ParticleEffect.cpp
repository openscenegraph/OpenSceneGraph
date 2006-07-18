/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgParticle/ParticleEffect>
#include <osgParticle/ParticleSystemUpdater>
#include <osg/Geode>

using namespace osgParticle;

ParticleEffect::ParticleEffect(const ParticleEffect& copy, const osg::CopyOp& copyop):
    osg::Group(copy,copyop),
    _automaticSetup(copy._automaticSetup),
    _useLocalParticleSystem(copy._useLocalParticleSystem),
    _textureFileName(copy._textureFileName),
    _defaultParticleTemplate(copy._defaultParticleTemplate),
    _position(copy._position),
    _scale(copy._scale),
    _intensity(copy._intensity),
    _startTime(copy._startTime),
    _emitterDuration(copy._emitterDuration),
    _wind(copy._wind)
{
    if (_automaticSetup) buildEffect();
}

void ParticleEffect::setUseLocalParticleSystem(bool local)
{
    if (_useLocalParticleSystem==local) return;
    
    _useLocalParticleSystem = local;

    if (_automaticSetup) buildEffect();
}

void ParticleEffect::setTextureFileName(const std::string& filename)
{
    _textureFileName = filename;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setDefaultParticleTemplate(const Particle& p)
{
    _defaultParticleTemplate = p;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setPosition(const osg::Vec3& position)
{
    if (_position==position) return;

    _position = position;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setScale(float scale)
{
    if (_scale==scale) return;

    _scale = scale;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setIntensity(float intensity)
{
    if (_intensity==intensity) return;

    _intensity = intensity;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setStartTime(double startTime)
{
    if (_startTime==startTime) return;
    
    _startTime =startTime;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setEmitterDuration(double duration)
{
    if (_emitterDuration==duration) return;

    _emitterDuration = duration;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setParticleDuration(double duration)
{
    if (_defaultParticleTemplate.getLifeTime()==duration) return;

    _defaultParticleTemplate.setLifeTime(duration);
    
    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setWind(const osg::Vec3& wind)
{
    if (_wind==wind) return;

    _wind = wind;

    if (_automaticSetup) setUpEmitterAndProgram();
}

void ParticleEffect::setParticleSystem(ParticleSystem* ps)
{
    if (_particleSystem==ps) return;
    
    _particleSystem = ps;

    if (_automaticSetup) buildEffect();
}

void ParticleEffect::setDefaults()
{
    _useLocalParticleSystem = true;
    _scale = 1.0f;
    _intensity = 1.0f;
    _startTime = 0.0;
    _emitterDuration = 1.0;
    _wind.set(0.0f,0.0f,0.0f);
}

void ParticleEffect::buildEffect()
{
    setUpEmitterAndProgram();

    osg::ref_ptr<Emitter> emitter = getEmitter();
    osg::ref_ptr<Program> program = getProgram();
    osg::ref_ptr<ParticleSystem> particleSystem = getParticleSystem();

    if (!emitter || !particleSystem || !program) return; 


    // clear the children.
    removeChildren(0,getNumChildren());
    
    // add the emitter
    addChild(emitter.get());
    
    // add the program to update the particles
    addChild(program.get());

    // add the particle system updater.
    osg::ref_ptr<osgParticle::ParticleSystemUpdater> psu = new osgParticle::ParticleSystemUpdater;
    psu->addParticleSystem(particleSystem.get());
    addChild(psu.get());

    if (_useLocalParticleSystem)
    {
        // add the geode to the scene graph
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(particleSystem.get());
        addChild(geode);
    }
}

