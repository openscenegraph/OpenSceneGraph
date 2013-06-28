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
#include <osgParticle/SinkOperator>

#define SINK_EPSILON 1e-3

using namespace osgParticle;

void SinkOperator::beginOperate( Program* prg )
{
    // Don't transform domains if they are used for sinking velocities
    if ( _sinkTarget==SINK_POSITION )
        DomainOperator::beginOperate(prg );
}

void SinkOperator::handlePoint( const Domain& domain, Particle* P, double /*dt*/ )
{
    const osg::Vec3& value = getValue(P);
    kill( P, (domain.v1==value) );
}

void SinkOperator::handleLineSegment( const Domain& domain, Particle* P, double /*dt*/ )
{
    const osg::Vec3& value = getValue(P);
    osg::Vec3 offset = value - domain.v1, normal = domain.v2 - domain.v1;
    normal.normalize();

    float diff = fabs(normal*offset - offset.length()) / domain.r1;
    kill( P, (diff<SINK_EPSILON) );
}

void SinkOperator::handleTriangle( const Domain& domain, Particle* P, double /*dt*/ )
{
    bool insideDomain = false;
    const osg::Vec3& value = getValue(P);
    osg::Vec3 offset = value - domain.v1;
    if ( offset*domain.plane.getNormal()>SINK_EPSILON )
        insideDomain = false;
    else
    {
        float upos = offset * domain.s1;
        float vpos = offset * domain.s2;
        insideDomain = !(upos<0.0f || vpos<0.0f || (upos+vpos)>1.0f);
    }
    kill( P, insideDomain );
}

void SinkOperator::handleRectangle( const Domain& domain, Particle* P, double /*dt*/ )
{
    bool insideDomain = false;
    const osg::Vec3& value = getValue(P);
    osg::Vec3 offset = value - domain.v1;
    if ( offset*domain.plane.getNormal()>SINK_EPSILON )
        insideDomain = false;
    else
    {
        float upos = offset * domain.s1;
        float vpos = offset * domain.s2;
        insideDomain = !(upos<0.0f || upos>1.0f || vpos<0.0f || vpos>1.0f);
    }
    kill( P, insideDomain );
}

void SinkOperator::handlePlane( const Domain& domain, Particle* P, double /*dt*/ )
{
    const osg::Vec3& value = getValue(P);
    bool insideDomain = (domain.plane.getNormal()*value>=-domain.plane[3]);
    kill( P, insideDomain );
}

void SinkOperator::handleSphere( const Domain& domain, Particle* P, double /*dt*/ )
{
    const osg::Vec3& value = getValue(P);
    float r = (value - domain.v1).length();
    kill( P, (r<=domain.r1) );
}

void SinkOperator::handleBox( const Domain& domain, Particle* P, double /*dt*/ )
{
    const osg::Vec3& value = getValue(P);
    bool insideDomain = !(
        (value.x() < domain.v1.x()) || (value.x() > domain.v2.x()) ||
        (value.y() < domain.v1.y()) || (value.y() > domain.v2.y()) ||
        (value.z() < domain.v1.z()) || (value.z() > domain.v2.z())
    );
    kill( P, insideDomain );
}

void SinkOperator::handleDisk( const Domain& domain, Particle* P, double /*dt*/ )
{
    bool insideDomain = false;
    const osg::Vec3& value = getValue(P);
    osg::Vec3 offset = value - domain.v1;
    if ( offset*domain.v2>SINK_EPSILON )
        insideDomain = false;
    else
    {
        float length = offset.length();
        insideDomain = (length<=domain.r1 && length>=domain.r2);
    }
    kill( P, insideDomain );
}
