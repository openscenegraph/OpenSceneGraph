#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

#include <iostream>
#include <iomanip>
#include <stdio.h>
#include <string.h>

#include <liblas/liblas.hpp>
#include <liblas/reader.hpp>
#include <liblas/point.hpp>
#include <liblas/detail/timer.hpp>

class ReaderWriterLAS : public osgDB::ReaderWriter
{
    public:

        ReaderWriterLAS()
        {
            supportsExtension("las","LAS point cloud format");
            supportsOption("v","Verbose output");
        }

        virtual const char* className() const { return "LAS point cloud reader"; }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO << "Reading file "<<fileName<<std::endl;

            // Reading options
            bool verbose = false;
            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt)
                {
                    if(opt=="v")
                    {
                        verbose = true;
                    }
                }
            }

            /// HEADER ///

            std::ifstream ifs;
            if (!liblas::Open(ifs, file))
            {
                return ReadResult::ERROR_IN_READING_FILE;
            }
            liblas::Reader reader(ifs);
            liblas::Header const& h = reader.GetHeader();
        
            if (verbose) {
                std::cout << "File name: " << file << '\n';
                //std::cout << "Version  : " << reader.GetVersion() << '\n';
                std::cout << "Signature: " << h.GetFileSignature() << '\n';
                std::cout << "Format   : " << h.GetDataFormatId() << '\n';
                std::cout << "Project  : " << h.GetProjectId() << '\n';
                std::cout << "Points count: " << h.GetPointRecordsCount() << '\n';
                std::cout << "VLRecords count: " << h.GetRecordsCount() << '\n';
                std::cout << "Points by return: ";
                std::copy(h.GetPointRecordsByReturnCount().begin(),
                          h.GetPointRecordsByReturnCount().end(),
                          std::ostream_iterator<uint32_t>(std::cout, " "));
                std::cout << std::endl;
            }


            // POINTS ////

            unsigned int targetNumVertices = 10000;

            osg::Geode* geode = new osg::Geode;

            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            osg::Vec4ubArray* colours = new osg::Vec4ubArray;

            vertices->reserve(targetNumVertices);
            colours->reserve(targetNumVertices);

            liblas::detail::Timer t;
            t.start();

            // This is legacy from libLas Read example.
            // The limits are not used in the OSG actual code
            // It is left for visual "verification" using the cout output
            typedef std::pair<double, double> minmax_t;
            minmax_t mx (DBL_MAX, -DBL_MAX);
            minmax_t my (DBL_MAX, -DBL_MAX);
            minmax_t mz (DBL_MAX, -DBL_MAX);

            while (reader.ReadNextPoint())
            {
                liblas::Point const& p = reader.GetPoint();

                mx.first = std::min<double>(mx.first, p[0]);
                mx.second = std::max<double>(mx.second, p[0]);
                my.first = std::min<double>(my.first, p[1]);
                my.second = std::max<double>(my.second, p[1]);
                mz.first = std::min<double>(mz.first, p[2]);
                mz.second = std::max<double>(mz.second, p[2]);
            }

            double const d1 = t.stop();
            t.start();

            if (verbose) {
                std::cout << "Min Max calculation. Elapsed Time: " << d1 << "\n"
                    << std::fixed << std::setprecision(6)
                    << "\nX: (" << mx.first << ", " << mx.second << ")"
                    << "\nY: (" << my.first << ", " << my.second << ")"
                    << "\nZ: (" << mz.first << ", " << mz.second << ")"
                    << std::endl;
            }

            // calculate the mid point of the point cloud
            double mid_x = 0.5*(mx.second - mx.first);
            double mid_y = 0.5*(my.second - my.first);
            double mid_z = 0.5*(mz.second - mz.first);

            // now we do a second pass subtracting the mid point to each point
            reader.Reset();
            uint32_t i = 0;
            while (reader.ReadNextPoint())
            {
                liblas::Point const& p = reader.GetPoint();
            
                // Extract color components from LAS point
                liblas::Color c = p.GetColor();
                uint32_t r = c.GetRed();
                uint32_t g = c.GetGreen();
                uint32_t b = c.GetBlue();
                uint32_t a = 255;    // default value, since LAS point has no alpha information

                if (vertices->size()>=targetNumVertices) 
                {
                    // finishing setting up the current geometry and add it to the geode.
                    geometry->setUseDisplayList(true);
                    geometry->setUseVertexBufferObjects(true);
                    geometry->setVertexArray(vertices);
                    geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
                    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

                    geode->addDrawable(geometry);

                    // allocate a new geometry
                    geometry = new osg::Geometry;

                    vertices = new osg::Vec3Array;
                    colours = new osg::Vec4ubArray;

                    vertices->reserve(targetNumVertices);
                    colours->reserve(targetNumVertices);
                }

                vertices->push_back(osg::Vec3d (p[0] - mid_x, p[1] - mid_y, p[2] - mid_z) );
                colours->push_back(osg::Vec4ub(r,g,b,a));

                // Warning: Printing zillion of points may take looong time
                //std::cout << i << ". " << p << '\n';
                i++;
            }
            double const d2 = t.stop();

            if (verbose) {
                std::cout << "Read points: " << i << " Elapsed Time: " << d2
                    << std::endl << std::endl;
            }

            geometry->setUseDisplayList(true);
            geometry->setUseVertexBufferObjects(true);
            geometry->setVertexArray(vertices);
            geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);
            geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,vertices->size()));

            geode->addDrawable(geometry);


            // MatrixTransform with the mid-point translation

            osg::MatrixTransform *mt = new osg::MatrixTransform;
            mt->setMatrix ( osg::Matrix::translate ( osg::Vec3d(mid_x, mid_y, mid_z)) );
            mt->addChild (geode);

            return mt;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(las, ReaderWriterLAS)
