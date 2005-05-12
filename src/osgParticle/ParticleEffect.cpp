/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
    osg::Group(copy,copyop),/*,
    _particleSystem(copy._particleSystem.valid()?copy._particleSystem->clone():0)*/
    _useLocalParticleSystem(copy._useLocalParticleSystem),
    _defaultParticleTemplate(copy._defaultParticleTemplate),
    _position(copy._position),
    _scale(copy._scale),
    _intensity(copy._intensity),
    _startTime(copy._startTime),
    _emitterDuration(copy._emitterDuration),
    _wind(copy._wind)
{
}

void ParticleEffect::setUseLocalParticleSystem(bool local)
{
    if (_useLocalParticleSystem==local) return;
    
    _useLocalParticleSystem = local;
    buildEffect();
}

void ParticleEffect::setTextureFileName(const std::string& filename)
{
    _textureFileName = filename;
    setUpEmitterAndProgram();
}

void ParticleEffect::setDefaultParticleTemplate(const Particle& p)
{
    _defaultParticleTemplate = p;
    setUpEmitterAndProgram();
}

void ParticleEffect::setPosition(const osg::Vec3& position)
{
    if (_position==position) return;

    _position = position;
    setUpEmitterAndProgram();
}

void ParticleEffect::setScale(float scale)
{
    if (_scale==scale) return;

    _scale = scale;
    setUpEmitterAndProgram();
}

void ParticleEffect::setIntensity(float intensity)
{
    if (_intensity==intensity) return;

    _intensity = intensity;
    setUpEmitterAndProgram();
}

void ParticleEffect::setStartTime(double startTime)
{
    if (_startTime==startTime) return;
    
    _startTime =startTime;
    setUpEmitterAndProgram();
}

void ParticleEffect::setEmitterDuration(double duration)
{
    if (_emitterDuration==duration) return;

    _emitterDuration = duration;
    setUpEmitterAndProgram();
}

void ParticleEffect::setParticleDuration(double duration)
{
    if (_defaultParticleTemplate.getLifeTime()==duration) return;

    _defaultParticleTemplate.setLifeTime(duration);
    
    setUpEmitterAndProgram();
}

void ParticleEffect::setWind(const osg::Vec3& wind)
{
    if (_wind==wind) return;

    _wind = wind;
    setUpEmitterAndProgram();
}

void ParticleEffect::setParticleSystem(ParticleSystem* ps)
{
    if (_particleSystem==ps) return;
    
    _particleSystem = ps;
    buildEffect();
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

    Emitter* emitter = getEmitter();
    Program* program = getProgram();
    ParticleSystem* particleSystem = getParticleSystem();

    if (!emitter || !particleSystem || !program) return; 


    // clear the children.
    removeChild(0,getNumChildren());
    
    // add the emitter
    addChild(emitter);
    
    // add the program to update the particles
    addChild(program);

    // add the particle system updater.
    osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;
    psu->addParticleSystem(particleSystem);
    addChild(psu);

    if (_useLocalParticleSystem)
    {
        // add the geode to the scene graph
        osg::Geode *geode = new osg::Geode;
        geode->addDrawable(particleSystem);
        addChild(geode);
    }
}

