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
    ps_(0)
{
    setCullingActive(false);
}

osgParticle::ParticleProcessor::ParticleProcessor(const ParticleProcessor &copy, const osg::CopyOp &copyop)
:    osg::Node(copy, copyop),
    rf_(copy.rf_),
    enabled_(copy.enabled_),
    t0_(copy.t0_),
    ps_(static_cast<ParticleSystem *>(copyop(copy.ps_.get())))
{
}

void osgParticle::ParticleProcessor::traverse(osg::NodeVisitor &nv)
{
    // continue only if enabled
    if (enabled_) {

        // typecast the NodeVisitor to CullVisitor
        osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);

        // continue only if the visitor actually is a cull visitor
        if (cv) {

            // continue only if the particle system is valid
            if (ps_.valid()) {

                // retrieve the current time
                double t = nv.getFrameStamp()->getReferenceTime();

                // skip if we haven't initialized t0_ yet
                if (t0_ != -1) {

                    // check whether the particle system is frozen/culled
                    if (!ps_->isFrozen() && (ps_->getLastFrameNumber() >= (nv.getFrameStamp()->getFrameNumber() - 1) || !ps_->getFreezeOnCull())) {

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

            } else {
                osg::notify(osg::WARN) << "ParticleProcessor \"" << getName() << "\": invalid particle system\n";
            }
        }

    }

    // call the inherited method
    Node::traverse(nv);
}
