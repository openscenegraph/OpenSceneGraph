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

#include <osgParticle/ExplosionEffect>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>
#include <osgParticle/ParticleSystemUpdater>

#include <osg/Geode>

using namespace osgParticle;

ExplosionEffect::ExplosionEffect(const osg::Vec3& position, float scale, float intensity)
{
    setDefaults();
    
    _position = position;
    _scale = scale;
    _intensity = intensity;
    
    _emitterDuration = 1.0;
    _particleDuration = 0.5+0.1*_scale;
    
    buildEffect();
}

ExplosionEffect::ExplosionEffect(const ExplosionEffect& copy, const osg::CopyOp& copyop):
    ParticleEffect(copy,copyop)
{
}

void ExplosionEffect::setDefaults()
{
    ParticleEffect::setDefaults();
    
    _emitterDuration = 1.0;
    _particleDuration = 0.5+0.1*_scale;
}


void ExplosionEffect::setUpEmitterAndProgram()
{
    // set up particle system
    if (!_particleSystem)
    {
        _particleSystem = new osgParticle::ParticleSystem;
    }

    if (_particleSystem.valid())
    {
        _particleSystem->setDefaultAttributes("Images/smoke.rgb", false, false);

        osgParticle::Particle& ptemplate = _particleSystem->getDefaultParticleTemplate();

        ptemplate.setLifeTime(_particleDuration);

        float radius = 0.4f*_scale; 
        float density = 1.2f; // 1.0kg/m^3

        // the following ranges set the envelope of the respective 
        // graphical properties in time.
        ptemplate.setSizeRange(osgParticle::rangef(radius*0.75f, radius*3.0f));
        ptemplate.setAlphaRange(osgParticle::rangef(0.1f, 1.0f));
        ptemplate.setColorRange(osgParticle::rangev4(
            osg::Vec4(1.0f, 0.8f, 0.2f, 1.0f), 
            osg::Vec4(1.0f, 0.4f, 0.1f, 0.0f)));

        // these are physical properties of the particle
        ptemplate.setRadius(radius);    // 5 cm wide particles
        ptemplate.setMass(density*osg::PI*4.0f*radius*radius*radius/3.0f);

    } 


    // set up emitter
    if (!_emitter)
    {
        _emitter = new osgParticle::ModularEmitter;
        _emitter->setCounter(new osgParticle::RandomRateCounter);
        _emitter->setPlacer(new osgParticle::SectorPlacer);
        _emitter->setShooter(new osgParticle::RadialShooter);
    }

    if (_emitter.valid())
    {
        _emitter->setParticleSystem(_particleSystem.get());
        _emitter->setReferenceFrame(_useLocalParticleSystem?
                                    osgParticle::ParticleProcessor::ABSOLUTE_RF:
                                    osgParticle::ParticleProcessor::RELATIVE_RF);

        _emitter->setStartTime(_startTime);
        _emitter->setLifeTime(_emitterDuration);
        _emitter->setEndless(false);

        osgParticle::RandomRateCounter* counter = dynamic_cast<osgParticle::RandomRateCounter*>(_emitter->getCounter());
        if (counter)
        {
            counter->setRateRange(50*_intensity,100*_intensity);
        }

        osgParticle::SectorPlacer* placer = dynamic_cast<osgParticle::SectorPlacer*>(_emitter->getPlacer());
        if (placer)
        {
            placer->setCenter(_position);
            placer->setRadiusRange(0.0f*_scale,0.25f*_scale);
        }

        osgParticle::RadialShooter* shooter = dynamic_cast<osgParticle::RadialShooter*>(_emitter->getShooter());
        if (shooter)
        {
            shooter->setThetaRange(0.0f, osg::PI_2);
            shooter->setInitialSpeedRange(1.0f*_scale,10.0f*_scale);
        }
    }

    // set up the program
    if (!_program)
    {        
        _program = new osgParticle::FluidProgram;
    }
    
    if (_program.valid())
    {
        _program->setParticleSystem(_particleSystem.get());
        _program->setWind(_wind);
    }
    
}
    
