#include <osgParticle/ParticleProcessor>

#include <osg/Node>
#include <osg/NodeVisitor>
#include <osg/CopyOp>
#include <osg/Matrix>
#include <osg/Notify>

#include <osgUtil/CullVisitor>


using namespace osg;

osgParticle::ParticleProcessor::ParticleProcessor()
:    osg::Node(),
    _rf(RELATIVE_RF),
    _enabled(true),
    _t0(-1),
    _ps(0),
    _first_ltw_compute(true),
    _need_ltw_matrix(false),
    _first_wtl_compute(true),
    _need_wtl_matrix(false),
    _current_nodevisitor(0),
    _endless(true),
    _lifeTime(0.0),
    _startTime(0.0),
    _currentTime(0.0),
    _resetTime(0.0),
    _frameNumber(0)
{
    setCullingActive(false);
}

osgParticle::ParticleProcessor::ParticleProcessor(const ParticleProcessor& copy, const osg::CopyOp& copyop)
:    osg::Node(copy, copyop),
    _rf(copy._rf),
    _enabled(copy._enabled),
    _t0(copy._t0),
    _ps(static_cast<ParticleSystem* >(copyop(copy._ps.get()))),
    _first_ltw_compute(copy._first_ltw_compute),
    _need_ltw_matrix(copy._need_ltw_matrix),
    _first_wtl_compute(copy._first_wtl_compute),
    _need_wtl_matrix(copy._need_wtl_matrix),
    _current_nodevisitor(0),
    _endless(copy._endless),
    _lifeTime(copy._lifeTime),
    _startTime(copy._startTime),
    _currentTime(copy._currentTime),
    _resetTime(copy._resetTime),
    _frameNumber(copy._frameNumber)
{
}

void osgParticle::ParticleProcessor::setParticleSystem(ParticleSystem* ps)
{
    _ps = ps;
}

void osgParticle::ParticleProcessor::traverse(osg::NodeVisitor& nv)
{
    // continue only if the visitor actually is a cull visitor
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR) {
        // continue only if the particle system is valid
        if (_ps.valid())
        {
            if (nv.getFrameStamp())
            {
                ParticleSystem::ScopedWriteLock lock(*(_ps->getReadWriteMutex()));

                //added- 1/17/06- bgandere@nps.edu
                //a check to make sure we haven't updated yet this frame
                if(_frameNumber < nv.getFrameStamp()->getFrameNumber())
                {


                    // retrieve the current time
                    double t = nv.getFrameStamp()->getSimulationTime();

                    // reset this processor if we've reached the reset point
                    if ((_currentTime >= _resetTime) && (_resetTime > 0))
                    {
                        _currentTime = 0;
                        _t0 = -1;
                    }

                    // skip if we haven't initialized _t0 yet
                    if (_t0 != -1)
                    {

                        // check whether the processor is alive
                        bool alive = false;
                        if (_currentTime >= _startTime)
                        {
                            if (_endless || (_currentTime < (_startTime + _lifeTime)))
                                alive = true;
                        }

                        // update current time
                        _currentTime += t - _t0;

                        // process only if the particle system is not frozen/culled
                        // We need to allow at least 2 frames difference, because the particle system's lastFrameNumber
                        // is updated in the draw thread which may not have completed yet.
                        if (alive &&
                            _enabled &&
                            !_ps->isFrozen() &&
                            (!_ps->getFreezeOnCull() || ((nv.getFrameStamp()->getFrameNumber()-_ps->getLastFrameNumber()) <= 2)) )
                        {
                            // initialize matrix flags
                            _need_ltw_matrix = true;
                            _need_wtl_matrix = true;
                            _current_nodevisitor = &nv;

                            // do some process (unimplemented in this base class)
                            process( t - _t0 );
                        } else {
                            //The values of _previous_wtl_matrix and _previous_ltw_matrix will be invalid
                            //since processing was skipped for this frame
                            _first_ltw_compute = true;
                            _first_wtl_compute = true;
                        }
                    }
                    _t0 = t;
                }

                //added- 1/17/06- bgandere@nps.edu
                //updates the _frameNumber, keeping it current
                _frameNumber = nv.getFrameStamp()->getFrameNumber();
            }
            else
            {
                OSG_WARN << "osgParticle::ParticleProcessor::traverse(NodeVisitor&) requires a valid FrameStamp to function, particles not updated.\n";
            }

        } else
        {
            OSG_WARN << "ParticleProcessor \"" << getName() << "\": invalid particle system\n";
        }
    }


    // call the inherited method
    Node::traverse(nv);
}

osg::BoundingSphere osgParticle::ParticleProcessor::computeBound() const
{
    return osg::BoundingSphere();
}

