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

//
// OpenFlight(R) loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/Texture2D>
#include <osg/StateSet>
#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include "AttrData.h"
#include "DataInputStream.h"
#include "DataOutputStream.h"

using namespace osg;
using namespace osgDB;
using namespace flt;


class ReaderWriterATTR : public osgDB::ReaderWriter
{
    public:

        ReaderWriterATTR()
        {
            supportsExtension("attr","OpenFlight texture attribute format");
        }

        virtual const char* className() const { return "ATTR Image Attribute Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return equalCaseInsensitive(extension,"attr");
        }

        virtual ReadResult readObject(const std::string& fileName, const ReaderWriter::Options*) const;
        virtual ReaderWriter::WriteResult writeObject(const osg::Object& object, const std::string& fileName, const Options* options) const;
};


ReaderWriter::ReadResult ReaderWriterATTR::readObject(const std::string& file, const ReaderWriter::Options* options) const
{
    using std::ios;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    osgDB::ifstream fin;
    fin.imbue(std::locale::classic());
    fin.open(fileName.c_str(), std::ios::in | std::ios::binary);

    if ( fin.fail())
        return ReadResult::ERROR_IN_READING_FILE;

    flt::DataInputStream in(fin.rdbuf());

    AttrData* attr = new AttrData;

    attr->texels_u  = in.readInt32();
    attr->texels_v  = in.readInt32();
    attr->direction_u  = in.readInt32();
    attr->direction_v  = in.readInt32();
    attr->x_up  = in.readInt32();
    attr->y_up  = in.readInt32();
    attr->fileFormat  = in.readInt32();
    attr->minFilterMode  = in.readInt32();
    attr->magFilterMode  = in.readInt32();
    attr->wrapMode  = in.readInt32(AttrData::WRAP_REPEAT);

    attr->wrapMode_u  = in.readInt32();
    if (attr->wrapMode_u == AttrData::WRAP_NONE)
        attr->wrapMode_u = attr->wrapMode;

    attr->wrapMode_v  = in.readInt32();
    if (attr->wrapMode_v == AttrData::WRAP_NONE)
        attr->wrapMode_v = attr->wrapMode;

    attr->modifyFlag = in.readInt32();
    attr->pivot_x  = in.readInt32();
    attr->pivot_y  = in.readInt32();

    // v11 ends here
//      if (in.eof() || (_flt_version <= 11)) return true;
#if 1
    attr->texEnvMode = in.readInt32(AttrData::TEXENV_MODULATE);
    attr->intensityAsAlpha = in.readInt32();
    in.forward(4*8);
    in.forward(4);
    attr->size_u = in.readFloat64();
    attr->size_v = in.readFloat64();
    attr->originCode = in.readInt32();
    attr->kernelVersion = in.readInt32();
    attr->intFormat = in.readInt32();
    attr->extFormat = in.readInt32();
    attr->useMips = in.readInt32();
    for (int n=0; n<8; n++)
        attr->of_mips[n] = in.readFloat32();
    attr->useLodScale = in.readInt32();
    attr->lod0 = in.readFloat32();
    attr->scale0 = in.readFloat32();
    attr->lod1 = in.readFloat32();
    attr->scale1 = in.readFloat32();
    attr->lod2 = in.readFloat32();
    attr->scale2 = in.readFloat32();
    attr->lod3 = in.readFloat32();
    attr->scale3 = in.readFloat32();
    attr->lod4 = in.readFloat32();
    attr->scale4 = in.readFloat32();
    attr->lod5 = in.readFloat32();
    attr->scale5 = in.readFloat32();
    attr->lod6 = in.readFloat32();
    attr->scale6 = in.readFloat32();
    attr->lod7 = in.readFloat32();
    attr->scale7 = in.readFloat32();
    attr->clamp = in.readFloat32();
    attr->magFilterAlpha = in.readInt32();
    attr->magFilterColor = in.readInt32();
    in.forward(4);
    in.forward(4*8);
    attr->lambertMeridian = in.readFloat64();
    attr->lambertUpperLat = in.readFloat64();
    attr->lambertlowerLat = in.readFloat64();
    in.forward(8);
    in.forward(4*5);
    attr->useDetail = in.readInt32(  );
    attr->txDetail_j = in.readInt32();
    attr->txDetail_k = in.readInt32();
    attr->txDetail_m = in.readInt32();
    attr->txDetail_n = in.readInt32();
    attr->txDetail_s = in.readInt32( );
    attr->useTile = in.readInt32();
    attr->txTile_ll_u= in.readFloat32();
    attr->txTile_ll_v = in.readFloat32();
    attr->txTile_ur_u = in.readFloat32();
    attr->txTile_ur_v = in.readFloat32();
    attr->projection = in.readInt32();
    attr->earthModel = in.readInt32();
    in.forward(4);
    attr->utmZone = in.readInt32();
    attr->imageOrigin = in.readInt32();
    attr->geoUnits = in.readInt32();
    in.forward(4);
    in.forward(4);
    attr->hemisphere = in.readInt32();
    in.forward(4);
    in.forward(4);
    in.forward(149*4);
    attr->comments = in.readString(512);

    // v12 ends here
//      if (in.eof() || (_flt_version <= 12)) return true;

    in.forward(14*4);
    attr->attrVersion = in.readInt32();
    attr->controlPoints = in.readInt32();
    attr->numSubtextures = in.readInt32();
#endif

    fin.close();

    return attr;
}


ReaderWriter::WriteResult
ReaderWriterATTR::writeObject(const osg::Object& object, const std::string& fileName, const Options* options) const
{
    using std::ios;

    std::string ext = osgDB::getLowerCaseFileExtension( fileName );
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    const AttrData* attr = dynamic_cast< const AttrData* >( &object );
    if (attr == NULL)
    {
        OSG_FATAL << "AttrWriter: Invalid Object." << std::endl;
        return WriteResult::FILE_NOT_HANDLED;
    }

    osgDB::ofstream fOut;
    fOut.open( fileName.c_str(), std::ios::out | std::ios::binary );

    if ( fOut.fail())
        return WriteResult::ERROR_IN_WRITING_FILE;

    flt::DataOutputStream out( fOut.rdbuf() );


    out.writeInt32( attr->texels_u );
    out.writeInt32( attr->texels_v );
    out.writeInt32( attr->direction_u );
    out.writeInt32( attr->direction_v );
    out.writeInt32( attr->x_up );
    out.writeInt32( attr->y_up );
    out.writeInt32( attr->fileFormat );
    out.writeInt32( attr->minFilterMode );
    out.writeInt32( attr->magFilterMode );
    out.writeInt32( attr->wrapMode );

    out.writeInt32( attr->wrapMode_u );
    out.writeInt32( attr->wrapMode_v );

    out.writeInt32( attr->modifyFlag );
    out.writeInt32( attr->pivot_x );
    out.writeInt32( attr->pivot_y );

    out.writeInt32( attr->texEnvMode );
    out.writeInt32( attr->intensityAsAlpha );
    out.writeFill( 4*8 );
    out.writeFloat64( attr->size_u );
    out.writeFloat64( attr->size_v );
    out.writeInt32( attr->originCode );
    out.writeInt32( attr->kernelVersion );
    out.writeInt32( attr->intFormat );
    out.writeInt32( attr->extFormat );
    out.writeInt32( attr->useMips );
    for (int n=0; n<8; n++)
        out.writeFloat32( attr->of_mips[n] );
    out.writeInt32( attr->useLodScale );
    out.writeFloat32( attr->lod0 );
    out.writeFloat32( attr->scale0 );
    out.writeFloat32( attr->lod1 );
    out.writeFloat32( attr->scale1 );
    out.writeFloat32( attr->lod2 );
    out.writeFloat32( attr->scale2 );
    out.writeFloat32( attr->lod3 );
    out.writeFloat32( attr->scale3 );
    out.writeFloat32( attr->lod4 );
    out.writeFloat32( attr->scale4 );
    out.writeFloat32( attr->lod5 );
    out.writeFloat32( attr->scale5 );
    out.writeFloat32( attr->lod6 );
    out.writeFloat32( attr->scale6 );
    out.writeFloat32( attr->lod7 );
    out.writeFloat32( attr->scale7 );
    out.writeFloat32( attr->clamp );
    out.writeInt32( attr->magFilterAlpha );
    out.writeInt32( attr->magFilterColor );
    out.writeFill( 4 );
    out.writeFill( 4*8 );
    out.writeFloat64( attr->lambertMeridian );
    out.writeFloat64( attr->lambertUpperLat );
    out.writeFloat64( attr->lambertlowerLat );
    out.writeFill( 8 );
    out.writeFill( 4*5 );
    out.writeInt32( attr->useDetail );
    out.writeInt32( attr->txDetail_j );
    out.writeInt32( attr->txDetail_k );
    out.writeInt32( attr->txDetail_m );
    out.writeInt32( attr->txDetail_n );
    out.writeInt32( attr->txDetail_s );
    out.writeInt32( attr->useTile );
    out.writeFloat32( attr->txTile_ll_u );
    out.writeFloat32( attr->txTile_ll_v );
    out.writeFloat32( attr->txTile_ur_u );
    out.writeFloat32( attr->txTile_ur_v );
    out.writeInt32( attr->projection );
    out.writeInt32( attr->earthModel );
    out.writeFill( 4 );
    out.writeInt32( attr->utmZone );
    out.writeInt32( attr->imageOrigin );
    out.writeInt32( attr->geoUnits );
    out.writeFill( 4 );
    out.writeFill( 4 );
    out.writeInt32( attr->hemisphere );
    out.writeFill( 4 );
    out.writeFill( 4 );
    out.writeFill( 149*4 );
    out.writeString( attr->comments, 512 );

    out.writeFill( 13*4 );
    out.writeInt32( attr->attrVersion );
    out.writeInt32( attr->controlPoints );
    out.writeInt32( attr->numSubtextures );


    fOut.close();

    return WriteResult::FILE_SAVED;
}



// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(attr, ReaderWriterATTR)
