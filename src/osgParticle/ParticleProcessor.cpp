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
    rf_(RELATIVE_TO_PARENTS),
    enabled_(true),
    t0_(-1),
    ps_(0),
    need_ltw_matrix_(false),
    need_wtl_matrix_(false),
    current_nodevisitor_(0),
	endless_(true),
	lifeTime_(0.0),
	startTime_(0.0),
	currentTime_(0.0),
	resetTime_(0.0)
{
    setCullingActive(false);
}

osgParticle::ParticleProcessor::ParticleProcessor(const ParticleProcessor &copy, const osg::CopyOp &copyop)
:    osg::Node(copy, copyop),
    rf_(copy.rf_),
    enabled_(copy.enabled_),
    t0_(copy.t0_),
    ps_(static_cast<ParticleSystem *>(copyop(copy.ps_.get()))),
    need_ltw_matrix_(copy.need_ltw_matrix_),
    need_wtl_matrix_(copy.need_wtl_matrix_),
    current_nodevisitor_(0),
	endless_(copy.endless_),
	lifeTime_(copy.lifeTime_),
	startTime_(copy.startTime_),
	currentTime_(copy.currentTime_),
	resetTime_(copy.resetTime_)
{
}

void osgParticle::ParticleProcessor::traverse(osg::NodeVisitor &nv)
{
    // typecast the NodeVisitor to CullVisitor
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);

    // continue only if the visitor actually is a cull visitor
    if (cv) {

        // continue only if the particle system is valid
        if (ps_.valid())
        {

            if (nv.getFrameStamp())
            {

                // retrieve the current time
                double t = nv.getFrameStamp()->getReferenceTime();

				// reset this processor if we've reached the reset point
				if ((currentTime_ >= resetTime_) && (resetTime_ > 0)) {
					currentTime_ = 0;
					t0_ = -1;
				}

				// skip if we haven't initialized t0_ yet
                if (t0_ != -1) {

					// check whether the processor is alive
					bool alive = false;
					if (currentTime_ >= startTime_)	{
						if (endless_ || (currentTime_ < (startTime_ + lifeTime_)))
							alive = true;
					}

					// update current time
					currentTime_ += t - t0_;

                    // process only if the particle system is not frozen/culled
                    if (alive && 
						enabled_ && 
						!ps_->isFrozen() && 
						(ps_->getLastFrameNumber() >= (nv.getFrameStamp()->getFrameNumber() - 1) || !ps_->getFreezeOnCull())) {

                        // initialize matrix flags
                        need_ltw_matrix_ = true;
                        need_wtl_matrix_ = true;
                        current_nodevisitor_ = &nv;

                        // do some process (unimplemented in this base class)
                        process(t - t0_);
                    }
                }

                // update t0_
                t0_ = t;

            }
            else
            {
                osg::notify(osg::WARN) << "osgParticle::ParticleProcessor::traverse(NodeVisitor&) requires a valid FrameStamp to function, particles not updated.\n";
            }

        } else 
        {
            osg::notify(osg::WARN) << "ParticleProcessor \"" << getName() << "\": invalid particle system\n";
        }
    }


    // call the inherited method
    Node::traverse(nv);
}
