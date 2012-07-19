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
#include <osgParticle/DomainOperator>

using namespace osgParticle;

void DomainOperator::operate( Particle* P, double dt )
{
    for ( std::vector<Domain>::iterator itr=_domains.begin(); itr!=_domains.end(); ++itr )
    {
        switch ( itr->type )
        {
        case Domain::POINT_DOMAIN:
            handlePoint( *itr, P, dt );
            break;
        case Domain::LINE_DOMAIN:
            handleLineSegment( *itr, P, dt );
            break;
        case Domain::TRI_DOMAIN:
            handleTriangle( *itr, P, dt );
            break;
        case Domain::RECT_DOMAIN:
            handleRectangle( *itr, P, dt );
            break;
        case Domain::PLANE_DOMAIN:
            handlePlane( *itr, P, dt );
            break;
        case Domain::SPHERE_DOMAIN:
            handleSphere( *itr, P, dt );
            break;
        case Domain::BOX_DOMAIN:
            handleBox( *itr, P, dt );
            break;
        case Domain::DISK_DOMAIN:
            handleDisk( *itr, P, dt );
            break;
        default: break;
        }
    }
}

void DomainOperator::beginOperate( Program* prg )
{
    if ( prg->getReferenceFrame()==ModularProgram::RELATIVE_RF )
    {
        _backupDomains = _domains;
        for ( std::vector<Domain>::iterator itr=_domains.begin(); itr!=_domains.end(); ++itr )
        {
            Domain& domain = *itr;
            switch ( domain.type )
            {
            case Domain::POINT_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                break;
            case Domain::LINE_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                domain.v2 = prg->transformLocalToWorld(domain.v2);
                break;
            case Domain::TRI_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                domain.v2 = prg->transformLocalToWorld(domain.v2);
                domain.v3 = prg->transformLocalToWorld(domain.v3);
                domain.plane.set(domain.v1, domain.v2, domain.v3);
                computeNewBasis( domain.v2-domain.v1, domain.v3-domain.v1, domain.s1, domain.s2 );
                break;
            case Domain::RECT_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                domain.v2 = prg->rotateLocalToWorld(domain.v2);  // Width vector
                domain.v3 = prg->rotateLocalToWorld(domain.v3);  // Height vector
                domain.plane.set(domain.v1, domain.v1+domain.v2, domain.v1+domain.v3);
                computeNewBasis( domain.v2, domain.v3, domain.s1, domain.s2 );
                break;
            case Domain::PLANE_DOMAIN:
                domain.plane.transformProvidingInverse( prg->getLocalToWorldMatrix() );
                break;
            case Domain::SPHERE_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                break;
            case Domain::BOX_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                domain.v2 = prg->transformLocalToWorld(domain.v2);
                break;
            case Domain::DISK_DOMAIN:
                domain.v1 = prg->transformLocalToWorld(domain.v1);
                domain.v2 = prg->rotateLocalToWorld(domain.v2);
                domain.v2.normalize();  // Normal
                break;
            default: break;
            }
        }
    }
}

void DomainOperator::endOperate()
{
    if ( _backupDomains.size()>0 )
    {
        _domains = _backupDomains;
        _backupDomains.clear();
    }
}
