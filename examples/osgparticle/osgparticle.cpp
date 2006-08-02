#include <osgProducer/Viewer>

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

    osg::Geode *geode = new osg::Geode;    
    geode->addDrawable(ps);

    // add the geode to the scene graph
    root->addChild(geode);

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
    placer->setPhiRange(0, 2 * osg::PI);    // 360° angle to make a circle
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
    // splitted the work into two functions which accept a Group node as
    // parameter, and return a pointer to the particle system they created.

    osgParticle::ParticleSystem *ps1 = create_simple_particle_system(root);
    osgParticle::ParticleSystem *ps2 = create_complex_particle_system(root);

    // Now that the particle systems and all other related objects have been
    // created, we have to add an "updater" node to the scene graph. This node
    // will react to cull traversal by updating the specified particles system.

    osgParticle::ParticleSystemUpdater *psu = new osgParticle::ParticleSystemUpdater;
    psu->addParticleSystem(ps1);
    psu->addParticleSystem(ps2);

    // add the updater node to the scene graph
    root->addChild(psu);

}


//////////////////////////////////////////////////////////////////////////////
// main()
//////////////////////////////////////////////////////////////////////////////


int main(int argc, char **argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of particle systems.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] image_file_left_eye image_file_right_eye");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    osg::Group *root = new osg::Group;
    build_world(root);
   
    // add a viewport to the viewer and attach the scene graph.
    viewer.setSceneData(root);
        
    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return 0;
}
