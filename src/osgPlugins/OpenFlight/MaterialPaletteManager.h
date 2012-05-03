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

#ifndef __FLTEXP_MATERIAL_PALETTE_MANAGER_H__
#define __FLTEXP_MATERIAL_PALETTE_MANAGER_H__ 1

#include "ExportOptions.h"
#include <map>

namespace osg {
    class Material;
}


namespace flt
{


class DataOutputStream;


class MaterialPaletteManager
{
  public:
      MaterialPaletteManager( ExportOptions& fltOpt );

    // Add a material to the palette and auto-assign it the next available index
    int add(osg::Material const* material);

    // Write the material palette records out to a DataOutputStream
    void write( DataOutputStream& dos ) const;


  private:
    int _currIndex;

    // Helper struct to hold material palette records
    struct MaterialRecord
    {
        MaterialRecord(osg::Material const* m, int i)
            : Material(m)
            , Index(i) { }

        osg::Material const* Material;
        int Index;

    };

    // Map to allow lookups by Material pointer, and permit sorting by index
    typedef std::map<osg::Material const*, MaterialRecord> MaterialPalette;
    MaterialPalette _materialPalette;

    ExportOptions& _fltOpt;

protected:

    MaterialPaletteManager& operator = (const MaterialPaletteManager&) { return *this; }
};


}  // End namespace fltexp

#endif  // !FLTEXP_MATERIAL_PALETTE_MANAGER_H
