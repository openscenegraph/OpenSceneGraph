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

#ifndef OSGDB_FILENAMEUTILS
#define OSGDB_FILENAMEUTILS 1

#include <osgDB/Export>

#include <string>
#include <vector>

namespace osgDB {

/** Gets the parent path from full name (Ex: /a/b/c.Ext => /a/b). */
extern OSGDB_EXPORT std::string getFilePath(const std::string& filename);
/** Gets the extension without dot (Ex: /a/b/c.Ext => Ext). */
extern OSGDB_EXPORT std::string getFileExtension(const std::string& filename);
/** Gets the extension including dot (Ex: /a/b/c.Ext => .Ext). */
extern OSGDB_EXPORT std::string getFileExtensionIncludingDot(const std::string& filename);
/** Gets the lowercase extension without dot (Ex: /a/b/c.Ext => ext). */
extern OSGDB_EXPORT std::string getLowerCaseFileExtension(const std::string& filename);
/** Gets file name with extension (Ex: /a/b/c.Ext => c.Ext). */
extern OSGDB_EXPORT std::string getSimpleFileName(const std::string& fileName);
/** Gets file path without last extension (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file.ext1). */
extern OSGDB_EXPORT std::string getNameLessExtension(const std::string& fileName);
/** Gets file path without \b all extensions (Ex: /a/b/c.Ext => /a/b/c ; file.ext1.ext2 => file). */
extern OSGDB_EXPORT std::string getNameLessAllExtensions(const std::string& fileName);
/** Gets file name without last extension (Ex: /a/b/c.Ext => c ; file.ext1.ext2 => file.ext1). */
extern OSGDB_EXPORT std::string getStrippedName(const std::string& fileName);
/** If 'to' is in a subdirectory of 'from' then this function returns the subpath, otherwise it just returns the file name.
  * The function does \b not automagically resolve paths as the system does, so be careful to give canonical paths.
  * However, the function interprets slashes ('/') and backslashes ('\') as they were equal.
  */
extern OSGDB_EXPORT std::string getPathRelative(const std::string& from, const std::string& to);
/** Gets root part of a path ("/" or "C:"), or an empty string if none found. */
extern OSGDB_EXPORT std::string getPathRoot(const std::string& path);
/** Tests if path is absolute, as !getPathRoot(path).empty(). */
extern OSGDB_EXPORT bool isAbsolutePath(const std::string& path);


/** Converts forward slashes (/) to back slashes (\). */
extern OSGDB_EXPORT std::string convertFileNameToWindowsStyle(const std::string& fileName);
/** Converts back slashes (\) to forward slashes (/). */
extern OSGDB_EXPORT std::string convertFileNameToUnixStyle(const std::string& fileName);
extern OSGDB_EXPORT std::string convertToLowerCase(const std::string& fileName);

const char UNIX_PATH_SEPARATOR = '/';
const char WINDOWS_PATH_SEPARATOR = '\\';

/** Get the path separator for the current platform. */
extern OSGDB_EXPORT char getNativePathSeparator();
/** Check if the path contains only the current platform's path separators. */
extern OSGDB_EXPORT bool isFileNameNativeStyle(const std::string& fileName);
/** Convert the path to contain only the current platform's path separators. */
extern OSGDB_EXPORT std::string convertFileNameToNativeStyle(const std::string& fileName);

extern OSGDB_EXPORT bool equalCaseInsensitive(const std::string& lhs,const std::string& rhs);
extern OSGDB_EXPORT bool equalCaseInsensitive(const std::string& lhs,const char* rhs);

extern OSGDB_EXPORT bool containsServerAddress(const std::string& filename);
extern OSGDB_EXPORT std::string getServerProtocol(const std::string& filename);
extern OSGDB_EXPORT std::string getServerAddress(const std::string& filename);
extern OSGDB_EXPORT std::string getServerFileName(const std::string& filename);

/** Concatenates two paths */
extern OSGDB_EXPORT std::string concatPaths(const std::string& left, const std::string& right);

/** Removes .. and . dirs in a path */
extern OSGDB_EXPORT std::string getRealPath(const std::string& path);

/** Splits a path into elements between separators (including Windows' root, if any). */
extern OSGDB_EXPORT void getPathElements(const std::string& path, std::vector<std::string> & out_elements);

/** Functor for helping sort filename in alphabetical and numerical order when using in conjunction with std::sort.*/
struct FileNameComparator
{
    inline bool operator() (const std::string& lhs, const std::string& rhs) const
    {
        std::string::size_type size_lhs = lhs.size();
        std::string::size_type size_rhs = rhs.size();
        std::string::size_type pos_lhs = 0;
        std::string::size_type pos_rhs = 0;
        while(pos_lhs<size_lhs && pos_rhs<size_rhs)
        {
            char c_lhs = lhs[pos_rhs];
            char c_rhs = rhs[pos_rhs];
            bool numeric_lhs = lhs[pos_lhs]>='0' && lhs[pos_lhs]<='9';
            bool numeric_rhs = rhs[pos_rhs]>='0' && rhs[pos_rhs]<='9';
            if (numeric_lhs && numeric_rhs)
            {
                std::string::size_type start_lhs = pos_lhs;
                ++pos_lhs;
                while(pos_lhs<size_lhs && (lhs[pos_lhs]>='0' && lhs[pos_lhs]<='9')) ++pos_lhs;

                std::string::size_type start_rhs = pos_rhs;
                ++pos_rhs;
                while(pos_rhs<size_rhs && (rhs[pos_rhs]>='0' && rhs[pos_rhs]<='9')) ++pos_rhs;

                if (pos_lhs<pos_rhs) return true;
                else if (pos_rhs<pos_lhs) return false;

                while(start_lhs<pos_lhs && start_rhs<pos_rhs)
                {
                    if (lhs[start_lhs]<rhs[start_rhs]) return true;
                    if (lhs[start_lhs]>rhs[start_rhs]) return false;
                    ++start_lhs;
                    ++start_rhs;
                }
            }
            else
            {
                if (c_lhs<c_rhs) return true;
                else if (c_rhs<c_lhs) return false;

                ++pos_lhs;
                ++pos_rhs;
            }
        }

        return pos_lhs<pos_rhs;
    }
};


extern OSGDB_EXPORT void stringcopy(char* dest, const char* src, size_t length);

#define stringcopyfixedsize(DEST, SRC) stringcopy(DEST, SRC, sizeof(DEST));

}

#endif
