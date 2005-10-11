/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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
#include <osgParticle/ConnectedParticleSystem>

using namespace osgParticle;

ConnectedParticleSystem::ConnectedParticleSystem():
    _startParticle(0),
    _lastParticleCreated(0)
{
}

ConnectedParticleSystem::ConnectedParticleSystem(const ConnectedParticleSystem& copy, const osg::CopyOp& copyop):
    ParticleSystem(copy,copyop),
    _startParticle(0),
    _lastParticleCreated(0)
{
    // need to think about how to copy _startParticle and _lastParticleCreated...
    // should we just use indices?  Should we compute offsets into the particle system?
    osg::notify(osg::NOTICE)<<"Warning: ConnectedParticleSystem copy constructor incomplete."<<std::endl;
}

ConnectedParticleSystem::~ConnectedParticleSystem()
{
}

Particle* ConnectedParticleSystem::createParticle(const Particle* ptemplate)
{
    return ParticleSystem::createParticle(ptemplate);
}
        
void ConnectedParticleSystem::destroyParticle(int i)
{
    return ParticleSystem::destroyParticle(i);
}
        
void ConnectedParticleSystem::update(double dt)
{
    ParticleSystem::update(dt);
}

void ConnectedParticleSystem::drawImplementation(osg::State& state) const
{
    ParticleSystem::drawImplementation(state);
}
