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

// DXWriter.h

#ifndef __OSG_DXWRITER_H
#define __OSG_DXWRITER_H

#include <osg/Vec4>
#include <osg/Node>

namespace dx {

//----------------------------------------------------------------------------

struct WriterParms {
    bool       set_default_color;       // Give color to uncolored objects
    osg::Vec4  default_color;           // Color to assign to uncolored objects
    char       outfile[ PATH_MAX ];     // Output pathname
    //bool     binary_mode;             // Write DX arrays in binary format

    WriterParms()
      { set_default_color = 0, outfile[0] = '\0'; }
};

//----------------------------------------------------------------------------

bool WriteDX( const osg::Node &node, WriterParms &parms, 
              std::string &messages );

};  // namespace dx

//----------------------------------------------------------------------------

#endif // __OSG_DXWRITER_H
