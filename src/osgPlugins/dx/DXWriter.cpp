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
// SUPPORTED:
//
//   - Objects without color are assigned a default opaque light gray color
//   - All GeoSet primitive types:
//         POINTS, LINES, TRIANGLES, QUADS, LINE_STRIP, FLAT_LINE_STRIP,
//         LINE_LOOP, TRIANGLE_STRIP, FLAT_TRIANGLE_STRIP, TRIANGLE_FAN,
//         FLAT_TRIANGLE_FAN, QUAD_STRIP, POLYGON
//     NOTE: area strips, fans, polygons, and quads are mapped to triangles 
//           fields; line strips and loops are mapped to polylines fields
//   - OSG Node Types:
//         Geode, Group, Billboards (hack), LOD (hack), Switch (hack)
//     NOTES: Billboards - written as a translation Transform parenting
//            the Geode Fields, so they do not rotate in DX like they should;
//            LODs - only the most detailed child is written;
//            Switches - only the active child is written
//   - Coords, colors, normals, tcoords; with all binding types
//     (per vertex/primitive/overall)
//   - MATERIAL attributes -- all except ColorMode
//   - Single texturing (TEXTURE_0), with RGB, RGBA, LUMINANCE, LUMINANCE_ALPHA 
//     textures (note:  you'll need a recent DX from CVS for this to work);
//     note that LUMINANCE* textures are expanded to RGB* in the conversion
//   - Texture function, texture min/mag filters
//   - Cull face
//
// UNSUPPORTED:
//
//   - OSG Scene Graph Node Types:
//       LightSource, Transform, Imposter, ClearNode, 
//       Switch - only active child written
//       LOD - only most detailed child  written
//       Billboard - only static translation transform parenting child written,
//                   and only one Geode per Billboard is supported; DX has
//                   no concept just like Performer/OSG billboards that rotate
//                   to face the camera but scale and translate with the scene
// 
//   - Interleaved GeoSet data (interleaved set, no coords set)
// 
//   - Report just the unhandled GL modes/attributes, not all of them
//   
//   - DX only supports UNSIGNED_BYTE GL_RGB (and now GL_RGBA in recent CVS) 
//     textures, so we convert LUMINANCE* textures to RGB* 
//     
//   - Handle other attributes in StateSet 
// 
//       - TEXTURE_[1-3], and rest of subattributes in TEXTURE_0
//       - MATERIAL - ColorMode (read but never processed); specifies that
//                    certain material attributes are set by glColor
//       - ALPHAFUNC
//       - ANTIALIAS
//       - COLORTABLE
//       - FOG
//       - FRONTFACE
//       - LIGHT, LIGHT_[0-7]
//       - POINT
//       - LINEWIDTH
//       - POLYGONMODE
//       - POLYGONOFFSET
//       - TEXENV
//       - TEXGEN
//       - TEXMAT
//       - TRANSPARENCY
//       - STENCIL
//       - COLORMASK
//       - DEPTH
//       - VIEWPORT
//       - CLIPPLANE, CLIPPLANE_[0-5]
//       - COLORMATRIX     
// 
// OTHER TO DO:
//  
//   - Use more meaningful names for nameless nodes (based on parent names)
//   - Don't write a colormap when OSG colors aren't indexed unless num
//     colors > 256 or something (see cane example)
//   - Support ReaderWriter::Options for exporter switches
//     - Support saving DX stream in both ASCII and binary
//   - Handle normal/color/tcoord collisions (aliasing) when coords are indexed
//     (See comment in DXArrayWriter::WritePerVertexNormals for details)
//     Same applies for normals, colors, and tcoords
//   - Handle accumulation of more than just Geodes in Groups (?)
//   - Consider support for writing DX file with interleaved arrays, 
//     arrays at end, arrays in diff file in DX file
//   - char [] -> std::string, globally
//
// DXWriter.h

#include <osg/Node>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/NodeVisitor>
#include <osg/StateAttribute>
#include <osg/StateSet>
#include <osg/Material>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/CullFace>
#include <osg/Billboard>
#include <osg/Math>

#include <osgDB/ReadFile>

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>


#include "DXWriter.h"
#include "AreaGeoSetTriangulator.h"
#include "StateSetStr.h"


#if defined(__sgi) || defined(__FreeBSD__)
    #include <unistd.h>
    #include <ieeefp.h>
#else
    #include <math.h>
    #if (defined(WIN32) || defined (__APPLE__)) && !defined(__CYGWIN__)
        #include <float.h>
    #else
        #include <unistd.h>
    #endif
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#define vsnprintf _vsnprintf
#define snprintf _snprintf
#endif


namespace dx {

//----------------------------------------------------------------------------
//  COMPILATION TWEAKS
//----------------------------------------------------------------------------

//#define SKIP_SINGLE_MEMBER_DXGROUPS

//----------------------------------------------------------------------------

const osg::Vec3 INVALID_NORMAL  ( -99,-99,-99 );
const osg::Vec3 INVALID_COLOR   ( 0,0,0 );
const float     INVALID_OPACITY = 1.0;
const float     ALPHA_OPAQUE    = 1.0;

#define ARRAY_LEN(a) (sizeof(a)/sizeof((a)[0]))

typedef unsigned char Vec4UB[4];
typedef unsigned char Vec3UB[3];

//----------------------------------------------------------------------------

class DXNameManager
  // Generates unique object names
{
  public:
    std::string GetUnique( const std::string &suggestion );

  protected:
    std::map< std::string, int > dict;
};

std::string DXNameManager::GetUnique( const std::string &suggestion )
{
  std::string name = suggestion;
  int         counter = 1;
  char        buf[30];

  while ( dict.find( name ) != dict.end() ) {
    sprintf( buf, " #%d", counter++ );
    name = ( suggestion.empty() ? "Object" : suggestion ) + buf;
  }
  dict[name] = 1;
  return name;
}

//----------------------------------------------------------------------------

class DXTextureManager
  // Keeps track of what textures we've already written and their field names
{
  public:
    void Register( const osg::Image *image, std::string dx_name )
      { dict[ image ] = dx_name; }
    bool IsRegistered( const osg::Image *image )
      { return dict.find( image ) != dict.end(); }
    std::string Lookup( const osg::Image *image )
      { return dict[ image ]; }

  protected:
    std::map< const osg::Image *, std::string > dict;
};

//----------------------------------------------------------------------------

class MessageBin
{
  protected:
    typedef std::vector< std::string > MessageList;
    MessageList msg_list;

  public:
    void Add( char fmt[], ... )
    {
      char msg[1024];
      va_list args;
      va_start(args, fmt);
      vsnprintf(msg, sizeof(msg), fmt, args);
      va_end(args);

      msg_list.push_back( std::string( msg ) );      
    }

    std::string GetPending()
    {
      std::string result;
      for ( unsigned i = 0; i < msg_list.size(); i++ ) {
        if ( !result.empty() )
          result += '\n';
        result += msg_list[i];
      }
      return result;
    }
    
    void Clear()
      { msg_list.clear(); }
};

//----------------------------------------------------------------------------

class DXField
{
  public:
    struct Component {
        std::string name;
        std::string value;
        Component( const char name[], const char value[] )
          : name(name), value(value) {}
    };

    std::vector< Component > comp;
    std::string name;

    DXField( DXNameManager &name_mgr, const std::string *name_suggestion ) 
    { if ( name_suggestion && !name_suggestion->empty() )
        name = *name_suggestion;
      else
        name = "Field";
      name = name_mgr.GetUnique( name );
    }

    std::string &GetName() { return name; }

    void AddComponent( const char name[], const char value[] )
      { comp.push_back( Component( name, value ) ); }

    void Write( FILE *fp, const char *attr=0 )
    {
      fprintf( fp, "object \"%s\" class field\n", name.c_str() );
      for ( unsigned i = 0; i < comp.size(); i++ )
        fprintf( fp, "component \"%s\" value \"%s\"\n",
                 comp[i].name.c_str(), comp[i].value.c_str() );
      if ( attr )
        fprintf( fp, "%s\n", attr );
      fprintf( fp, "#\n\n" );
    }
};

//----------------------------------------------------------------------------

class DXGroup
{
  public:
    typedef std::vector< std::string > MemberList;
    MemberList members;
    std::string name;
    
    DXGroup( DXNameManager &name_mgr, 
             const std::string *name_suggestion     = 0,
             const std::string *fallback_suggestion = 0 ) 
    { if ( name_suggestion && !name_suggestion->empty() )
        name = *name_suggestion;
    else if ( fallback_suggestion )
        name = fallback_suggestion->c_str();
    else
        name = "Group";
      name = name_mgr.GetUnique( name );
    }

    void AddMember( const char member_name[] )
      { members.push_back( std::string( member_name ) ); }

    void RemoveMember( const char member_name[] )
    { 
      for(MemberList::iterator itr=members.begin(); 
          itr!=members.end();
          ++itr)
        if ( *itr == member_name ) {
          members.erase(itr);
          break;
        }
    }

    int GetNumMembers() { return members.size(); }

    std::string &GetName() { return name; }

    void Write( FILE *fp, const char *attr=0 )
    {
      fprintf( fp, "object \"%s\" class group\n", GetName().c_str() );
      for ( unsigned i = 0; i < members.size(); i++ )
        fprintf( fp, "member %d value \"%s\"\n", i, members[i].c_str() );
      if ( attr )
        fprintf( fp, "%s\n", attr );
      fprintf( fp, "#\n\n" );
    }
};

//----------------------------------------------------------------------------

class DXArrayWriter
{
  protected:
    FILE       *fp;
    MessageBin *msg_bin;

  public:
    void SetFP    ( FILE *_fp ) { fp = _fp; }
    void SetMsgBin( MessageBin *_msg_bin ) { msg_bin = _msg_bin; }

    void DeNanify( float &f, float subst );
    void DeNanify( osg::Vec3 &v, const osg::Vec3 &subst );
    void OSGColorToDX( const osg::Vec4 &osg_color, 
                       osg::Vec3 &dx_color, float &dx_opacity );
    void OSGColorToDX( const Vec4UB osg_color, 
                       Vec3UB dx_color, float &dx_opacity );
    int  WriteMapsYN( osg::GeoSet::IndexPointer *ip, int num_colors );
    void WriteAttributes( const char *ref, const char *dep, const char *attr );
    void WriteFloatConstArray( float f, int len, const char name[], 
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteVec3ConstArray( const osg::Vec3 &v, int len, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteFloatArray( const float *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteVec2Array( const osg::Vec2 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteVec3Array( const osg::Vec3 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteVec4Array( const osg::Vec4 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteUByte3Array( 
                    const Vec3UB *arrayp, osg::GeoSet::IndexPointer *ip,
                    int len, const char name[],
                    const char *ref, const char *dep, const char *attr=0 );

    void WriteIndexArray( const osg::GeoSet::IndexPointer &ip, int len,
                         int rank, int shape, int ubyte, const char name[],
                         const char *ref, const char *dep, const char *attr=0 );
    void WriteColors( const osg::Vec4 *colors, int num_colors,
                      osg::GeoSet::IndexPointer *ip, int num_cindices,
                      const char colors_name[], const char colormap_name[],
                      const char opacities_name[], const char opacitymap_name[],
                      int do_opacities, const char *dep, int &wrote_maps );

    void WriteColors( const Vec4UB colors[], int num_colors,
                      osg::GeoSet::IndexPointer *ip, int num_cindices,
                      const char colors_name[], const char colormap_name[],
                      const char opacities_name[], const char opacitymap_name[],
                      int do_opacities, const char *dep, int &wrote_maps );

    void WritePerVertexNormals( const osg::GeoSet &geoset, const char name[] );
    void WritePerVertexColors( const osg::GeoSet &geoset, 
                               const char colors_name[], 
                               const char colormap_name[],
                               const char opacities_name[],
                               const char opacitymap_name[],
                               int do_opacities, // FIXME 
                               int &wrote_maps );
    void WritePerVertexTCoords( const osg::GeoSet &geoset, const char name[] );
};

//----------------------------------------------------------------------------

class MyStateSet
{
  protected:
    MessageBin &msg_bin;

  public:

    std::map< osg::StateAttribute::Type, int > attr;

    // MATERIAL (osg::Material)
    osg::Material::ColorMode colormode;
    osg::Vec4 ambient_f, ambient_b;      // Front and back colors
    bool      ambient_f_and_b;           // Use front color for front and back
    osg::Vec4 diffuse_f, diffuse_b;
    bool      diffuse_f_and_b;
    osg::Vec4 specular_f, specular_b;
    bool      specular_f_and_b;
    osg::Vec4 emission_f, emission_b;
    bool      emission_f_and_b;
    float     shininess_f, shininess_b;
    bool      shininess_f_and_b;

    // TEXTURE / TEXTURE_0
    const osg::Image *image;
    osg::Texture2D::WrapMode   wrap_s, wrap_t;
    osg::Texture2D::FilterMode min_filter, mag_filter;

    // TEXENV
    osg::TexEnv::Mode texture_func;

    MyStateSet( MessageBin &msg_bin ) : msg_bin(msg_bin), image(0) {}

    // CULLFACE
    osg::CullFace::Mode cullface_mode;

    void Show( osg::StateSet &sset );
    void AddAttribute( osg::StateAttribute::Type type )
      { attr[ type ] = 1; }
    bool HasAttribute( osg::StateAttribute::Type type )
      { return attr.find( type ) != attr.end(); }
    void Query( const osg::StateSet &sset );
    void Write();
};

//----------------------------------------------------------------------------

class StateSetCopy : public osg::StateSet
{
  public:
    StateSetCopy() {}
    ~StateSetCopy() {}

    StateSetCopy( const osg::StateSet &sset )
    {
      // Can't use operator = because StateSet broke it
      _modeList      = sset.getModeList();
      _attributeList = sset.getAttributeList();
      _renderingHint = sset.getRenderingHint();
      _binMode       = sset.getRenderBinMode();
      _binNum        = sset.getBinNumber();
      _binName       = sset.getBinName();
    }
};

//----------------------------------------------------------------------------

class DXWriter
{
  protected:
    DXArrayWriter w;

    typedef std::map<osg::StateAttribute::GLMode,int> ModeList;
    typedef std::map<osg::StateAttribute::Type,int>   AttrList;
    ModeList unsupported_modes;
    AttrList unsupported_attrs;

  public:
    FILE            *fp;
    WriterParms      parms;
    DXNameManager    name_mgr;
    DXTextureManager texture_mgr;
    MessageBin       msg_bin;

    void SetParms( const WriterParms &p )
      { memcpy( &parms, &p, sizeof(parms) ); }

    void Open();
    void Close();

    std::string BuildStateSetAttributes( MyStateSet &sset, 
                                         int diffuse_is_color,
                                         int diffuse_is_opacity );

    std::string WriteImage( const osg::Image &image );

    void WritePolylineConnections( osg::GeoSet &geoset,
                                   DXField &field );
    std::string WriteGeoSetField( const std::string &field_name, 
                                  osg::GeoSet &geoset,
                                  MyStateSet  &sset );
    std::string WriteGeoSet( osg::GeoSet         &geoset, 
                             const osg::StateSet &active_sset,
                             std::string         &field_name );
    std::string WriteGeode( osg::Geode          &geode, 
                            const osg::StateSet &active_sset );

    void CollectUnhandledModesAndAttrs( osg::StateSet *sset );
    void ReportUnhandledModesAndAttrs();
};

//----------------------------------------------------------------------------

inline int WARNING( char fmt[], ... )
{
  va_list args;
  va_start(args, fmt);
  int ret = vfprintf(stderr, fmt, args);
  va_end(args);
  return ret;
}

//----------------------------------------------------------------------------

void GetParms( int argc, char *argv[], 
               char infile [ DX_PATH_MAX ], WriterParms &parms )
{
  WriterParms defaults;

  /*  Init defaults  */
  parms = defaults;

  /*  Parse user args  */
  int   c, i, errflg = 0;
  char *arg = NULL;

  /*  We'd use getopt(), but we have args with leading minus chars...  */
  for ( i = 1; i < argc && !errflg; i++ ) {
    if ( argv[i][0] == '\0' ) continue;

    if ( argv[i][0] != '-' ) 
      break;

    c = argv[i][1];
    if ( strchr( "c", c ) ) {
      if ( i+1 >= argc ) {
        WARNING( "Missing argument to -%c option.\n\n", c );
        errflg++; 
        break;
      }

      arg = argv[++i];
    }

    switch ( c ) {
      case 'c': parms.set_default_color = 1;
                sscanf( arg, "%g,%g,%g,%g", 
                        &parms.default_color[0], &parms.default_color[1],
                        &parms.default_color[2], &parms.default_color[3] );
                break;
      case '?':
      default :
           errflg++;
    }
  }

  if ( i != argc - 2 )
    errflg++;

  if (errflg) {
    WARNING( "\n"
             "Converts 3D model files to IBM Data Explorer format.\n\n"
             "Usage: osg2dx\n"
             "        [-c r,g,b,a]          # Give uncolored objs this color\n"
             "        <infile>              # Input model pathname\n"
             "        <outfile>             # Output DX pathname (- = stdout)\n"
             "\n" );
    exit(2);
  }

  infile[0] = parms.outfile[0] = '\0';
  strncat( infile       , argv[ i ], DX_PATH_MAX-1              );
  strncat( parms.outfile, argv[i+1], sizeof(parms.outfile)-1 );
}

//----------------------------------------------------------------------------

inline int IsNaNorInf( float f )
{
#if defined(__sgi)
  int c = fpclass( double(f) );
  switch (c) 
  {
    case FP_SNAN :
    case FP_QNAN :
    case FP_NINF :
    case FP_PINF : return 1;
    default      : return 0;
  }
#elif defined(__FreeBSD__) || defined(__linux) || defined(__CYGWIN__) 
  return isnanf(f) || isinf(f);
#elif  defined(__APPLE__)
  return __isnanf(f) || __isinf(f);
#elif defined(__sun)
  return isnan(f);  // KLUDGE - hack to get this to compile w/g++. 
#elif defined(WIN32)
  return _isnan(f) || !_finite(f);
#elif defined(__hpux__)
  // this is a hack - cmath from gcc 3.1 #undef's isinf() but does not
  // redefine it (at least for me), enabling _GLIBCPP_USE_C99 also
  // doesn't help (loads of compilation errors), so I just repeat the
  // definition of isinf() from math.h here.
  return isnan(f) || (_ISINF(f));
#else
#  error Teach me how to find non-numbers on this platform.
return 0;
#endif
}


void DXArrayWriter::DeNanify( float &f, float subst )
{
  if ( IsNaNorInf((double)f) ) {
    msg_bin->Add( "WARNING:  Denanifying double.\n" );
    f = subst;
  }
}

void DXArrayWriter::DeNanify( osg::Vec3 &v, const osg::Vec3 &subst )
{
  // An old OSG bug generated NaN normals from 0,0,0 normals;
  //   consider removing this someday.
  // Also, Performer town has a NaN in one of the colormaps.
  if ( IsNaNorInf((double)v[0]) || IsNaNorInf((double)v[1]) || 
       IsNaNorInf((double)v[2]) ) {
    msg_bin->Add( "WARNING:  Denanifying 3D vector.\n" );
    v = subst;
  }
}

//----------------------------------------------------------------------------

void DXArrayWriter::OSGColorToDX( const osg::Vec4 &osg_color, 
                                  osg::Vec3 &dx_color, float &dx_opacity )
{
  dx_color.set( osg_color[0], osg_color[1], osg_color[2] );
  dx_opacity = osg_color[3];     // alpha==1.0 -> opaque
}

//----------------------------------------------------------------------------

void DXArrayWriter::OSGColorToDX( const Vec4UB osg_color, 
                                  Vec3UB dx_color, float &dx_opacity )
{
  dx_color[0] = osg_color[0];
  dx_color[1] = osg_color[1];
  dx_color[2] = osg_color[2];
  dx_opacity = osg_color[3] / 255.0;  // alpha==1.0 -> opaque
}

//----------------------------------------------------------------------------

int DXArrayWriter::WriteMapsYN( osg::GeoSet::IndexPointer *ip, int num_colors )
{
  // Yes, if we have indexed colors and the number of colors mapped to
  //   is <= 256.
  return ( ip && ip->valid() && num_colors <= 256 );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteAttributes( const char *ref, const char *dep, 
                                     const char *attr )
{
  if ( attr && attr[0] )
    fprintf( fp, "%s\n", attr );
  if ( ref )
    fprintf( fp, "attribute \"ref\" string \"%s\"\n", ref );
  if ( dep )
    fprintf( fp, "attribute \"dep\" string \"%s\"\n", dep );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteFloatConstArray( 
                         float f, int len, const char name[], 
                         const char *ref, const char *dep, const char *attr )
{
  fprintf( fp, "object \"%s\" class constantarray type float"
               " rank 0 items %d data follows\n",
           name, len );
  fprintf( fp, "  %g\n", f );
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteVec3ConstArray( 
                         const osg::Vec3 &v, int len, const char name[],
                         const char *ref, const char *dep, const char *attr )
{
  fprintf( fp, "object \"%s\" class constantarray type float"
               " rank 1 shape 3 items %d data follows\n",
           name, len );
  fprintf( fp, "  %g %g %g\n", v[0], v[1], v[2] );
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteFloatArray( 
                          const float *array, osg::GeoSet::IndexPointer *ip,
                          int len, const char name[],
                          const char *ref, const char *dep, const char *attr )
  // If ip && ip.valid(), use it to index; else index array directly
{
  int index;

  fprintf( fp, "object \"%s\" class array type float"
               " rank 0 items %d data follows\n",
           name, len );
  for ( int i = 0; i < len; i++ ) {
    index = ( ip && ip->valid() ) ? (*ip)[i] : i;
      
    fprintf( fp, "  %g\n", array[index] );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteVec2Array( 
                         const osg::Vec2 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr )
  // If ip && ip.valid(), use it to index; else index array directly
{
  int index;

  fprintf( fp, "object \"%s\" class array type float"
               " rank 1 shape 2 items %d data follows\n",
           name, len );
  for ( int i = 0; i < len; i++ ) {
    index = ( ip && ip->valid() ) ? (*ip)[i] : i;
      
    fprintf( fp, "  %g %g\n", array[index][0], array[index][1] );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteVec3Array( 
                         const osg::Vec3 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr )
  // If ip && ip.valid(), use it to index; else index array directly
{
  int index;

  fprintf( fp, "object \"%s\" class array type float"
               " rank 1 shape 3 items %d data follows\n",
           name, len );
  for ( int i = 0; i < len; i++ ) {
    index = ( ip && ip->valid() ) ? (*ip)[i] : i;
      
    fprintf( fp, "  %g %g %g\n", 
             array[index][0], array[index][1], array[index][2] );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteVec4Array( 
                         const osg::Vec4 *array, osg::GeoSet::IndexPointer *ip,
                         int len, const char name[],
                         const char *ref, const char *dep, const char *attr )
  // If ip && ip.valid(), use it to index; else index array directly
{
  int index;

  fprintf( fp, "object \"%s\" class array type float"
               " rank 1 shape 4 items %d data follows\n",
           name, len );
  for ( int i = 0; i < len; i++ ) {
    index = ( ip && ip->valid() ) ? (*ip)[i] : i;
      
    fprintf( fp, "  %g %g %g %g\n", 
             array[index][0], array[index][1], 
             array[index][2], array[index][3] );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteIndexArray( 
                          const osg::GeoSet::IndexPointer &ip, int len,
                          int rank, int shape, int ubyte,
                          const char name[],
                          const char *ref, const char *dep, const char *attr )
  // If ip.null(), write a sequential index (0,1,2,3,...)
{
  const char *type_str = ( ubyte ? "unsigned byte" : "int" );
  char shape_str[40];

  assert( rank == 0 || rank == 1 );
  if ( rank == 0 )
    shape_str[0] = '\0';
  else
    sprintf( shape_str, "shape %d ", shape );
  fprintf( fp, "object \"%s\" class array type %s"
               " rank %d %sitems %d data follows\n",
           name, type_str, rank, shape_str, len/shape );

  for ( int i = 0; i < len; i++ ) {
    if ( i % shape == 0 )
      fprintf( fp, " " );
    {
      if ( ip.null() ) fprintf( fp, " %d", i );
      else             fprintf( fp, " %d", static_cast<int>( ip[i] ) );
    }
    if ( (i+1) % shape == 0 )
      fprintf( fp, "\n" );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteUByte3Array( 
                   const Vec3UB *array, osg::GeoSet::IndexPointer *ip,
                   int len, const char name[],
                   const char *ref, const char *dep, const char *attr )
  // If ip && ip.valid(), use it to index; else index array directly
{
  int index;

  fprintf( fp, "object \"%s\" class array type unsigned byte"
               " rank 1 shape 3 items %d data follows\n",
           name, len );
  for ( int i = 0; i < len; i++ ) {
    index = ( ip && ip->valid() ) ? (*ip)[i] : i;
      
    fprintf( fp, "  %d %d %d\n", 
             array[index][0], array[index][1], array[index][2] );
  }
  WriteAttributes( ref, dep, attr );
  fprintf( fp, "#\n\n" );
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteColors( 
                      const osg::Vec4 *colors, int num_colors,
                      osg::GeoSet::IndexPointer *ip, int num_cindices,
                      const char colors_name[], const char colormap_name[],
                      const char opacities_name[], const char opacitymap_name[],
                      int do_opacities, const char *dep, int &wrote_maps )
{
  int do_maps = WriteMapsYN( ip, num_colors );
  int i;

  // If should write color/opacity maps...
  if ( do_maps ) {

    // Separate colors and opacities
    osg::Vec3 *colors2    = new osg::Vec3[256];
    float     *opacities2 = new float    [256];
    for ( i = 0; i < num_colors; i++ ) {
      OSGColorToDX( colors[i], colors2[i], opacities2[i] );
      DeNanify( colors2[i], INVALID_COLOR );
      DeNanify( opacities2[i], INVALID_OPACITY );
    }

    for ( ; i < 256; i++ )
      colors2[i].set( 0,0,0 ), opacities2[i] = 0;

    // Write color/opacity maps
    WriteVec3Array( colors2, 0, 256, colormap_name, 0, 0 );
    if ( do_opacities )
      WriteFloatArray( opacities2, 0, 256, opacitymap_name, 0, 0);

    // Write colors/opacities
    WriteIndexArray( *ip, num_cindices, 0, 1, 1, colors_name, 
                     "color map", dep );

    if ( do_opacities )
      WriteIndexArray( *ip, num_cindices, 0, 1, 1, opacities_name, 
                       "opacity map", dep );

    delete [] colors2;
    delete [] opacities2;
  }
  
  // Else write direct colors/opacities...
  else {
    int len = ip && ip->valid() ? num_cindices : num_colors;

    // Separate colors and opacities
    osg::Vec3 *colors2    = new osg::Vec3[ len ];
    float     *opacities2 = new float    [ len ];
    for ( i = 0; i < len; i++ ) {
      int index = ip && ip->valid() ? (*ip)[i] : i;
      OSGColorToDX( colors[ index ], colors2[i], opacities2[i] );
      DeNanify( colors2[i], INVALID_COLOR );
      DeNanify( opacities2[i], INVALID_OPACITY );
    }

    WriteVec3Array( colors2, 0, len, colors_name, 0, dep );
    if ( do_opacities ) 
      WriteFloatArray( opacities2, 0, len, opacities_name, 0, dep );

    delete [] colors2;
    delete [] opacities2;
  }

  wrote_maps = do_maps;
}

//----------------------------------------------------------------------------

void DXArrayWriter::WriteColors( 
                      const Vec4UB colors[], int num_colors,
                      osg::GeoSet::IndexPointer *ip, int num_cindices,
                      const char colors_name[], const char colormap_name[],
                      const char opacities_name[], const char opacitymap_name[],
                      int do_opacities, const char *dep, int &wrote_maps )
{
  int do_maps = WriteMapsYN( ip, num_colors );
  int i;

  // If should write color/opacity maps...
  if ( do_maps ) {

    // Separate colors and opacities
    Vec3UB *colors2    = new Vec3UB[256];
    float  *opacities2 = new float [256];
    for ( i = 0; i < num_colors; i++ )
      OSGColorToDX( colors[i], colors2[i], opacities2[i] );

    for ( ; i < 256; i++ ) {
      colors2[i][0] = colors2[i][1] = colors2[i][2] = 0;
      opacities2[i] = 0.0;
    }

    // Write color/opacity maps
    WriteUByte3Array( colors2, 0, 256, colormap_name, 0, 0 );
    if ( do_opacities )
      WriteFloatArray( opacities2, 0, 256, opacitymap_name, 0, 0);

    // Write colors/opacities
    WriteIndexArray( *ip, num_cindices, 0, 1, 1, colors_name, 
                     "color map", dep );

    if ( do_opacities )
      WriteIndexArray( *ip, num_cindices, 0, 1, 1, opacities_name, 
                       "opacity map", dep );

    delete [] colors2;
    delete [] opacities2;
  }
  
  // Else write direct colors/opacities...
  else {
    int len = ip && ip->valid() ? num_cindices : num_colors;

    // Separate colors and opacities
    Vec3UB *colors2    = new Vec3UB[ len ];
    float  *opacities2 = new float [ len ];
    for ( i = 0; i < len; i++ ) {
      int index = ip && ip->valid() ? (*ip)[i] : i;
      OSGColorToDX( colors[ index ], colors2[i], opacities2[i] );
    }

    WriteUByte3Array( colors2, 0, len, colors_name, 0, dep );
    if ( do_opacities ) 
      WriteFloatArray( opacities2, 0, len, opacities_name, 0, dep );

    delete [] colors2;
    delete [] opacities2;
  }

  wrote_maps = do_maps;
}

//----------------------------------------------------------------------------

void DXArrayWriter::WritePerVertexNormals( 
                                const osg::GeoSet &geoset,
                                const char name[] )
  // throws 1
{
  // FIXME:
  // NOTE:  Strictly speaking this routine is not correct, but is 
  //   probably good enough for now.
  //
  // THE ISSUE:
  //   In OSG, PER_VERTEX normals, colors, and texture coordinates are
  //   stored per vertex "instance", while in DX normals, colors, and
  //   tcoords dep positions are stored per vertex "definition" (DX has no
  //   vertex instances to which normals, colors, and tcoords can be
  //   separately attached).
  //
  //   (So e.g. for normals:)  in OSG/Performer when coordinates (positions)
  //   are indexed, C coords and CI coord indices are defined to draw the
  //   primitives.  When normals are defined per-vertex, there are N
  //   normals (and optionally NI normal indices).
  //  
  //   Note that CI == N (or NI, when normals are indexed).  C != N (or NI).
  //   So, while we have one normal instance per point "instance", we may 
  //   potentially have multiple normal instances mapped to the
  //   same point "definition".
  //
  // THE CORRECT SOLUTION:
  //   The right way to approach this is to pre-scan the Geodes whose
  //   Coords are indexed and which have either normals, colors, or tcoords
  //   bound to vertices, and replicate points (positions) which have
  //   multiple "different" normals, colors, or texture coordinates mapped 
  //   To them.
  //
  // WHAT IS IMPLEMENTED BELOW:
  //   Below we take a short cut and map normals for vertex instances to
  //     vertex definitions.  If a collision occurs, we just report it
  //     so someone will know it's a problem.
  //

  int num_points     = geoset.getNumCoords();
  int num_pindices   = geoset.getNumCoordIndices();
  int num_normals    = geoset.getNumNormals();
  int num_nindices   = geoset.getNumNormalIndices();
  //const osg::Vec3 *points  = geoset.getCoords();
  const osg::Vec3 *normals = geoset.getNormals();
  const osg::GeoSet::IndexPointer &pindices = geoset.getCoordIndices();
  const osg::GeoSet::IndexPointer &nindices = geoset.getNormalIndices();

  // Create a list of normals per vertex "definition"
  osg::Vec3 *pt_normals  = new osg::Vec3[ num_points ];
  int       *set         = new int[ num_points ];

  try {
    memset( set, '\0', num_points * sizeof(int) );

    int num_pt_instances   = num_pindices ? num_pindices : num_points;
    int num_norm_instances = num_nindices ? num_nindices : num_normals;
    if ( num_pt_instances != num_norm_instances ) {
      msg_bin->Add( "ERROR:  Incorrect number of normals found\n" );
      throw 1;
    }

    for ( int i = 0; i < num_pt_instances; i++ )
    {
      int pindex = pindices.valid() ? pindices[i] : i;
      int nindex = nindices.valid() ? nindices[i] : i;
      osg::Vec3 normal = normals[nindex];

      DeNanify( normal, INVALID_NORMAL );

      if ( set[pindex] && !( pt_normals[pindex] == normal ) ) {
        msg_bin->Add("ERROR:  Vertex normal aliasing!!!  Ask somebody to expand\n"
                     "        the implementation of DXArrayWriter::"
                     "WritePerVertexNormals().\n" );
        //printf( "%d: %g %g %g\n", 
        //        nindex, normal[0], normal[1], normal[2] );
        //printf( "  %g %g %g\n", 
        //        pt_normals[pindex][0], 
        //        pt_normals[pindex][1], 
        //        pt_normals[pindex][2] );
      }
      set[pindex]        = 1;
      pt_normals[pindex] = normal;
    }

    // Now write the normals
    WriteVec3Array( pt_normals, 0, num_points, name, 0, "positions" );
  }
  catch (...) {
    delete [] set;
    delete [] pt_normals;
    throw;
  }
  delete [] set;
  delete [] pt_normals;
}

//----------------------------------------------------------------------------

void DXArrayWriter::WritePerVertexColors( 
                               const osg::GeoSet &geoset,
                               const char colors_name[], 
                               const char colormap_name[],
                               const char opacities_name[],
                               const char opacitymap_name[],
                               int do_opacities, 
                               int &wrote_maps )
  // throws 1
{
  // FIXME:
  // NOTE:  Strictly speaking this routine is not correct, but is 
  //   probably good enough for now.
  //
  // THE ISSUE:
  //   (See the opening comment in DXArrayWriter::WritePerVertexNormals.  The
  //   same applies here, namely that there may be multiple colors
  //   mapping to the same point definiton.)

  unsigned int num_points     = geoset.getNumCoords();
  unsigned int num_pindices   = geoset.getNumCoordIndices();
  unsigned int num_colors     = geoset.getNumColors();
  unsigned int num_cindices   = geoset.getNumColorIndices();
  const osg::Vec4 *colors = geoset.getColors();
  const osg::GeoSet::IndexPointer &pindices = geoset.getCoordIndices();
  const osg::GeoSet::IndexPointer &cindices = geoset.getColorIndices();

  // Create a list of color indices per vertex "definition"
  GLuint *pt_cindices = new GLuint[ num_points ];
  int       *set         = new int[ num_points ];

  try {
    memset( set, '\0', num_points * sizeof(int) );

    unsigned int num_pt_instances    = num_pindices ? num_pindices : num_points;
    unsigned int num_color_instances = num_cindices ? num_cindices : num_colors;
    if ( num_pt_instances != num_color_instances ) {
      msg_bin->Add( "ERROR:  Incorrect number of colors found\n" );
      throw 1;
    }

    for ( unsigned int i = 0; i < num_pt_instances; i++ )
    {
      unsigned int pindex = pindices.valid() ? pindices[i] : i;
      unsigned int cindex = cindices.valid() ? cindices[i] : i;

      if ( set[pindex] && ( pt_cindices[pindex] != cindex ) )
        msg_bin->Add( "ERROR:  Vertex color aliasing!!!  Ask somebody to expand\n"
                      "        the implementation of DXArrayWriter::"
                      "WritePerVertexColors()." );

      set[pindex]         = 1;
      pt_cindices[pindex] = cindex;
    }

    // Cook an IndexPointer to write colors/opacities via std methods
    osg::GeoSet::IndexPointer ip;
    ip.set( num_points, pt_cindices );

    // Write colors (and opacities, and possibly associated maps)
    WriteColors( colors, num_colors, &ip, num_points, 
                 colors_name, colormap_name, opacities_name, opacitymap_name,
                 do_opacities, "positions", wrote_maps );

  }
  catch (...) {
    delete [] set;
    delete [] pt_cindices;
    throw;
  }
  delete [] set;
  delete [] pt_cindices;
}

//----------------------------------------------------------------------------

void DXArrayWriter::WritePerVertexTCoords( 
                                const osg::GeoSet &geoset, const char name[] )
  // throws 1
{
  // FIXME:
  // NOTE:  Strictly speaking this routine is not correct, but is 
  //   probably good enough for now.
  //
  // THE ISSUE:
  //   (See the opening comment in DXArrayWriter::WritePerVertexNormals.  The
  //   same applies here, namely that there may be multiple tcoords
  //   mapping to the same point definiton.)

  int num_points    = geoset.getNumCoords();
  int num_pindices  = geoset.getNumCoordIndices();
  int num_tcoords   = geoset.getNumTextureCoords();
  int num_tindices  = geoset.getNumTextureIndices();
  //const osg::Vec3 *points  = geoset.getCoords();
  const osg::Vec2 *tcoords = geoset.getTextureCoords();
  const osg::GeoSet::IndexPointer &pindices = geoset.getCoordIndices();
  const osg::GeoSet::IndexPointer &tindices = geoset.getTextureIndices();

  // Create a list of tcoords per vertex "definition"
  osg::Vec2 *pt_tcoords  = new osg::Vec2[ num_points ];
  int       *set         = new int[ num_points ];

  try {
    memset( set, '\0', num_points * sizeof(int) );

    int num_pt_instances     = num_pindices ? num_pindices : num_points;
    int num_tcoord_instances = num_tindices ? num_tindices : num_tcoords;
    if ( num_pt_instances != num_tcoord_instances ) {
      msg_bin->Add( "ERROR:  Incorrect number of texture coordinates found\n" );
      throw 1;
    }

    for ( int i = 0; i < num_pt_instances; i++ )
    {
      int pindex = pindices.valid() ? pindices[i] : i;
      int tindex = tindices.valid() ? tindices[i] : i;

      if ( set[pindex] && !( pt_tcoords[pindex] == tcoords[tindex] ) )
        msg_bin->Add( "ERROR:  Vertex texture coordinate aliasing!!!\n"
                      "        Ask somebody to expand the implementation of\n"
                      "        DXArrayWriter::WritePerVertexTcoords().\n" );

      set[pindex]        = 1;
      pt_tcoords[pindex] = tcoords[tindex];
    }

    // Now write the tcoords
    WriteVec2Array( pt_tcoords, 0, num_points, name, 0, "positions" );

  }
  catch (...) {
    delete [] set;
    delete [] pt_tcoords;
    throw;
  }
  delete [] set;
  delete [] pt_tcoords;
}

//----------------------------------------------------------------------------

inline float Luminance( const osg::Vec4 &v )
  { return 0.2122*v[0] + 0.7013*v[1] * 0.0865*v[2]; }

inline char *Vec4AttributeString( char        buf[],
                                  char        name[],
                                  int         front_and_back,
                                  osg::Vec4  &front,
                                  osg::Vec4  &back )
{
  if ( front_and_back )
    sprintf( buf, "attribute \"osg %s\" string \"%g %g %g %g\"\n", 
             name, front[0], front[1], front[2], front[3] );
  else
    sprintf( buf, "attribute \"osg front %s\" string \"%g %g %g %g\"\n"
                  "attribute \"osg back %s\" string \"%g %g %g %g\"\n", 
             name, front[0], front[1], front[2], front[3],
             name, back[0], back[1], back[2], back[3] );
  return buf;
}

  
std::string DXWriter::BuildStateSetAttributes( MyStateSet &sset,
                                               int diffuse_is_color,
                                               int diffuse_is_opacity )
{
  std::string str;
  char        buf[160];

  // Material
  if ( sset.HasAttribute( osg::StateAttribute::MATERIAL ) ) {

    // DX Lighting Model:
    //   I = KaAC+KdLC(n*l)+KsL(n*h)^shiny
    // OpenGL:
    //   I = Ke+KaA+sum(stuff*KdA(n*l)+KsLs(n*h)^shiny
    // NOTES:
    //   1. DX specular doesn't depend on object color
    //   2. DX doesn't have separate RGB color fields for specular, diffuse,
    //      ambient, emission
    //   3. DX's material attributes are all scalars
    //   4. DX lights don't separate specular and diffuse
    //   etc.
    // Notice that the lighting models differ enough that it doesn't make
    //   much sense to set the DX material attributes (they're all scalars).
    //   So we'll just attach what the original attributes were to
    //   non-reserved attribute names and let the user fiddle with Shade()
    //   to set "shininess", "specular", "diffuse", "ambient" attributes to
    //   get the look they want.

    if ( diffuse_is_color ) {
      sprintf( buf, "attribute \"diffuse is colors component\" number 1\n" );
      str += buf;
    }

    if ( diffuse_is_opacity ) {
      sprintf( buf, "attribute \"diffuse is opacities component\" number 1\n" );
      str += buf;
    }

    if ( sset.shininess_f_and_b )
      sprintf( buf, "attribute \"osg shininess\" number %g\n", 
               sset.shininess_f );
    else
      sprintf( buf, "attribute \"osg front shininess\" number %g\n"
                    "attribute \"osg back shininess\" number %g\n", 
               sset.shininess_f, sset.shininess_b );
    str += buf;

    str += Vec4AttributeString( buf, "emission", sset.emission_f_and_b,
                                sset.emission_f, sset.emission_b );
    str += Vec4AttributeString( buf, "specular", sset.specular_f_and_b,
                                sset.specular_f, sset.specular_b );
    str += Vec4AttributeString( buf, "diffuse", sset.diffuse_f_and_b,
                                sset.diffuse_f, sset.diffuse_b );
    str += Vec4AttributeString( buf, "ambient", sset.ambient_f_and_b,
                                sset.ambient_f, sset.ambient_b );

    // NOTE:  Didn't write "colormode"
  }

  // Texture
  if ( sset.HasAttribute( osg::StateAttribute::TEXTURE ) ) {
    sprintf( buf, "attribute \"texture wrap s\" string \"%s\"\n",
             sset.wrap_s == GL_CLAMP ? "clamp" : "repeat" );
    str += buf;
    sprintf( buf, "attribute \"texture wrap t\" string \"%s\"\n",
             sset.wrap_t == GL_CLAMP ? "clamp" : "repeat" );
    str += buf;

    static struct 
      { GLenum val; char *str; }
    filter_to_str[] = {
       { GL_NEAREST               , "nearest"                 },  
       { GL_LINEAR                , "linear"                  },  
       { GL_NEAREST_MIPMAP_NEAREST, "nearest_mipmap_nearest"  },  
       { GL_NEAREST_MIPMAP_LINEAR , "nearest_mipmap_linear"   },  
       { GL_LINEAR_MIPMAP_NEAREST , "linear_mipmap_nearest"   },  
       { GL_LINEAR_MIPMAP_LINEAR  , "linear_mipmap_linear"    } };
    unsigned i;
    for ( i = 0; i < ARRAY_LEN(filter_to_str); i++ )
      if ( filter_to_str[i].val == (GLenum) sset.min_filter )
        break;
    if ( i >= ARRAY_LEN(filter_to_str) )
      msg_bin.Add( "WARNING:  Bad texture min filter: %d\n", sset.min_filter );
    else {
      sprintf( buf, "attribute \"texture min filter\" string \"%s\"\n",
               filter_to_str[i].str );
      str += buf;
    }

    for ( i = 0; i < ARRAY_LEN(filter_to_str); i++ )
      if ( filter_to_str[i].val == (GLenum) sset.mag_filter )
        break;
    if ( i >= ARRAY_LEN(filter_to_str) )
      msg_bin.Add( "WARNING:  Bad texture mag filter: %d\n", sset.mag_filter );
    else {
      sprintf( buf, "attribute \"texture mag filter\" string \"%s\"\n",
               filter_to_str[i].str );
      str += buf;
    }
  }

  // TexEnv
  if ( sset.HasAttribute( osg::StateAttribute::TEXENV ) ) {
    char *func = 0;
    switch ( sset.texture_func ) {
      case osg::TexEnv::DECAL    : func = "decal"   ; break;
      case osg::TexEnv::MODULATE : func = "modulate"; break;
      case osg::TexEnv::BLEND    : func = "blend"   ; break;
      case osg::TexEnv::REPLACE  : func = "replace" ; break;
      default:
        msg_bin.Add( "WARNING:  Bad texture function: %d\n", sset.texture_func );
    }
    if ( func ) {
      sprintf( buf, "attribute \"texture function\" string \"%s\"\n", func );
      str += buf;
    }
  }

  // CullFace
  if ( sset.HasAttribute( osg::StateAttribute::CULLFACE ) ) {
    sprintf( buf, "attribute \"cull face\" string \"%s\"\n",
             sset.cullface_mode == osg::CullFace::FRONT ? "front" :
             sset.cullface_mode == osg::CullFace::BACK  ? "back" :
             "front and back" );
    str += buf;
  }
  else {
    sprintf( buf, "attribute \"cull face\" string \"off\"\n" );
    str += buf;
  }

  return str;
}

//----------------------------------------------------------------------------

std::string DXWriter::WriteImage( const osg::Image &image )
  // Ok, so this writes a field and not just an array.  Clean this up later...
  // throws 1
{
  // If this image has already been written, don't write it again
  if ( texture_mgr.IsRegistered( &image ) )
    return texture_mgr.Lookup( &image );

  // Get the path to the image file, and build a uniquely named DX field
  const std::string &path = image.getFileName();
  const char *base = strrchr( path.c_str(), '/' );
  if ( base )
    base++;
  else
    base = path.c_str();
  std::string basename( base );
  DXField field( name_mgr, &basename );
  std::string name = field.GetName();

  // Write positions
  std::string pos_name = name_mgr.GetUnique( name + " image positions" );
  fprintf( fp, "object \"%s\" class gridpositions counts %d %d\n"
               "  origin             0             0\n"
               "  delta              0             1\n"
               "  delta              1             0\n"
               "attribute \"dep\" string \"positions\"\n"
               "#\n",
           pos_name.c_str(), image.t(), image.s() );
  field.AddComponent( "positions", pos_name.c_str() );

  // Write connections
  std::string conn_name = name_mgr.GetUnique( name + " image connections" );
  fprintf( fp, "object \"%s\" class gridconnections counts %d %d\n"
               "attribute \"element type\" string \"quads\"\n"
               "attribute \"dep\" string \"connections\"\n"
               "attribute \"ref\" string \"positions\"\n"
               "#\n",
           conn_name.c_str(), image.t(), image.s() );
  field.AddComponent( "connections", conn_name.c_str() );
  
  // Generate colors
  unsigned int pixel_fmt = image.getPixelFormat();     // A user-friendly int_fmt
  unsigned int data_type = image.getDataType();        // Sample data type
  const unsigned char *data = image.data();
  int          r_dim     = image.r();

  // FIXME:  Support other image formats
  if ( r_dim != 1 ) {
    msg_bin.Add( "ERROR:  Currently only 2D textures are supported.\n" );
    throw 1;
  }
  if ( data_type != GL_UNSIGNED_BYTE ) {
    msg_bin.Add( "ERROR:  Currently only ubyte textures are supported.\n" );
    throw 1;
  }

  switch ( pixel_fmt ) {
    case GL_RGB             :
    case GL_RGBA            :
    case GL_LUMINANCE       :
    case GL_LUMINANCE_ALPHA :
      break;
    default :
      msg_bin.Add( "ERROR:  Currently only RGB, RGBA, LUMINANCE, and \n"
               "        LUMINANCE_ALPHA textures are supported.  Found %d.\n",
               pixel_fmt );
      throw 1;
  }

  unsigned num_pixels = image.s() * image.t();
  unsigned char opaque = (unsigned char) (ALPHA_OPAQUE * 255.0 );
  Vec4UB *colors = new Vec4UB[ num_pixels ];
  unsigned i;

  if ( pixel_fmt == GL_RGB || pixel_fmt == GL_RGBA )
    for ( i = 0; i < num_pixels; i++, data += (pixel_fmt==GL_RGB?3:4))
      colors[i][0] = data[0],
        colors[i][1] = data[1],
        colors[i][2] = data[2],
        colors[i][3] = ( pixel_fmt == GL_RGB ? opaque : data[3] );
  else
    for ( i = 0; i < num_pixels; 
          i++, data += (pixel_fmt==GL_LUMINANCE?1:2) )
      colors[i][0] = colors[i][1] = colors[i][2] = data[0],
        colors[i][3] = ( pixel_fmt == GL_LUMINANCE ? opaque : data[1] );

  // Write colors
  int found_transparent = 0;
  for ( i = 0; i < num_pixels && !found_transparent; i++ )
    found_transparent = (colors[i][3] != opaque);

  int do_opacities = ((pixel_fmt == GL_RGBA || 
                       pixel_fmt == GL_LUMINANCE_ALPHA ) &&
                      found_transparent);

  // NOTE: DX used to only support UNSIGNED_BYTE GL_RGB textures.  Added
  //   Jan 2002: Added support to DX for RGBA textures w/ depth sorting.
  //   So we always write RGBA textures as image texture maps with opacities.

  std::string colors_name     = name_mgr.GetUnique( name + " image colors");
  std::string colormap_name   = name_mgr.GetUnique( name + " color map" );
  std::string opacities_name  = name_mgr.GetUnique( name + " opacities" );
  std::string opacitymap_name = name_mgr.GetUnique( name + " opacity map" );
  int wrote_maps;
  w.WriteColors( colors, num_pixels, 0, 0, 
                 colors_name.c_str(), colormap_name.c_str(), 
                 opacities_name.c_str(), opacitymap_name.c_str(),
                 do_opacities, "positions", wrote_maps );
  delete [] colors;

  field.AddComponent( "colors", colors_name.c_str() );
  if ( do_opacities )
    field.AddComponent( "opacities", opacities_name.c_str() );
  if ( wrote_maps )
  {
    field.AddComponent( "color map", colormap_name.c_str() );
    if ( do_opacities )
      field.AddComponent( "opacity map", opacitymap_name.c_str() );
  }

  // Finally, write the image field
  field.Write( fp, 0 );
  
  // And keep track of this image to attach to future referring fields
  texture_mgr.Register( &image, name );
  return name;
}

//----------------------------------------------------------------------------

void DXWriter::WritePolylineConnections( osg::GeoSet &geoset,
                                         DXField &field )
  // Writes the "edges" and "polylines" field components for LINE_STRIPs,
  //   FLAT_LINE_STRIPs, and LINE_LOOPs.
{
  assert( geoset.getPrimType() == osg::GeoSet::LINE_STRIP ||
          geoset.getPrimType() == osg::GeoSet::FLAT_LINE_STRIP ||
          geoset.getPrimType() == osg::GeoSet::LINE_LOOP );

  int num_prims     = geoset.getNumPrims();
  const int *prim_lengths = geoset.getPrimLengths();
  int line_loops = geoset.getPrimType() == osg::GeoSet::LINE_LOOP;
  int i,j;
  const osg::GeoSet::IndexPointer &pindices = geoset.getCoordIndices();

  // ----  Write "edges" components  ----
  std::string edges_name = name_mgr.GetUnique( field.GetName() + " edges" );

  // Count number of points in all polylines, and set up an index pointer
  int count = 0;
  for ( i = 0; i < num_prims; i++ )
    count += ( line_loops ? prim_lengths[i]+1 : prim_lengths[i] );
  GLuint *indices = new GLuint[ count ];
  osg::GeoSet::IndexPointer ip;
  ip.set( count, indices );

  // If no coordinate indices, then position indices are implicit; else explicit
  int idx = 0;
  for ( i = 0; i < num_prims; i++ ) {
    int start_idx = idx;
    for ( j = 0; j < prim_lengths[i]; j++, idx++ )
      indices[ idx ] = pindices.valid() ? pindices[   idx   ] : idx;
    if ( line_loops ) {
      indices[ idx ] = pindices.valid() ? pindices[start_idx] : start_idx;
      idx++;
    }
  }

  // Write component
  w.WriteIndexArray( ip, count, 0, 1, 0, edges_name.c_str(), "positions", 0 );
  field.AddComponent( "edges", edges_name.c_str() );

  // ----  Write "polylines" components  ----
  std::string polylines_name = name_mgr.GetUnique( field.GetName() + 
                                                   " polylines" );

  idx = 0;
  for ( i = 0; i < num_prims; i++ ) {
    indices[i] = idx;
    idx += ( line_loops ? prim_lengths[i]+1 : prim_lengths[i] );
  }
  w.WriteIndexArray( ip, num_prims, 0, 1, 0, polylines_name.c_str(), 
                     "edges", 0 );
  field.AddComponent( "polylines", polylines_name.c_str() );

  delete indices;
}

//----------------------------------------------------------------------------

std::string DXWriter::WriteGeoSetField( const std::string &field_name, 
                                        osg::GeoSet &geoset,
                                        MyStateSet  &sset )
  // throw 1

  // Writes POINTS, LINES, TRIANGLES, LINE_STRIPs, FLAT_LINE_STRIPs, and
  //   LINE_LOOPs.
  // 
  //   QUADS NOTE: This routine almost supports QUADS.  However, OSG/OpenGL
  //   quad vertices are ordered as a walk around the boundary of the polygon
  //   whereas DX swaps the last two vertices.  So to support quads, 
  //   we'd need to swap the 3rd and 4th coords (or coord indices) in
  //   each primitive, and for any normals/colors/tcoords, we'd also need
  //   to swap the 3rd and 4th data value (or data index) in each primitive.
  //   FIXME:  For now, we'll just triangulate QUADS into TRIANGLES, and
  //   write those (see DXWriter::WriteGeoSet).  Later consider swapping and
  //   writing quads connections if it doesn't obfuscate this routine too
  //   badly.
{
  // No QUADS -- see above comment
  assert( geoset.getPrimType() != osg::GeoSet::QUADS );

  geoset.computeNumVerts();  // Update # coords/# normals from index arrays

  int num_prims     = geoset.getNumPrims();
  int num_points    = geoset.getNumCoords();
  int num_pindices  = geoset.getNumCoordIndices();
  int num_colors    = geoset.getNumColors();
  int num_cindices  = geoset.getNumColorIndices();
  int num_normals   = geoset.getNumNormals();
  //int num_nindices  = geoset.getNumNormalIndices();
  int num_tcoords   = geoset.getNumTextureCoords();
  osg::GeoSet::BindingType cbinding = geoset.getColorBinding();
  osg::GeoSet::BindingType nbinding = geoset.getNormalBinding();
  osg::GeoSet::BindingType tbinding = geoset.getTextureBinding();
  int i, prim_length, polylines = 0;
  char *connection_type = 0;

  // Give the file pointer to the DX array writer
  w.SetFP( fp );
  w.SetMsgBin( &msg_bin );
  msg_bin.Clear();

  // If texturing is on, write the texture as a DX image field, and give
  //   us back the name of field
  int has_uv = num_tcoords > 0 && tbinding == osg::GeoSet::BIND_PERVERTEX;
  int has_texture = sset.HasAttribute( osg::StateAttribute::TEXTURE );
  int has_texgen  = sset.HasAttribute( osg::StateAttribute::TEXGEN );

  std::string texture_name;
  if ( has_texture && has_uv )
    try {
      texture_name = WriteImage( *sset.image );
    } catch (...) { throw; }
  else if ( has_texture && !has_uv && has_texgen ) {
    static int been_here = 0;
    if ( !been_here ) {
      msg_bin.Add( "WARNING:  "
                   "Texture/uv not written; TEXGEN not supported yet\n" );
      been_here = 1;
    }
  }

  // Create a new DX field with a unique name
  DXField field( name_mgr, &field_name );
  std::string name = field.GetName();

  // Determine "element type" and primitive length of connections component
  switch ( geoset.getPrimType() ) {
    case osg::GeoSet::POINTS    : 
      prim_length = 1, connection_type = 0          ;  break;
    case osg::GeoSet::LINES     : 
      prim_length = 2, connection_type = "lines"    ;  break;
    case osg::GeoSet::TRIANGLES : 
      prim_length = 3, connection_type = "triangles";  break;
    case osg::GeoSet::QUADS     : 
      prim_length = 4, connection_type = "quads"    ;  break;
    case osg::GeoSet::LINE_STRIP :
    case osg::GeoSet::FLAT_LINE_STRIP :
    case osg::GeoSet::LINE_LOOP :
      prim_length = 0, connection_type = "polylines";  
      polylines = 1;
      break;
      
    default:
      msg_bin.Add( "ERROR:  GeoSet passed to WriteGeoSetField is wrong type\n" );
      throw 1;
  }

  // Ensure we "don't" have interleaved GeoSet data
  //   FIXME:  Add support for this
  if (( num_points == 0 ) && geoset.getInterleavedArray() != 0 ) {
    msg_bin.Add( "ERROR:  Interleaved Array GeoSets not yet supported\n" );
    throw 1;
  }

  // Write "positions" component
  std::string pos_name = name_mgr.GetUnique( name + " points" );
  w.WriteVec3Array( geoset.getCoords(), 0, num_points, pos_name.c_str(),
                    0, "positions" );
  field.AddComponent( "positions", pos_name.c_str() );

  // Write "connections" component (lines, triangles, or quads); pts have none
  char *conn_dep_name = 0;

  if ( prim_length == 1 )
    // For points, there are no connections, so don't make colors or normals
    //   dep on them in the OVERALL or PERPRIM cases
    conn_dep_name = "positions";
  else if ( polylines ) {
    WritePolylineConnections( geoset, field );
    conn_dep_name = "polylines";
  }
  else {
    std::string conn_name = name_mgr.GetUnique( name + " connections" );
    int         len       = num_pindices ? num_pindices : num_points;
    char        attr[80];
    sprintf( attr, "attribute \"element type\" string \"%s\"", connection_type);

    w.WriteIndexArray( geoset.getCoordIndices(), len, 1, prim_length, 0,
                       conn_name.c_str(), "positions", "connections", attr );
    field.AddComponent( "connections", conn_name.c_str() );
    conn_dep_name = "connections";
  }

  // Write "normals" component
  if ( num_normals > 0 ) {
    std::string normals_name = name_mgr.GetUnique( name + " normals" );

    // DX doesn't support a "normal map", so expand to Vec3 if normals indexed
    //   Also see notes in WritePerVertexNormals.
    int index;
    osg::GeoSet::IndexPointer *ip;
    switch( nbinding ) {

      case osg::GeoSet::BIND_OVERALL   : 
        ip = &geoset.getNormalIndices();
        index = ip->valid() ? (*ip)[0] : 0;
        w.WriteVec3ConstArray( geoset.getNormals()[index], num_prims, 
                               normals_name.c_str(), 0, conn_dep_name );
        break;

      case osg::GeoSet::BIND_PERPRIM   :
        w.WriteVec3Array( geoset.getNormals(), &geoset.getNormalIndices(),
                          num_prims, normals_name.c_str(), 
                          0, conn_dep_name );
        break;

      case osg::GeoSet::BIND_PERVERTEX :
        try {
          w.WritePerVertexNormals( geoset, normals_name.c_str() );
        } catch (...) { throw; }
        break;

      case osg::GeoSet::BIND_OFF:
      case osg::GeoSet::BIND_DEFAULT:
        // Nonsensical cases
        break;
    }
    field.AddComponent( "normals", normals_name.c_str() );
  }
  
  // Write "colors" (and possibly "opacities")
  //   If indexed colors and numcolors <= 256, create "color map"/"opacity map"
  std::string colors_name     = name_mgr.GetUnique( name + " colors" );
  std::string colormap_name   = name_mgr.GetUnique( name + " color map" );
  std::string opacities_name  = name_mgr.GetUnique( name + " opacities" );
  std::string opacitymap_name = name_mgr.GetUnique( name + " opacity map" );
  int has_maps = 0;
  int diffuse_is_color = 0, diffuse_is_opacity = 0;

  // 4 cases:  Choose/write colors/opacities in order of preference from:
  //           colors, diffuse material, default color. 
  //           If none specified, write no colors/opacities components.

  if ( num_colors > 0 ) {

    osg::Vec4                 *colors = geoset.getColors();
    osg::GeoSet::IndexPointer &ip = geoset.getColorIndices();
    int                        index, do_opacities, wrote_maps;

    // See if any non-opaque colors used
    for ( i = 0; i < num_colors; i++ )
      if ( colors[i][3] != ALPHA_OPAQUE )
        break;
    do_opacities = ( i < num_colors );

    osg::Vec3 color;
    float opacity;

    switch( cbinding ) {

      case osg::GeoSet::BIND_OVERALL   : 
        index = ip.valid() ? ip[0] : 0;
        w.OSGColorToDX( colors[index], color, opacity );
        w.WriteVec3ConstArray( color, num_prims, colors_name.c_str(), 
                               0, conn_dep_name );
        if ( do_opacities )
          w.WriteFloatConstArray( opacity, num_prims, opacities_name.c_str(),
                                  0, conn_dep_name );
        wrote_maps = 0;
        break;

      case osg::GeoSet::BIND_PERPRIM   :
        w.WriteColors( colors, num_colors, 
                       &geoset.getColorIndices(), num_cindices,
                       colors_name.c_str(), colormap_name.c_str(), 
                       opacities_name.c_str(), opacitymap_name.c_str(),
                       do_opacities, conn_dep_name, wrote_maps );
        break;

      case osg::GeoSet::BIND_PERVERTEX :
        try {
          w.WritePerVertexColors( geoset, 
                                colors_name.c_str()   , colormap_name.c_str(),
                                opacities_name.c_str(), opacitymap_name.c_str(),
                                do_opacities, wrote_maps );
        } catch (...) { throw; }
        break;
      case osg::GeoSet::BIND_OFF:
      case osg::GeoSet::BIND_DEFAULT:
        // Nonsensical cases
        break;
    }

    // Register added components
    field.AddComponent( "colors", colors_name.c_str() );
    if ( do_opacities )
      field.AddComponent( "opacities", opacities_name.c_str() );
    if ( wrote_maps )
    {
      field.AddComponent( "color map", colormap_name.c_str() );
      if ( do_opacities )
        field.AddComponent( "opacity map", opacitymap_name.c_str() );
    }
  }

  // Else no explicit colors specific.  Fall back on diffuse Material
  else if ( sset.HasAttribute( osg::StateAttribute::MATERIAL ) ) {
    osg::Vec3 color;
    float opacity;
    w.OSGColorToDX( sset.diffuse_f, color, opacity );
    int do_opacities = opacity != ALPHA_OPAQUE;

    // Same as the BIND_OVERALL case above, except can have front/back colors
    if ( do_opacities ) {
      w.WriteFloatConstArray( opacity, num_prims, opacities_name.c_str(),
                              0, conn_dep_name );
      field.AddComponent( "opacities", opacities_name.c_str() );
    }

    std::string front_colors_name = name_mgr.GetUnique( name +" front colors" );
    std::string back_colors_name  = name_mgr.GetUnique( name +" back colors" );

    if ( sset.diffuse_f_and_b ) {
      w.WriteVec3ConstArray( color, num_prims, colors_name.c_str(), 
                             0, conn_dep_name );
      field.AddComponent( "colors", colors_name.c_str() );
    }
    else {
      w.WriteVec3ConstArray( color, num_prims, front_colors_name.c_str(), 
                             0, conn_dep_name );
      field.AddComponent( "colors", front_colors_name.c_str() );
      w.OSGColorToDX( sset.diffuse_b, color, opacity );
      w.WriteVec3ConstArray( color, num_prims, back_colors_name.c_str(), 
                             0, conn_dep_name );
      field.AddComponent( "colors", back_colors_name.c_str() );
    }
    diffuse_is_color   = 1;
    diffuse_is_opacity = do_opacities;
  }

  // No explicit colors or diffuse Material color.  Give user provided default.
  else if ( parms.set_default_color ) {
    osg::Vec3 color;
    float opacity;
    w.OSGColorToDX( parms.default_color, color, opacity );
    int do_opacities = opacity != ALPHA_OPAQUE;

    // Same as the BIND_OVERALL case above, except can have front/back colors
    w.WriteVec3ConstArray( color, num_prims, colors_name.c_str(), 
                           0, conn_dep_name );
    field.AddComponent( "colors", colors_name.c_str() );

    if ( do_opacities ) {
      w.WriteFloatConstArray( opacity, num_prims, opacities_name.c_str(),
                              0, conn_dep_name );
      field.AddComponent( "opacities", opacities_name.c_str() );
    }

    diffuse_is_color   = 1;
    diffuse_is_opacity = do_opacities;
  }

  // Write "uv" texture coordinates
  std::string uv_name = name_mgr.GetUnique( name + " uv" );
  if ( num_tcoords > 0 && tbinding == osg::GeoSet::BIND_PERVERTEX ) {
    try {
      w.WritePerVertexTCoords( geoset, uv_name.c_str() );
    } catch (...) { throw; }
    field.AddComponent( "uv", uv_name.c_str() );
  }

  // Generate field attributes
  std::string attr( has_maps ? "attribute \"direct color map\" number 1\n" :"");
  attr += BuildStateSetAttributes( sset, diffuse_is_color, diffuse_is_opacity );
  // If texturing is on, tack on a "texture" attribute with it's field name
  if ( has_uv && has_texture ) {
    char        buf[160];
    //int         size_ok = sset.image->s() >= 64 && sset.image->t() >= 64;
    //char       *attr_name = size_ok ? "texture" : "texture - too small";
    char       *attr_name = "texture";

    // FIXME:  DX textures must be powers of two and >= 64 on a side.
    //   Now that we generate MIPmaps that's no longer true, and just remove
    //   the 64x64 min size check from DX.
    // NOTE: FIXED in DX now that we mipmap everything

    //if ( !size_ok ) {
    //  // FIXME:  When we generalize this, remove the static hack
    //  static int been_here = 0;
    //  if ( !been_here ) {
    //    msg_bin.Add( 
    //             "WARNING:  DX min texture size is 64x64.  Renaming texture\n"
    //             "          attribute to \"texture - too small\" for these" 
    //             " small textures.\n" );
    //    been_here = 1;
    //  }
    //}

    sprintf( buf, "attribute \"%s\" value \"%s\"\n", 
             attr_name, texture_name.c_str() );
    attr += buf;
  }

  if ( attr[ attr.length()-1 ] == '\n' )
    attr.assign( attr, 0, attr.length()-1 );

  // Finally, write the field
  field.Write( fp, attr.c_str() );
  return name;

  // FIXME:  Need to handle "texture" image attribute, GeoState colors
  //   storing GeoState attributes on field object, etc.
}

//----------------------------------------------------------------------------

void MyStateSet::Show( osg::StateSet &sset )
{
  osg::StateSet::ModeList      &mode_list = sset.getModeList();
  osg::StateSet::AttributeList &attr_list = sset.getAttributeList();
  //int            rend_hint                   = sset.getRenderingHint();
  //bool           use_rend_bin_details        = sset.useRenderBinDetails();
  //int            bin_num                     = sset.getBinNumber();
  //const std::string            &bin_name     = sset.getBinName();
  //osg::StateSet::RenderBinMode  rendbin_mode = sset.getRenderBinMode();

  //  Iterate over all mode mappings (GLMode->GLModeValue)
  for( osg::StateSet::ModeList::const_iterator mitr = mode_list.begin();
       mitr != mode_list.end();
       ++mitr )
    msg_bin.Add( "  GLMode %d = GLValue %d\n", mitr->first, mitr->second );

  //  Iterate over all attribute mappings 
  //    (Type -> pair< ref_ptr<StateAttribute>, OverrideValue >)
  for( osg::StateSet::AttributeList::const_iterator aitr = attr_list.begin();
       aitr != attr_list.end();
       ++aitr )
    msg_bin.Add( "  Attr Type %d (Attr Name \"%s\"), OverrideValue = %d\n", 
                 aitr->first, aitr->second.first.get()->className(),
                 aitr->second.second );

  // mgs_bin.Add( "  Rendering Hint       = %d\n", rend_hint );
  // mgs_bin.Add( "  Use Rend Bin Details = %d\n", use_rend_bin_details );
  // mgs_bin.Add( "  Bin Number           = %d\n", bin_num );
  // mgs_bin.Add( "  Bin Name             = %s\n", bin_name.c_str() );
  // mgs_bin.Add( "  Bin Mode             = %d\n", rendbin_mode );
}

//----------------------------------------------------------------------------

void MyStateSet::Query( const osg::StateSet &sset )
{
  const osg::StateAttribute *attr;

  //MyStateSet::Show( *drawable_state );

  // FIXME:  Ignore override flags for now

  // MATERIAL
  attr = sset.getAttribute( osg::StateAttribute::MATERIAL );
  if ( attr ) {
    AddAttribute( osg::StateAttribute::MATERIAL );

    const osg::Material &mat = (const osg::Material &) (*attr);
    colormode  = mat.getColorMode();         // GL_COLOR_MATERIAL

    ambient_f   = mat.getAmbient  ( osg::Material::FRONT );
    ambient_b   = mat.getAmbient  ( osg::Material::BACK  );
    diffuse_f   = mat.getDiffuse  ( osg::Material::FRONT );
    diffuse_b   = mat.getDiffuse  ( osg::Material::BACK  );
    specular_f  = mat.getSpecular ( osg::Material::FRONT );
    specular_b  = mat.getSpecular ( osg::Material::BACK  );
    emission_f  = mat.getEmission ( osg::Material::FRONT );
    emission_b  = mat.getEmission ( osg::Material::BACK  );
    shininess_f = mat.getShininess( osg::Material::FRONT );
    shininess_b = mat.getShininess( osg::Material::BACK  );

    ambient_f_and_b   = mat.getAmbientFrontAndBack  ();
    diffuse_f_and_b   = mat.getDiffuseFrontAndBack  ();
    specular_f_and_b  = mat.getSpecularFrontAndBack ();
    emission_f_and_b  = mat.getEmissionFrontAndBack ();
    shininess_f_and_b = mat.getShininessFrontAndBack();
  }

  // TEXTURE / TEXTURE_0 
  // ( and only for 2D textures right now... RO August 2002)
  attr = dynamic_cast<const osg::Texture2D*>(sset.getTextureAttribute(0, osg::StateAttribute::TEXTURE ));
  if ( attr &&
       ( sset.getTextureMode(0, GL_TEXTURE_2D ) & osg::StateAttribute::ON )) {

    const osg::Texture2D &texture = (const osg::Texture2D &) (*attr);

    // NOTE:  If OSG failed to load the texture, we'll get a NULL right here
    image = texture.getImage();
    if ( image ) 
      AddAttribute( osg::StateAttribute::TEXTURE );
    
    // NOTE:  DX limitations
    //   Traditional DX doesn't support any texture control besides specifying
    //     the texture ("texture" attribute) and the texture coordinates ("uv"
    //     component).  DX was hard-coded to use these 2D texture settings:
    //          GL_TEXTURE_WRAP_S     = GL_CLAMP
    //          GL_TEXTURE_WRAP_T     = GL_CLAMP
    //          GL_TEXTURE_MIN_FILTER = GL_NEAREST
    //          GL_TEXTURE_MAG_FILTER = GL_NEAREST
    //          GL_TEXTURE_ENV_MODE   = GL_MODULATE
    //     DX also registers a level 0 texture
    //   A DX patch I recently committed (1/02) creates mipmapped textures and
    //     adds the following DX textured field attributes:
    //       attribute "texture wrap s" string ["clamp"|"repeat"]
    //       attribute "texture wrap t" string ["clamp"|"repeat"]
    //       attribute "texture min filter" string ["nearest"|"linear"|
    //                     "nearest_mipmap_nearest"|"nearest_mipmap_linear"|
    //                     "linear_mipmap_nearest"|"linear_mipmap_linear"]
    //       attribute "texture mag filter" string ["nearest"|"linear"]
    //       attribute "texture function" string ["decal"|"replace"|"modulate"
    //                                            "blend"]

    wrap_s     = texture.getWrap( osg::Texture2D::WRAP_S );
    wrap_t     = texture.getWrap( osg::Texture2D::WRAP_T );
    min_filter = texture.getFilter( osg::Texture2D::MIN_FILTER );
    mag_filter = texture.getFilter( osg::Texture2D::MAG_FILTER );

    if ( texture.getInternalFormatMode() != 
         osg::Texture2D::USE_IMAGE_DATA_FORMAT ) {
      // FIXME:  When we generalize this, remove the static hack
      static int been_here = 0;
      if ( !been_here ) {
        msg_bin.Add( "WARNING:  Only texture image data format supported.\n");
        been_here = 1;
      }
    }

  }

  // TEXENV
  attr = sset.getTextureAttribute(0, osg::StateAttribute::TEXENV );
  if ( attr &&
       ( sset.getTextureMode(0, GL_TEXTURE_2D ) & osg::StateAttribute::ON )) {
    AddAttribute( osg::StateAttribute::TEXENV );

    const osg::TexEnv &texenv = (const osg::TexEnv &) (*attr);

    texture_func = texenv.getMode();
  }

  // TEXGEN
  attr = sset.getTextureAttribute(0, osg::StateAttribute::TEXGEN );
  if ( attr &&
       ( sset.getTextureMode(0, GL_TEXTURE_2D ) & osg::StateAttribute::ON )) {

    // FIXME: Just note that it is there for now.  We don't actually support
    //   texture coordinate generation yet (i.e. generating the "uv" component
    //   for DX without explicit texture coordinates in the model)
    AddAttribute( osg::StateAttribute::TEXGEN );
  }

  // CULLFACE
  attr = sset.getAttribute( osg::StateAttribute::CULLFACE );
  if ( attr &&
       ( sset.getMode( GL_CULL_FACE ) & osg::StateAttribute::ON )) {

    AddAttribute( osg::StateAttribute::CULLFACE );
    const osg::CullFace &cullface = (const osg::CullFace &) (*attr);
    cullface_mode = cullface.getMode();
  }

  // FIXME:  Also consider supporting:
  //exenv
  //   ALPHAFUNC, ANTIALIAS, COLORTABLE, FOG, FRONTFACE,
  //   LIGHTING, POINT, POLYGONMODE, POLYGONOFFSET, TEXGEN, TEXMAT,
  //   TEXTURE_1, TEXTURE_2, TEXTURE_3, TRANSPARENCY,
  //   STENCIL, COLORMASK, CLIPPLANE, CLIPPLANE_0, CLIPPLANE_1,
  //   CLIPPLANE_2, CLIPPLANE_3, CLIPPLANE_4, CLIPPLANE_5, DEPTH
}

//----------------------------------------------------------------------------

void DXWriter::Open()
{
  if ( strcmp( parms.outfile, "-" ) == 0 )
    fp = stdout;
  else
    fp = fopen( parms.outfile, "wt" );

  w.SetFP( fp );
  w.SetMsgBin( &msg_bin );
}

//----------------------------------------------------------------------------

void DXWriter::Close()
{
  if ( fp != stdout )
    fclose( fp );
  w.SetFP( NULL );
}

//----------------------------------------------------------------------------

std::string DXWriter::WriteGeoSet( osg::GeoSet         &geoset, 
                                   const osg::StateSet &active_sset,
                                   std::string         &field_name )
  // throws 1
{
  std::string dx_name;
  MyStateSet  my_sset(msg_bin);

  // Update the active state for this Drawable (GeoSet), if it has a StateSet
  const osg::StateSet *sset;
  StateSetCopy        *new_sset       = 0;
  osg::StateSet       *drawable_state = geoset.getStateSet();

  if ( drawable_state ) {
    new_sset = new StateSetCopy( active_sset );
    new_sset->merge( *drawable_state );
    sset = new_sset;

    // Collect unhandled modes/attributes in this geoset's stateset
    CollectUnhandledModesAndAttrs( drawable_state );
  }
  else
    sset = &active_sset;

  my_sset.Query( *sset );
  delete new_sset;

  // Now write the GeoSet to a DX field
  osg::GeoSet::PrimitiveType prim_type = geoset.getPrimType();
  osg::GeoSet *tri_geoset;

  switch( prim_type ) {
    case osg::GeoSet::POINTS :
    case osg::GeoSet::LINES :
    case osg::GeoSet::TRIANGLES :
    case osg::GeoSet::LINE_STRIP :
    case osg::GeoSet::FLAT_LINE_STRIP :
    case osg::GeoSet::LINE_LOOP :

      try {
        dx_name = WriteGeoSetField( field_name, geoset, my_sset );
      } catch (...) { throw; }
      break;

    case osg::GeoSet::QUADS :
    case osg::GeoSet::TRIANGLE_STRIP :
    case osg::GeoSet::FLAT_TRIANGLE_STRIP :
    case osg::GeoSet::TRIANGLE_FAN :
    case osg::GeoSet::FLAT_TRIANGLE_FAN :
    case osg::GeoSet::QUAD_STRIP :
    case osg::GeoSet::POLYGON :

      // For all of these cases, bust up the area primitive GeoSet into
      //   a TRIANGLES GeoSet, and then simply write that.  DX doesn't
      //   support any of these primitive types except POLYGON and QUADS
      //   anyway, and its polygon support is a bit quirky (e.g. 
      //   $DX_NESTED_LOOPS).
      tri_geoset = TriangulateAreaGeoSet( geoset );
      try {
        dx_name = WriteGeoSetField( field_name, *tri_geoset, my_sset );
      } catch (...) { 
        tri_geoset->unref();
        throw; 
      }
      tri_geoset->unref();
      break;

    case osg::GeoSet::NO_TYPE :
      // Nonsensical cases
      break;
  }

  // Return the name of the written field, or "" if not written
  return dx_name;
}

//----------------------------------------------------------------------------

std::string DXWriter::WriteGeode( osg::Geode          &geode, 
                                  const osg::StateSet &active_sset )
  // throws 1
{
  const std::string &name = geode.getName();
  std::string  field_name;
  DXGroup     *group = 0;
  std::string  dx_name;

  // If there are multiple GeoSets in this Geode, then we will encase them
  //   in a DX group
  if ( geode.getNumDrawables() > 1 )
    group = new DXGroup( name_mgr, &name );

  // For all GeoSets in this Geode...
  for (unsigned int i = 0; i < geode.getNumDrawables(); i++ ) {
    osg::GeoSet *geoset = dynamic_cast<osg::GeoSet*>( geode.getDrawable(i) );

    // If we have multiple GeoSets per Geode, we need to generate different
    //   DX field names for each
    if ( geode.getNumDrawables() > 1 ) {
      char prim_num_str[20];
      sprintf( prim_num_str, " %d", i+1 );
      field_name = name + prim_num_str;
    }
    else
      field_name = name;

    try {
      dx_name = WriteGeoSet( *geoset, active_sset, field_name );
    } catch (...) { throw; }

    if ( !dx_name.empty() && group )
      group->AddMember( dx_name.c_str() );
  }

  // If collecting fields in a Group, write the Group
  if ( group ) {
    // If no members were written successfully, don't write this group
    //   or include it in any parent groups
    if ( group->GetNumMembers() == 0 ) 
      dx_name = "";          
    else {
      group->Write( fp );
      dx_name = group->GetName();
    }
    delete group;
  }

  return dx_name;
}

//----------------------------------------------------------------------------

void DXWriter::CollectUnhandledModesAndAttrs( osg::StateSet *sset )
{
  if ( !sset )
    return;

  // FIXME:  For now we're collecting all of them, not just the 
  //   unhandled ones
  osg::StateSet::ModeList      &modelist = sset->getModeList();
  osg::StateSet::AttributeList &attrlist = sset->getAttributeList();

  // Loop through the mode settings
  for( osg::StateSet::ModeList::const_iterator mitr = modelist.begin(); 
       mitr != modelist.end(); 
       ++mitr )
    unsupported_modes[ mitr->first ] = 1;

  // Loop through the attribute settings
  for( osg::StateSet::AttributeList::const_iterator aitr = attrlist.begin(); 
       aitr != attrlist.end(); 
       ++aitr )
    unsupported_attrs[ aitr->first ] = 1;
}

//----------------------------------------------------------------------------

void DXWriter::ReportUnhandledModesAndAttrs()
{
  char msg[1024];

  msg_bin.Add( "\n" );

  // Loop through the mode settings...
  msg_bin.Add( "OpenGL Modes Encounted:\n " );
  msg[0] = '\0';
  for( DXWriter::ModeList::const_iterator mitr = unsupported_modes.begin(); 
       mitr != unsupported_modes.end(); 
       ++mitr ) {
    const char *mode_str = GLModeToModeStr( mitr->first );
    if ( mode_str )
      snprintf( msg+strlen(msg), sizeof(msg)-strlen(msg), 
                " %s", mode_str );
    else
      snprintf( msg+strlen(msg), sizeof(msg)-strlen(msg), 
                " %d", mitr->first );
  }
  msg_bin.Add( msg );
  msg_bin.Add( "" );

  // ...and the attribute settings
  msg_bin.Add( "OpenSceneGraph Attributes Encountered:\n  " );
  msg[0] = '\0';
  for( DXWriter::AttrList::const_iterator aitr = unsupported_attrs.begin(); 
       aitr != unsupported_attrs.end(); 
       ++aitr ) {
    const char *attr_str = OSGAttrToAttrStr( aitr->first );
    if ( attr_str )
      snprintf( msg+strlen(msg), sizeof(msg)-strlen(msg), 
                " %s", attr_str );
    else
      snprintf( msg+strlen(msg), sizeof(msg)-strlen(msg), 
                " %d", aitr->first );
  }
  msg_bin.Add( msg );
  msg_bin.Add( "" );
}

//----------------------------------------------------------------------------

// NOTES ON TRAVERSAL:
//    
//     So what's up with the StateSetVisitor and StateSetActionVisitor
//     pair?  Well it's all to make sure that when the user's apply()
//     method is called for a Node, we can just hand over the current state
//     (attributes and modes) without the user having to go figure it out.
//     
//     So why are two passes needed?  One to update the state.  One to
//     execute the user's apply method with that state.  Basically,
//     StateSetVisitor is the master traversal object, and we gosub into
//     the StateSetActionVisitor just long enough to invoke the apply
//     method stack for a Node, and then we gosub back out to continue
//     traversal on the main StateSetVisitor.  StateSetVisitor's handles
//     keeping the active StateSet up-to-date.
//     
//     Here's how it works: traversal is invoked on a StateSetVisitor (a
//     NodeVisitor).  It's constructor accepts a reference to a
//     StateSetActionVisitor, which has the user's apply methods.
//     
//     StateSetVisitor: In it's constructor, we give the
//     StateSetActionVisitor we've been handed a handle to ourself.  Then
//     during traversal when we hit a Node, we push a new StateSet onto our
//     StateSet stack which is a copy of the top of the stack merged with
//     this Node's StateSet.  This new StateSet is then given to the
//     StateSetActionVisitor instance, and a new traversal is invoked on
//     this Node with the StateSetActionVisitor instance as the visitor.
//     When it returns, we pop the stack.
//     
//     StateSetActionVisitor: When StateSetVisitor initiates a traversal on
//     a Node with us, we simply call the apply() method stack for the
//     current Node.  At the bottom in apply(Node&), we traverse back to
//     the StateSetVisitor traversal using node.traverse, which continues
//     traversal on the node's children.

// need to add this to get round incompatibilities between the MipsPro7.3
// and VisualStudio compiles handling of NodeVisitor:: method calls, VS
// seems broke so can't handle osg::NodeVisitor intpreting it as a 
// static method call?!? Anyway no osg:: but using namespace osg seems to
// keep both happy. Robert, Feb 2002.
using namespace osg;

class StateSetVisitor;

class StateSetActionVisitor : public osg::NodeVisitor
  // Second pass for StateSetVisitor.  Users should override apply()
  //   methods on this object, pass an instance of this object to 
  //   the StateSetVisitor constructor, and initiate traversal on the 
  //   resulting StateSetVisitor instance.
  //
  // NOTE: We need two passes per Node to update the state correctly so that
  //   users can still override accept() and get the current state, without
  //   the user having to call methods to keep it current.
  //   The first pass updates the state for the node; the second actually
  //   calls the user's accept methods.  In those methods, they can
  //   call GetActiveStateSet() to get the current state inclusive of that
  //   node.
  // The trick that allows this all to work is that this object's
  //   apply(Node&) method (at the bottom of the apply stack) transfers
  //   traversal back over to the StateSetVisitor starting with the children
  //   of the current node (_traversalMode is also set to TRAVERSE_NONE, 
  //   not that that ever comes into play).  So basically we gosub
  //   into StateSetActionVisitor just long enough to invoke the apply
  //   methods and then gosub right back out to the main traverser, 
  //   alternating back and forth as we move down the scene graph.
{
  public:
    friend class StateSetVisitor;
    
    StateSetActionVisitor() 
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_NONE ) {}
    
    // Give first pass traversal control back to StateSetVisitor
    void apply(osg::Node& node);

    void apply(osg::Geode& node)      { NodeVisitor::apply(node); }
    void apply(osg::Billboard& node)  { NodeVisitor::apply(node); }
    void apply(osg::LightSource& node){ NodeVisitor::apply(node); }
    void apply(osg::Group& node)      { NodeVisitor::apply(node); }
    void apply(osg::Transform& node)  { NodeVisitor::apply(node); }
    void apply(osg::Switch& node)     { NodeVisitor::apply(node); }
    void apply(osg::LOD& node)        { NodeVisitor::apply(node); }
    void apply(osg::Impostor& node)   { NodeVisitor::apply(node); }
    void apply(osg::ClearNode& node)   { NodeVisitor::apply(node); }


    const osg::StateSet &GetActiveStateSet()
      { return *_current_stateset; }

  protected:
    void SetStateSetVisitor( StateSetVisitor *nv )
      { _ssvisitor = nv; }
    void SetActiveStateSet( osg::StateSet *sset )
      { _current_stateset = sset; }

    StateSetVisitor               *_ssvisitor;
    osg::StateSet                 *_current_stateset;
};

//----------------------------------------------------------------------------

class StateSetVisitor : public osg::NodeVisitor 
  // First pass for StateSetVisitor.  Users should override apply()
  //   methods on the StateSetActionVisitor object, pass an instance of it 
  //   to this object's constructor, and initiate traversal on the 
  //   resulting StateSetVisitor instance.
  //
  // See StateSetActionVisitor for more details.  
{
  public:

    StateSetVisitor( StateSetActionVisitor *av ) 
      // ACTIVE_CHILDREN - Only traverse the most-detailed LOD and
      //   active children in a Switch
      : osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN ),
        _actionTraversalVisitor(av) 
    {
      _actionTraversalVisitor->SetStateSetVisitor( this );
    }

    void apply(osg::Node& node)
    { 
      // In this first apply() pass, update state with node's StateSet.
      osg::StateSet *node_state = node.getStateSet();
      osg::ref_ptr<StateSetCopy> sset;

      if ( node_state ) {
        sset = new StateSetCopy( *_state_stack.back() );
        sset->merge( *node_state );
      }
      else
        if ( !_state_stack.empty() )
          sset = _state_stack.back();
        else
          sset = new StateSetCopy();

      // Now push state, traverse using action visitor, and then pop state
      _state_stack.push_back( sset );
      //traverse(node);
      _actionTraversalVisitor->SetActiveStateSet( sset.get() );
      node.accept(*_actionTraversalVisitor);
      _state_stack.pop_back();
    }

  protected:
    StateSetActionVisitor *_actionTraversalVisitor;
    std::vector< osg::ref_ptr<StateSetCopy> > _state_stack;
};

//----------------------------------------------------------------------------

// Give first pass traversal control back to StateSetVisitor
void StateSetActionVisitor::apply(osg::Node& node)  
  { assert( _ssvisitor );
  // Flip back to StateSetVisitor and traverse node's kids
  node.traverse( *_ssvisitor ); }

//----------------------------------------------------------------------------

class DXWriteVisitor : public StateSetActionVisitor
{
  protected:
    DXWriter   &dx;
    std::vector< DXGroup * > group_stack;
    MessageBin &msg_bin;

    enum NodeTypes { LOD, BILLBOARD, LIGHTSOURCE, TRANSFORM, SWITCH, 
                     IMPOSTER, CLEARNODE };

    typedef std::map< NodeTypes, int > StringMap;
    StringMap problem_nodes;

  public:

    DXWriteVisitor( DXWriter &dx ) : dx(dx), msg_bin(dx.msg_bin) {}

    void apply(osg::Node& node)
    {
      dx.CollectUnhandledModesAndAttrs( node.getStateSet() );
      StateSetActionVisitor::apply( node );  // Flip back to StateSetVisitor
    }

    void apply(osg::Geode& node)      
    { 
      std::string name;
      try {
        name = dx.WriteGeode( node, GetActiveStateSet() );
      } catch (...) { throw; }

      // Accumulate fields (Geodes) in groups
      if ( !name.empty() && group_stack.size() > 0 )
        group_stack[ group_stack.size()-1 ]->AddMember( name.c_str() );
      apply((osg::Node&)node);
    }

    void apply(osg::Group& node)      
    { 
      DXGroup *group = 0;

      // Only start a new DX group if there are 2 or more kids
#ifdef SKIP_SINGLE_MEMBER_DXGROUPS
      if ( node.getNumChildren() >= 2 ) {
#else
      if ( 1 ) {
#endif
        // New group
        group = new DXGroup( dx.name_mgr, &node.getName() );
        group_stack.push_back( group );

        // Add to any parent group
        if ( group_stack.size() >= 2 )
          group_stack[ group_stack.size()-2 ]->AddMember(
                                                  group->GetName().c_str() );
      }

      // Process children
      //   (Children will be added to this DX group, or a parent DX group
      //   if we didn't start a new DX group above, or no DX group if
      //   none active right now).
      apply((osg::Node&)node); 
      
      // If we started a new DX group above
      if ( group ) {
        // If wrote some children, write group; else remove from parent group
        if ( group_stack.back()->GetNumMembers() > 0 )
          group_stack.back()->Write( dx.fp );
        else if ( group_stack.size() >= 2 )
          group_stack[ group_stack.size()-2 ]->RemoveMember(
                                                  group->GetName().c_str() );

        // Delete group
        group_stack.pop_back();
        delete group;
      }
    }

    void apply(osg::LOD& node)
         { problem_nodes[ LOD ]++;         apply((osg::Node&)node); }
    void apply(osg::Switch& node)     
         { problem_nodes[ SWITCH ]++;      apply((osg::Node&)node); }

    // FIXME
    void apply(osg::Billboard& node)
    { 
      // FIXME:  Right now we're just treating this like a transformed Geode
      //   ignoring the other Billboard properties
      problem_nodes[ BILLBOARD ]++;

      DXGroup *group = 0;

      // New dummy group (to snap up our child Geode's DX object name)
      std::string fallback_name( "Billboard" );
      group = new DXGroup( dx.name_mgr, &node.getName(), &fallback_name );
      group_stack.push_back( group );

      // Add to any parent group
      if ( group_stack.size() >= 2 )
        group_stack[ group_stack.size()-2 ]->AddMember(
                                                  group->GetName().c_str() );

      // Process child
      apply((osg::Geode&)node);
      
      // FIXME:  Currently only support one GeoSet per Billboard
      if ( group->GetNumMembers() > 1 )
        msg_bin.Add(
                "WARNING:  Currently only 1 GeoSet per Billboard is supported\n"
                "          Using same transform for all GeoSets.\n");

      // Write billboard transform
      fprintf( dx.fp, "object \"%s\" class transform of \"%s\"\n", 
               group->GetName().c_str(), 
               group->members[ group->members.size()-1 ].c_str() );
      fprintf( dx.fp, 
               " times 1 0 0\n"
               "       0 1 0\n"
               "       0 0 1\n"
               " plus  %g %g %g\n",
               node.getPos(0).x(), node.getPos(0).y(), node.getPos(0).z() );
      fprintf( dx.fp, "#\n\n" );

      // Delete group
      group_stack.pop_back();
      delete group;
    }
    void apply(osg::LightSource& node)
         { problem_nodes[ LIGHTSOURCE ]++; apply((osg::Node&)node); }
    void apply(osg::Transform& node)  
         { problem_nodes[ TRANSFORM ]++;   apply((osg::Group&)node); }
    void apply(osg::Impostor& node)   
         { problem_nodes[ IMPOSTER ]++;    apply((osg::LOD&)node); }
    void apply(osg::ClearNode& node)   
         { problem_nodes[ CLEARNODE ]++;    apply((osg::Group&)node); }

    void ReportProblems();
};

//----------------------------------------------------------------------------

void DXWriteVisitor::ReportProblems()
{
  for( StringMap::const_iterator smitr = problem_nodes.begin();
   smitr != problem_nodes.end();
   ++smitr )
    switch ( smitr->first ) {
      case LOD : msg_bin.Add( "WARNING:  %d LOD(s) found ... "
                            "Traversed only the most detailed child of each.\n",
                            smitr->second );
                 break;
      case SWITCH      :
                 msg_bin.Add( "WARNING:  %d Switch(s) found ... "
                              "Traversed only the active child of each.\n",
                              smitr->second );
                 break;
      case BILLBOARD   : 
                 msg_bin.Add( "WARNING:  %d Billboard(s) found ... "
                              "represented as simple Geodes with Transforms.\n",
                              smitr->second );
                 break;
      case LIGHTSOURCE :
                 msg_bin.Add( "WARNING:  %d LightSource(s) found ... Skipped.\n",
                              smitr->second );
                 break;
      case TRANSFORM   :
                 msg_bin.Add( "WARNING:  %d Transform(s) found ... Skipped.\n",
                              smitr->second );
                 break;
      case IMPOSTER    :
                 msg_bin.Add( "WARNING:  %d Imposter(s) found ... Skipped.\n",
                              smitr->second );
                 break;
      case CLEARNODE    :
                 msg_bin.Add( "WARNING:  %d ClearNode(s) found ... Skipped.\n",
                              smitr->second );
                 break;
    }
}

//----------------------------------------------------------------------------

bool WriteDX( const osg::Node &node, WriterParms &parms, std::string &messages )
{
  messages = "";

  DXWriter dx;
  dx.SetParms( parms );
  dx.Open();

  // Yes, this looks a bit weird.  
  //   See the comment in the StateSetActionVisitor decl for more details.
  DXWriteVisitor  dxvisitor(dx);
  StateSetVisitor ssv(&dxvisitor);

  bool success = false;
  try {
    // Arg.  Node::accept isn't a const method.
    ((osg::Node &)node).accept( ssv );
    success = true;
  } 
  catch (...) { 
    unlink( parms.outfile );
  }
  dx.Close();

  dx.ReportUnhandledModesAndAttrs();
  dxvisitor.ReportProblems();
  messages = dx.msg_bin.GetPending();
  return success;
  
}

}; // namespace dx
