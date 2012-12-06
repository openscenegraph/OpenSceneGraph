// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// specification : http://www.trackvis.org/docs/?subsect=fileformat
#include <osg/Geode>

#include <osg/GL>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

struct TrkHeader
{
    char                id_string[6];
    short               dim[3];
    float               voxel_size[3];
    float               origin[3];
    short               n_scalars;
    char                scalar_name[10][20];
    short               n_properties;
    char                property_name[10][20];
    float               vox_to_ras[4][4];
    char                reserved[444];
    char                voxel_order[4];
    char                pad2[4];
    float               image_orientation_patient[6];
    char                pad1[2];
    unsigned char       invert_x;
    unsigned char       invert_y;
    unsigned char       invert_z;
    unsigned char       swap_xy;
    unsigned char       swap_yz;
    unsigned char       swap_zx;
    int                 n_count;
    int                 version;
    int                 hdr_size;
};


class ReaderWriterTRK : public osgDB::ReaderWriter
{
    public:

        ReaderWriterTRK()
        {
            supportsExtension("trk","Track file format");

            OSG_NOTICE<<"sizeof(TrkHeader)="<<sizeof(TrkHeader)<<std::endl;
        }

        virtual const char* className() const { return "Track Reader/Writer"; }


        virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readNode(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readNode(file, options);
        }

        virtual ReadResult readNode(std::istream& fin,const osgDB::ReaderWriter::Options* =NULL) const
        {
            TrkHeader header;
            fin.read(reinterpret_cast<char*>(&header), sizeof(TrkHeader));

            if (fin.fail()) return ReadResult::ERROR_IN_READING_FILE; 

            OSG_NOTICE<<"Read header successfuly ["<<header.id_string<<"]"<<std::endl;
            OSG_NOTICE<<"version = "<<header.version<<std::endl;
            int swaped_version = header.version;
            osg::swapBytes(swaped_version);
            if (header.version>swaped_version)
            {
                OSG_NOTICE<<"Requires byteswap"<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"No byteswap required"<<std::endl;
            }
            
            
            return ReadResult::FILE_NOT_HANDLED;
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!fin) return ReadResult::FILE_NOT_HANDLED;
            
            return readNode(fin, options);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(trk, ReaderWriterTRK)
