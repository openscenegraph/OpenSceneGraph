/* 
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#include "LightSourcePaletteManager.h"
#include "DataOutputStream.h"
#include "Opcodes.h"
#include <osg/Notify>
#include <osg/Light>
#include <osg/Vec4f>
#include <cassert>
#include <sstream>
#include <stdio.h>

namespace flt
{


LightSourcePaletteManager::LightSourcePaletteManager( ExportOptions& fltOpt )
  : _currIndex( -1 ),
    _fltOpt( fltOpt )
    
{
    // TODO: Pay attention to the version here(?)
}


int LightSourcePaletteManager::add(osg::Light const* light)
{
    int index = -1;
    if (light == NULL) return -1;


    // If this light has already been cached, set 'index' to the cached value
    LightPalette::const_iterator it = _lightPalette.find(light);
    if ( it != _lightPalette.end() )
    {
        index = it->second.Index;
    }

    // New light? Add it to the cache...
    else
    {
        index = ++_currIndex;
        _lightPalette.insert(std::make_pair(light,
                                               LightRecord(light, index) ) );
    }

    return index;
}

void
LightSourcePaletteManager::write( DataOutputStream& dos ) const
{
    using osg::Vec4f;

    static int const INFINITE_LIGHT = 0;
    static int const LOCAL_LIGHT    = 1;
    static int const SPOT_LIGHT     = 2;

    LightPalette::const_iterator it = _lightPalette.begin();
    for ( ; it != _lightPalette.end(); ++it)
    {
        LightRecord m = it->second;

        static char lightName[64];
        sprintf(lightName, "Light%02d", m.Light->getLightNum() );

        int lightType = INFINITE_LIGHT;
        Vec4f const& lightPos = m.Light->getPosition();
        if (lightPos.w() != 0)
        {
            if (m.Light->getSpotCutoff() < 180)
                lightType = SPOT_LIGHT;
            else
                lightType = LOCAL_LIGHT;
        }

        dos.writeInt16( (int16) LIGHT_SOURCE_PALETTE_OP );
        dos.writeInt16( 240 );
        dos.writeInt32( m.Index );
        dos.writeFill(2*4, '\0');                     // Reserved
        dos.writeString( lightName, 20 );
        dos.writeFill(4, '\0');                       // Reserved

        dos.writeVec4f(m.Light->getAmbient() );
        dos.writeVec4f(m.Light->getDiffuse() );
        dos.writeVec4f(m.Light->getSpecular() );
        dos.writeInt32(lightType);
        dos.writeFill(4*10, '\0');                     // Reserved
        dos.writeFloat32(m.Light->getSpotExponent() );
        dos.writeFloat32(m.Light->getSpotCutoff() );
        dos.writeFloat32(0);                           // Yaw (N/A)
        dos.writeFloat32(0);                           // Pitch (N/A)
        dos.writeFloat32(m.Light->getConstantAttenuation() );
        dos.writeFloat32(m.Light->getLinearAttenuation() );
        dos.writeFloat32(m.Light->getQuadraticAttenuation() );
        dos.writeInt32(0);                             // Modeling flag (N/A)
        dos.writeFill(4*19, '\0');                     // Reserved

    }
}



}  // End namespace fltexp
