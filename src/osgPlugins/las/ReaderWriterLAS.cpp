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

#include <chrono>
#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>
#include <pdal/io/LasReader.hpp>
#include <pdal/io/LasHeader.hpp>
#include <pdal/Options.hpp>

class ReaderWriterLAS : public osgDB::ReaderWriter
{
    public:

        ReaderWriterLAS()
        {
            supportsExtension("las", "LAS point cloud format");
            supportsExtension("laz", "compressed LAS point cloud format");
            supportsOption("v", "Verbose output");
            supportsOption("noScale", "don't scale vertices according to las header - put scale in matrixTransform");
            supportsOption("noReCenter", "don't transform vertex coords to re-center the pointcloud");
        }

        virtual const char* className() const { return "LAS point cloud reader"; }

        virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const
        {
            return readNode(filename, options);
        }

        virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const 
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            OSG_INFO << "Reading file " << fileName << std::endl;

            try
            {
                pdal::Option las_opt("filename", fileName);
                pdal::Options las_opts;
                las_opts.add(las_opt);
                pdal::PointTable table;
                pdal::LasReader las_reader;
                las_reader.setOptions(las_opts);
                las_reader.prepare(table);
                pdal::PointViewSet point_view_set = las_reader.execute(table);
                pdal::PointViewPtr point_view = *point_view_set.begin();
                pdal::Dimension::IdList dims = point_view->dims();
                pdal::LasHeader h = las_reader.header();

                // Reading options
                bool _verbose = false;
                bool _scale = true;
                bool _recenter = true;
                if (options)
                {
                    std::istringstream iss(options->getOptionString());
                    std::string opt;
                    while (iss >> opt)
                    {
                        if (opt == "v")
                        {
                            _verbose = true;
                        }
                        if (opt == "noScale")
                        {
                            _scale = false;
                        }
                        if (opt == "noReCenter")
                        {
                            _recenter = false;
                        }
                    }
                }

                if (_verbose)
                {
                    //std::cout << "File name: " << file << '\n';
                    //std::cout << "Version  : " << reader.GetVersion() << '\n';
                    std::cout << "Signature: " << h.fileSignature() << '\n';
                    std::cout << "Format   : " << h.pointFormat() << '\n';
                    std::cout << "Project  : " << h.projectId() << '\n';
                    std::cout << "Points count: " << h.pointCount() << '\n';
                    std::cout << "VLRecords count: " << h.vlrCount() << '\n';
                    std::cout << "Points by return: ";
                    for (std::size_t i = 0; i < h.maxReturnCount(); ++i)
                        std::cout << h.pointCountByReturn(i) << " ";
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

                std::chrono::system_clock::time_point t_start = std::chrono::system_clock::now();
                typedef std::pair<double, double> minmax_t;
                minmax_t mx (DBL_MAX, -DBL_MAX);
                minmax_t my (DBL_MAX, -DBL_MAX);
                minmax_t mz (DBL_MAX, -DBL_MAX);

                uint32_t i = 0;
                bool singleColor = true;
                osg::Vec4ub singleColorValue;
                for (pdal::PointId idx = 0; idx < point_view->size(); ++idx)
                {
                    // Extract color components from LAS point
                    uint32_t r = point_view->getFieldAs<int>(pdal::Dimension::Id::Red, idx) >> 8;
                    uint32_t g = point_view->getFieldAs<int>(pdal::Dimension::Id::Green, idx) >> 8;
                    uint32_t b = point_view->getFieldAs<int>(pdal::Dimension::Id::Blue, idx) >> 8;
                    uint32_t a = 255;    // default value, since LAS point has no alpha information

                    if (vertices->size() == 0)
                    {
                        singleColorValue = osg::Vec4ub(r, g, b, a);
                        singleColor = true;
                    }
                    else
                    {
                        if (singleColor)
                        {
                            singleColor = singleColorValue == osg::Vec4ub(r, g, b, a);//set false if different color found
                        }
                    }
                    if (vertices->size() >= targetNumVertices)
                    {
                        // finishing setting up the current geometry and add it to the geode.
                        geometry->setUseDisplayList(true);
                        geometry->setUseVertexBufferObjects(true);
                        geometry->setVertexArray(vertices);
                        if (singleColor)
                        {
                            colours->resize(1);
                            geometry->setColorArray(colours, osg::Array::BIND_OVERALL);
                        }
                        else
                        {
                            geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);

                        }
                        geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));

                        geode->addDrawable(geometry);

                        // allocate a new geometry
                        geometry = new osg::Geometry;

                        vertices = new osg::Vec3Array;
                        colours = new osg::Vec4ubArray;

                        vertices->reserve(targetNumVertices);
                        colours->reserve(targetNumVertices);
                    }
                    double X = point_view->getFieldAs<double>(pdal::Dimension::Id::X, idx) - h.offsetX();
                    double Y = point_view->getFieldAs<double>(pdal::Dimension::Id::Y, idx) - h.offsetY();
                    double Z = point_view->getFieldAs<double>(pdal::Dimension::Id::Z, idx) - h.offsetZ();
                    if (!_scale)
                    {
                        X /= h.scaleX();
                        Y /= h.scaleY();
                        Z /= h.scaleZ();
                    }
                    if (_recenter)
                    {
                        mx.first = std::min<double>(mx.first, X);
                        mx.second = std::max<double>(mx.second, X);
                        my.first = std::min<double>(my.first, Y);
                        my.second = std::max<double>(my.second, Y);
                        mz.first = std::min<double>(mz.first, Z);
                        mz.second = std::max<double>(mz.second, Z);
                    }
                    vertices->push_back(osg::Vec3(X, Y, Z));

                    colours->push_back(osg::Vec4ub(r, g, b, a));

                    // Warning: Printing zillion of points may take looong time
                    //std::cout << i << ". " << p << '\n';
                    i++;
                }
                // calculate the mid point of the point cloud
                double mid_x = 0.5*(mx.second + mx.first);
                double mid_y = 0.5*(my.second + my.first);
                double mid_z = 0.5*(mz.second + mz.first);
                osg::Vec3 midVec(mid_x, mid_y, mid_z);

                geometry->setUseDisplayList(true);
                geometry->setUseVertexBufferObjects(true);
                geometry->setVertexArray(vertices);
                if (singleColor)
                {
                    colours->resize(1);
                    geometry->setColorArray(colours, osg::Array::BIND_OVERALL);
                }
                else
                {
                    geometry->setColorArray(colours, osg::Array::BIND_PER_VERTEX);

                }
                geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, vertices->size()));

                geode->addDrawable(geometry);

                if (_recenter)
                {
                    //Transform vertices to midpoint
                    for (unsigned int geomIndex = 0; geomIndex < geode->getNumDrawables(); ++geomIndex)
                    {
                        osg::Geometry *geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(geomIndex));
                        if (geom)
                        {
                            osg::Vec3Array* vertArray = dynamic_cast<osg::Vec3Array*>(geom->getVertexArray());
                            size_t vertArraySize = vertArray->size();
                            for (size_t vertexIndex = 0; vertexIndex < vertArraySize; ++vertexIndex)
                            {
                                (*vertArray)[vertexIndex] -= midVec;
                            }
                        }
                    }
                }
                std::chrono::system_clock::time_point t_end = std::chrono::system_clock::now();
                std::chrono::microseconds t_duration = std::chrono::duration_cast<std::chrono::microseconds>(t_end - t_start);
                double const d2 = double(t_duration.count())*std::chrono::microseconds::period::num / std::chrono::microseconds::period::den;

                if (_verbose)
                {
                    std::cout << "Read points: " << i << " Elapsed Time: " << d2
                        << std::endl << std::endl;
                }

                // MatrixTransform with the mid-point translation

                osg::MatrixTransform *mt = new osg::MatrixTransform;
                mt->setDataVariance(osg::Object::STATIC);//can be optimized away
                if (_scale)//vertex positions are scaled already
                {
                    if (_recenter)
                    {
                        mt->setMatrix(osg::Matrix::translate(osg::Vec3d(h.offsetX() + mid_x, h.offsetY() + mid_y, h.offsetZ() + mid_z)));
                    }
                    else
                    {
                        mt->setMatrix(osg::Matrix::translate(osg::Vec3d(h.offsetX(), h.offsetY(), h.offsetZ())));
                    }
                }
                else
                {
                    if (_recenter)
                    {
                        mid_x *= h.scaleX();
                        mid_y *= h.scaleY();
                        mid_z *= h.scaleZ();
                        mt->setMatrix(osg::Matrix::scale(osg::Vec3d(h.scaleX(), h.scaleY(), h.scaleZ())) * osg::Matrix::translate(osg::Vec3d(h.offsetX() + mid_x, h.offsetY() + mid_y, h.offsetZ() + mid_z)));
                    }
                    else
                    {
                        mt->setMatrix(osg::Matrix::scale(osg::Vec3d(h.scaleX(), h.scaleY(), h.scaleZ())) * osg::Matrix::translate(osg::Vec3d(h.offsetX(), h.offsetY(), h.offsetZ())));
                    }
                }

                mt->addChild (geode);

                return mt;
            }
            catch (...)
            {
            }
            return ReadResult::ERROR_IN_READING_FILE;
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(las, ReaderWriterLAS)
