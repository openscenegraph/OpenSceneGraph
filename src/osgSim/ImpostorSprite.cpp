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
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/AlphaFunc>
#include <osg/Notify>

#include <osgSim/ImpostorSprite>

using namespace osg;
using namespace osgSim;

ImpostorSprite::ImpostorSprite():
    _parent(0),
    _ism(0),
    _previous(0),
    _next(0),
    _lastFrameUsed(osg::UNINITIALIZED_FRAME_NUMBER),
    _texture(0),
    _s(0),
    _t(0)
{
    // don't use display list since we will be updating the geometry.
    setUseDisplayList(false);

    init();
}

ImpostorSprite::ImpostorSprite(const ImpostorSprite&):
    osg::Geometry(),
    _parent(0),
    _ism(0),
    _previous(0),
    _next(0),
    _lastFrameUsed(osg::UNINITIALIZED_FRAME_NUMBER),
    _texture(0),
    _s(0),
    _t(0)
{
    setUseDisplayList(false);

    init();
}

ImpostorSprite::~ImpostorSprite()
{
    if (_ism)
    {
        _ism->remove(this);
    }
}

void ImpostorSprite::init()
{
    _coords = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX, 4);
    _texcoords = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX, 4);

    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(osg::Array::BIND_OVERALL);
    colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    setVertexArray(_coords.get());
    setColorArray(colors.get());
    setTexCoordArray(0, _texcoords.get());

    addPrimitiveSet(new osg::DrawArrays(GL_QUADS, 0, 4));
}

void ImpostorSprite::dirty()
{
    _coords->dirty();
    _texcoords->dirty();

    dirtyGLObjects();
    dirtyBound();
}

float ImpostorSprite::calcPixelError(const osg::Matrix& MVPW) const
{
    // find the maximum screen space pixel error between the control coords and the quad corners.
    float max_error_sqrd = 0.0f;

    for(int i=0;i<4;++i)
    {

        osg::Vec3 projected_coord = (*_coords)[i]*MVPW;
        osg::Vec3 projected_control = _controlcoords[i]*MVPW;

        float dx = (projected_coord.x()-projected_control.x());
        float dy = (projected_coord.y()-projected_control.y());

        float error_sqrd = dx*dx+dy*dy;

        if (error_sqrd > max_error_sqrd) max_error_sqrd = error_sqrd;

    }

    return sqrtf(max_error_sqrd);
}

void ImpostorSprite::setTexture(osg::Texture2D* tex,int s,int t)
{
    _texture = tex;
    _s = s;
    _t = t;
}


///////////////////////////////////////////////////////////////////////////
// Helper class for managing the reuse of ImpostorSprite resources.
///////////////////////////////////////////////////////////////////////////

ImpostorSpriteManager::ImpostorSpriteManager():
    _first(NULL),
    _last(NULL)
{
    _texenv = new osg::TexEnv;
    _texenv->setMode(osg::TexEnv::REPLACE);

    _alphafunc = new osg::AlphaFunc;
    _alphafunc->setFunction( osg::AlphaFunc::GREATER, 0.000f );

    _reuseStateSetIndex = 0;
}


ImpostorSpriteManager::~ImpostorSpriteManager()
{
    while (_first)
    {
        ImpostorSprite* next = _first->_next;
        _first->_ism = NULL;
        _first->_previous = NULL;
        _first->_next = NULL;
        _first = next;
    }

}

void ImpostorSpriteManager::push_back(ImpostorSprite* is)
{
    if (is==NULL || is==_last) return;

    // remove entry for existing position in linked list
    // if it is already inserted.
    if (is->_previous)
    {
        (is->_previous)->_next = is->_next;
    }

    if (is->_next)
    {
        (is->_next)->_previous = is->_previous;
    }

    if (_first==is) _first = is->_next;

    if (empty())
    {
        _first = is;
        _last = is;
        is->_ism = this;
        is->_previous = NULL;
        is->_next = NULL;
    }
    else
    {

        // now add the element into the list.
        ImpostorSprite* previous_last = _last;
        previous_last->_next = is;
        _last = is;
        _last->_ism = this;
        _last->_previous = previous_last;
        _last->_next = NULL;
    }
}

void ImpostorSpriteManager::remove(ImpostorSprite* is)
{
    if (is==NULL) return;

    // remove entry for existing position in linked list
    // if it is already inserted.
    if (is->_previous)
    {
        (is->_previous)->_next = is->_next;
    }

    if (is->_next)
    {
        (is->_next)->_previous = is->_previous;
    }

    if (_first==is) _first = is->_next;
    if (_last==is) _last = is->_previous;
}

ImpostorSprite* ImpostorSpriteManager::createOrReuseImpostorSprite(int s,int t,unsigned int frameNumber)
{
    if (!empty())
    {

        // search for a valid impostor to reuse.
        ImpostorSprite* curr = _first;
        while (curr)
        {
            if (curr->getLastFrameUsed()<=frameNumber &&
                curr->s()==s &&
                curr->t()==t)
            {
                push_back(curr);
                return curr;
            }
            else
            {
                curr = curr->_next;
            }
        }

    }

    // creating new impostor sprite.


    osg::StateSet* stateset = new osg::StateSet;

    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);

    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    stateset->setAttributeAndModes( _alphafunc.get(), osg::StateAttribute::ON );

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    stateset->setTextureAttribute(0,_texenv.get());

/*
    TexEnv* texenv = new TexEnv;
    texenv->setMode(TexEnv::REPLACE);
    stateset->setAttribute(texenv);

    AlphaFunc* alphafunc = new osg::AlphaFunc;
    alphafunc->setFunction( osg::AlphaFunc::GREATER, 0.000f );
    stateset->setAttributeAndModes( alphafunc, osg::StateAttribute::ON );
*/


    //    stateset->setMode( GL_ALPHA_TEST, osg::StateAttribute::OFF );

    ImpostorSprite* is = new ImpostorSprite;
    is->setStateSet(stateset);
    is->setTexture(texture,s,t);

    push_back(is);

    return is;

}

osg::StateSet* ImpostorSpriteManager::createOrReuseStateSet()
{
    if (_reuseStateSetIndex<_stateSetList.size())
    {
        return _stateSetList[_reuseStateSetIndex++].get();
    }
    _stateSetList.push_back(new osg::StateSet);
    _reuseStateSetIndex=_stateSetList.size();
    return _stateSetList.back().get();
}

void ImpostorSpriteManager::reset()
{
    _reuseStateSetIndex = 0;
}
