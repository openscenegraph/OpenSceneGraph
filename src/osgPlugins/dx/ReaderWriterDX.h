// This plugin writes an IBM Data Explorer (aka OpenDX) native file.
// (c) Randall Hopper, 2002.
//
// For details on the OpenDX visualization tool, its use, and its file format,
//   refer to:
//
//   http://www.opendx.org/
//   http://www.opendx.org/support.html#docs
//   http://www.research.ibm.com/dx/
//   http://ftp.cs.umt.edu/DX/
//
// SUPPORTED  : Refer to DXWriter.cpp
// UNSUPPORTED: Refer to DXWriter.cpp
//

// ReaderWriterDX.h

#ifndef __OSG_READER_WRITER_DX_H
#define __OSG_READER_WRITER_DX_H

#include <string>
#include <osg/Object>
#include <osg/Node>
#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>


class ReaderWriterDX : public osgDB::ReaderWriter
{
 public:

  virtual const char* className() { return "OpenDX Writer"; }
  virtual bool acceptsExtension( const std::string &extension ) const
    { return osgDB::equalCaseInsensitive( extension, "dx" ); }

  virtual WriteResult writeObject( const osg::Object &obj, const std::string &filename, const Options *options = NULL ) const;

  virtual WriteResult writeNode  ( const osg::Node&node, const std::string &filename, const Options *options = NULL ) const;

};

#endif // __READER_WRITER_DX_H
