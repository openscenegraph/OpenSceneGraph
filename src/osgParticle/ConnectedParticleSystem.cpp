/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osg/Notify>
#include <osg/CullingSet>
#include <osg/io_utils>
#include <osgParticle/ConnectedParticleSystem>

using namespace osgParticle;

ConnectedParticleSystem::ConnectedParticleSystem():
    _lastParticleCreated(Particle::INVALID_INDEX),
    _maxNumberOfParticlesToSkip(200),
    _startParticle(Particle::INVALID_INDEX)
{
}

ConnectedParticleSystem::ConnectedParticleSystem(const ConnectedParticleSystem& copy, const osg::CopyOp& copyop):
    ParticleSystem(copy,copyop),
    _lastParticleCreated(copy._lastParticleCreated),
    _maxNumberOfParticlesToSkip(200),
    _startParticle(copy._startParticle)
{
}

ConnectedParticleSystem::~ConnectedParticleSystem()
{
}

Particle* ConnectedParticleSystem::createParticle(const Particle* ptemplate)
{
    // OSG_NOTICE<<this<< " Creating particle "<<std::endl;

    Particle* particle = ParticleSystem::createParticle(ptemplate);
    int particleIndex = (int)(particle - &_particles[0]);

    if (particle)
    {

        if (_startParticle == Particle::INVALID_INDEX)
        {
            // we are the fisrt particle create, so start the connect particle list
            _startParticle = particleIndex;
        }

        if (_lastParticleCreated != Particle::INVALID_INDEX)
        {
            // OSG_NOTICE<<this<< " Connecting "<<_lastParticleCreated<<" to "<<particleIndex<<std::endl;

            // write up the last created particle to this new particle
            _particles[_lastParticleCreated].setNextParticle(particleIndex);
            particle->setPreviousParticle(_lastParticleCreated);
        }

        // set the new particle as the last particle created.
        _lastParticleCreated = particleIndex;

    }

    return particle;
}

void ConnectedParticleSystem::reuseParticle(int particleIndex)
{
    // OSG_NOTICE<<this<< " Reusing particle "<<particleIndex<<std::endl;

    if (particleIndex<0 || particleIndex>=(int)_particles.size()) return;

    Particle* particle = &_particles[particleIndex];
    int previous = particle->getPreviousParticle();
    int next = particle->getNextParticle();

    // update start and last entries
    if (_startParticle == particleIndex)
    {
        _startParticle = particle->getNextParticle();
    }

    if (_lastParticleCreated == particleIndex)
    {
        _lastParticleCreated = Particle::INVALID_INDEX;
    }

    // join up the previous and next particles to account for
    // the deletion of the this particle
    if (previous != Particle::INVALID_INDEX)
    {
        _particles[previous].setNextParticle(next);
    }

    if (next != Particle::INVALID_INDEX)
    {
        _particles[next].setPreviousParticle(previous);
    }

    // reset the next and previous particle entries of this particle
    particle->setPreviousParticle(Particle::INVALID_INDEX);
    particle->setNextParticle(Particle::INVALID_INDEX);

    // put the particle on the death stack
    ParticleSystem::reuseParticle(particleIndex);

}

void ConnectedParticleSystem::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();
    osg::GLBeginEndAdapter& gl = state.getGLBeginEndAdapter();

    ScopedReadLock lock(_readWriteMutex);

    const Particle* particle = (_startParticle != Particle::INVALID_INDEX) ? &_particles[_startParticle] : 0;
    if (!particle) return;


    osg::Vec4 pixelSizeVector = osg::CullingSet::computePixelSizeVector(*state.getCurrentViewport(),state.getProjectionMatrix(),state.getModelViewMatrix());
    float unitPixelSize = fabs(1.0/(particle->getPosition()*pixelSizeVector));
    float pixelSizeOfFirstParticle = unitPixelSize * particle->getCurrentSize();
    //float desiredGapBetweenDrawnParticles = 50.0f/unitPixelSize;
    //float desiredGapBetweenDrawnParticles2 = desiredGapBetweenDrawnParticles*desiredGapBetweenDrawnParticles;

    float maxPixelError2 = osg::square(1.0f/unitPixelSize);

    if (pixelSizeOfFirstParticle<1.0)
    {
        // draw the connected particles as a line
        gl.Begin(GL_LINE_STRIP);
        while(particle != 0)
        {

            const osg::Vec4& color = particle->getCurrentColor();
            const osg::Vec3& pos = particle->getPosition();
            gl.Color4f( color.r(), color.g(), color.b(), color.a() * particle->getCurrentAlpha());
            gl.TexCoord2f( particle->getSTexCoord(), 0.5f );
            gl.Vertex3fv(pos.ptr());

            const Particle* nextParticle = (particle->getNextParticle() != Particle::INVALID_INDEX) ? &_particles[particle->getNextParticle()] : 0;
            if (nextParticle)
            {
                const osg::Vec3& nextPos = nextParticle->getPosition();
                osg::Vec3 startDelta = nextPos-pos;
                startDelta.normalize();
                float distance2 = 0.0;

                // now skip particles of required
                for(unsigned int i=0;
                    i<_maxNumberOfParticlesToSkip && ((distance2<maxPixelError2) && (nextParticle->getNextParticle()!=Particle::INVALID_INDEX));
                    ++i)
                {
                    nextParticle = &_particles[nextParticle->getNextParticle()];
                    const osg::Vec3& nextPos = nextParticle->getPosition();
                    osg::Vec3 delta = nextPos-pos;
                    distance2 = (delta^startDelta).length2();
                }
            }
            particle = nextParticle;
        }
        gl.End();
    }
    else
    {

        // draw the connected particles as a quad stripped aligned to be orthogonal to the eye
        osg::Matrix eyeToLocalTransform;
        eyeToLocalTransform.invert(state.getModelViewMatrix());
        osg::Vec3 eyeLocal = osg::Vec3(0.0f,0.0,0.0f)*eyeToLocalTransform;

        osg::Vec3 delta(0.0f,0.0f,1.0f);

        gl.Begin(GL_QUAD_STRIP);
        while(particle != 0)
        {
            const osg::Vec4& color = particle->getCurrentColor();
            const osg::Vec3& pos = particle->getPosition();

            const Particle* nextParticle = (particle->getNextParticle() != Particle::INVALID_INDEX) ? &_particles[particle->getNextParticle()] : 0;

            if (nextParticle)
            {
                const osg::Vec3& nextPos = nextParticle->getPosition();
                osg::Vec3 startDelta = nextPos-pos;
                startDelta.normalize();
                float distance2 = 0.0;

                // now skip particles of required
                for(unsigned int i=0;
                    i<_maxNumberOfParticlesToSkip && ((distance2<maxPixelError2) && (nextParticle->getNextParticle()!=Particle::INVALID_INDEX));
                    ++i)
                {
                    nextParticle = &_particles[nextParticle->getNextParticle()];
                    const osg::Vec3& nextPos = nextParticle->getPosition();
                    delta = nextPos-pos;
                    distance2 = (delta^startDelta).length2();
                }

                delta = nextPos-pos;
            }

            osg::Vec3 normal( delta ^ (pos-eyeLocal));
            normal.normalize();
            normal *= particle->getCurrentSize();

            osg::Vec3 bottom(pos-normal);
            osg::Vec3 top(pos+normal);

            gl.Color4f( color.r(), color.g(), color.b(), color.a() * particle->getCurrentAlpha());

            gl.TexCoord2f( particle->getSTexCoord(), 0.0f );
            gl.Vertex3fv(bottom.ptr());

            gl.TexCoord2f( particle->getSTexCoord(), 1.0f );
            gl.Vertex3fv(top.ptr());

            particle = nextParticle;
        }
        gl.End();
    }
}

