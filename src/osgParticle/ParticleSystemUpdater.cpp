#include <osgParticle/ParticleSystemUpdater>

#include <osg/CopyOp>
#include <osg/Node>

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater()
: osg::Node(), t0_(-1)
{
    setCullingActive(false);
}

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater(const ParticleSystemUpdater &copy, const osg::CopyOp &copyop)
: osg::Node(copy, copyop), t0_(copy.t0_)
{
    ParticleSystem_Vector::const_iterator i;
    for (i=copy.psv_.begin(); i!=copy.psv_.end(); ++i) {
        psv_.push_back(static_cast<ParticleSystem *>(copyop(i->get())));
    }
}

void osgParticle::ParticleSystemUpdater::traverse(osg::NodeVisitor &nv)
{
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if (cv) {
        double t = nv.getFrameStamp()->getReferenceTime();
        if (t0_ != -1) {
            ParticleSystem_Vector::iterator i;
            for (i=psv_.begin(); i!=psv_.end(); ++i) {
                if (!i->get()->isFrozen() && (i->get()->getLastFrameNumber() >= (nv.getFrameStamp()->getFrameNumber() - 1) || !i->get()->getFreezeOnCull())) {
                    i->get()->update(t - t0_);
                }
            }
        }
        t0_ = t;
    }
    osg::Node::traverse(nv);
}
