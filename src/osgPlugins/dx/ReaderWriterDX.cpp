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

// ReaderWriterDX.cpp

#include <osg/Notify>

#include <iostream>
#include <string.h>

#include "ReaderWriterDX.h"
#include "DXWriter.h"

// FIXME:  Support options
osgDB::ReaderWriter::WriteResult ReaderWriterDX::writeObject(const osg::Object &obj, const std::string &filename, const Options     *options ) const
{ 
  const osg::Node *node = dynamic_cast<const osg::Node *>(&obj);

  if ( node != 0 )
    return writeNode( *node, filename, options );
  else
    // FIXME:  This should probably be "OBJECT_NOT_HANDLED"
    return osgDB::ReaderWriter::WriteResult::FILE_NOT_HANDLED;
}


osgDB::ReaderWriter::WriteResult ReaderWriterDX::writeNode(const osg::Node &node, const std::string &filename, const Options     *) const
{
  dx::WriterParms parms;
  std::string     messages;
  bool            ret;
  osgDB::ReaderWriter::WriteResult result;

  std::string ext = osgDB::getFileExtension(filename);
  if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

  parms.outfile[0] = '\0';
  strncat( parms.outfile, filename.c_str(), sizeof(parms.outfile)-1 );

  // FIXME:  Allow specification via Options
  parms.set_default_color = 1;
  parms.default_color[0] = 
    parms.default_color[1] = 
    parms.default_color[2] = 0.75;
  parms.default_color[3] = 1.0;     // opaque light gray

  ret = dx::WriteDX( node, parms, messages );

  if ( ret )
    result = osgDB::ReaderWriter::WriteResult( 
                osgDB::ReaderWriter::WriteResult::FILE_SAVED );
  else
    result = osgDB::ReaderWriter::WriteResult( messages );

  osg::notify( osg::DEBUG_INFO ) << messages;

  return result;
}


// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterDX> g_dxReaderWriterProxy;
