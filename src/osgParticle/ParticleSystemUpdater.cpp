#include <osgParticle/ParticleSystemUpdater>

#include <osg/CopyOp>
#include <osg/Geode>

using namespace osg;

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater()
: osg::Node(), _t0(-1), _frameNumber(0)
{
    setCullingActive(false);
}

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater(const ParticleSystemUpdater& copy, const osg::CopyOp& copyop)
: osg::Node(copy, copyop), _t0(copy._t0), _frameNumber(0)
{
    ParticleSystem_Vector::const_iterator i;
    for (i=copy._psv.begin(); i!=copy._psv.end(); ++i) {
        _psv.push_back(static_cast<ParticleSystem* >(copyop(i->get())));
    }
}

void osgParticle::ParticleSystemUpdater::traverse(osg::NodeVisitor& nv)
{
    if (nv.getVisitorType() == osg::NodeVisitor::CULL_VISITOR)
    {
        if (nv.getFrameStamp())
        {
            if( _frameNumber < nv.getFrameStamp()->getFrameNumber())
            {
                _frameNumber = nv.getFrameStamp()->getFrameNumber();

                double t = nv.getFrameStamp()->getSimulationTime();
                if (_t0 != -1.0)
                {
                    ParticleSystem_Vector::iterator i;
                    for (i=_psv.begin(); i!=_psv.end(); ++i)
                    {
                        ParticleSystem* ps = i->get();

                        ParticleSystem::ScopedWriteLock lock(*(ps->getReadWriteMutex()));

                        // We need to allow at least 2 frames difference, because the particle system's lastFrameNumber
                        // is updated in the draw thread which may not have completed yet.
                        if (!ps->isFrozen() &&
                            (!ps->getFreezeOnCull() || ((nv.getFrameStamp()->getFrameNumber()-ps->getLastFrameNumber()) <= 2)) )
                        {
                            ps->update(t - _t0, nv);
                        }
                    }
                }
                _t0 = t;
            }

        }
        else
        {
            OSG_WARN << "osgParticle::ParticleSystemUpdater::traverse(NodeVisitor&) requires a valid FrameStamp to function, particles not updated.\n";
        }

    }
    Node::traverse(nv);
}

osg::BoundingSphere osgParticle::ParticleSystemUpdater::computeBound() const
{
    return osg::BoundingSphere();
}

bool osgParticle::ParticleSystemUpdater::addParticleSystem(ParticleSystem* ps)
{
    _psv.push_back(ps);
    return true;
}

bool osgParticle::ParticleSystemUpdater::removeParticleSystem(ParticleSystem* ps)
{
   unsigned int i = getParticleSystemIndex( ps );
   if( i >= _psv.size() ) return false;

   removeParticleSystem( i );
   return true;
}

bool osgParticle::ParticleSystemUpdater::removeParticleSystem(unsigned int pos, unsigned int numParticleSystemsToRemove)
{
   if( (pos < _psv.size()) && (numParticleSystemsToRemove > 0) )
   {
      unsigned int endOfRemoveRange = pos + numParticleSystemsToRemove;
      if( endOfRemoveRange > _psv.size() )
      {
         OSG_DEBUG<<"Warning: ParticleSystem::removeParticleSystem(i,numParticleSystemsToRemove) has been passed an excessive number"<<std::endl;
         OSG_DEBUG<<"         of ParticleSystems to remove, trimming just to end of ParticleSystem list."<<std::endl;
         endOfRemoveRange = _psv.size();
      }
      _psv.erase(_psv.begin()+pos, _psv.begin()+endOfRemoveRange);
      return true;
   }
   return false;
}

bool osgParticle::ParticleSystemUpdater::replaceParticleSystem( ParticleSystem* origPS, ParticleSystem* newPS )
{
   if( (newPS == NULL) || (origPS == newPS) ) return false;

   unsigned int pos = getParticleSystemIndex( origPS );
   if( pos < _psv.size() )
   {
      return setParticleSystem( pos, newPS );
   }
   return false;
}

bool osgParticle::ParticleSystemUpdater::setParticleSystem( unsigned int i, ParticleSystem* ps )
{
   if( (i < _psv.size()) && ps )
   {
      _psv[i] = ps;
      return true;
   }
   return false;
}
