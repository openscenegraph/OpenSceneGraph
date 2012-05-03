/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield
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

#ifndef IVE_LAYER
#define IVE_LAYER 1

#include <osgTerrain/Layer>
#include "ReadWrite.h"

namespace ive
{

class LayerHelper
{
    public:

        LayerHelper() {}

        void writeLayer(DataOutputStream* out, osgTerrain::Layer* layer);
        osgTerrain::Layer* readLayer(DataInputStream* in);

        void writeLocator(DataOutputStream* out, osgTerrain::Locator* locator);
        osgTerrain::Locator* readLocator(DataInputStream* in);
};

class Layer : public osgTerrain::Layer, public ReadWrite
{
    public:
    void write(DataOutputStream* out);
    void read(DataInputStream* in);

    void writeValidDataOperator(DataOutputStream* out, osgTerrain::ValidDataOperator* validDataOperator);
    osgTerrain::ValidDataOperator* readValidDataOperator(DataInputStream* in);
};


}

#endif
