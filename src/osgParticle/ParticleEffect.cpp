/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
    osg::Group(copy,copyop)/*,
    _particleSystem(copy._particleSystem.valid()?copy._particleSystem->clone():0)*/
{
}

void ParticleEffect::setPosition(const osg::Vec3& position)
{
    _position = position;
}

void ParticleEffect::setScale(float scale)
{
    _scale = scale;
}

void ParticleEffect::setIntensity(float intensity)
{
    _intensity = intensity;
}

void ParticleEffect::setStartTime(double startTime)
{
    _startTime = startTime;
}

void ParticleEffect::setDuration(double duration)
{
    _duration = duration;
}


void ParticleEffect::setDefaults()
{
    _scale = 1.0f;
    _intensity = 1.0f;
    _startTime = 0.0;
    _duration = 1.0;
    _direction.set(0.0f,0.0f,1.0f);
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

    // add the geode to the scene graph
    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(particleSystem);
    addChild(geode);
}

