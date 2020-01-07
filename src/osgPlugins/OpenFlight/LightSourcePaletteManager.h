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

#ifndef __FLTEXP_LIGHT_SOURCE_PALETTE_MANAGER_H__
#define __FLTEXP_LIGHT_SOURCE_PALETTE_MANAGER_H__ 1

#include "ExportOptions.h"

#include <set>

namespace osg {
    class Light;
}


namespace flt
{


class DataOutputStream;


class LightSourcePaletteManager : public osg::Referenced
{
  public:
      LightSourcePaletteManager();

    // Add a light to the palette and auto-assign it the next available index
    int add(osg::Light const* light);

    // Write the light palette records out to a DataOutputStream
    void write( DataOutputStream& dos ) const;


  private:
    int _currIndex;

    // Helper struct to hold light palette records
    struct LightRecord
    {
        LightRecord(osg::Light const* light, int i)
            : Light(light)
            , Index(i) { }

        osg::Light const* Light;
        int Index;

    };

    // Map to allow lookups by Light pointer
    typedef std::map<osg::Light const*, LightRecord> LightPalette;
    LightPalette _lightPalette;

protected:

    LightSourcePaletteManager& operator = (const LightSourcePaletteManager&) { return *this; }
};


}  // End namespace fltexp

#endif  // !__FLTEXP_LIGHT_SOURCE_PALETTE_MANAGER_H__
