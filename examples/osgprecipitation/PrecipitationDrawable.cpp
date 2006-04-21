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

#include "PrecipitationDrawable.h"

#include <osg/Notify>
#include <osg/io_utils>

using namespace osgParticle;

PrecipitationDrawable::PrecipitationDrawable()
{
    setSupportsDisplayList(false);
}

PrecipitationDrawable::PrecipitationDrawable(const PrecipitationDrawable& copy, const osg::CopyOp& copyop):
    Drawable(copy,copyop)
{
}

void PrecipitationDrawable::setParameters(PrecipitationParameters* parameters)
{
    _parameters = parameters;
}


void PrecipitationDrawable::compileGLObjects(osg::State& state) const
{
    osg::notify(osg::NOTICE)<<"PrecipitationDrawable::compileGLObjects()"<<this<<std::endl;
}


void PrecipitationDrawable::drawImplementation(osg::State& state) const
{
    if (!_geometry) return;

    const osg::Geometry::Extensions* extensions = osg::Geometry::getExtensions(state.getContextID(),true);
    
    // save OpenGL matrices
    glPushMatrix();
    glMatrixMode( GL_TEXTURE );
    glPushMatrix();

    state.setActiveTextureUnit(0);

    for(CellMatrixMap::const_iterator itr = _currentCellMatrixMap.begin();
        itr != _currentCellMatrixMap.end();
        ++itr)
    {

        float _startTime = 0.0f;
        extensions->glMultiTexCoord1f(GL_TEXTURE0+1, _startTime);

        // load cells current modelview matrix
        glMatrixMode( GL_MODELVIEW );
        glLoadMatrix(itr->second.ptr());


        CellMatrixMap::const_iterator pitr = _previousCellMatrixMap.find(itr->first);
        if (pitr != _previousCellMatrixMap.end())
        {
            // load previous frame modelview matrix for motion blurr effect
            glMatrixMode( GL_TEXTURE );
            glLoadMatrix(pitr->second.ptr());    
        }
        else
        {
            // use current modelview matrix as "previous" frame value, cancelling motion blurr effect
            glMatrixMode( GL_TEXTURE );
            glLoadMatrix(itr->second.ptr());    
        }

        _geometry->draw(state);

    }

    // restore OpenGL matrices
    glMatrixMode( GL_TEXTURE );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );
    glPopMatrix();

    
}
