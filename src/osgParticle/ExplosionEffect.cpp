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

#include <osgParticle/ExplosionEffect>

#include <osgParticle/ExplosionEffect>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>

#include <osg/Geode>

using namespace osgParticle;

ExplosionEffect::ExplosionEffect()
{
    setDefaults();
}

ExplosionEffect::ExplosionEffect(const ExplosionEffect& copy, const osg::CopyOp& copyop):
    ParticleEffect(copy,copyop)
{
}

void ExplosionEffect::setDefaults()
{
    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;
    ps->setDefaultAttributes("Images/particle.rgb", false, false);
    
    _particleSystem = ps;

    // set up the emitter
    {
        osgParticle::ModularEmitter *emitter = new osgParticle::ModularEmitter;
        emitter->setParticleSystem(ps);

        osgParticle::Particle ptemplate;

        ptemplate.setLifeTime(3);        // 3 seconds of life

        // the following ranges set the envelope of the respective 
        // graphical properties in time.
        ptemplate.setSizeRange(osgParticle::rangef(0.75f, 3.0f));
        ptemplate.setAlphaRange(osgParticle::rangef(0.0f, 1.0f));
        ptemplate.setColorRange(osgParticle::rangev4(
            osg::Vec4(1, 1.0f, 0.0f, 1.0f), 
            osg::Vec4(0.5, 0.5f, 0.0f, 0.0f)));

        // these are physical properties of the particle
        ptemplate.setRadius(0.1f);    // 5 cm wide particles
        ptemplate.setMass(1.0f);    // 1kg heavy

        // assign the particle template to the system.
        ps->setDefaultParticleTemplate(ptemplate);

        //osgParticle::LimitedDurationRandomRateCounter* counter = new osgParticle::LimitedDurationRandomRateCounter;
        osgParticle::RandomRateCounter* counter = new osgParticle::RandomRateCounter;
        counter->setRateRange(2000,2000);    // generate 1000 particles per second
        emitter->setCounter(counter);

        osgParticle::SectorPlacer* placer = new osgParticle::SectorPlacer;
        placer->setCenter(osg::Vec3(0.0,0.0,0.0));
        placer->setRadiusRange(0.0f,1.0f);
        emitter->setPlacer(placer);

        osgParticle::RadialShooter* shooter = new osgParticle::RadialShooter;
        shooter->setThetaRange(0.0f, osg::PI_2);
        shooter->setInitialSpeedRange(5.0f,30.0f);
        emitter->setShooter(shooter);
        
        emitter->setStartTime(0.0f);
        emitter->setResetTime(5.0f);
        emitter->setLifeTime(0.1f);
        emitter->setEndless(false);

        _emitter = emitter;
    }


    // set up the program
    {
        osgParticle::ModularProgram *program = new osgParticle::ModularProgram;
        program->setParticleSystem(ps);

        // create an operator that simulates the gravity acceleration.
        osgParticle::AccelOperator *op1 = new osgParticle::AccelOperator;
        op1->setToGravity();
        program->addOperator(op1);

        // let's add a fluid operator to simulate air friction.
        osgParticle::FluidFrictionOperator *op3 = new osgParticle::FluidFrictionOperator;
        op3->setFluidToAir();
        program->addOperator(op3);
        
        _program = program;
    }
    
    
    buildEffect();
}

void ExplosionEffect::buildEffect()
{
    // clear the children.
    removeChild(0,getNumChildren());
    
    if (!_emitter || !_particleSystem || !_program) return; 
    
    // add the emitter
    addChild(_emitter.get());
    
    // add the program to update the particles
    addChild(_program.get());

    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(_particleSystem.get());

    // add the geode to the scene graph
    addChild(geode);
}
