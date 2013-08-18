/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield 
 *
 * This software is open source and may be redistributed and/or modified under  
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * include LICENSE.txt for more details.
*/

#ifndef P3D_READFILE
#define P3D_READFILE 1

#include <osg/ArgumentParser>
#include <osgDB/ReaderWriter>

namespace p3d {

typedef std::vector<std::string> FileNameList;

bool getFileNames(osg::ArgumentParser& arguments, FileNameList& xmlFiles, FileNameList& normalFiles);

bool readEnvVars(osg::ArgumentParser& arguments);

bool readEnvVars(const std::string& filename);

osg::ref_ptr<osg::Node> readHoldingSlide(const std::string& filename);

osg::ref_ptr<osg::Node> readPresentation(const std::string& filename,const osgDB::ReaderWriter::Options* options);

osg::ref_ptr<osg::Node> readShowFiles(osg::ArgumentParser& arguments,const osgDB::ReaderWriter::Options* options);

}

#endif
