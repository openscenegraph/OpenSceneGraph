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
#include <osg/Light>
#include <osg/StateSet>
#include <osg/Notify>

using namespace osg;

Light::Light( void )
{
    init();
}

Light::Light(unsigned int lightnum)
{
    init();
    _lightnum = lightnum;
}

Light::~Light( void )
{
}


void Light::init( void )
{
    _lightnum = 0;
    _ambient.set(0.05f,0.05f,0.05f,1.0f);
    _diffuse.set(0.8f,0.8f,0.8f,1.0f);
    _specular.set(0.05f,0.05f,0.05f,1.0f);
    _position.set(0.0f,0.0f,1.0f,0.0f);
    _direction.set(0.0f,0.0f,-1.0f);
    _spot_exponent = 0.0f;
    _spot_cutoff = 180.0f;
    _constant_attenuation = 1.0f;
    _linear_attenuation = 0.0f;
    _quadratic_attenuation = 0.0f;

    //     notify(DEBUG) << "_ambient "<<_ambient<<std::endl;
    //     notify(DEBUG) << "_diffuse "<<_diffuse<<std::endl;
    //     notify(DEBUG) << "_specular "<<_specular<<std::endl;
    //     notify(DEBUG) << "_position "<<_position<<std::endl;
    //     notify(DEBUG) << "_direction "<<_direction<<std::endl;
    //     notify(DEBUG) << "_spot_exponent "<<_spot_exponent<<std::endl;
    //     notify(DEBUG) << "_spot_cutoff "<<_spot_cutoff<<std::endl;
    //     notify(DEBUG) << "_constant_attenuation "<<_constant_attenuation<<std::endl;
    //     notify(DEBUG) << "_linear_attenuation "<<_linear_attenuation<<std::endl;
    //     notify(DEBUG) << "_quadratic_attenuation "<<_quadratic_attenuation<<std::endl;
}

void Light::setLightNum(int num) 
{
    if (_lightnum==num) return;

    if (_parents.empty())
    {
        _lightnum = num;
        return;
    }

    // take a reference to this clip plane to prevent it from going out of scope
    // when we remove it temporarily from its parents.
    osg::ref_ptr<Light> lightRef = this;

    // copy the parents as they _parents list will be changed by the subsequent removeAttributes.
    ParentList parents = _parents;

    // remove this attribute from its parents as its position is being changed
    // and would no longer be valid.
    ParentList::iterator itr;
    for(itr = parents.begin();
        itr != parents.end();
        ++itr)
    {
        osg::StateSet* stateset = *itr;
        stateset->removeAttribute(this);
    }
    
    // assign the hint target
    _lightnum = num;

    // add this attribute back into its original parents with its new position
    for(itr = parents.begin();
        itr != parents.end();
        ++itr)
    {
        osg::StateSet* stateset = *itr;
        stateset->setAttribute(this);
    }
}



void Light::captureLightState()
{
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_AMBIENT, _ambient.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_DIFFUSE, _diffuse.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPECULAR, _specular.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_POSITION, _position.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_DIRECTION, _direction.ptr() );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_EXPONENT, &_spot_exponent );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_CUTOFF,   &_spot_cutoff );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_CONSTANT_ATTENUATION,   &_constant_attenuation );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_LINEAR_ATTENUATION,   &_linear_attenuation );
    glGetLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_QUADRATIC_ATTENUATION,   &_quadratic_attenuation );
}

void Light::apply(State&) const
{
    glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_AMBIENT,               _ambient.ptr() );
    glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_DIFFUSE,               _diffuse.ptr() );
    glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPECULAR,              _specular.ptr() );
    glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_POSITION,              _position.ptr() );
    glLightfv( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_DIRECTION,        _direction.ptr() );
    glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_EXPONENT,         _spot_exponent );
    glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_SPOT_CUTOFF,           _spot_cutoff );
    glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_CONSTANT_ATTENUATION,  _constant_attenuation );
    glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_LINEAR_ATTENUATION,    _linear_attenuation );
    glLightf ( (GLenum)((int)GL_LIGHT0 + _lightnum), GL_QUADRATIC_ATTENUATION, _quadratic_attenuation );
}
