/* OpenSceneGraph example, osgparticleshader.
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

#include <iostream>

#include <osg/ShapeDrawable>
#include <osg/MatrixTransform>
#include <osg/Point>
#include <osg/PointSprite>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Viewer>

#include <osgParticle/ParticleSystem>
#include <osgParticle/ParticleSystemUpdater>
#include <osgParticle/ModularEmitter>
#include <osgParticle/ModularProgram>

#include <osgParticle/AccelOperator>
#include <osgParticle/DampingOperator>
#include <osgParticle/BounceOperator>
#include <osgParticle/SinkOperator>

void createFountainEffect( osgParticle::ModularEmitter* emitter, osgParticle::ModularProgram* program )
{
    // Emit specific number of particles every frame
    osg::ref_ptr<osgParticle::RandomRateCounter> rrc = new osgParticle::RandomRateCounter;
    rrc->setRateRange( 500, 2000 );
    
    // Accelerate particles in the given gravity direction.
    osg::ref_ptr<osgParticle::AccelOperator> accel = new osgParticle::AccelOperator;
    accel->setToGravity();
    
    // Multiply each particle's velocity by a damping constant.
    osg::ref_ptr<osgParticle::DampingOperator> damping = new osgParticle::DampingOperator;
    damping->setDamping( 0.9f );
    
    // Bounce particles off objects defined by one or more domains.
    // Supported domains include triangle, rectangle, plane, disk and sphere.
    // Since a bounce always happens instantaneously, it will not work correctly with unstable delta-time.
    // At present, even the floating error of dt (which are applied to ParticleSystem and Operator separately)
    // causes wrong bounce results. Some one else may have better solutions for this.
    osg::ref_ptr<osgParticle::BounceOperator> bounce = new osgParticle::BounceOperator;
    bounce->setFriction( -0.05 );
    bounce->setResilience( 0.35 );
    bounce->addDiskDomain( osg::Vec3(0.0f, 0.0f, -2.0f), osg::Z_AXIS, 8.0f );
    bounce->addPlaneDomain( osg::Plane(osg::Z_AXIS, 5.0f) );
    
    // Kill particles going inside/outside of specified domains.
    osg::ref_ptr<osgParticle::SinkOperator> sink = new osgParticle::SinkOperator;
    sink->setSinkStrategy( osgParticle::SinkOperator::SINK_OUTSIDE );
    sink->addSphereDomain( osg::Vec3(), 20.0f );
    
    emitter->setCounter( rrc.get() );
    program->addOperator( accel.get() );
    program->addOperator( damping.get() );
    program->addOperator( bounce.get() );
    program->addOperator( sink.get() );
}

int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );
    
    std::string textureFile("Images/smoke.rgb");
    while ( arguments.read("--texture", textureFile) ) {}
    
    float pointSize = 20.0f;
    while ( arguments.read("--point", pointSize) ) {}
    
    double visibilityDistance = -1.0f;
    while ( arguments.read("--visibility", visibilityDistance) ) {}
    
    bool customShape = false;
    while ( arguments.read("--enable-custom") ) { customShape = true; }
    
    bool useShaders = true;
    while ( arguments.read("--disable-shaders") ) { useShaders = false; }
    
    /***
    Customize particle template and system attributes
    ***/
    osg::ref_ptr<osgParticle::ParticleSystem> ps = new osgParticle::ParticleSystem;
    
    ps->getDefaultParticleTemplate().setLifeTime( 5.0f );
    
    if ( customShape )
    {
        // osgParticle now supports making use of customized drawables. The draw() method will be executed
        // and display lists will be called for each particle. It is always a huge consumption of memory, and
        // hardly to use shaders to render them, so please be careful using this feature.
        ps->getDefaultParticleTemplate().setShape( osgParticle::Particle::USER );
        ps->getDefaultParticleTemplate().setDrawable( new osg::ShapeDrawable(new osg::Box(osg::Vec3(), 1.0f)) );
        useShaders = false;
    }
    else
    {
        // The shader only supports rendering points at present.
        ps->getDefaultParticleTemplate().setShape( osgParticle::Particle::POINT );
    }
    
    // Set the visibility distance of particles, due to their Z-value in the eye coordinates.
    // Particles that are out of the distance (or behind the eye) will not be rendered.
    ps->setVisibilityDistance( visibilityDistance );
    
    if ( useShaders )
    {
        // Set using local GLSL shaders to render particles.
        // At present, this is slightly efficient than ordinary methods. The bottlenack here seems to be the cull
        // traversal time. Operators go through the particle list again and again...
        ps->setDefaultAttributesUsingShaders( textureFile, true, 0 );
    }
    else
    {
        // The default methods uses glBegin()/glEnd() pairs. Fortunately the GLBeginEndAdapter does improve the
        // process, which mimics the immediate mode with glDrawArrays().
        ps->setDefaultAttributes( textureFile, true, false, 0 );
        
        // Without the help of shaders, we have to sort particles to make the visibility distance work. Sorting is
        // also useful in rendering transparent particles in back-to-front order.
        if ( visibilityDistance>0.0 )
            ps->setSortMode( osgParticle::ParticleSystem::SORT_BACK_TO_FRONT );
    }
    
    // At last, to make the point sprite work, we have to set the points size and the sprite attribute.
    osg::StateSet* stateset = ps->getOrCreateStateSet();
    stateset->setAttribute( new osg::Point(pointSize) );
    stateset->setTextureAttributeAndModes( 0, new osg::PointSprite, osg::StateAttribute::ON );
    
    /***
    Construct other particle system elements, including the emitter and program
    ***/
    osg::ref_ptr<osgParticle::ModularEmitter> emitter = new osgParticle::ModularEmitter;
    emitter->setParticleSystem( ps.get() );
    
    osg::ref_ptr<osgParticle::ModularProgram> program = new osgParticle::ModularProgram;
    program->setParticleSystem( ps.get() );
    
    createFountainEffect( emitter.get(), program.get() );
    
    /***
    Add the entire particle system to the scene graph
    ***/
    osg::ref_ptr<osg::MatrixTransform> parent = new osg::MatrixTransform;
    parent->addChild( emitter.get() );
    parent->addChild( program.get() );
    
    // The updater can receive particle systems as child drawables now. The addParticleSystem() method
    // is still usable, with which we should define another geode to contain a particle system.
    osg::ref_ptr<osgParticle::ParticleSystemUpdater> updater = new osgParticle::ParticleSystemUpdater;
    //updater->addDrawable( ps.get() );
    
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( parent.get() );
    root->addChild( updater.get() );
    
    // FIXME 2010.9.19: the updater can't be a drawable; otehrwise the ParticleEffect will not work properly. why?
    updater->addParticleSystem( ps.get() );
    
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    geode->addDrawable( ps.get() );
    root->addChild( geode.get() );
    
    /***
    Start the viewer
    ***/
    osgViewer::Viewer viewer;
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );
    viewer.addEventHandler( new osgViewer::StatsHandler );
    viewer.addEventHandler( new osgViewer::WindowSizeHandler );
    viewer.setSceneData( root.get() );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator );
    
    // A floating error of delta-time should be explained here:
    // The particles emitter, program and updater all use a 'dt' to compute the time value in every frame.
    // Because the 'dt' is a double value, it is not suitable to keep three copies of it separately, which
    // is the previous implementation. The small error makes some opeartors unable to work correctly, e.g.
    // the BounceOperator.
    // Now we make use of the getDeltaTime() of ParticleSystem to maintain and dispatch the delta time. But..
    // it is not the best solution so far, since there are still very few particles acting unexpectedly.
    return viewer.run();
    
    // FIXME 2010.9.19: At present, getDeltaTime() is not used and the implementations in the updater and processors still
    // use a (t - _t0) as the delta time, which is of course causing floating errors. ParticleEffect will not work if we
    // replace the delta time with getDeltaTime()... Need to find a solution.
}
