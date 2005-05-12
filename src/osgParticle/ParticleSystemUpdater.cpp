#include <osgParticle/ParticleSystemUpdater>

#include <osg/CopyOp>
#include <osg/Node>

using namespace osg;

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater()
: osg::Node(), _t0(-1)
{
    setCullingActive(false);
}

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater(const ParticleSystemUpdater& copy, const osg::CopyOp& copyop)
: osg::Node(copy, copyop), _t0(copy._t0)
{
    ParticleSystem_Vector::const_iterator i;
    for (i=copy._psv.begin(); i!=copy._psv.end(); ++i) {
        _psv.push_back(static_cast<ParticleSystem* >(copyop(i->get())));
    }
}

void osgParticle::ParticleSystemUpdater::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if (cv) {
        if (nv.getFrameStamp())
        {
            double t = nv.getFrameStamp()->getReferenceTime();
            if (_t0 != -1)
            {
                ParticleSystem_Vector::iterator i;
                for (i=_psv.begin(); i!=_psv.end(); ++i)
                {
                    if (!i->get()->isFrozen() && (i->get()->getLastFrameNumber() >= (nv.getFrameStamp()->getFrameNumber() - 1) || !i->get()->getFreezeOnCull()))
                    {
                        i->get()->update(t - _t0);
                    }
                }
            }
            _t0 = t;
        }
        else
        {
            osg::notify(osg::WARN) << "osgParticle::ParticleSystemUpdater::traverse(NodeVisitor&) requires a valid FrameStamp to function, particles not updated.\n";
        }

    }
    Node::traverse(nv);
}

osg::BoundingSphere osgParticle::ParticleSystemUpdater::computeBound() const
{
    return osg::BoundingSphere();
}

