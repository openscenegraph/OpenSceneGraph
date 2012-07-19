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

#include <osgDB/ExternalFileWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <stdexcept>

namespace osgDB
{

/// Counts the number of directories the given relative path goes "up" the current dir, or 0 if not.
/// This returns 0 for absolute paths.
/// Examples:
///    - "../a" goes 1 level up
///    - "../../a/b/c/d/e" goes 2
///    - "../a/../b/../.." goes 2
///    - "a/b/../c" goes 0
static unsigned int countNbDirsUp(const std::string & path)
{
    // Algorithm:
    //    - For each path component, count +1 for "..", 0 for ".", and -1 for anything else
    //    - Ignore everything after  the last ".." of the path.
    if (isAbsolutePath(path)) return 0;
    int result(0), tempResult(0);
    std::vector<std::string> pathElems;
    getPathElements(path, pathElems);
    for(std::vector<std::string>::const_iterator it(pathElems.begin()), itEnd(pathElems.end()); it!=itEnd; ++it)
    {
        if (*it == "..")
        {
            // Count +1, and "validates" temporary result
            ++tempResult;
            result = tempResult;
        }
        else if (*it != ".") --tempResult;
    }
    return result<=0 ? 0 : static_cast<unsigned int>(result);
}


/// Local hash function for a path.
/// Does not canonize the given path, but is not confused with mixed separators.
static unsigned int pathHash(const std::string & s)
{
    // This is based on the DJB hash algorithm
    // Note: SDBM Hash initializes at 0 and is
    //        hash = c + (hash << 6) + (hash << 16) - hash;
    unsigned int hash = 5381;
    for(std::string::const_iterator it=s.begin(), itEnd=s.end(); it!=itEnd; ++it)
    {
        std::string::value_type c = *it;
        if (c == '\\') c = '/';        // We're processing a path and don't want to be affected by differences in separators
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}


//virtual ReaderWriter::WriteResult writeObject(const osg::Object& obj, const std::string& fileName,const Options* options);
//virtual ReaderWriter::WriteResult writeImage(const osg::Image& obj, const std::string& fileName,const Options* options);
//virtual ReaderWriter::WriteResult writeHeightField(const osg::HeightField& obj, const std::string& fileName,const Options* options);
//virtual ReaderWriter::WriteResult writeNode(const osg::Node& obj, const std::string& fileName,const Options* options);
//virtual ReaderWriter::WriteResult writeShader(const osg::Shader& obj, const std::string& fileName,const Options* options);

enum WriteType {
    WRITE_TYPE_OBJECT,
    WRITE_TYPE_IMAGE,
    WRITE_TYPE_HEIGHT_FIELD,
    WRITE_TYPE_NODE,
    WRITE_TYPE_SHADER,

    MAX_WRITE_TYPE
};

/// Default prefixes for unnamed objects.
static const char * const FILE_PREFIX[/*MAX_WRITE_TYPE*/] = {
    "Object_",
    "Image_",
    "HF_",
    "Node_",
    "Shader_"
};

/// Default prefixes for unnamed objects.
static const char * const FILE_EXTENSION[/*MAX_WRITE_TYPE*/] = {
    ".osgb",
    ".tga",
    ".osgb",
    ".osgb",
    ".glsl"
};

//.frag
//.vert

inline WriteType getType(const osg::Object & obj)
{
    // Is there something faster than a dynamic_cast<>?
    if (dynamic_cast<const osg::Image       *>(&obj)) return WRITE_TYPE_IMAGE;
    if (dynamic_cast<const osg::HeightField *>(&obj)) return WRITE_TYPE_HEIGHT_FIELD;
    if (dynamic_cast<const osg::Node        *>(&obj)) return WRITE_TYPE_NODE;
    if (dynamic_cast<const osg::Shader      *>(&obj)) return WRITE_TYPE_SHADER;
    return WRITE_TYPE_OBJECT;
}

/// Returns the object filename if available, or its name otherwise.
inline const std::string & getFileName(const osg::Object & obj, WriteType type)
{
    switch(type) {
        case WRITE_TYPE_IMAGE:        return static_cast<const osg::Image       &>(obj).getFileName();
        case WRITE_TYPE_SHADER:       return static_cast<const osg::Shader      &>(obj).getFileName();
        default:        // WRITE_TYPE_OBJECT, WRITE_TYPE_NODE, WRITE_TYPE_HEIGHT_FIELD
            return obj.getName();
    }
}


inline bool doWrite(const osg::Object & obj, WriteType type, const std::string& fileName, const Options * options)
{
    switch(type) {
        case WRITE_TYPE_IMAGE:        return writeImageFile      (static_cast<const osg::Image       &>(obj), fileName, options);
        case WRITE_TYPE_HEIGHT_FIELD: return writeHeightFieldFile(static_cast<const osg::HeightField &>(obj), fileName, options);
        case WRITE_TYPE_NODE:         return writeNodeFile       (static_cast<const osg::Node        &>(obj), fileName, options);
        case WRITE_TYPE_SHADER:       return writeShaderFile     (static_cast<const osg::Shader      &>(obj), fileName, options);
        // WRITE_TYPE_OBJECT
        default:                      return writeObjectFile(obj, fileName, options);
    }
}



// --------------------------------------------------------------------------------


ExternalFileWriter::ExternalFileWriter(const std::string & srcDirectory, const std::string & destDirectory, bool keepRelativePaths, unsigned int allowUpDirs)
    : _lastGeneratedObjectIndex(0), _srcDirectory(srcDirectory), _destDirectory(destDirectory), _keepRelativePaths(keepRelativePaths), _allowUpDirs(allowUpDirs)
{}

ExternalFileWriter::ExternalFileWriter(const std::string & destDirectory)
    : _lastGeneratedObjectIndex(0), _destDirectory(destDirectory), _keepRelativePaths(false), _allowUpDirs(0)
{}


bool ExternalFileWriter::write(const osg::Object & obj, const Options * options, std::string * out_absolutePath, std::string * out_relativePath)
{
    ObjectsSet::iterator it( _objects.find(&obj) );
    if (it != _objects.end())
    {
        // Object has already been passed to this method
        if (out_absolutePath) *out_absolutePath = it->second.absolutePath;
        if (out_relativePath) *out_relativePath = it->second.relativePath;
        return it->second.written;
    }

    // Object is a new entry

    // Get absolute source path
    WriteType type( getType(obj) );
    std::string originalFileName( getFileName(obj, type) );
    std::string absoluteSourcePath;
    if (_keepRelativePaths && !originalFileName.empty())        // if keepRelativePaths is false, absoluteSourcePath is not used, then we can skip this part
    {
        if (isAbsolutePath(originalFileName)) absoluteSourcePath = originalFileName;
        else absoluteSourcePath = concatPaths(_srcDirectory, originalFileName);
        absoluteSourcePath = getRealPath(convertFileNameToNativeStyle(absoluteSourcePath));      // getRealPath() here is only used to canonize the path, not to add current directory in front of relative paths, hence the "concatPaths(_srcDirectory, ...)" just above
    }

    // Compute destination paths from the source path
    std::string relativeDestinationPath;
    std::string absoluteDestinationPath;
    if (absoluteSourcePath.empty())
    {
        // We have no name. Generate one.
        generateObjectName(relativeDestinationPath, absoluteDestinationPath, type);
    }
    else
    {
        // We have a name.
        if (_keepRelativePaths)
        {
            // We'll try to keep images relative path.
            relativeDestinationPath = getPathRelative(_srcDirectory, absoluteSourcePath);
            unsigned int nbDirsUp = countNbDirsUp(relativeDestinationPath);
            // TODO if nbDirsUp>nb dirs in _destDirectory, then issue a warning, and use simple file name
            if (nbDirsUp > _allowUpDirs) relativeDestinationPath = getSimpleFileName(absoluteSourcePath);
        }
        else
        {
            // We keep only the simple file name.
            relativeDestinationPath = getSimpleFileName(absoluteSourcePath);
        }
        absoluteDestinationPath = getRealPath(convertFileNameToNativeStyle( concatPaths(_destDirectory, relativeDestinationPath) ));
        // TODO Check for absolute paths collisions between multiple objects
    }

    // Write object
    bool written(false);
    if (!makeDirectoryForFile(absoluteDestinationPath))
    {
        OSG_NOTICE << "Can't create directory for file '" << absoluteDestinationPath << "'. May fail creating the image file." << std::endl;
    }
    if (!doWrite(obj, type, absoluteDestinationPath, options))
    {
        OSG_WARN << "Can't write file '" << absoluteDestinationPath << "'." << std::endl;
    }
    else written = true;

    // Add entry
    _objects.insert(ObjectsSet::value_type(&obj, ObjectData(absoluteDestinationPath, relativeDestinationPath, written)));
    _searchMap.insert(SearchMap::value_type(pathHash(absoluteDestinationPath), &obj));

    // Fill output strings
    if (out_absolutePath) *out_absolutePath = absoluteDestinationPath;
    if (out_relativePath) *out_relativePath = relativeDestinationPath;

    return written;
}


bool ExternalFileWriter::absoluteObjectPathExists(const std::string & path)
{
    // For all paths in the search map having the same hash as 'path', check if paths correspond
    std::pair<SearchMap::iterator, SearchMap::iterator> bounds( _searchMap.equal_range(pathHash(path)) );
    for(SearchMap::iterator it=bounds.first; it!=bounds.second; ++it)
    {
        const osg::Object * img( it->second );
        if (_objects[img].absolutePath == path) return true;
    }
    return false;
}

void ExternalFileWriter::generateObjectName(std::string & out_relativePath, std::string & out_absolutePath, int type)
{
    static const ObjectIndex MAX_NUMBER = UINT_MAX-1;        // -1 to allow doing +1 without an overflow
    for (ObjectIndex number=_lastGeneratedObjectIndex+1; number<MAX_NUMBER; ++number)
    {
        std::ostringstream oss;
        oss << FILE_PREFIX[type] << number << FILE_EXTENSION[type];
        out_relativePath = oss.str();
        out_absolutePath = concatPaths(_destDirectory, out_relativePath);

        if (!absoluteObjectPathExists(out_absolutePath))
        {
            _lastGeneratedObjectIndex = number;
            return;
        }
    }
    throw std::runtime_error("Could not get a free index to write image.");
}

}
