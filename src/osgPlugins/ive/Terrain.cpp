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

#include "Exception.h"
#include "Terrain.h"
#include "TerrainTile.h"
#include "CoordinateSystemNode.h"

#include <osgTerrain/GeometryTechnique>

using namespace ive;

void Terrain::write(DataOutputStream* out)
{
    // Write Terrain's identification.
    out->writeInt(IVETERRAIN);

    // If the osg class is inherited by any other class we should also write this to file.
    osg::CoordinateSystemNode*  csn = dynamic_cast<osg::CoordinateSystemNode*>(this);
    if(csn)
        ((ive::CoordinateSystemNode*)(csn))->write(out);
    else
        out_THROW_EXCEPTION("Terrain::write(): Could not cast this osgTerrain::Terrain to an osg::CoordinateSystemNode.");

    out->writeFloat(getSampleRatio());
    out->writeFloat(getVerticalScale());
    out->writeInt(getBlendingPolicy());

    TerrainTile::writeTerrainTechnique(out, getTerrainTechniquePrototype());
}

void Terrain::read(DataInputStream* in)
{
    // Peek on Terrain's identification.
    int id = in->peekInt();
    if (id != IVETERRAIN) in_THROW_EXCEPTION("TerrainTile::read(): Expected Terrain identification.");

    // Read Terrain's identification.
    id = in->readInt();

    osg::CoordinateSystemNode*  csn = dynamic_cast<osg::CoordinateSystemNode*>(this);
    if(csn)
        ((ive::CoordinateSystemNode*)(csn))->read(in);
    else
        in_THROW_EXCEPTION("Terrain::read(): Could not cast this osgTerran::Terrain to an osg::CoordinateSystemNode.");

    setSampleRatio(in->readFloat());
    setVerticalScale(in->readFloat());
    setBlendingPolicy(static_cast<osgTerrain::TerrainTile::BlendingPolicy>(in->readInt()));

    setTerrainTechniquePrototype(TerrainTile::readTerrainTechnique(in));
}

