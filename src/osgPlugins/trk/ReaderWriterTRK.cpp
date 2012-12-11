// Released under the OSGPL license, as part of the OpenSceneGraph distribution.
//
// specification : http://www.trackvis.org/docs/?subsect=fileformat
//
#include <osg/Geode>
#include <osg/io_utils>

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
            bool requiresByteSwap = header.hdr_size!=1000;
            if (requiresByteSwap)
            {
                OSG_NOTICE<<"Requires byteswap, but not implemented yet."<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }
            else
            {
                OSG_NOTICE<<"No byteswap required"<<std::endl;
            }

            OSG_NOTICE<<"version = "<<header.version<<std::endl;
            OSG_NOTICE<<"dim = "<<header.dim[0]<<", "<<header.dim[1]<<", "<<header.dim[2]<<std::endl;
            OSG_NOTICE<<"voxel_size = "<<header.voxel_size[0]<<", "<<header.voxel_size[1]<<", "<<header.voxel_size[2]<<std::endl;
            OSG_NOTICE<<"origin = "<<header.origin[0]<<", "<<header.origin[1]<<", "<<header.origin[2]<<std::endl;
            OSG_NOTICE<<"n_scalars = "<<header.n_scalars<<std::endl;
            OSG_NOTICE<<"n_properties = "<<header.n_properties<<std::endl;

            osg::Matrixf matrix(reinterpret_cast<const float*>(header.vox_to_ras));
            OSG_NOTICE<<"vox_to_ras="<<matrix<<std::endl;
            
            OSG_NOTICE<<"invert_x="<<static_cast<int>(header.invert_x)<<std::endl;
            OSG_NOTICE<<"invert_y="<<static_cast<int>(header.invert_y)<<std::endl;
            OSG_NOTICE<<"invert_z="<<static_cast<int>(header.invert_z)<<std::endl;
            OSG_NOTICE<<"swap_xy="<<static_cast<int>(header.swap_xy)<<std::endl;
            OSG_NOTICE<<"swap_yz="<<static_cast<int>(header.swap_yz)<<std::endl;
            OSG_NOTICE<<"swap_zx="<<static_cast<int>(header.swap_zx)<<std::endl;
            OSG_NOTICE<<"n_count="<<header.n_count<<std::endl;
            OSG_NOTICE<<"hdr_size="<<header.hdr_size<<std::endl;

            if (header.n_count==0)
            {
                OSG_NOTICE<<"No tracks stored."<<std::endl;
                return ReadResult::FILE_NOT_HANDLED;
            }

            int n_s = header.n_scalars;
            int n_p = header.n_properties;

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
            geode->addDrawable(geometry.get());

            osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet;
            stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            geometry->setStateSet(stateset.get());
            
            osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
            geometry->setColorArray(colours.get());
            geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
            colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

            osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
            geometry->setVertexArray(vertices.get());

            osg::ref_ptr<osg::DrawElementsUInt> lines = new osg::DrawElementsUInt(GL_LINES);
            geometry->addPrimitiveSet(lines.get());

            for(int i=0; i<header.n_count; ++i)
            {
                // read track
                // OSG_NOTICE<<"reading track #"<<i<<std::endl;

                int n_points=0;
                fin.read(reinterpret_cast<char*>(&n_points),4);
                if (fin.fail()) break;

                // OSG_NOTICE<<"  n_points="<<n_points<<std::endl;

                int n_floats_per_vertex = (3+n_s);
                int n_point_floats = n_floats_per_vertex*n_points;
                float* point_data = new float[n_point_floats];
                fin.read(reinterpret_cast<char*>(point_data), n_point_floats*4);
                if (fin.fail()) break;
                
                if (n_p!=0)
                {
                    float* property_data = new float[n_p];
                    fin.read(reinterpret_cast<char*>(&property_data), n_p*4);
                    if (fin.fail()) break;

                    delete [] property_data;

                }

                if (n_points>0)
                {
                    // record the index of the first vertex of the new track
                    int vi = vertices->size();

                    // add the vertices of the track
                    for(int pi=0; pi<n_points; ++pi)
                    {
                        float* vptr = &point_data[pi*n_floats_per_vertex];
                        vertices->push_back(osg::Vec3(vptr[0],vptr[1],vptr[2]));
                    }

                    // add the line segmenets for track
                    for(int pi=0; pi<n_points-1; ++pi, ++vi)
                    {
                        lines->push_back(vi);
                        lines->push_back(vi+1);
                    }
                }

                delete [] point_data;
            }
            
            if (fin.fail())
            {
                OSG_NOTICE<<"Error on reading track file."<<std::endl;
                return ReadResult::ERROR_IN_READING_FILE;
            }

            return geode.release();
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
