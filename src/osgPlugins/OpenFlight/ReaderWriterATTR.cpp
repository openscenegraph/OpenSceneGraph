//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
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

using namespace osg;
using namespace osgDB;
using namespace flt;


class ReaderWriterATTR : public osgDB::ReaderWriter
{
    public:
    
        virtual const char* className() const { return "ATTR Image Attribute Reader/Writer"; }
        
        virtual bool acceptsExtension(const std::string& extension) const
        {
            return equalCaseInsensitive(extension,"attr");
        }

        virtual ReadResult readObject(const std::string& fileName, const ReaderWriter::Options*) const;
};


ReaderWriter::ReadResult ReaderWriterATTR::readObject(const std::string& file, const ReaderWriter::Options* options) const
{
    using std::ios;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    std::ifstream fin;
    fin.imbue(std::locale::classic());
    fin.open(fileName.c_str(), std::ios::in | std::ios::binary);

    if ( fin.fail())
        return ReadResult::ERROR_IN_READING_FILE;

    flt::DataInputStream in(fin.rdbuf());

    AttrData* attr = new AttrData;

    try
    {
        attr->texels_u  = in.readInt32();
        attr->textel_v  = in.readInt32();
        attr->direction_u  = in.readInt32();
        attr->direction_v  = in.readInt32();
        attr->x_up  = in.readInt32();
        attr->y_up  = in.readInt32();
        attr->fileFormat  = in.readInt32();
        attr->minFilterMode  = in.readInt32();
        attr->magFilterMode  = in.readInt32();
        attr->wrapMode  = in.readInt32();

        attr->wrapMode_u  = in.readInt32();
        if ((attr->wrapMode_u != AttrData::WRAP_CLAMP) && ((attr->wrapMode_u != AttrData::WRAP_REPEAT)))
            attr->wrapMode_u = attr->wrapMode;

        attr->wrapMode_v  = in.readInt32();
        if ((attr->wrapMode_v != AttrData::WRAP_CLAMP) && ((attr->wrapMode_v != AttrData::WRAP_REPEAT)))
            attr->wrapMode_v = attr->wrapMode;

        attr->modifyFlag = in.readInt32();
        attr->pivot_x  = in.readInt32();
        attr->pivot_y  = in.readInt32();

        // v11 ends here
//      if (in.eof() || (_flt_version <= 11)) return true;
#if 1
        attr->texEnvMode = in.readInt32();
        attr->intensityAsAlpha = in.readInt32();
        in.forward(4*8);
        attr->size_u = in.readFloat64();
        attr->size_v = in.readFloat64();
        attr->originCode = in.readInt32();
        attr->kernelVersion = in.readInt32();
        attr->intFormat = in.readInt32();
        attr->extFormat = in.readInt32();
        attr->useMips = in.readInt32();
        for (int n=0; n<8; n++)
            attr->_mips[n] = in.readFloat32();
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
        in.forward(149);
        attr->comments = in.readString(512);

        // v12 ends here
//      if (in.eof() || (_flt_version <= 12)) return true;

        in.forward(13*4);
        attr->attrVersion = in.readInt32();
        attr->controlPoints = in.readInt32();
        in.forward(4);
#endif
    }
    catch(...)
    {
        if (!fin.eof())
        {
            throw;
        }
    }

    fin.close();

    return attr;
}



// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterATTR> g_readerWriter_ATTR_Proxy;












