/* -*-c++-*-
 * Copyright (C) 2020 Scott Giese
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 1.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <iomanip>
#include <iostream>
#include <string>
#include <tuple>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Matrix>
#include <osg/MatrixTransform>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/fstream>

#include <LASlib/lasreader.hpp>
#include <LASlib/lasutility.hpp>


// LAS Specification 1.4 - R15 - Point Data Record Formats 0 - 5
// Valid codes range from 0 - 31
// Codes 14 - 31 are suppressed, being duplicates of the last table entry
static const char * LASClassifications_0_5[14] = {
    "Never Classified",
    "Unclassified",
    "Ground",
    "Low Vegetation",
    "Medium Vegetation",
    "High Vegetation",
    "Building",
    "Low Point (noise)",
    "Model Keypoint",
    "Water",
    "Reserved for ASPRS Definition",
    "Reserved for ASPRS Definition",
    "Overlap Points",
    "Reserved for ASPRS Definition"
};

// LAS Specification 1.4 - R15 -  - Point Data Record Formats 6 - 10
// Valid codes range from 0 - 255
// Codes 24 - 63 are suppressed, being duplicates of the last table entry
// Codes 64 - 255 are suppressed, being User Definable
static const char * LASClassifications_6_10[24] = {
    "Never Classified",
    "Unclassified",
    "Ground",
    "Low Vegetation",
    "Medium Vegetation",
    "High Vegetation",
    "Building",
    "Low Point (noise)",
    "Reserved",
    "Water",
    "Rail",
    "Road Surface",
    "Reserved",
    "Wire Guard (shield)",
    "Wire Conductor (phase)",
    "Transmission Tower",
    "Wire Structure Connector (insulators)",
    "Bridge Deck",
    "High Noise",
    "Overhead Structure",
    "Ignored Ground",
    "Snow",
    "Temporal Exclusion",
    "Reserved"
};

class ReaderWriterLAS : public osgDB::ReaderWriter
{
public:
    ReaderWriterLAS() {
        supportsExtension("las", "LAS point cloud format");
        supportsExtension("laz", "compressed LAS point cloud format");
        supportsOption("v", "Verbose output");
        supportsOption("noScale", "don't scale vertices according to las haeder - put scale in matixTransform");
        supportsOption("noReCenter", "don't transform vertex coords to re-center the pointcloud");
    }

    virtual const char* className() const {
        return "LAS point cloud reader";
    }

    virtual ReadResult readObject(const std::string& filename, const osgDB::ReaderWriter::Options* options) const {
        return readNode(filename, options);
    }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const {
        std::string ext = osgDB::getLowerCaseFileExtension(file);
        if (!acceptsExtension(ext))
            return ReadResult::FILE_NOT_HANDLED;

        std::string fileName = osgDB::findDataFile(file, options);
        if (fileName.empty())
            return ReadResult::FILE_NOT_FOUND;

        OSG_INFO << "Reading file " << fileName << std::endl;

        LASreadOpener lasreadopener;
        lasreadopener.set_file_name(fileName.c_str());

        LASreader* lasreader = lasreadopener.open();
        if (!lasreader) {
            OSG_INFO << "LASLib: Failed to open " << file << std::endl;
            return ReadResult::ERROR_IN_READING_FILE;
        }

        return readNode(*lasreader, options);
    }

    virtual ReadResult readObject(LASreader& lasreader, const osgDB::ReaderWriter::Options* options) const {
        return readNode(lasreader, options);
    }

    virtual ReadResult readNode(LASreader& lasreader, const Options* options) const {
        // Reading options
        bool _verbose = false;
        bool _scale = true;
        bool _recenter = true;

        if (options)
            std::tie(_verbose, _scale, _recenter) = processOptions(options);

        if (_verbose)
            displayHeaderSummary(lasreader);

        osg::ref_ptr<osg::MatrixTransform> mt = new osg::MatrixTransform;

        lasreader.close();
        delete &lasreader;

        return mt;
    }

private:

    std::tuple<bool, bool, bool> processOptions(const Options* options) const {
        bool _verbose = false;
        bool _scale = true;
        bool _recenter = true;

        std::istringstream iss(options->getOptionString());
        std::string opt;

        while (iss >> opt) {
            if (opt == "v")
                _verbose = true;
            if (opt == "noScale")
                _scale = false;
            if (opt == "noReCenter")
                _recenter = false;
        }

        return std::make_tuple(_verbose, _scale, _recenter);
    }

    void displayHeaderSummary(LASreader& lasreader) const {
        LASheader h = lasreader.header;

        std::cout << "file signature:             '" << \
            h.file_signature << "'" << std::endl;
        std::cout << "version major.minor:        " << \
            static_cast<unsigned>(h.version_major) << "." << \
            static_cast<unsigned>(h.version_minor) << std::endl;
        std::cout << "file source ID:             " << \
            h.file_source_ID << std::endl;
        std::cout << "global_encoding:            " << \
            h.global_encoding << std::endl << std::endl;

        std::cout << "project ID:                 " << \
            std::hex << std::uppercase << \
            std::setw(8) << std::setfill('0') << h.project_ID_GUID_data_1 << "-" << \
            std::setw(4) << h.project_ID_GUID_data_2 << "-" << \
            std::setw(4) << h.project_ID_GUID_data_3 << "-" << \
            std::setw(12) << h.project_ID_GUID_data_4 << \
            std::dec << std::setfill(' ') << std::endl;
        std::cout << "system identifier:          '" << \
            std::string(h.system_identifier) << "'" << std::endl;
        std::cout << "generating software:        '" << \
            std::string(h.generating_software) << "'" << std::endl;
        std::cout << "file creation day/year:     " << \
            static_cast<unsigned>(h.file_creation_day) << "/" << \
            static_cast<unsigned>(h.file_creation_year) << std::endl << std::endl;

        std::cout << "header size:                " << \
            h.header_size << std::endl;
        std::cout << "offset to point data:       " << \
            h.offset_to_point_data << std::endl;
        std::cout << "number var. length records: " << \
            h.number_of_variable_length_records << std::endl << std::endl;

        std::cout << "point data format:          " << \
            static_cast<unsigned>(h.point_data_format) << std::endl;
        std::cout << "point data record length:   " << \
            h.point_data_record_length << std::endl;
        std::cout << "number of points:           " << \
            h.number_of_point_records << std::endl;
        std::cout << "number of points by return: " << \
            h.number_of_points_by_return[0] << " " << \
            h.number_of_points_by_return[1] << " " << \
            h.number_of_points_by_return[2] << " " << \
            h.number_of_points_by_return[3] << " " << \
            h.number_of_points_by_return[4] << std::endl << std::endl;

        std::cout << "scale factor x y z:         " << \
            h.x_scale_factor << " " << \
            h.y_scale_factor << " " << \
            h.z_scale_factor << std::endl;
        std::cout << "offset x y z:               " << \
            h.x_offset << " " << \
            h.y_offset << " " << \
            h.z_offset << std::endl;
        std::cout << "min x y z:                  " << \
            h.min_x << "(" << h.min_x * h.x_scale_factor << ") " << \
            h.min_y << "(" << h.min_y * h.y_scale_factor<< ") " << \
            h.min_z << "(" << h.min_z * h.z_scale_factor<< ")" << std::endl;
        std::cout << "max x y z:                  " << \
            h.max_x << "(" << h.max_x * h.x_scale_factor << ") " << \
            h.max_y << "(" << h.max_y * h.y_scale_factor<< ") " << \
            h.max_z << "(" << h.max_z * h.z_scale_factor<< ")" << std::endl << std::endl;

        std::cout << "extended VLR:               " << \
            h.number_of_extended_variable_length_records << std::endl;
        std::cout << "extended points:            " << \
            h.extended_number_of_point_records << std::endl;
        std::cout << "extended points by return:  " << \
            h.extended_number_of_points_by_return[0] << " " << \
            h.extended_number_of_points_by_return[1] << " " << \
            h.extended_number_of_points_by_return[2] << " " << \
            h.extended_number_of_points_by_return[3] << " " << \
            h.extended_number_of_points_by_return[4] << " " << \
            h.extended_number_of_points_by_return[5] << " " << \
            h.extended_number_of_points_by_return[6] << " " << \
            h.extended_number_of_points_by_return[7] << " " << \
            h.extended_number_of_points_by_return[8] << " " << \
            h.extended_number_of_points_by_return[9] << " " << \
            h.extended_number_of_points_by_return[10] << " " << \
            h.extended_number_of_points_by_return[11] << " " << \
            h.extended_number_of_points_by_return[12] << " " << \
            h.extended_number_of_points_by_return[13] << " " << \
            h.extended_number_of_points_by_return[14] << std::endl << std::endl;

        I64 num_first_returns = 0;
        I64 num_intermediate_returns = 0;
        I64 num_last_returns = 0;
        I64 num_single_returns = 0;

        LASsummary lassummary;
        while (lasreader.read_point()) {
            lassummary.add(&lasreader.point);

            if (lasreader.point.is_first())
                num_first_returns++;
            if (lasreader.point.is_intermediate())
                num_intermediate_returns++;
            if (lasreader.point.is_last())
                num_last_returns++;
            if (lasreader.point.is_single())
                num_single_returns++;
        }

        std::cout << "first returns:              " << \
            num_first_returns << std::endl;
        std::cout << "intermediate returns:       " << \
            num_intermediate_returns << std::endl;
        std::cout << "last returns:               " << \
            num_last_returns << std::endl;
        std::cout << "single returns:             " << \
            num_single_returns << std::endl << std::endl;

        std::cout << "classification histogram:" << std::endl;
        for (int i = 0; i < 32; ++i) {
            if (lassummary.classification[i])
                std::cout << std::setw(16) << lassummary.classification[i] << \
                    "  " << LASClassifications_0_5[i] << \
                    " (" << i << ")" << std::endl;
            if (lassummary.classification_synthetic)
                std::cout << " +-> flagged as synthetic: " << \
                    lassummary.classification_synthetic << std::endl;
            if (lassummary.classification_keypoint)
                std::cout << " +-> flagged as keypoints: " << \
                    lassummary.classification_keypoint << std::endl;
            if (lassummary.classification_withheld)
                std::cout << " +-> flagged as withheld:  " << \
                    lassummary.classification_withheld << std::endl;
        }

        if (lasreader.point.extended_point_type) {
            if (lassummary.classification_extended_overlap)
                std::cout << " +-> flagged as extended overlap: " << \
                    lassummary.classification_extended_overlap << std::endl;

            bool wrong_entry = false;
            for (int i = 32; i < 256; ++i)
                if (lassummary.extended_classification[i]) {
                    wrong_entry = true;
                    break;
                }

            if (wrong_entry) {
                std::cout << "extended classification histogram:" << std::endl;
                for (int i = 32; i < 256; ++i)
                    if (lassummary.extended_classification[i])
                        std::cout << std::setw(16) << \
                            lassummary.extended_classification[i] << \
                            "  extended classification (" << \
                            i << ")" << std::endl;
            }
        }
    }
};

// now register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(las, ReaderWriterLAS)
