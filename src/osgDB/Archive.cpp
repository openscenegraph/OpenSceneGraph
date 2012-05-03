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

#include <osg/Notify>
#include <osg/Endian>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/Archive>

#include <streambuf>

using namespace osgDB;

osgDB::Archive* osgDB::openArchive(const std::string& filename, ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint)
{
    return openArchive(filename, status, indexBlockSizeHint, Registry::instance()->getOptions());
}

osgDB::Archive* osgDB::openArchive(const std::string& filename, ReaderWriter::ArchiveStatus status, unsigned int indexBlockSizeHint,Options* options)
{
    // ensure archive extension is in the registry list
    std::string::size_type dot = filename.find_last_of('.');
    if (dot != std::string::npos)
    {
        std::string ext = filename.substr(dot+1);
        Registry::instance()->addArchiveExtension(ext);
    }
    ReaderWriter::ReadResult result = osgDB::Registry::instance()->openArchive(filename, status, indexBlockSizeHint, options);
    return result.takeArchive();
}

Archive::Archive()
{
    OSG_INFO<<"Archive::Archive() open"<<std::endl;
}

Archive::~Archive()
{
    OSG_INFO<<"Archive::~Archive() closed"<<std::endl;
}

void cleanupFileString(std::string& strFileOrDir)
{
   if (strFileOrDir.empty())
   {
      return;
   }

   // convert all separators to unix-style for conformity
   for (unsigned int i = 0; i < strFileOrDir.length(); ++i)
   {
      if (strFileOrDir[i] == '\\')
      {
         strFileOrDir[i] = '/';
      }
   }

   // get rid of trailing separators
   if (strFileOrDir[strFileOrDir.length()-1] == '/')
   {
      strFileOrDir = strFileOrDir.substr(0, strFileOrDir.length()-1);
   }

   //add a beginning separator
   if(strFileOrDir[0] != '/')
   {
      strFileOrDir.insert(0, "/");
   }
}

osgDB::DirectoryContents osgDB::Archive::getDirectoryContents(const std::string& dirName) const
{
   DirectoryContents filenames;
   getFileNames(filenames);

   std::string searchPath = dirName;
   cleanupFileString(searchPath);

   osgDB::DirectoryContents dirContents;

   DirectoryContents::const_iterator iter = filenames.begin();
   DirectoryContents::const_iterator iterEnd = filenames.end();

   for(; iter != iterEnd; ++iter)
   {
      std::string currentFile = *iter;

      cleanupFileString(currentFile);

      if(currentFile.size() > searchPath.size())
      {
         size_t endSubElement = currentFile.find(searchPath);

         //we match the whole string in the beginning of the path
         if(endSubElement == 0)
         {
            std::string remainingFile = currentFile.substr(searchPath.size() + 1, std::string::npos);
            size_t endFileToken = remainingFile.find_first_of('/');

            if(endFileToken == std::string::npos)
            {
               dirContents.push_back(remainingFile);
            }
         }
      }
   }

   return dirContents;


}
