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

ImpostorSprite::ImpostorSprite()
{
    // don't use display list since we will be updating the geometry.
    _useDisplayList = false;
    _parent = NULL;
    
    _ism = NULL;
    _previous = NULL;
    _next = NULL;

    _texture = NULL;
    _s = 0;
    _t = 0;        

    _color.set(1.0f, 1.0f, 1.0f, 1.0f );
}

ImpostorSprite::ImpostorSprite(const ImpostorSprite&): Drawable() {}


ImpostorSprite::~ImpostorSprite()
{
    if (_ism)
    {
        _ism->remove(this);
    }
}

float ImpostorSprite::calcPixelError(const osg::Matrix& MVPW) const
{
    // find the maximum screen space pixel error between the control coords and the quad coners.
    float max_error_sqrd = 0.0f;

    for(int i=0;i<4;++i)
    {

        osg::Vec3 projected_coord = _coords[i]*MVPW;
        osg::Vec3 projected_control = _controlcoords[i]*MVPW;

        float dx = (projected_coord.x()-projected_control.x());
        float dy = (projected_coord.y()-projected_control.y());

        float error_sqrd = dx*dx+dy*dy;
        
        if (error_sqrd > max_error_sqrd) max_error_sqrd = error_sqrd;
        
    }

    return sqrtf(max_error_sqrd);
}
void ImpostorSprite::drawImplementation(osg::State&) const
{
    // when the tex env is set to REPLACE, and the 
    // texture is set up correctly the color has no effect.
    glColor4fv( _color.ptr() );
    
    
    glBegin( GL_QUADS );
    
    glTexCoord2fv( (GLfloat *)&_texcoords[0] );
    glVertex3fv( (GLfloat *)&_coords[0] );

    glTexCoord2fv( (GLfloat *)&_texcoords[1] );
    glVertex3fv( (GLfloat *)&_coords[1] );

    glTexCoord2fv( (GLfloat *)&_texcoords[2] );
    glVertex3fv( (GLfloat *)&_coords[2] );

    glTexCoord2fv( (GLfloat *)&_texcoords[3] );
    glVertex3fv( (GLfloat *)&_coords[3] );

    glEnd();

}

osg::BoundingBox ImpostorSprite::computeBound() const
{
    osg::BoundingBox bbox;
    bbox.expandBy(_coords[0]);
    bbox.expandBy(_coords[1]);
    bbox.expandBy(_coords[2]);
    bbox.expandBy(_coords[3]);

    if (!bbox.valid())
    {
        osg::notify(osg::WARN) << "******* ImpostorSprite::computeBound() problem"<<std::endl;
    }

    return bbox;
}

void ImpostorSprite::setTexture(osg::Texture2D* tex,int s,int t)
{
    _texture = tex;
    _s = s;
    _t = t;
}


void ImpostorSprite::accept(AttributeFunctor& af)
{
    af.apply(VERTICES,4,_coords);
    af.apply(TEXTURE_COORDS_0,4,_texcoords);
}

void ImpostorSprite::accept(ConstAttributeFunctor& af) const
{
    af.apply(VERTICES,4,_coords);
    af.apply(TEXTURE_COORDS_0,4,_texcoords);
}

void ImpostorSprite::accept(osg::PrimitiveFunctor& functor) const
{
    functor.setVertexArray(4,_coords);
    functor.drawArrays( GL_QUADS, 0, 4);
    
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

    // remove entry for exisiting position in linked list
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

    // remove entry for exisiting position in linked list
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

ImpostorSprite* ImpostorSpriteManager::createOrReuseImpostorSprite(int s,int t,int frameNumber)
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
