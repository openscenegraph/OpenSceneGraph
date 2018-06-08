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
