// -*-c++-*-

/*
 * $Id$
 *
 * STL importer for OpenSceneGraph.
 *
 * Copyright (c) 2004 Ulrich Hertlein <u.hertlein@sandbox.de>
 * Copyright (c) 2012 Piotr Domagalski <piotr@domagalski.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgUtil/TriStripVisitor>
#include <osgUtil/SmoothingVisitor>
#include <osg/TriangleFunctor>

#include <osg/Geode>
#include <osg/Geometry>

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <string.h>


#if defined(_WIN32) && !defined(__MINGW32__) && (!defined(_MSC_VER) || _MSC_VER<1600)

typedef unsigned __int8 uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef signed __int8 int8_t;
typedef signed __int16 int16_t;
typedef signed __int32 int32_t;

#else

#include <stdint.h>

#endif

#include <memory>

struct STLOptionsStruct {
    bool smooth;
    bool separateFiles;
    bool dontSaveNormals;
    bool noTriStripPolygons;
};

STLOptionsStruct parseOptions(const osgDB::ReaderWriter::Options* options)  {

    STLOptionsStruct localOptions;
    localOptions.smooth = false;
    localOptions.separateFiles = false;
    localOptions.dontSaveNormals = false;
    localOptions.noTriStripPolygons = false;

    if (options != NULL)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            if (opt == "smooth")
            {
                localOptions.smooth = true;
            }
            else if (opt == "separateFiles")
            {
                localOptions.separateFiles = true;
            }
            else if (opt == "dontSaveNormals")
            {
                localOptions.dontSaveNormals = true;
            }
            else if (opt == "noTriStripPolygons")
            {
                localOptions.noTriStripPolygons = true;
            }
        }
    }

    return localOptions;
}

/**
 * STL importer for OpenSceneGraph.
 */
class ReaderWriterSTL : public osgDB::ReaderWriter
{
public:
    ReaderWriterSTL()
    {
        supportsExtension("stl", "STL binary format");
        supportsExtension("sta", "STL ASCII format");
        supportsOption("smooth", "Run SmoothingVisitor");
        supportsOption("separateFiles", "Save each geode in a different file. Can result in a huge amount of files!");
        supportsOption("dontSaveNormals", "Set all normals to [0 0 0] when saving to a file.");
    }

    virtual const char* className() const
    {
        return "STL Reader";
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*) const;
    virtual WriteResult writeNode(const osg::Node& node, const std::string& fileName, const Options* = NULL) const;

private:
    class ReaderObject
    {
    public:
        ReaderObject(bool noTriStripPolygons, bool generateNormals = true):
            _noTriStripPolygons(noTriStripPolygons),
            _generateNormal(generateNormals),
            _numFacets(0)
        {
        }

        virtual ~ReaderObject()
        {
        }

        enum ReadResult
        {
            ReadSuccess,
            ReadError,
            ReadEOF
        };

        virtual ReadResult read(FILE *fp) = 0;

        osg::ref_ptr<osg::Geometry> asGeometry() const
        {
            osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

            geom->setVertexArray(_vertex.get());

            if (_normal.valid())
            {
                // need to convert per triangle normals to per vertex
                osg::ref_ptr<osg::Vec3Array> perVertexNormals = new osg::Vec3Array;
                perVertexNormals->reserveArray(_normal->size() * 3);
                for(osg::Vec3Array::iterator itr = _normal->begin();
                    itr != _normal->end();
                    ++itr)
                {
                    perVertexNormals->push_back(*itr);
                    perVertexNormals->push_back(*itr);
                    perVertexNormals->push_back(*itr);
                }

                geom->setNormalArray(perVertexNormals.get(), osg::Array::BIND_PER_VERTEX);
            }

            if (_color.valid())
            {
                // need to convert per triangle colours to per vertex
                OSG_INFO << "STL file with color" << std::endl;
                osg::ref_ptr<osg::Vec4Array> perVertexColours = new osg::Vec4Array;
                perVertexColours->reserveArray(_color->size() * 3);
                for(osg::Vec4Array::iterator itr = _color->begin();
                    itr != _color->end();
                    ++itr)
                {
                    perVertexColours->push_back(*itr);
                    perVertexColours->push_back(*itr);
                    perVertexColours->push_back(*itr);
                }

                if(perVertexColours->size() == geom->getVertexArray()->getNumElements()) {
                    geom->setColorArray(perVertexColours.get(), osg::Array::BIND_PER_VERTEX);
                }
            }

            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, _numFacets * 3));

            if(!_noTriStripPolygons) {
                osgUtil::TriStripVisitor tristripper;
                tristripper.stripify(*geom);
            }

            return geom;
        }

        bool isEmpty()
        {
            return _numFacets == 0;
        }

        std::string& getName()
        {
            return _solidName;
        }

    protected:
        bool _noTriStripPolygons;
        bool _generateNormal;
        unsigned int _numFacets;

        std::string _solidName;
        osg::ref_ptr<osg::Vec3Array> _vertex;
        osg::ref_ptr<osg::Vec3Array> _normal;
        osg::ref_ptr<osg::Vec4Array> _color;

        void clear()
        {
            _solidName = "";
            _numFacets = 0;
            _vertex = osg::ref_ptr<osg::Vec3Array>();
            _normal = osg::ref_ptr<osg::Vec3Array>();
            _color = osg::ref_ptr<osg::Vec4Array>();
        }
    };

    class AsciiReaderObject : public ReaderObject
    {
    public:
        AsciiReaderObject(bool noTriStripPolygons)
        : ReaderObject(noTriStripPolygons)
        {
        }

        ReadResult read(FILE *fp);
    };

    class BinaryReaderObject : public ReaderObject
    {
    public:
        BinaryReaderObject(unsigned int expectNumFacets, bool noTriStripPolygons, bool generateNormals = true)
            : ReaderObject(noTriStripPolygons, generateNormals),
            _expectNumFacets(expectNumFacets)
        {
        }

        ReadResult read(FILE *fp);

    protected:
        unsigned int _expectNumFacets;
    };

    class CreateStlVisitor : public osg::NodeVisitor
    {
    public:
        CreateStlVisitor(std::string const & fout, const osgDB::ReaderWriter::Options* options = 0):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
            counter(0)
        {
            m_localOptions = parseOptions(options);
            if (m_localOptions.separateFiles)
            {
                OSG_INFO << "ReaderWriterSTL::writeNode: Files are written separately" << std::endl;
                m_fout_ext = osgDB::getLowerCaseFileExtension(fout);
                m_fout = fout.substr(0, fout.rfind(m_fout_ext) - 1);
            }
            else
            {
                m_fout = fout;
                m_f = new osgDB::ofstream(m_fout.c_str());
            }

            if (m_localOptions.dontSaveNormals)
            {
                OSG_INFO << "ReaderWriterSTL::writeNode: Not saving normals" << std::endl;
            }
        }

        std::string i2s(int i)
        {
            char buf[16];  // -2^31 == -2147483648 needs 11 chars + \0  -> 12 (+4 for security ;-)
            sprintf(buf, "%d", i);
            return buf;
        }

        virtual void apply(osg::Geode& node)
        {
            osg::Matrix mat = osg::computeLocalToWorld(getNodePath());

            if (m_localOptions.separateFiles)
            {
                std::string sepFile = m_fout + i2s(counter) + "." + m_fout_ext;
                m_f = new osgDB::ofstream(sepFile.c_str());
            }

            if (node.getName().empty())
                *m_f << "solid " << counter << std::endl;
            else
                *m_f << "solid " << node.getName() << std::endl;

            for (unsigned int i = 0; i < node.getNumDrawables(); ++i)
            {
                osg::TriangleFunctor<PushPoints> tf;
                tf.m_stream = m_f;
                tf.m_mat = mat;
                tf.m_dontSaveNormals = m_localOptions.dontSaveNormals;
                node.getDrawable(i)->accept(tf);
            }

            if (node.getName().empty())
                *m_f << "endsolid " << counter << std::endl;
            else
                *m_f << "endsolid " << node.getName() << std::endl;

            if (m_localOptions.separateFiles)
            {
                m_f->close();
                delete m_f;
            }

            ++counter;
            traverse(node);
        }

        ~CreateStlVisitor()
        {
            if (m_localOptions.separateFiles)
            {
                OSG_INFO << "ReaderWriterSTL::writeNode: " << counter - 1 << " files were written" << std::endl;
            }
            else
            {
                m_f->close();
                delete m_f;
            }
        }

        const std::string& getErrorString() const { return m_ErrorString; }

    private:
        int counter;
        std::ofstream* m_f;
        std::string m_fout;
        std::string m_fout_ext;
        std::string m_ErrorString;
        STLOptionsStruct m_localOptions;

        struct PushPoints
        {
            std::ofstream* m_stream;
            osg::Matrix m_mat;
            bool m_dontSaveNormals;

            inline void operator () (const osg::Vec3& _v1, const osg::Vec3& _v2, const osg::Vec3& _v3, bool treatVertexDataAsTemporary)
            {
                osg::Vec3 v1 = _v1 * m_mat;
                osg::Vec3 v2 = _v2 * m_mat;
                osg::Vec3 v3 = _v3 * m_mat;
                osg::Vec3 vV1V2 = v2 - v1;
                osg::Vec3 vV1V3 = v3 - v1;
                osg::Vec3 vNormal = vV1V2.operator ^(vV1V3);
                if (m_dontSaveNormals)
                    *m_stream << "facet normal 0 0 0" << std::endl;
                else
                    *m_stream << "facet normal " << vNormal[0] << " " << vNormal[1] << " " << vNormal[2] << std::endl;
                *m_stream << "outer loop" << std::endl;
                *m_stream << "vertex " << v1[0] << " " << v1[1] << " " << v1[2] << std::endl;
                *m_stream << "vertex " << v2[0] << " " << v2[1] << " " << v2[2] << std::endl;
                *m_stream << "vertex " << v3[0] << " " << v3[1] << " " << v3[2] << std::endl;
                *m_stream << "endloop" << std::endl;
                *m_stream << "endfacet" << std::endl;
            }
        };
    };
};

// Register with Registry to instantiate the above reader/writer.
REGISTER_OSGPLUGIN(stl, ReaderWriterSTL)

struct StlHeader
{
    char text[80];
    unsigned int numFacets;
};
const unsigned int sizeof_StlHeader = 84;

struct StlVector
{
    float x, y, z;
};
struct StlFacet
{
    StlVector normal;
    StlVector vertex[3];
    unsigned short color;
};
const unsigned int sizeof_StlFacet = 50;

const unsigned short StlHasColor = 0x8000;
const unsigned short StlColorSize = 0x1f;        // 5 bit
const float StlColorDepth = float(StlColorSize); // 2^5 - 1

// Check if the file comes from magics, and retrieve the corresponding data
// Magics files have a header with a "COLOR=" field giving the color of the whole model
bool fileComesFromMagics(FILE *fp, osg::Vec4& magicsColor)
{
    char header[80];
    const float magicsColorDepth = 255.f;

    ::rewind(fp);

    size_t bytes_read = fread((void*) &header, sizeof(header), 1, fp);
    if (bytes_read!=sizeof(header)) return false;

    ::fseek(fp, sizeof_StlHeader, SEEK_SET);

    std::string magicsColorPattern ("COLOR=");
    std::string headerStr = std::string(header);
    if(size_t colorFieldPos = headerStr.find(magicsColorPattern) != std::string::npos)
    {
        int colorIndex = colorFieldPos + magicsColorPattern.size() - 1;
        float r = (uint8_t)header[colorIndex] / magicsColorDepth;
        float g = (uint8_t)header[colorIndex + 1] / magicsColorDepth;
        float b = (uint8_t)header[colorIndex + 2] / magicsColorDepth;
        float a = (uint8_t)header[colorIndex + 3] / magicsColorDepth;
        magicsColor = osg::Vec4(r, g, b, a);
        return true;
    }

    return false;
}

osgDB::ReaderWriter::ReadResult ReaderWriterSTL::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile(file, options);
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    STLOptionsStruct localOptions = parseOptions (options);

    if (sizeof(unsigned int) != 4)
    {
        OSG_NOTICE<<"Waring: STL reading not supported as unsigned int is not 4 bytes on this system."<<std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    OSG_INFO << "ReaderWriterSTL::readNode(" << fileName.c_str() << ")" << std::endl;

    // determine ASCII vs. binary mode
    FILE* fp = osgDB::fopen(fileName.c_str(), "rb");
    if (!fp)
    {
        return ReadResult::FILE_NOT_FOUND;
    }

    // assumes "unsigned int" is 4 bytes...
    StlHeader header;
    if (fread((void*) &header, sizeof(header), 1, fp) != 1)
    {
        fclose(fp);
        return ReadResult::ERROR_IN_READING_FILE;
    }
    bool isBinary = false;

    // calculate expected file length from number of facets
    unsigned int expectFacets = header.numFacets;
    if (osg::getCpuByteOrder() == osg::BigEndian)
    {
        osg::swapBytes4((char*) &expectFacets);
    }
    off_t expectLen = sizeof_StlHeader + expectFacets * sizeof_StlFacet;

    struct stat stb;
    if (fstat(fileno(fp), &stb) < 0)
    {
        OSG_FATAL << "ReaderWriterSTL::readNode: Unable to stat '" << fileName << "'" << std::endl;
        fclose(fp);
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if (stb.st_size == expectLen)
    {
        isBinary = true;
    }
    else if (strstr(header.text, "solid") != 0)
    {
        isBinary = false;
    }
    else
    {
        OSG_FATAL << "ReaderWriterSTL::readNode(" << fileName.c_str() << ") unable to determine file format" << std::endl;
        fclose(fp);
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if (!isBinary)
    {
        fclose(fp);
        fp = osgDB::fopen(fileName.c_str(), "r");
    }

    osg::ref_ptr<osg::Group> group = new osg::Group;

    // read
    rewind(fp);

    ReaderObject *readerObject;

    if (isBinary)
        readerObject = new BinaryReaderObject(expectFacets, localOptions.noTriStripPolygons);
    else
        readerObject = new AsciiReaderObject(localOptions.noTriStripPolygons);

    std::auto_ptr<ReaderObject> readerPtr(readerObject);

    while (1)
    {
        ReaderObject::ReadResult result;

        if ((result = readerPtr->read(fp)) == ReaderObject::ReadError)
        {
            fclose(fp);
            return ReadResult::FILE_NOT_HANDLED;
        }

        if (!readerPtr->isEmpty())
        {
            osg::ref_ptr<osg::Geometry> geom = readerPtr->asGeometry();
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(geom.get());
            geode->setName(readerPtr->getName());
            group->addChild(geode.get());
        }

        if (result == ReaderObject::ReadEOF)
            break;
    }

    fclose(fp);

    if (localOptions.smooth)
    {
        osgUtil::SmoothingVisitor smoother;
        group->accept(smoother);
    }

    return group.get();
}

/**********************************************************************
 *
 * Private
 *
 **********************************************************************/

ReaderWriterSTL::ReaderObject::ReadResult ReaderWriterSTL::AsciiReaderObject::read(FILE* fp)
{
    unsigned int vertexCount = 0;
    unsigned int facetIndex[] = { 0, 0, 0 };
    unsigned int vertexIndex = 0;
    unsigned int normalIndex = 0;

    const int MaxLineSize = 256;
    char buf[MaxLineSize];
    char sx[MaxLineSize], sy[MaxLineSize], sz[MaxLineSize];

    if (!isEmpty())
    {
        clear();
    }

    while (fgets(buf, sizeof(buf), fp))
    {
        // strip '\n' or '\r\n' and trailing whitespace
        unsigned int len = strlen(buf) - 1;

        while (len && (buf[len] == '\n' || buf[len] == '\r' || isspace(buf[len])))
        {
            buf[len--] = '\0';
        }

        if (len == 0 || buf[0] == '\0')
        {
            continue;
        }

        // strip leading whitespace
        char* bp = buf;
        while (isspace(*bp))
        {
            ++bp;
        }

        if (strncmp(bp, "vertex", 6) == 0)
        {
            if (sscanf(bp + 6, "%s %s %s", sx, sy, sz) == 3)
            {
                if (!_vertex.valid())
                    _vertex = new osg::Vec3Array;

                float vx = osg::asciiToFloat(sx);
                float vy = osg::asciiToFloat(sy);
                float vz = osg::asciiToFloat(sz);

                vertexIndex = _vertex->size();
                if (vertexCount < 3)
                {
                    _vertex->push_back(osg::Vec3(vx, vy, vz));
                    facetIndex[vertexCount++] = vertexIndex;
                }
                else
                {
                    /*
                     * There are some invalid ASCII files around (at least one ;-)
                     * that have more than three vertices per facet - add an
                     * additional triangle.
                     */
                    _normal->push_back((*_normal)[normalIndex]);
                    _vertex->push_back((*_vertex)[facetIndex[0]]);
                    _vertex->push_back((*_vertex)[facetIndex[2]]);
                    _vertex->push_back(osg::Vec3(vx, vy, vz));
                    facetIndex[1] = facetIndex[2];
                    facetIndex[2] = vertexIndex;
                    _numFacets++;
                }
            }
        }
        else if (strncmp(bp, "facet", 5) == 0)
        {
            if (sscanf(bp + 5, "%*s %s %s %s", sx, sy, sz) == 3)
            {
                float nx = osg::asciiToFloat(sx);
                float ny = osg::asciiToFloat(sy);
                float nz = osg::asciiToFloat(sz);

                if (!_normal.valid())
                    _normal = new osg::Vec3Array;

                osg::Vec3 normal(nx, ny, nz);
                normal.normalize();

                normalIndex = _normal->size();
                _normal->push_back(normal);

                _numFacets++;
                vertexCount = 0;
            }
        }
        else if (strncmp(bp, "solid", 5) == 0)
        {
            OSG_INFO << "STL loader parsing '" << bp + 6 << "'" << std::endl;
            _solidName = bp + 6;
        }
        else if (strncmp(bp, "endsolid", 8) == 0)
        {
            OSG_INFO << "STL loader done parsing '" << _solidName << "'" << std::endl;
            return ReadSuccess;
        }
    }

    return ReadEOF;
}

ReaderWriterSTL::ReaderObject::ReadResult ReaderWriterSTL::BinaryReaderObject::read(FILE* fp)
{
    if (isEmpty())
    {
        clear();
    }

    _numFacets = _expectNumFacets;

    // Check if the file comes from Magics and retrieve the global color from the header
    osg::Vec4 magicsHeaderColor;
    bool comesFromMagics = fileComesFromMagics(fp, magicsHeaderColor);

    // seek to beginning of facets
    ::fseek(fp, sizeof_StlHeader, SEEK_SET);

    StlFacet facet;
    for (unsigned int i = 0; i < _expectNumFacets; ++i)
    {
        if (::fread((void*) &facet, sizeof_StlFacet, 1, fp) != 1)
        {
            OSG_FATAL << "ReaderWriterSTL::readStlBinary: Failed to read facet " << i << std::endl;
            return ReadError;
        }

        // vertices
        if (!_vertex.valid())
            _vertex = new osg::Vec3Array;

        osg::Vec3 v0(facet.vertex[0].x, facet.vertex[0].y, facet.vertex[0].z);
        osg::Vec3 v1(facet.vertex[1].x, facet.vertex[1].y, facet.vertex[1].z);
        osg::Vec3 v2(facet.vertex[2].x, facet.vertex[2].y, facet.vertex[2].z);
        _vertex->push_back(v0);
        _vertex->push_back(v1);
        _vertex->push_back(v2);

        // per-facet normal
        osg::Vec3 normal;
        if (_generateNormal)
        {
            osg::Vec3 d01 = v1 - v0;
            osg::Vec3 d02 = v2 - v0;
            normal = d01 ^ d02;
            normal.normalize();
        }
        else
        {
            normal.set(facet.normal.x, facet.normal.y, facet.normal.z);
        }

        if (!_normal.valid())
            _normal = new osg::Vec3Array;
        _normal->push_back(normal);

        /*
         * color extension
         * RGB555 with most-significat bit indicating if color is present
         *
         * The magics files may use whether per-face or per-object colors
         * for a given face, according to the value of the last bit (0 = per-face, 1 = per-object)
         * Moreover, magics uses RGB instead of BGR (as the other softwares)
         */
        if (!_color.valid())
        {
            _color = new osg::Vec4Array;
        }

        // Case of a Magics file
        if(comesFromMagics)
        {
            if(facet.color & StlHasColor) // The last bit is 1, the per-object color is used
            {
                _color->push_back(magicsHeaderColor);
            }
            else // the last bit is 0, the facet has its own unique color
            {
                float b = ((facet.color >> 10) & StlColorSize) / StlColorDepth;
                float g = ((facet.color >> 5) & StlColorSize) / StlColorDepth;
                float r = (facet.color & StlColorSize) / StlColorDepth;
                _color->push_back(osg::Vec4(r, g, b, 1.0f));
            }
        }
        // Case of a generic file
        else if (facet.color & StlHasColor) // The color is valid if the last bit is 1
            {
                float r = ((facet.color >> 10) & StlColorSize) / StlColorDepth;
                float g = ((facet.color >> 5) & StlColorSize) / StlColorDepth;
                float b = (facet.color & StlColorSize) / StlColorDepth;
                _color->push_back(osg::Vec4(r, g, b, 1.0f));
            }
    }

    return ReadEOF;
}

osgDB::ReaderWriter::WriteResult ReaderWriterSTL::writeNode(const osg::Node& node, const std::string& fileName, const Options* opts) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

    if (ext != "stl")
    {
        OSG_FATAL << "ReaderWriterSTL::writeNode: Only STL ASCII files supported" << std::endl;
        return WriteResult::FILE_NOT_HANDLED;
    }

    CreateStlVisitor createStlVisitor(fileName, opts);
    const_cast<osg::Node&>(node).accept(createStlVisitor);

    if (createStlVisitor.getErrorString().empty())
    {
        return WriteResult::FILE_SAVED;
    }
    else
    {
        OSG_FATAL << "Error: " << createStlVisitor.getErrorString() << std::endl;
        return WriteResult::ERROR_IN_WRITING_FILE;
    }
}

/* vim: set ts=4 sw=4 expandtab: */
