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
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/

#include <osgShadow/ViewDependentShadowTechnique>
#include <osgShadow/ShadowedScene>

using namespace osgShadow;


ViewDependentShadowTechnique::ViewDependentShadowTechnique()
{
    dirty();
}

ViewDependentShadowTechnique::ViewDependentShadowTechnique
    (const ViewDependentShadowTechnique& copy, const osg::CopyOp& copyop):
        ShadowTechnique(copy,copyop)
{
    dirty();
}

ViewDependentShadowTechnique::~ViewDependentShadowTechnique(void)
{

}

void ViewDependentShadowTechnique::traverse(osg::NodeVisitor& nv)
{
    osgShadow::ShadowTechnique::traverse(nv);
}

void ViewDependentShadowTechnique::dirty()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);

    osgShadow::ShadowTechnique::_dirty = true;

    for( ViewDataMap::iterator mitr = _viewDataMap.begin();
         mitr != _viewDataMap.end();
         ++mitr )
    {
        mitr->second->dirty( true );
    }
}

void ViewDependentShadowTechnique::init()
{
    //osgShadow::ShadowTechnique::init( );
    osgShadow::ShadowTechnique::_dirty = false;
}

void ViewDependentShadowTechnique::update(osg::NodeVisitor& nv)
{
    //osgShadow::ShadowTechnique::update( nv );
    osgShadow::ShadowTechnique::_shadowedScene->osg::Group::traverse(nv);
}

void ViewDependentShadowTechnique::cull(osgUtil::CullVisitor& cv)
{
    //osgShadow::ShadowTechnique::cull( cv );

    ViewData * vd = getViewDependentData( &cv );

    if ( !vd || vd->_dirty || vd->_cv != &cv || vd->_st != this ) {
        vd = initViewDependentData( &cv, vd );
        setViewDependentData( &cv, vd );
    }

    if( vd ) {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(vd->_mutex);
        vd->cull();
    } else {
        osgShadow::ShadowTechnique::_shadowedScene->osg::Group::traverse(cv);
    }
}

void ViewDependentShadowTechnique::cleanSceneGraph()
{
    //osgShadow::ShadowTechnique::cleanSceneGraph( );
}

ViewDependentShadowTechnique::ViewData *
ViewDependentShadowTechnique::getViewDependentData( osgUtil::CullVisitor * cv )
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);
    return _viewDataMap[ cv ].get();
}

void ViewDependentShadowTechnique::setViewDependentData
    ( osgUtil::CullVisitor * cv, ViewData * data )
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_viewDataMapMutex);
    _viewDataMap[ cv ] = data;
}

void ViewDependentShadowTechnique::ViewData::dirty( bool flag )
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
    _dirty = flag;
}

void ViewDependentShadowTechnique::ViewData::init
    (  ViewDependentShadowTechnique *st, osgUtil::CullVisitor * cv )
{
    _cv = cv;
    _st = st;
    dirty( false );
}

void ViewDependentShadowTechnique::ViewData::cull( void )
{

}

