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

#include <osgParticle/FireEffect>

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

FireEffect::FireEffect(const osg::Vec3& position, float scale, float intensity)
{
    setDefaults();
    
    _position = position;
    _scale = scale;
    _intensity = intensity;
        
    buildEffect();
}

FireEffect::FireEffect(const FireEffect& copy, const osg::CopyOp& copyop):
    ParticleEffect(copy,copyop)
{
}

void FireEffect::setDefaults()
{
    ParticleEffect::setDefaults();
}

void FireEffect::setUpEmitterAndProgram()
{
    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;
    ps->setDefaultAttributes("Images/smoke.rgb", true, false);

    _particleSystem = ps;
//    _particleSystem->setUseIntialViewMatrix(true);

    // set up the emitter
    {
        osgParticle::ModularEmitter *emitter = new osgParticle::ModularEmitter;
        emitter->setParticleSystem(ps);
        emitter->setReferenceFrame(osgParticle::ParticleProcessor::ABSOLUTE_RF);


        osgParticle::Particle ptemplate;

        ptemplate.setLifeTime(2);        // 3 seconds of life

        // the following ranges set the envelope of the respective 
        // graphical properties in time.
        ptemplate.setSizeRange(osgParticle::rangef(0.75f, 3.0f));
        ptemplate.setAlphaRange(osgParticle::rangef(0.0f, 1.0f));
        ptemplate.setColorRange(osgParticle::rangev4(
            osg::Vec4(1, 1.0f, 0.2f, 1.0f), 
            osg::Vec4(1, 0.0f, 0.f, 0.0f)));

        // these are physical properties of the particle
        ptemplate.setRadius(0.05f*_scale);    // 5 cm wide particles
        ptemplate.setMass(0.01f*_scale);    // 10g heavy

        // assign the particle template to the system.
        ps->setDefaultParticleTemplate(ptemplate);

        osgParticle::RandomRateCounter* counter = new osgParticle::RandomRateCounter;
        counter->setRateRange(1*_intensity*_scale,10*_intensity*_scale);    // generate 1000 particles per second
        emitter->setCounter(counter);

        osgParticle::SectorPlacer* placer = new osgParticle::SectorPlacer;
        placer->setCenter(_position);
        placer->setRadiusRange(0.0f*_scale,0.25f*_scale);
        emitter->setPlacer(placer);

        osgParticle::RadialShooter* shooter = new osgParticle::RadialShooter;
        shooter->setThetaRange(0.0f, osg::PI_4);
        shooter->setInitialSpeedRange(0.0f*_scale,0.0f*_scale);
        emitter->setShooter(shooter);

        emitter->setStartTime(_startTime);

        _emitter = emitter;
    }


    // set up the program
    {
        osgParticle::ModularProgram *program = new osgParticle::ModularProgram;
        program->setParticleSystem(ps);

        // create an operator that simulates the gravity acceleration.
        osgParticle::AccelOperator *op1 = new osgParticle::AccelOperator;
        op1->setToGravity(-0.2);
        program->addOperator(op1);

        // let's add a fluid operator to simulate air friction.
        osgParticle::FluidFrictionOperator *op3 = new osgParticle::FluidFrictionOperator;
        op3->setFluidToAir();
        program->addOperator(op3);

        // add the program to the scene graph
        _program = program;
    }
}
