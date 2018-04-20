/* OpenSceneGraph example, osgparticle.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Group>
#include <osg/Geode>

#include <osgParticle/Particle>
#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>
#include <osgParticle/RandomRateCounter>
#include <osgParticle/SectorPlacer>
#include <osgParticle/RadialShooter>
#include <osgParticle/AccelOperator>
#include <osgParticle/FluidFrictionOperator>



//////////////////////////////////////////////////////////////////////////////
// CUSTOM OPERATOR CLASS
//////////////////////////////////////////////////////////////////////////////

// This class demonstrates Operator subclassing. This way you can create
// custom operators to apply your motion effects to the particles. See docs
// for more details.
class VortexOperator: public osgParticle::Operator {
public:
    VortexOperator()
        : osgParticle::Operator(), center_(0, 0, 0), axis_(0, 0, 1), intensity_(0.1f) {}

    VortexOperator(const VortexOperator &copy, const osg::CopyOp &copyop = osg::CopyOp::SHALLOW_COPY)
        : osgParticle::Operator(copy, copyop), center_(copy.center_), axis_(copy.axis_), intensity_(copy.intensity_) {}

    META_Object(osgParticle, VortexOperator);

    void setCenter(const osg::Vec3 &c)
    {
        center_ = c;
    }

    void setAxis(const osg::Vec3 &a)
    {
        axis_ = a / a.length();
    }

    // this method is called by ModularProgram before applying
    // operators on the particle set via the operate() method.
    void beginOperate(osgParticle::Program *prg)
    {
        // we have to check whether the reference frame is RELATIVE_RF to parents
        // or it's absolute; in the first case, we must transform the vectors
        // from local to world space.
        if (prg->getReferenceFrame() == osgParticle::Program::RELATIVE_RF) {
            // transform the center point (full transformation)
            xf_center_ = prg->transformLocalToWorld(center_);
            // transform the axis vector (only rotation and scale)
            xf_axis_ = prg->rotateLocalToWorld(axis_);
        } else {
            xf_center_ = center_;
            xf_axis_ = axis_;
        }
    }

    // apply a vortex-like acceleration. This code is not optimized,
    // it's here only for demonstration purposes.
    void operate(osgParticle::Particle *P, double dt)
    {
        float l = xf_axis_ * (P->getPosition() - xf_center_);
        osg::Vec3 lc = xf_center_ + xf_axis_ * l;
        osg::Vec3 R = P->getPosition() - lc;
        osg::Vec3 v = (R ^ xf_axis_) * P->getMassInv() * intensity_;

        // compute new position
        osg::Vec3 newpos = P->getPosition() + v * dt;

        // update the position of the particle without modifying its
        // velocity vector (this is unusual, normally you should call
        // the Particle::setVelocity() or Particle::addVelocity()
        // methods).
        P->setPosition(newpos);
    }

protected:
    virtual ~VortexOperator() {}

private:
    osg::Vec3 center_;
    osg::Vec3 xf_center_;
    osg::Vec3 axis_;
    osg::Vec3 xf_axis_;
    float intensity_;
};


//////////////////////////////////////////////////////////////////////////////
// SIMPLE PARTICLE SYSTEM CREATION
//////////////////////////////////////////////////////////////////////////////


osgParticle::ParticleSystem *create_simple_particle_system(osg::Group *root)
{

    // Ok folks, this is the first particle system we build; it will be
    // very simple, with no textures and no special effects, just default
    // values except for a couple of attributes.

    // First of all, we create the ParticleSystem object; it will hold
    // our particles and expose the interface for managing them; this object
    // is a Drawable, so we'll have to add it to a Geode later.

    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;

    // As for other Drawable classes, the aspect of graphical elements of
    // ParticleSystem (the particles) depends on the StateAttribute's we
    // give it. The ParticleSystem class has an helper function that let
    // us specify a set of the most common attributes: setDefaultAttributes().
    // This method can accept up to three parameters; the first is a texture
    // name (std::string), which can be empty to disable texturing, the second
    // sets whether particles have to be "emissive" (additive blending) or not;
    // the third parameter enables or disables lighting.

    ps->setDefaultAttributes("", true, false);

    // Now that our particle system is set we have to create an emitter, that is
    // an object (actually a Node descendant) that generate new particles at
    // each frame. The best choice is to use a ModularEmitter, which allow us to
    // achieve a wide variety of emitting styles by composing the emitter using
    // three objects: a "counter", a "placer" and a "shooter". The counter must
    // tell the ModularEmitter how many particles it has to create for the
    // current frame; then, the ModularEmitter creates these particles, and for
    // each new particle it instructs the placer and the shooter to set its
    // position vector and its velocity vector, respectively.
    // By default, a ModularEmitter object initializes itself with a counter of
    // type RandomRateCounter, a placer of type PointPlacer and a shooter of
    // type RadialShooter (see documentation for details). We are going to leave
    // these default objects there, but we'll modify the counter so that it
    // counts faster (more particles are emitted at each frame).

    osgParticle::ModularEmitter *emitter = new osgParticle::ModularEmitter;

    // the first thing you *MUST* do after creating an emitter is to set the
    // destination particle system, otherwise it won't know where to create
    // new particles.

    emitter->setParticleSystem(ps);

    // Ok, get a pointer to the emitter's Counter object. We could also
    // create a new RandomRateCounter object and assign it to the emitter,
    // but since the default counter is already a RandomRateCounter, we
    // just get a pointer to it and change a value.

    osgParticle::RandomRateCounter *rrc =
        static_cast<osgParticle::RandomRateCounter *>(emitter->getCounter());

    // Now set the rate range to a better value. The actual rate at each frame
    // will be chosen randomly within that range.

    rrc->setRateRange(20, 30);    // generate 20 to 30 particles per second

    // The emitter is done! Let's add it to the scene graph. The cool thing is
    // that any emitter node will take into account the accumulated local-to-world
    // matrix, so you can attach an emitter to a transform node and see it move.

    root->addChild(emitter);

    // Ok folks, we have almost finished. We don't add any particle modifier
    // here (see ModularProgram and Operator classes), so all we still need is
    // to create a Geode and add the particle system to it, so it can be
    // displayed.

    // add the ParticleSystem to the scene graph
    root->addChild(ps);

    return ps;

}



//////////////////////////////////////////////////////////////////////////////
// COMPLEX PARTICLE SYSTEM CREATION
//////////////////////////////////////////////////////////////////////////////


osgParticle::ParticleSystem *create_complex_particle_system(osg::Group *root)
{
    // Are you ready for a more complex particle system? Well, read on!

    // Now we take one step we didn't before: create a particle template.
    // A particle template is simply a Particle object for which you set
    // the desired properties (see documentation for details). When the
    // particle system has to create a new particle and it's been assigned
    // a particle template, the new particle will inherit the template's
    // properties.
    // You can even assign different particle templates to each emitter; in
    // this case, the emitter's template will override the particle system's
    // default template.

    osgParticle::Particle ptemplate;

    ptemplate.setLifeTime(3);        // 3 seconds of life

    // the following ranges set the envelope of the respective
    // graphical properties in time.
    ptemplate.setSizeRange(osgParticle::rangef(0.75f, 3.0f));
    ptemplate.setAlphaRange(osgParticle::rangef(0.0f, 1.5f));
    ptemplate.setColorRange(osgParticle::rangev4(
        osg::Vec4(1, 0.5f, 0.3f, 1.5f),
        osg::Vec4(0, 0.7f, 1.0f, 0.0f)));

    // these are physical properties of the particle
    ptemplate.setRadius(0.05f);    // 5 cm wide particles
    ptemplate.setMass(0.05f);    // 50 g heavy

    // As usual, let's create the ParticleSystem object and set its
    // default state attributes. This time we use a texture named
    // "smoke.rgb", you can find it in the data distribution of OSG.
    // We turn off the additive blending, because smoke has no self-
    // illumination.
    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;
    ps->setDefaultAttributes("Images/smoke.rgb", false, false);

    // assign the particle template to the system.
    ps->setDefaultParticleTemplate(ptemplate);

    // now we have to create an emitter; this will be a ModularEmitter, for which
    // we define a RandomRateCounter as counter, a SectorPlacer as placer, and
    // a RadialShooter as shooter.
    osgParticle::ModularEmitter *emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem(ps);

    // setup the counter
    osgParticle::RandomRateCounter *counter = new osgParticle::RandomRateCounter;
    counter->setRateRange(60, 60);
    emitter->setCounter(counter);

    // setup the placer; it will be a circle of radius 5 (the particles will
    // be placed inside this circle).
    osgParticle::SectorPlacer *placer = new osgParticle::SectorPlacer;
    placer->setCenter(8, 0, 10);
    placer->setRadiusRange(2.5, 5);
    placer->setPhiRange(0, 2 * osg::PI);    // 360 angle to make a circle
    emitter->setPlacer(placer);

    // now let's setup the shooter; we use a RadialShooter but we set the
    // initial speed to zero, because we want the particles to fall down
    // only under the effect of the gravity force. Since we se the speed
    // to zero, there is no need to setup the shooting angles.
    osgParticle::RadialShooter *shooter = new osgParticle::RadialShooter;
    shooter->setInitialSpeedRange(0, 0);
    emitter->setShooter(shooter);

    // add the emitter to the scene graph
    root->addChild(emitter);

    // WELL, we got our particle system and a nice emitter. Now we want to
    // simulate the effect of the earth gravity, so first of all we have to
    // create a Program. It is a particle processor just like the Emitter
    // class, but it allows to modify particle properties *after* they have
    // been created.
    // The ModularProgram class can be thought as a sequence of operators,
    // each one performing some actions on the particles. So, the trick is:
    // create the ModularProgram object, create one or more Operator objects,
    // add those operators to the ModularProgram, and finally add the
    // ModularProgram object to the scene graph.
    // NOTE: since the Program objects perform actions after the particles
    // have been emitted by one or more Emitter objects, all instances of
    // Program (and its descendants) should be placed *after* the instances
    // of Emitter objects in the scene graph.

    osgParticle::ModularProgram *program = new osgParticle::ModularProgram;
    program->setParticleSystem(ps);

    // create an operator that simulates the gravity acceleration.
    osgParticle::AccelOperator *op1 = new osgParticle::AccelOperator;
    op1->setToGravity();
    program->addOperator(op1);

    // now create a custom operator, we have defined it before (see
    // class VortexOperator).
    VortexOperator *op2 = new VortexOperator;
    op2->setCenter(osg::Vec3(8, 0, 0));
    program->addOperator(op2);

    // let's add a fluid operator to simulate air friction.
    osgParticle::FluidFrictionOperator *op3 = new osgParticle::FluidFrictionOperator;
    op3->setFluidToAir();
    program->addOperator(op3);

    // add the program to the scene graph
    root->addChild(program);

    // add the particle system to the scene graph.
    root->addChild(ps);

    return ps;
}



//////////////////////////////////////////////////////////////////////////////
// ANIMATED PARTICLE SYSTEM CREATION
//////////////////////////////////////////////////////////////////////////////


osgParticle::ParticleSystem *create_animated_particle_system(osg::Group *root)
{

    // Now we will create a particle system that uses two emitters to
    // display two animated particles, one showing an explosion, the other
    // a smoke cloud. A particle system can only use one texture, so
    // the animations for both particles are stored in a single bitmap.
    // The frames of the animation are stored in tiles. For each particle
    // template, the start and end tile of their animation have to be given.
    // The example file used here has 64 tiles, stored in eight rows with
    // eight images each.

    // First create a prototype for the explosion particle.
    osgParticle::Particle pexplosion;

    // The frames of the explosion particle are played from birth to
    // death of the particle. So if lifetime is one second, all 16 images
    // of the particle are shown in this second.
    pexplosion.setLifeTime(1);

    // some other particle properties just as in the last example.
    pexplosion.setSizeRange(osgParticle::rangef(0.75f, 3.0f));
    pexplosion.setAlphaRange(osgParticle::rangef(0.5f, 1.0f));
    pexplosion.setColorRange(osgParticle::rangev4(
        osg::Vec4(1, 1, 1, 1),
        osg::Vec4(1, 1, 1, 1)));
    pexplosion.setRadius(0.05f);
    pexplosion.setMass(0.05f);

    // This command sets the animation tiles to be shown for the particle.
    // The first two parameters define the tile layout of the texture image.
    // 8, 8 means the texture has eight rows of tiles with eight columns each.
    // 0, 15 defines the start and end tile
    pexplosion.setTextureTileRange(8, 8, 0, 15);

    // The smoke particle is just the same, only plays another tile range.
    osgParticle::Particle psmoke = pexplosion;
    psmoke.setTextureTileRange(8, 8, 32, 45);

    // Create a single particle system for both particle types
    osgParticle::ParticleSystem *ps = new osgParticle::ParticleSystem;

    // Assign the tiled texture
    ps->setDefaultAttributes("Images/fireparticle8x8.png", false, false);

    // Create two emitters, one for the explosions, one for the smoke balls.
    osgParticle::ModularEmitter *emitter1 = new osgParticle::ModularEmitter;
    emitter1->setParticleSystem(ps);
    emitter1->setParticleTemplate(pexplosion);

    osgParticle::ModularEmitter *emitter2 = new osgParticle::ModularEmitter;
    emitter2->setParticleSystem(ps);
    emitter2->setParticleTemplate(psmoke);

    // create a counter each. We could reuse the counter for both emitters, but
    // then we could not control the ratio of smoke balls to explosions
    osgParticle::RandomRateCounter *counter1 = new osgParticle::RandomRateCounter;
    counter1->setRateRange(10, 10);
    emitter1->setCounter(counter1);

    osgParticle::RandomRateCounter *counter2 = new osgParticle::RandomRateCounter;
    counter2->setRateRange(3, 4);
    emitter2->setCounter(counter2);

    // setup a single placer for both emitters.
    osgParticle::SectorPlacer *placer = new osgParticle::SectorPlacer;
    placer->setCenter(-8, 0, 0);
    placer->setRadiusRange(2.5, 5);
    placer->setPhiRange(0, 2 * osg::PI);    // 360 angle to make a circle
    emitter1->setPlacer(placer);
    emitter2->setPlacer(placer);

    // the shooter is reused for both emitters
    osgParticle::RadialShooter *shooter = new osgParticle::RadialShooter;
    shooter->setInitialSpeedRange(0, 0);

    // give particles a little spin
    shooter->setInitialRotationalSpeedRange(osgParticle::rangev3(
       osg::Vec3(0, 0, -1),
       osg::Vec3(0, 0, 1)));
    emitter1->setShooter(shooter);
    emitter2->setShooter(shooter);

    // add both emitters to the scene graph
    root->addChild(emitter1);
    root->addChild(emitter2);

    // create a program, just as before
    osgParticle::ModularProgram *program = new osgParticle::ModularProgram;
    program->setParticleSystem(ps);

    // create an operator that moves the particles upwards
    osgParticle::AccelOperator *op1 = new osgParticle::AccelOperator;
    op1->setAcceleration(osg::Vec3(0, 0, 2.0f));
    program->addOperator(op1);

    // add the program to the scene graph
    root->addChild(program);

    // create a Geode to contain our particle system.
    osg::Geode *geode = new osg::Geode;
    geode->addDrawable(ps);

    // add the geode to the scene graph.
    root->addChild(geode);

    return ps;
}

//////////////////////////////////////////////////////////////////////////////
// MAIN SCENE GRAPH BUILDING FUNCTION
//////////////////////////////////////////////////////////////////////////////


void build_world(osg::Group *root)
{

    // In this function we are going to create two particle systems;
    // the first one will be very simple, based mostly on default properties;
    // the second one will be a little bit more complex, showing how to
    // create custom operators.
    // To avoid inserting too much code in a single function, we have
    // split the work into two functions which accept a Group node as
    // parameter, and return a pointer to the particle system they created.

    osgParticle::ParticleSystem *ps1 = create_simple_particle_system(root);
    osgParticle::ParticleSystem *ps2 = create_complex_particle_system(root);
    osgParticle::ParticleSystem *ps3 = create_animated_particle_system(root);

    // Now that the particle systems and all other related objects have been
    // created, we have to add an "updater" node to the scene graph. This node
    // will react to cull traversal by updating the specified particles system.

    osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;
    psu->addParticleSystem(ps1);
    psu->addParticleSystem(ps2);
    psu->addParticleSystem(ps3);

    // add the updater node to the scene graph
    root->addChild(psu);

}


//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::Group *root = new osg::Group;
    build_world(root);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);

    return viewer.run();
}
