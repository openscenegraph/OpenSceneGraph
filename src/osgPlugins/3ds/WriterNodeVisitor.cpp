// -*-c++-*-

/*
 * 3DS reader/writer for Open Scene Graph
 *
 * Copyright (C) ???
 *
 * Writing support added 2009 by Sukender (Benoit Neil), http://sukender.free.fr,
 * strongly inspired by the OBJ writer object by Stephan Huber
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for
 * real-time rendering of large 3D photo-realistic models.
 * The OSG homepage is http://www.openscenegraph.org/
 */


/// [EXPERIMENTAL] Disables animation data (and matrix transforms) for compatibility with some 3rd party apps.
/// Animations are not read by all 3DS importers. Thus disabling them may allow some 3rd-party apps, such as Rhinoceros (tested with 4.0) to correctly import 3DS files.
/// However, having proper hierarchy with matrix transforms will become impossible.
///\warning This is still experimental, hence the compile flag. This should become a reader/writer option as soon as it works as intended (maybe "noMatrixTransforms" could become a read/write option?).
#define DISABLE_3DS_ANIMATION 0            // Default = 0

#include <osg/io_utils>
#include <osg/CullFace>
#include <osg/Billboard>
#include <osgDB/WriteFile>

#include "WriterNodeVisitor.h"
#include <assert.h>
#include <string.h>


void copyOsgMatrixToLib3dsMatrix(Lib3dsMatrix lib3ds_matrix, const osg::Matrix& osg_matrix)
{
    for(int row=0; row<4; ++row)
    {
        lib3ds_matrix[row][0] = osg_matrix.ptr()[row*4+0];
        lib3ds_matrix[row][1] = osg_matrix.ptr()[row*4+1];
        lib3ds_matrix[row][2] = osg_matrix.ptr()[row*4+2];
        lib3ds_matrix[row][3] = osg_matrix.ptr()[row*4+3];
    }
}

inline void copyOsgVectorToLib3dsVector(Lib3dsVector lib3ds_vector, const osg::Vec3f& osg_vector)
{
    lib3ds_vector[0] = osg_vector[0];
    lib3ds_vector[1] = osg_vector[1];
    lib3ds_vector[2] = osg_vector[2];
}
inline void copyOsgVectorToLib3dsVector(Lib3dsVector lib3ds_vector, const osg::Vec3d& osg_vector)
{
    lib3ds_vector[0] = osg_vector[0];
    lib3ds_vector[1] = osg_vector[1];
    lib3ds_vector[2] = osg_vector[2];
}

inline void copyOsgColorToLib3dsColor(Lib3dsVector lib3ds_vector, const osg::Vec4f& osg_vector)
{
    lib3ds_vector[0] = osg_vector[0];
    lib3ds_vector[1] = osg_vector[1];
    lib3ds_vector[2] = osg_vector[2];
}
inline void copyOsgColorToLib3dsColor(Lib3dsVector lib3ds_vector, const osg::Vec4d& osg_vector)
{
    lib3ds_vector[0] = osg_vector[0];
    lib3ds_vector[1] = osg_vector[1];
    lib3ds_vector[2] = osg_vector[2];
}

inline void copyOsgQuatToLib3dsQuat(float lib3ds_vector[4], const osg::Quat& osg_quat)
{
    //lib3ds_vector[0] = osg_quat[3];        // Not sure
    //lib3ds_vector[1] = osg_quat[0];
    //lib3ds_vector[2] = osg_quat[1];
    //lib3ds_vector[3] = osg_quat[2];
    // 3DS seems to store (angle in radians, axis_x, axis_y, axis_z), but it works with (axis_x, axis_y, axis_z, -angle in radians)!
    osg::Quat::value_type angle, x, y, z;
    osg_quat.getRotate(angle, x, y, z);
    lib3ds_vector[0] = static_cast<float>(x);
    lib3ds_vector[1] = static_cast<float>(y);
    lib3ds_vector[2] = static_cast<float>(z);
    lib3ds_vector[3] = static_cast<float>(-angle);
}


/// Checks if a filename (\b not path) is 8.3 (an empty name is never 8.3, and a path is never 8.3).
/// Please note the '8' and '3' limitations are in \b bytes, not in characters (which is different when using UTF8).
bool is83(const std::string & s)
{
    // 012345678901
    // ABCDEFGH.ABC
    if (s.find_first_of("/\\") != std::string::npos) return false;            // It should not be a path, but a filename
    unsigned int len = s.length();
    if (len > 12 || len == 0) return false;
    size_t pointPos = s.rfind('.');
    if (pointPos == std::string::npos) return len <= 8;        // Without point
    // With point
    if (pointPos > 8) return false;
    if (len-1 - pointPos > 3) return false;
    return true;
}

inline std::string::size_type maxNameLen(bool extendedFilePaths, bool isNodeName) {
    if (extendedFilePaths) return 63;
    return isNodeName ? 8 : (8+1+3);
}

/// Tests if the given string is a path supported by 3DS format (8.3, 63 chars max, non empty).
inline bool is3DSName(const std::string & s, bool extendedFilePaths, bool isNodeName)
{
    unsigned int len = s.length();
    if (len > maxNameLen(extendedFilePaths, isNodeName) || len == 0) return false;
    // Extended paths are simply those that fits the 64 bytes buffer.
    if (extendedFilePaths) return true;
    // Short paths simply must have no subdirectory.
    return is83(s);
}


// Use namespace qualification to avoid static-link symbol collitions
// from multiply defined symbols.
namespace plugin3ds
{


/** writes all primitives of a primitive-set out to a stream, decomposes quads to triangles, line-strips to lines etc */
class PrimitiveIndexWriter : public osg::PrimitiveIndexFunctor
{
public:
      PrimitiveIndexWriter(osg::Geometry  *    geo,
                           ListTriangle  &    listTriangles,
                           unsigned int        drawable_n,
                           unsigned int        material) :
          osg::PrimitiveIndexFunctor(),
          _drawable_n(drawable_n),
          _listTriangles(listTriangles),
          _modeCache(0),
          _hasNormalCoords(geo->getNormalArray() != NULL),
          _hasTexCoords(geo->getTexCoordArray(0) != NULL),
          _lastFaceIndex(0),
          _material(material)
      {
      }

      unsigned int getNextFaceIndex() { return _lastFaceIndex; }

      virtual void setVertexArray(unsigned int,const osg::Vec2*) {}

      virtual void setVertexArray(unsigned int,const osg::Vec3*) {}

      virtual void setVertexArray(unsigned int,const osg::Vec4*) {}

      virtual void setVertexArray(unsigned int,const osg::Vec2d*) {}

      virtual void setVertexArray(unsigned int ,const osg::Vec3d*) {}

      virtual void setVertexArray(unsigned int,const osg::Vec4d*) {}


      // operator for triangles
      void writeTriangle(unsigned int i1, unsigned int i2, unsigned int i3)
      {
          Triangle triangle;
          triangle.t1 = i1;
          triangle.t2 = i2;
          triangle.t3 = i3;
          triangle.material = _material;
          _listTriangles.push_back(ListTriangle::value_type(triangle, _drawable_n));
      }
      virtual void begin(GLenum mode)
      {
          _modeCache = mode;
          _indexCache.clear();
      }

      virtual void vertex(unsigned int vert)
      {
          _indexCache.push_back(vert);
      }

      virtual void end()
      {
          if (!_indexCache.empty())
          {
              drawElements(_modeCache,_indexCache.size(),&_indexCache.front());
          }
      }

      virtual void drawArrays(GLenum mode,GLint first,GLsizei count);

      virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
      {
          drawElementsImplementation<GLubyte>(mode, count, indices);
      }
      virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
      {
          drawElementsImplementation<GLushort>(mode, count, indices);
      }

      virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
      {
          drawElementsImplementation<GLuint>(mode, count, indices);
      }

protected:

    template<typename T>void drawElementsImplementation(GLenum mode, GLsizei count, const T* indices)
    {
        if (indices==0 || count==0) return;

        typedef const T* IndexPointer;

        switch(mode)
        {
        case(GL_TRIANGLES):
            {
                //lib3ds_mesh_resize_faces(_mesh, _lastFaceIndex + count / 3);
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    writeTriangle(*iptr,*(iptr+1),*(iptr+2));

                break;
            }
        case(GL_TRIANGLE_STRIP):
            {
                //lib3ds_mesh_resize_faces(_mesh, _lastFaceIndex + count -2);
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    if ((i%2)) writeTriangle(*(iptr),*(iptr+2),*(iptr+1));
                    else       writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                }
                break;
            }
        case(GL_QUADS):
            {
                //lib3ds_mesh_resize_faces(_mesh, _lastFaceIndex + count /2);        // count/4*2
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                    writeTriangle(*(iptr),*(iptr+2),*(iptr+3));
                }
                break;
            }
        case(GL_QUAD_STRIP):
            {
                //lib3ds_mesh_resize_faces(_mesh, _lastFaceIndex + (count / 2 -1)*2);
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    writeTriangle(*(iptr),*(iptr+1),*(iptr+2));
                    writeTriangle(*(iptr+1),*(iptr+3),*(iptr+2));
                }
                break;
            }
        case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
        case(GL_TRIANGLE_FAN):
            {
                //lib3ds_mesh_resize_faces(_mesh, _lastFaceIndex + count -2);
                IndexPointer iptr = indices;
                unsigned int first = *iptr;
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    writeTriangle(first,*(iptr),*(iptr+1));
                }
                break;
            }
        case(GL_POINTS):
        case(GL_LINES):
        case(GL_LINE_STRIP):
        case(GL_LINE_LOOP):
            // Not handled
            break;

        default:
            // uhm should never come to this point :)
            break;
        }
    }

private:

    PrimitiveIndexWriter& operator = (const PrimitiveIndexWriter&) { return *this; }

    unsigned int         _drawable_n;
    ListTriangle    &     _listTriangles;
    GLenum               _modeCache;
    std::vector<GLuint>  _indexCache;
    bool                 _hasNormalCoords, _hasTexCoords;
    unsigned int         _lastFaceIndex;
    unsigned int         _material;
};


void PrimitiveIndexWriter::drawArrays(GLenum mode,GLint first,GLsizei count)
{
    switch(mode)
    {
    case(GL_TRIANGLES):
        {
            unsigned int pos=first;
            for(GLsizei i=2;i<count;i+=3,pos+=3)
            {
                writeTriangle(pos,pos+1,pos+2);
            }
            break;
        }
    case(GL_TRIANGLE_STRIP):
        {
            unsigned int pos=first;
            for(GLsizei i=2;i<count;++i,++pos)
            {
                if ((i%2)) writeTriangle(pos,pos+2,pos+1);
                else       writeTriangle(pos,pos+1,pos+2);
            }
            break;
        }
    case(GL_QUADS):
        {
            unsigned int pos=first;
            for(GLsizei i=3;i<count;i+=4,pos+=4)
            {
                writeTriangle(pos,pos+1,pos+2);
                writeTriangle(pos,pos+2,pos+3);
            }
            break;
        }
    case(GL_QUAD_STRIP):
        {
            unsigned int pos=first;
            for(GLsizei i=3;i<count;i+=2,pos+=2)
            {
                writeTriangle(pos,pos+1,pos+2);
                writeTriangle(pos+1,pos+3,pos+2);
            }
            break;
        }
    case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
    case(GL_TRIANGLE_FAN):
        {
            unsigned int pos=first+1;
            for(GLsizei i=2;i<count;++i,++pos)
            {
                writeTriangle(first,pos,pos+1);
            }
            break;
        }
    case(GL_POINTS):
    case(GL_LINES):
    case(GL_LINE_STRIP):
    case(GL_LINE_LOOP):
        //break;
    default:
        OSG_WARN << "3DS WriterNodeVisitor: can't handle mode " << mode << std::endl;
        break;
    }
}



WriterNodeVisitor::Material::Material(WriterNodeVisitor & writerNodeVisitor, osg::StateSet * stateset, osg::Material* mat, osg::Texture* tex, bool preserveName, int ind) :
    index(ind),
    diffuse(1,1,1,1),
    ambient(0.2,0.2,0.2,1),
    specular(0,0,0,1),
    shininess(0),
    transparency(0),
    double_sided(false),
    image(NULL),
    texture_transparency(false),
    texture_no_tile(true) // matches lib3ds_material.cpp initialize_texture_map(..) default flag setting
{
    //static unsigned int s_objmaterial_id = 0;
    //++s_objmaterial_id;
    if (mat)
    {
        assert(stateset);
        diffuse = mat->getDiffuse(osg::Material::FRONT);
        ambient = mat->getAmbient(osg::Material::FRONT);
        specular = mat->getSpecular(osg::Material::FRONT);
        shininess = mat->getShininess(osg::Material::FRONT) / 128.f;
        // OpenGL shininess = pow(2, 10.0*mat->shininess);            (As in lib3ds example)
        // => mat->shininess = log.2( OpenGL shininess ) /10        (if values are >0)
        // => mat->shininess = log( OpenGL shininess ) / log(2) /10
        //shininess = mat->getShininess(osg::Material::FRONT) <= 0 ? 0 : log( mat->getShininess(osg::Material::FRONT) ) / log(2.f) / 10.f;

        transparency = 1-diffuse.w();
        if (preserveName == false)
        {
            name = writerNodeVisitor.getUniqueName(mat->getName(),true,"mat");
        }
        else
        {
            name = writerNodeVisitor.getMaterialName(mat->getName());
        }
        osg::StateAttribute * attribute = stateset->getAttribute(osg::StateAttribute::CULLFACE);
        if (!attribute)
        {
            double_sided = true;
        }
        else
        {
            assert(dynamic_cast<osg::CullFace *>(attribute));
            osg::CullFace::Mode mode = static_cast<osg::CullFace *>(attribute)->getMode();
            if (mode == osg::CullFace::BACK) double_sided = false;
            else if (mode == osg::CullFace::FRONT)
            {
                OSG_WARN << "3DS Writer: Reversed face (culled FRONT) not supported yet." << std::endl;
                double_sided = false;
            }
            else
            {
                assert(mode == osg::CullFace::FRONT_AND_BACK);
                OSG_WARN << "3DS Writer: Invisible face (culled FRONT_AND_BACK) not supported yet." << std::endl;
                double_sided = false;
            }
        }
    }
    if (tex)
    {
        osg::Image* img = tex->getImage(0);
        if(img)
        {
            texture_transparency = (stateset->getMode(GL_BLEND) == osg::StateAttribute::ON);
            osg::Texture::WrapMode wrapS = tex->getWrap(osg::Texture2D::WRAP_S);
            texture_no_tile = !(wrapS == osg::Texture2D::REPEAT || wrapS == osg::Texture2D::MIRROR);
            image = img;
        }
    }

    if (name.empty())
    {
        std::stringstream ss;
        ss << "m" << index;
        name = ss.str();
    }
}


/// Converts an extension to a 3-letters long one equivalent.
std::string convertExt(const std::string & path, bool extendedFilePaths)
{
    if (extendedFilePaths) return path;        // Extensions are not truncated for extended filenames

    std::string ext = osgDB::getFileExtensionIncludingDot(path);
    if (ext == ".tiff") ext = ".tif";
    else if (ext == ".jpeg") ext = ".jpg";
    else if (ext == ".jpeg2000" || ext == ".jpg2000") ext = ".jpc";
    return osgDB::getNameLessExtension(path) + ext;
}


WriterNodeVisitor::WriterNodeVisitor(Lib3dsFile * file3ds, const std::string & fileName,
                const osgDB::ReaderWriter::Options* options,
                const std::string & srcDirectory) :
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _succeeded(true),
    _srcDirectory(srcDirectory),
    _file3ds(file3ds),
    _currentStateSet(new osg::StateSet()),
    _lastMaterialIndex(0),
    _lastMeshIndex(0),
    _cur3dsNode(NULL),
    _options(options),
    _imageCount(0),
    _extendedFilePaths(false),
    _preserveMaterialNames(false)
{
    if (!fileName.empty())
        _directory = options->getDatabasePathList().empty() ? osgDB::getFilePath(fileName) : options->getDatabasePathList().front();

    if (options)
    {
        std::istringstream iss(options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            if (opt == "extended3dsFilePaths" || opt == "extended3DSFilePaths")
                _extendedFilePaths = true;
            if (opt == "preserveMaterialNames")
                _preserveMaterialNames = true;
        }
    }
}


void WriterNodeVisitor::writeMaterials()
{
    unsigned int nbMat = _materialMap.size();
    lib3ds_file_reserve_materials(_file3ds, nbMat, 1);
    // Ugly thing: it seems lib3ds_file_insert_material() doesn't support insertion in a random order (else materials are not assigned the right way)
    for (unsigned int iMat=0; iMat<nbMat; ++iMat)
    {
        for(MaterialMap::iterator itr = _materialMap.begin(); itr != _materialMap.end(); ++itr)
        {
            const Material & mat = itr->second;
            if (mat.index != static_cast<int>(iMat)) continue;        // Ugly thing (2)

            Lib3dsMaterial * mat3ds = lib3ds_material_new(osgDB::getSimpleFileName(mat.name).c_str());
            copyOsgColorToLib3dsColor(mat3ds->ambient,  mat.ambient);
            copyOsgColorToLib3dsColor(mat3ds->diffuse,  mat.diffuse);
            copyOsgColorToLib3dsColor(mat3ds->specular, mat.specular);
            mat3ds->shininess = mat.shininess;
            mat3ds->transparency = mat.transparency;
            mat3ds->two_sided = mat.double_sided ? 1 : 0;
            if (mat.image)
            {
                std::string path;
                ImageSet::const_iterator itImage( _imageSet.find(mat.image.get()) );
                if (itImage != _imageSet.end())
                {
                    // Image has been already used
                    path = itImage->second;
                }
                else
                {
                    // First time we 'see' this image
                    if (mat.image->getFileName().empty())
                    {
                        std::ostringstream oss;
                        oss << "Image_" << _imageCount++ << ".rgb";
                        path = oss.str();
                    }
                    else
                    {
                        path = osgDB::getPathRelative(_srcDirectory, mat.image->getFileName());
                    }
                    path = convertExt(path, _extendedFilePaths);
                    path = getUniqueName(path, false, "");

                    // Write
                    const std::string fullPath( osgDB::concatPaths(_directory, path) );
                    osgDB::makeDirectoryForFile(fullPath);
                    osgDB::writeImageFile(*(mat.image), fullPath, _options);

                    // Insert in map
                    _imageSet.insert(ImageSet::value_type(mat.image.get(), path));
                }

                Lib3dsTextureMap & tex = mat3ds->texture1_map;
                osgDB::stringcopyfixedsize(tex.name, path.c_str());
                // Here we don't assume anything about initial flags state (actually it is set to LIB3DS_TEXTURE_NO_TILE by lib3DS, but this is subject to change)
                if (mat.texture_transparency) tex.flags |= LIB3DS_TEXTURE_ALPHA_SOURCE;
                else tex.flags &= ~LIB3DS_TEXTURE_ALPHA_SOURCE;
                if (mat.texture_no_tile) tex.flags |= LIB3DS_TEXTURE_NO_TILE;
                else tex.flags &= ~LIB3DS_TEXTURE_NO_TILE;
            }
            if (!succeeded())
            {
                lib3ds_material_free(mat3ds);
                return;
            }
            lib3ds_file_insert_material(_file3ds, mat3ds, itr->second.index);
            break;        // Ugly thing (3)
        }
    }
}

/// Truncates an UTF8 string so that it does not takes more than a given \b bytes amount (\b excluding the potential NULL end character).
/// The function assumes the UTF8 string is valid.
///\return A valid UTF8 string which size is less or equal to \c byteLimit.
// May be moved in osgDB/ConvertUTF?
std::string utf8TruncateBytes(const std::string & s, std::string::size_type byteLimit) {
    // Untruncated strings
    if (s.size() <= byteLimit) return s;

    // Truncated strings
    std::string::const_iterator it=s.begin(), itEnd=s.begin()+byteLimit;
    std::string::const_iterator itStop=it;
    // Note: itEnd is < s.end(), so that we can always write "it+1"
    for(; it!=itEnd; ++it) {
        unsigned char c = static_cast<unsigned char>(*it);
        if ((c & 0x80) == 0) itStop=it+1;        // 7 bits ANSI. itStop must then point after that character.
        else if ((c & 0x40) != 0) itStop=it;    // UTF8 sequence start: this is also past-the-end for the previous character (ANSI or UTF8)
    }
    return std::string(s.begin(), itStop);
}

#ifdef OSG_USE_UTF8_FILENAME
#    define truncateFilenameBytes(str, size) utf8TruncateBytes(str, size)
#else
#    define truncateFilenameBytes(str, size) std::string(str, 0, size)
#endif

std::string WriterNodeVisitor::getUniqueName(const std::string& _defaultValue, bool isNodeName, const std::string & _defaultPrefix, int currentPrefixLen)
{
    //const unsigned int MAX_LENGTH = maxNameLen(_extendedFilePaths);
    const unsigned int MAX_PREFIX_LENGTH = _extendedFilePaths ? 52 : 6;        // Arbitrarily defined for short names, kept enough room for displaying UINT_MAX (10 characters) for long names.
    assert(_defaultPrefix.length()<=4);        // Default prefix is too long (implementation error)
    const std::string defaultPrefix(_defaultPrefix.empty() ? "_" : _defaultPrefix);
    if (currentPrefixLen<0) currentPrefixLen = osg::maximum(_defaultPrefix.length(), _defaultValue.length());
    currentPrefixLen = osg::clampBelow(currentPrefixLen, static_cast<int>(MAX_PREFIX_LENGTH));

    // Tests if default name is valid and unique
    NameMap & nameMap = isNodeName ? _nodeNameMap : _imageNameMap;
    PrefixMap & prefixMap = isNodeName ? _nodePrefixMap : _imagePrefixMap;

    // Handling of paths is simple. Algorithm:
    //    - For short names, subdirectories are simply forbidden. Use the simple file name.
    //    - Else, the whole (relative) path must simply be <64 chars.
    // After this, begin enumeration.
    std::string parentPath, filename, ext, namePrefix;
    unsigned int max_val = 0;

    // TODO Move the two parts of this giant if/else into two separate functions for better readability.
    if (_extendedFilePaths)
    {
        // Tests if default name is valid and unique
        if (is3DSName(_defaultValue, _extendedFilePaths, isNodeName))
        {
            std::pair<NameMap::iterator, bool> insertion( nameMap.insert(_defaultValue) );
            if (insertion.second) return _defaultValue;        // Return if element is newly inserted in the map (else there is a naming collision)
        }

        // Simply separate name and last extension
        filename = osgDB::getNameLessExtension(osgDB::getSimpleFileName(_defaultValue));
        if (!isNodeName)
        {
            ext = osgDB::getFileExtensionIncludingDot(_defaultValue);
            if (ext == ".") ext = "";
        }

        // Compute parent path
        // Pre-condition: paths are supposed to be relative.
        // If full path is too long (>MAX_PREFIX_LENGTH), cut path to let enough space for simple file name.
        // Do not cut in the middle of a name, but at path separators.
        parentPath = osgDB::getFilePath(_defaultValue);
        if (_defaultValue.length() >MAX_PREFIX_LENGTH)        // Not parentPath but _defaultValue!
        {
            // Nodes names: keep last directories (used only for the root name, generally named after the full file path)
            // Images names: keep first directories (for images)
            if (isNodeName) std::reverse(parentPath.begin(), parentPath.end());
            unsigned lenToDelete(filename.length() + ext.length() + 1);
            lenToDelete = osg::clampBelow(lenToDelete, MAX_PREFIX_LENGTH);
            parentPath = truncateFilenameBytes(parentPath, MAX_PREFIX_LENGTH - lenToDelete);        // +1 for the path separator
            std::string::size_type separator = parentPath.find_last_of("/\\");
            if (separator != std::string::npos) parentPath = parentPath.substr(0, separator);
            if (isNodeName) std::reverse(parentPath.begin(), parentPath.end());
        }

        // Assert "MAX_PREFIX_LENGTH - parent path length - extension length -1" is >=0 and truncate name to this length to get our new prefix.
        assert(parentPath.length() + ext.length() <= MAX_PREFIX_LENGTH);
        const unsigned int len = MAX_PREFIX_LENGTH - (parentPath.length() + ext.length() +1);
        namePrefix = truncateFilenameBytes(filename, len);
        if (namePrefix.empty()) namePrefix = defaultPrefix;

        // Truncate the filename to get our new prefix
        namePrefix = truncateFilenameBytes(filename, currentPrefixLen);

        // Enough space has been reserved for UINT_MAX values
        max_val = UINT_MAX;
    }
    else
    {
        // Get last extension, and make filename have no extension
        filename = osgDB::getNameLessAllExtensions(osgDB::getSimpleFileName(_defaultValue));
        if (!isNodeName)
        {
            ext = truncateFilenameBytes(osgDB::getFileExtensionIncludingDot(_defaultValue), 4);        // 4 chars = dot + 3 chars
            if (ext == ".") ext = "";
        }

        // Tests if STRIPPED default name is valid and unique
        const std::string strippedName( filename + ext );
        if (is3DSName(strippedName, _extendedFilePaths, isNodeName))
        {
            std::pair<NameMap::iterator, bool> insertion( nameMap.insert(strippedName) );
            if (insertion.second) return strippedName;        // Return if element is newly inserted in the map (else there is a naming collision)
        }

        namePrefix = filename;
        if (namePrefix.empty()) namePrefix = defaultPrefix;
        // Truncate the filename to get our new prefix
        namePrefix = truncateFilenameBytes(namePrefix, currentPrefixLen);

        // Compute the maximum enumeration value
        max_val = static_cast<unsigned int>(pow(10., 8. - namePrefix.length())) -1;
    }
    assert(namePrefix.size() <= MAX_PREFIX_LENGTH);

    // Find the current enumeration value (searchStart)
    unsigned int searchStart(0);
    PrefixMap::iterator pairPrefix( prefixMap.find(namePrefix) );
    if (pairPrefix != prefixMap.end())
    {
        searchStart = pairPrefix->second;
    }
    else
    {
        // Check if truncated name is ok
        const std::string res( osgDB::concatPaths(parentPath, namePrefix + ext) );
        if (nameMap.find(res) == nameMap.end()) {
            prefixMap.insert(std::pair<std::string, unsigned int>(namePrefix, 0));
            nameMap.insert(res);
            return res;
        }
    }

    // Search for a free value
    for(unsigned int i = searchStart; i <= max_val; ++i)
    {
        std::stringstream ss;
        ss << namePrefix << i;
        const std::string res( osgDB::concatPaths(parentPath, ss.str() + ext) );
        if (nameMap.find(res) == nameMap.end())
        {
            if (pairPrefix != prefixMap.end())
            {
                pairPrefix->second = i + 1;
            } else
            {
                prefixMap.insert(std::pair<std::string, unsigned int>(namePrefix, i + 1));
            }
            nameMap.insert(res);
            return res;
        }
    }

    // Failed finding a name
    // Try with a shorter prefix if possible
    if (currentPrefixLen>1) return getUniqueName(_defaultValue, isNodeName, defaultPrefix, currentPrefixLen-1);
    // Try with default prefix if not arleady done
    if (defaultPrefix != std::string("_")) return getUniqueName(_defaultValue, isNodeName, "_", 1);

    // No more names
    OSG_NOTIFY(osg::FATAL) << "No more names available!" << std::endl;
    _succeeded = false;
    return "ERROR";
}

std::string WriterNodeVisitor::getMaterialName(const std::string& inputMaterialName)
{
    std::string result;

    // Search in map if this material name already exists
    MaterialNameMap::const_iterator mapIter=_materialNameMap.find(inputMaterialName);

    if (mapIter!=_materialNameMap.end())
    {
        // Found, use it !
        result = mapIter->second;
    }
    else
    {
        // Not found, create a new (unique) entry mapped to this material name
        std::string resultPrefix = inputMaterialName.substr(0, 60);    // Up to 64 characters : truncate to 60 characters : 60 + "_" + number
        result = resultPrefix;

        NameMap::const_iterator setIter=_materialNameSet.find(result);
        int suffixValue=-1;
        while (setIter!=_materialNameSet.end())
        {
            if (suffixValue < 0)
            {
                // First add of the suffix
                resultPrefix = resultPrefix + "_";
                suffixValue++;
            }

            std::stringstream ss;
            ss << resultPrefix << suffixValue;

            result = ss.str();

            setIter=_materialNameSet.find(result);
            suffixValue++;
        }

        // Add entry in map
        _materialNameMap[inputMaterialName] = result;
    }

    return result;
}

int WriterNodeVisitor::processStateSet(osg::StateSet* ss)
{
    MaterialMap::const_iterator itr = _materialMap.find(ss);
    if (itr != _materialMap.end())
    {
        assert(itr->second.index>=0);
        return itr->second.index;
    }

    osg::Material* mat = dynamic_cast<osg::Material*>(ss->getAttribute(osg::StateAttribute::MATERIAL));
    osg::Texture* tex = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(0, osg::StateAttribute::TEXTURE));

    if (mat || tex)
    {
        int matNum = _lastMaterialIndex;
        _materialMap.insert(std::make_pair(osg::ref_ptr<osg::StateSet>(ss), Material(*this, ss, mat, tex, _preserveMaterialNames, matNum) ));
        ++_lastMaterialIndex;
        return matNum;
    }
    return -1;
}

/**
*  Add a vertice to the index and link it with the Triangle index and the drawable.
*  \param index_vert is the map where the vertice are stored.
*  \param index is the indice of the vertice's position in the vec3.
*  \param drawable_n is the number of the drawable.
*  \return the position of the vertice in the final mesh.
*/
unsigned int
WriterNodeVisitor::getMeshIndexForGeometryIndex(MapIndices & index_vert,
                                                unsigned int index,
                                                unsigned int drawable_n)
{
    MapIndices::iterator itIndex = index_vert.find(std::pair<unsigned int, unsigned int>(index, drawable_n));
    if (itIndex == index_vert.end())
    {
        unsigned int indexMesh = index_vert.size();
        index_vert.insert(std::make_pair(std::pair<unsigned int, unsigned int>(index, drawable_n), indexMesh));
        return indexMesh;
    }
    return itIndex->second;
}


void
WriterNodeVisitor::buildMesh(osg::Geode        & geo,
                             const osg::Matrix & mat,
                             MapIndices        & index_vert,
                             bool                texcoords,
                             Lib3dsMesh        * mesh)
{
    OSG_DEBUG << "Building Mesh" << std::endl;
    assert(mesh);

    // Write points
    assert(index_vert.size() <= MAX_VERTICES);
    lib3ds_mesh_resize_vertices(mesh, index_vert.size(), texcoords ? 1 : 0, 0);

    for(MapIndices::iterator it = index_vert.begin(); it != index_vert.end();++it)
    {
        osg::Geometry *g = geo.getDrawable( it->first.second )->asGeometry();
        const osg::Array * basevecs = g->getVertexArray();
        assert(basevecs);
        if (!basevecs || basevecs->getNumElements()==0) continue;
        if (basevecs->getType() == osg::Array::Vec3ArrayType)
        {
            const osg::Vec3Array & vecs= *static_cast<const osg::Vec3Array *>(basevecs);
            copyOsgVectorToLib3dsVector(mesh->vertices[it->second], vecs[it->first.first]*mat);
        }
        else if (basevecs->getType() == osg::Array::Vec3dArrayType)
        {
            // Handle double presision vertices by converting them to float with a warning
            OSG_NOTICE << "3DS format only supports single precision vertices. Converting double precision to single." << std::endl;
            const osg::Vec3dArray & vecs= *static_cast<const osg::Vec3dArray *>(basevecs);
            copyOsgVectorToLib3dsVector(mesh->vertices[it->second], vecs[it->first.first]*mat);
        }
        else
        {
            OSG_NOTIFY(osg::FATAL) << "Vertex array is not Vec3 or Vec3d. Not implemented" << std::endl;
            _succeeded = false;
            return;
        }
    }

    // Write texture coords (Texture 0 only)
    if (texcoords)
    {
        for(MapIndices::iterator it = index_vert.begin(); it != index_vert.end(); ++it)
        {
            osg::Geometry *g = geo.getDrawable( it->first.second )->asGeometry();
            const osg::Array * texarray = g->getNumTexCoordArrays()>=1 ? g->getTexCoordArray(0) : NULL;
            if (!texarray || texarray->getNumElements()==0) continue;

            if (g->getTexCoordArray(0)->getType() != osg::Array::Vec2ArrayType)
            {
                OSG_NOTIFY(osg::FATAL) << "Texture coords array is not Vec2. Not implemented" << std::endl;
                _succeeded = false;
                return;
            }
            const osg::Vec2Array & vecs= *static_cast<const osg::Vec2Array *>(texarray);
            mesh->texcos[it->second][0] = vecs[it->first.first][0];
            mesh->texcos[it->second][1] = vecs[it->first.first][1];
        }
    }
    lib3ds_file_insert_mesh(_file3ds, mesh, _lastMeshIndex);
    ++_lastMeshIndex;

    Lib3dsMeshInstanceNode * node3ds = lib3ds_node_new_mesh_instance(mesh, mesh->name, NULL, NULL, NULL);
    lib3ds_file_append_node(_file3ds, reinterpret_cast<Lib3dsNode*>(node3ds), reinterpret_cast<Lib3dsNode*>(_cur3dsNode));
}

unsigned int
WriterNodeVisitor::calcVertices(osg::Geode & geo)
{
    unsigned int numVertice = 0;
    for (unsigned int i = 0; i < geo.getNumDrawables(); ++i)
    {
        osg::Geometry *g = geo.getDrawable( i )->asGeometry();
        if (g!=NULL && g->getVertexArray()) numVertice += g->getVertexArray()->getNumElements();
    }
    return numVertice;
}


void
WriterNodeVisitor::buildFaces(osg::Geode        & geo,
                              const osg::Matrix & mat,
                              ListTriangle      & listTriangles,
                              bool                texcoords)
{
    unsigned int nbTrianglesRemaining = listTriangles.size();
    unsigned int nbVerticesRemaining  = calcVertices(geo);        // May set _succeded to false
    if (!succeeded()) return;

    std::string name( getUniqueName(geo.getName().empty() ? geo.className() : geo.getName(), true, "geo") );
    if (!succeeded()) return;
    Lib3dsMesh *mesh = lib3ds_mesh_new( name.c_str() );
    if (!mesh)
    {
        OSG_NOTIFY(osg::FATAL) << "Allocation error" << std::endl;
        _succeeded = false;
        return;
    }

    //copyOsgMatrixToLib3dsMatrix(mesh->matrix, mat);
    lib3ds_mesh_resize_faces   (mesh, osg::minimum(nbTrianglesRemaining, MAX_FACES));
    lib3ds_mesh_resize_vertices(mesh, osg::minimum(nbVerticesRemaining,  MAX_VERTICES), texcoords ? 0 : 1, 0);        // Not mandatory but will allocate once a big block

    // Test if the mesh will be split and needs sorting
    if (nbVerticesRemaining >= MAX_VERTICES || nbTrianglesRemaining >= MAX_FACES)
    {
        OSG_INFO << "Sorting elements..." << std::endl;
        WriterCompareTriangle cmp(geo, nbVerticesRemaining);
        std::sort(listTriangles.begin(), listTriangles.end(), cmp);
    }

    MapIndices index_vert;
    unsigned int numFace = 0;        // Current face index
    for (ListTriangle::iterator it = listTriangles.begin(); it != listTriangles.end(); ++it) //Go through the triangle list to define meshs
    {
        // Test if the mesh will be full after adding a face
        if (index_vert.size()+3 >= MAX_VERTICES || numFace+1 >= MAX_FACES)
        {
            // Finnish mesh
            lib3ds_mesh_resize_faces   (mesh, numFace);
            //lib3ds_mesh_resize_vertices() will be called in buildMesh()
            buildMesh(geo, mat, index_vert, texcoords, mesh);        // May set _succeded to false
            if (!succeeded())
            {
                lib3ds_mesh_free(mesh);
                return;
            }

            // "Reset" values and start over a new mesh
            index_vert.clear();
            nbTrianglesRemaining -= numFace;
            numFace = 0;
            // We can't call a thing like "nbVerticesRemaining -= ...;" because points may be used multiple times.
            // [Sukender: An optimisation here would take too much time I think.]

            mesh = lib3ds_mesh_new( getUniqueName(geo.getName().empty() ? geo.className() : geo.getName(), true, "geo").c_str());
            if (!mesh)
            {
                OSG_NOTIFY(osg::FATAL) << "Allocation error" << std::endl;
                _succeeded = false;
                return;
            }
            lib3ds_mesh_resize_faces   (mesh, osg::minimum(nbTrianglesRemaining, MAX_FACES));
            lib3ds_mesh_resize_vertices(mesh, osg::minimum(nbVerticesRemaining,  MAX_VERTICES), texcoords ? 0 : 1, 0);        // Not mandatory but will allocate once a big block
        }
        Lib3dsFace & face = mesh->faces[numFace++];
        face.index[0] = getMeshIndexForGeometryIndex(index_vert, it->first.t1, it->second);
        face.index[1] = getMeshIndexForGeometryIndex(index_vert, it->first.t2, it->second);
        face.index[2] = getMeshIndexForGeometryIndex(index_vert, it->first.t3, it->second);
        face.material = it->first.material;
    }

    buildMesh(geo, mat, index_vert, texcoords, mesh);        // May set _succeded to false
    if (!succeeded())
    {
        lib3ds_mesh_free(mesh);
        return;
    }
}

void
WriterNodeVisitor::createListTriangle(osg::Geometry * geo,
                                      ListTriangle  & listTriangles,
                                      bool          & texcoords,
                                      unsigned int  & drawable_n)
{
    const osg::Array * basevecs = geo->getVertexArray();
    if (!basevecs || basevecs->getNumElements()==0) return;

    // Texture coords
    const osg::Array * basetexvecs = geo->getNumTexCoordArrays()>=1 ? geo->getTexCoordArray(0) : NULL;
    if (basetexvecs)
    {
        unsigned int nb = basetexvecs->getNumElements();
        if (nb != geo->getVertexArray()->getNumElements())
        {
            OSG_NOTIFY(osg::FATAL) << "There are more/less texture coords than vertices (corrupted geometry)" << std::endl;
            _succeeded = false;
            return;
        }
        texcoords = true;
    }

    int material = processStateSet(_currentStateSet.get());

    for(unsigned int i = 0; i < geo->getNumPrimitiveSets(); ++i) //Fill the Triangle List
    {
        osg::PrimitiveSet* ps = geo->getPrimitiveSet(i);
        PrimitiveIndexWriter pif(geo, listTriangles, drawable_n, material);
        ps->accept(pif);
    }
}

void WriterNodeVisitor::apply( osg::Geode &node )
{
    pushStateSet(node.getStateSet());
    //_nameStack.push_back(node.getName());
    unsigned int count = node.getNumDrawables();
    ListTriangle listTriangles;
    bool texcoords = false;
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *g = node.getDrawable( i )->asGeometry();
        if ( g != NULL )
        {
            pushStateSet(g->getStateSet());
            createListTriangle(g, listTriangles, texcoords, i);        // May set _succeded to false
            popStateSet(g->getStateSet());
            if (!succeeded()) break;
        }
    }
    if (succeeded() && count > 0)
    {
#if DISABLE_3DS_ANIMATION
        osg::Matrix mat( osg::computeLocalToWorld(getNodePath()) );
        buildFaces(node, mat, listTriangles, texcoords);        // May set _succeded to false
#else
        buildFaces(node, osg::Matrix(), listTriangles, texcoords);        // May set _succeded to false
#endif
    }
    popStateSet(node.getStateSet());
    //_nameStack.pop_back();
    if (succeeded())
        traverse(node);
}

void WriterNodeVisitor::apply( osg::Billboard &node )
{
    // TODO Does not handle Billboards' points yet

    pushStateSet(node.getStateSet());
    Lib3dsMeshInstanceNode * parent = _cur3dsNode;

    unsigned int count = node.getNumDrawables();
    ListTriangle listTriangles;
    bool texcoords = false;
    OSG_NOTICE << "Warning: 3DS writer is incomplete for Billboards (rotation not implemented)." << std::endl;
#if DISABLE_3DS_ANIMATION
    osg::Matrix m( osg::computeLocalToWorld(getNodePath()) );
#endif
    for ( unsigned int i = 0; i < count; i++ )
    {
        osg::Geometry *g = node.getDrawable( i )->asGeometry();
        if ( g != NULL )
        {
            listTriangles.clear();
            _cur3dsNode = parent;

            pushStateSet(g->getStateSet());
            createListTriangle(g, listTriangles, texcoords, i);
            popStateSet(g->getStateSet());        // May set _succeded to false
            if (!succeeded()) break;

            osg::Matrix pointLocalMat(osg::Matrix::translate(node.getPosition(i)));        // TODO handle rotation
#if DISABLE_3DS_ANIMATION
            osg::Matrix currentBillboardWorldMat(pointLocalMat * m);
            apply3DSMatrixNode(node, &pointLocalMat, "bil");                            // Add a 3DS matrix node (with local matrix)
            buildFaces(node, currentBillboardWorldMat, listTriangles, texcoords);        // May set _succeded to false
#else
            apply3DSMatrixNode(node, &pointLocalMat, "bil");                            // Add a 3DS matrix node (with local matrix)
            buildFaces(node, osg::Matrix(), listTriangles, texcoords);                    // May set _succeded to false
#endif
            if (!succeeded()) break;
        }
    }

    if (succeeded())
        traverse(node);
    _cur3dsNode = parent;
    popStateSet(node.getStateSet());
}



void WriterNodeVisitor::apply(osg::Group &node)
{
    pushStateSet(node.getStateSet());
    Lib3dsMeshInstanceNode * parent = _cur3dsNode;
#if DISABLE_3DS_ANIMATION
    osg::Matrix mat( osg::computeLocalToWorld(getNodePath()) );
    apply3DSMatrixNode(node, &mat, "grp");
#else
    apply3DSMatrixNode(node, NULL, "grp");
#endif
    if (succeeded())
        traverse(node);
    _cur3dsNode = parent;
    popStateSet(node.getStateSet());
}

void WriterNodeVisitor::apply(osg::MatrixTransform &node)
{
    pushStateSet(node.getStateSet());
    Lib3dsMeshInstanceNode * parent = _cur3dsNode;
#if DISABLE_3DS_ANIMATION
    osg::Matrix mat( osg::computeLocalToWorld(getNodePath()) );
#else
    osg::Matrix mat( node.getMatrix() );
#endif
    apply3DSMatrixNode(node, &mat, "mtx");
    if (succeeded())
        traverse(node);
    _cur3dsNode = parent;
    popStateSet(node.getStateSet());
}

void WriterNodeVisitor::apply3DSMatrixNode(osg::Node &node, const osg::Matrix * m, const char * const prefix)
{
    // Note: Creating a mesh instance with no transform and then copying the matrix doesn't work (matrix seems to be a temporary/computed value)
    Lib3dsMeshInstanceNode * parent = _cur3dsNode;
    Lib3dsMeshInstanceNode * node3ds = NULL;
    if (m)
    {
        osg::Vec3 osgScl, osgPos;
        osg::Quat osgRot, osgSo;
        m->decompose(osgPos, osgRot, osgScl, osgSo);

        float pos[3];
        float scl[3];
        float rot[4];
        copyOsgVectorToLib3dsVector(pos, osgPos);
        copyOsgVectorToLib3dsVector(scl, osgScl);
        copyOsgQuatToLib3dsQuat(rot, osgRot);
        node3ds = lib3ds_node_new_mesh_instance(NULL, getUniqueName(node.getName().empty() ? node.className() : node.getName(), true, prefix).c_str(), pos, scl, rot);
    }
    else
    {
        node3ds = lib3ds_node_new_mesh_instance(NULL, getUniqueName(node.getName().empty() ? node.className() : node.getName(), true, prefix).c_str(), NULL, NULL, NULL);
    }

    lib3ds_file_append_node(_file3ds, reinterpret_cast<Lib3dsNode*>(node3ds), reinterpret_cast<Lib3dsNode*>(parent));
    _cur3dsNode = node3ds;
}

// end namespace plugin3ds
}
