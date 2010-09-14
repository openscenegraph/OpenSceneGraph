#include <osgParticle/ParticleSystemUpdater>

#include <osg/CopyOp>
#include <osg/Geode>

using namespace osg;

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater()
: osg::Geode(), _frameNumber(0)
{
    setCullingActive(false);
}

osgParticle::ParticleSystemUpdater::ParticleSystemUpdater(const ParticleSystemUpdater& copy, const osg::CopyOp& copyop)
: osg::Geode(copy, copyop), _frameNumber(0)
{
    ParticleSystem_Vector::const_iterator i;
    for (i=copy._psv.begin(); i!=copy._psv.end(); ++i) {
        _psv.push_back(static_cast<ParticleSystem* >(copyop(i->get())));
    }
}

void osgParticle::ParticleSystemUpdater::traverse(osg::NodeVisitor& nv)
{
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);
    if (cv) 
    {
        if (nv.getFrameStamp())
        {
            
            if( _frameNumber < nv.getFrameStamp()->getFrameNumber())
            {
                _frameNumber = nv.getFrameStamp()->getFrameNumber();

                double t = nv.getFrameStamp()->getSimulationTime();
                ParticleSystem_Vector::iterator i;
                for (i=_psv.begin(); i!=_psv.end(); ++i)
                {
                    ParticleSystem* ps = i->get();
                    
                    ParticleSystem::ScopedWriteLock lock(*(ps->getReadWriteMutex()));

                    if (!ps->isFrozen() && (ps->getLastFrameNumber() >= (nv.getFrameStamp()->getFrameNumber() - 1) || !ps->getFreezeOnCull()))
                    {
                        ps->update(ps->getDeltaTime(t), nv);
                    }
                }
            }

        }
        else
        {
            OSG_WARN << "osgParticle::ParticleSystemUpdater::traverse(NodeVisitor&) requires a valid FrameStamp to function, particles not updated.\n";
        }

    }
    Geode::traverse(nv);
}

osg::BoundingSphere osgParticle::ParticleSystemUpdater::computeBound() const
{
    return Geode::computeBound();
}

bool osgParticle::ParticleSystemUpdater::addDrawable(Drawable* drawable)
{
    ParticleSystem* ps = dynamic_cast<ParticleSystem*>(drawable);
    if (ps) addParticleSystem(ps);
    return Geode::addDrawable(drawable);
}

bool osgParticle::ParticleSystemUpdater::removeDrawable(Drawable* drawable)
{
    ParticleSystem* ps = dynamic_cast<ParticleSystem*>(drawable);
    if (ps) removeParticleSystem(ps);
    return Geode::removeDrawable(drawable);
}

bool osgParticle::ParticleSystemUpdater::addParticleSystem(ParticleSystem* ps)
{
    unsigned int i = getParticleSystemIndex( ps );
    if( i >= _psv.size() ) _psv.push_back(ps);
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
