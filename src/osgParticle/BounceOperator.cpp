/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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
// Written by Wang Rui, (C) 2010

#include <osg/Notify>
#include <osgParticle/ModularProgram>
#include <osgParticle/BounceOperator>

using namespace osgParticle;

void BounceOperator::handleTriangle( const Domain& domain, Particle* P, double dt )
{
    osg::Vec3 nextpos = P->getPosition() + P->getVelocity() * dt;
    float distance = domain.plane.distance( P->getPosition() );
    if ( distance*domain.plane.distance(nextpos)>=0 ) return;

    osg::Vec3 normal = domain.plane.getNormal();
    float nv = normal * P->getVelocity();
    osg::Vec3 hitPoint = P->getPosition() - P->getVelocity() * (distance / nv);

    float upos = (hitPoint - domain.v1) * domain.s1;
    float vpos = (hitPoint - domain.v1) * domain.s2;
    if ( upos<0.0f || vpos<0.0f || (upos + vpos)>1.0f ) return;

    // Compute tangential and normal components of velocity
    osg::Vec3 vn = normal * nv;
    osg::Vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if ( vt.length2()<=_cutoff ) P->setVelocity( vt - vn*_resilience );
    else P->setVelocity( vt*(1.0f-_friction) - vn*_resilience );
}

void BounceOperator::handleRectangle( const Domain& domain, Particle* P, double dt )
{
    osg::Vec3 nextpos = P->getPosition() + P->getVelocity() * dt;
    float distance = domain.plane.distance( P->getPosition() );
    if ( distance*domain.plane.distance(nextpos)>=0 ) return;

    osg::Vec3 normal = domain.plane.getNormal();
    float nv = normal * P->getVelocity();
    osg::Vec3 hitPoint = P->getPosition() - P->getVelocity() * (distance / nv);

    float upos = (hitPoint - domain.v1) * domain.s1;
    float vpos = (hitPoint - domain.v1) * domain.s2;
    if ( upos<0.0f || upos>1.0f || vpos<0.0f || vpos>1.0f ) return;

    // Compute tangential and normal components of velocity
    osg::Vec3 vn = normal * nv;
    osg::Vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if ( vt.length2()<=_cutoff ) P->setVelocity( vt - vn*_resilience );
    else P->setVelocity( vt*(1.0f-_friction) - vn*_resilience );
}

void BounceOperator::handlePlane( const Domain& domain, Particle* P, double dt )
{
    osg::Vec3 nextpos = P->getPosition() + P->getVelocity() * dt;
    float distance = domain.plane.distance( P->getPosition() );
    if ( distance*domain.plane.distance(nextpos)>=0 ) return;

    osg::Vec3 normal = domain.plane.getNormal();
    float nv = normal * P->getVelocity();

    // Compute tangential and normal components of velocity
    osg::Vec3 vn = normal * nv;
    osg::Vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if ( vt.length2()<=_cutoff ) P->setVelocity( vt - vn*_resilience );
    else P->setVelocity( vt*(1.0f-_friction) - vn*_resilience );
}

void BounceOperator::handleSphere( const Domain& domain, Particle* P, double dt )
{
    osg::Vec3 nextpos = P->getPosition() + P->getVelocity() * dt;
    float distance1 = (P->getPosition() - domain.v1).length();
    if ( distance1<=domain.r1 )  // Within the sphere
    {
        float distance2 = (nextpos - domain.v1).length();
        if ( distance2<=domain.r1 ) return;

        // Bounce back in if going outside
        osg::Vec3 normal = domain.v1 - P->getPosition(); normal.normalize();
        float nmag = P->getVelocity() * normal;

        // Compute tangential and normal components of velocity
        osg::Vec3 vn = normal * nmag;
        osg::Vec3 vt = P->getVelocity() - vn;
        if ( nmag<0 ) vn = -vn;

        // Compute new velocity
        float tanscale = (vt.length2()<=_cutoff) ? 1.0f : (1.0f - _friction);
        P->setVelocity( vt * tanscale + vn * _resilience );

        // Make sure the particle is fixed to stay inside
        nextpos = P->getPosition() + P->getVelocity() * dt;
        distance2 = (nextpos - domain.v1).length();
        if ( distance2>domain.r1 )
        {
            normal = domain.v1 - nextpos; normal.normalize();

            osg::Vec3 wishPoint = domain.v1 - normal * (0.999f * domain.r1);
            P->setVelocity( (wishPoint - P->getPosition()) / dt );
        }
    }
    else  // Outside the sphere
    {
        float distance2 = (nextpos - domain.v1).length();
        if ( distance2>domain.r1 ) return;

        // Bounce back out if going inside
        osg::Vec3 normal = P->getPosition() - domain.v1; normal.normalize();
        float nmag = P->getVelocity() * normal;

        // Compute tangential and normal components of velocity
        osg::Vec3 vn = normal * nmag;
        osg::Vec3 vt = P->getVelocity() - vn;
        if ( nmag<0 ) vn = -vn;

        // Compute new velocity
        float tanscale = (vt.length2()<=_cutoff) ? 1.0f : (1.0f - _friction);
        P->setVelocity( vt * tanscale + vn * _resilience );
    }
}

void BounceOperator::handleDisk( const Domain& domain, Particle* P, double dt )
{
    osg::Vec3 nextpos = P->getPosition() + P->getVelocity() * dt;
    float distance = domain.plane.distance( P->getPosition() );
    if ( distance*domain.plane.distance(nextpos)>=0 ) return;

    osg::Vec3 normal = domain.plane.getNormal();
    float nv = normal * P->getVelocity();
    osg::Vec3 hitPoint = P->getPosition() - P->getVelocity() * (distance / nv);

    float radius = (hitPoint - domain.v1).length();
    if ( radius>domain.r1 || radius<domain.r2 ) return;

    // Compute tangential and normal components of velocity
    osg::Vec3 vn = normal * nv;
    osg::Vec3 vt = P->getVelocity() - vn;

    // Compute new velocity
    if ( vt.length2()<=_cutoff ) P->setVelocity( vt - vn*_resilience );
    else P->setVelocity( vt*(1.0f-_friction) - vn*_resilience );
}
