#include <osg/StateAttribute>

#include <map>
#include <string>
#include <string.h>

namespace dx {

typedef std::map<osg::StateAttribute::GLMode,std::string> ModeMap;

static ModeMap S_mode_map;

#define ADD_NAME(str,mode) (S_mode_map[(mode)] = (str));

// This is a copy of some of the code in src/osgPlugins/osg/StateSet.cpp.
//   We need to translate GLModes and StateSet attributes into strings,
//   and that's not available in an OSG public API (yet).

static void initGLNames()
{
    static bool first_time = true;
    if (!first_time) return;
    
    ADD_NAME("GL_ALPHA_TEST",GL_ALPHA_TEST)
    ADD_NAME("GL_BLEND",GL_BLEND)
    ADD_NAME("GL_COLOR_MATERIAL",GL_COLOR_MATERIAL)
    ADD_NAME("GL_CULL_FACE",GL_CULL_FACE)
    ADD_NAME("GL_DEPTH_TEST",GL_DEPTH_TEST)
    ADD_NAME("GL_FOG",GL_FOG)
    ADD_NAME("GL_LIGHTING",GL_LIGHTING)
    ADD_NAME("GL_POINT_SMOOTH",GL_POINT_SMOOTH)
    ADD_NAME("GL_POLYGON_OFFSET_FILL",GL_POLYGON_OFFSET_FILL)
    ADD_NAME("GL_POLYGON_OFFSET_LINE",GL_POLYGON_OFFSET_LINE)
    ADD_NAME("GL_POLYGON_OFFSET_POINT",GL_POLYGON_OFFSET_POINT)
    
    ADD_NAME("GL_TEXTURE_2D",GL_TEXTURE_2D)
    ADD_NAME("GL_TEXTURE_GEN_Q",GL_TEXTURE_GEN_Q)
    ADD_NAME("GL_TEXTURE_GEN_R",GL_TEXTURE_GEN_R)
    ADD_NAME("GL_TEXTURE_GEN_S",GL_TEXTURE_GEN_S)
    ADD_NAME("GL_TEXTURE_GEN_T",GL_TEXTURE_GEN_T)
    
    ADD_NAME("GL_STENCIL_TEST",GL_STENCIL_TEST)
    
    ADD_NAME("GL_CLIP_PLANE0",GL_CLIP_PLANE0);
    ADD_NAME("GL_CLIP_PLANE1",GL_CLIP_PLANE1);
    ADD_NAME("GL_CLIP_PLANE2",GL_CLIP_PLANE2);
    ADD_NAME("GL_CLIP_PLANE3",GL_CLIP_PLANE3);
    ADD_NAME("GL_CLIP_PLANE4",GL_CLIP_PLANE4);
    ADD_NAME("GL_CLIP_PLANE5",GL_CLIP_PLANE5);

    ADD_NAME("GL_LIGHT0",GL_LIGHT0);
    ADD_NAME("GL_LIGHT1",GL_LIGHT1);
    ADD_NAME("GL_LIGHT2",GL_LIGHT2);
    ADD_NAME("GL_LIGHT3",GL_LIGHT3);
    ADD_NAME("GL_LIGHT4",GL_LIGHT4);
    ADD_NAME("GL_LIGHT5",GL_LIGHT5);
    ADD_NAME("GL_LIGHT6",GL_LIGHT6);
    ADD_NAME("GL_LIGHT7",GL_LIGHT7);

    first_time = false;
}

const char *GLModeToModeStr( osg::StateAttribute::GLMode mode )
{
  initGLNames();
  ModeMap::const_iterator mitr = S_mode_map.find(mode);
  if ( mitr != S_mode_map.end() )
    return mitr->second.c_str();
  else
    return 0;
}

osg::StateAttribute::GLMode GLModeStrToMode( const char mode_str[] )
{
  initGLNames();
  for( ModeMap::const_iterator mitr = S_mode_map.begin();
       mitr != S_mode_map.end();
       ++mitr )
    if ( strcmp( mode_str, mitr->second.c_str() ) == 0 )
      return mitr->first;
  return (osg::StateAttribute::GLMode) -1;
}

//===========================================================================

typedef std::map<osg::StateAttribute::Type,std::string> AttrMap;

static AttrMap S_attr_map;

#define ADD_ATTR(attr,str) (S_attr_map[(attr)] = (str));

static void initOSGAttrNames()
{
  static bool first_time = true;
  if (!first_time) return;

  ADD_ATTR( osg::StateAttribute::TEXTURE         , "TEXTURE"      );
  ADD_ATTR( osg::StateAttribute::MATERIAL        , "MATERIAL"     );
  ADD_ATTR( osg::StateAttribute::ALPHAFUNC       , "ALPHAFUNC"    );
  ADD_ATTR( osg::StateAttribute::ANTIALIAS       , "ANTIALIAS"    );
  ADD_ATTR( osg::StateAttribute::COLORTABLE      , "COLORTABLE"   );
  ADD_ATTR( osg::StateAttribute::CULLFACE        , "CULLFACE"     );
  ADD_ATTR( osg::StateAttribute::FOG             , "FOG"          );
  ADD_ATTR( osg::StateAttribute::FRONTFACE       , "FRONTFACE"    );
  ADD_ATTR( osg::StateAttribute::LIGHT           , "LIGHT"        );
  ADD_ATTR( osg::StateAttribute::LIGHT_0         , "LIGHT_0"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_1         , "LIGHT_1"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_2         , "LIGHT_2"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_3         , "LIGHT_3"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_4         , "LIGHT_4"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_5         , "LIGHT_5"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_6         , "LIGHT_6"      );
  ADD_ATTR( osg::StateAttribute::LIGHT_7         , "LIGHT_7"      );
  ADD_ATTR( osg::StateAttribute::POINT           , "POINT"        );      
  ADD_ATTR( osg::StateAttribute::LINEWIDTH       , "LINEWIDTH"    );
  ADD_ATTR( osg::StateAttribute::POLYGONMODE     , "POLYGONMODE"  );
  ADD_ATTR( osg::StateAttribute::POLYGONOFFSET   , "POLYGONOFFSET");
  ADD_ATTR( osg::StateAttribute::TEXENV          , "TEXENV"       );
  ADD_ATTR( osg::StateAttribute::TEXGEN          , "TEXGEN"       );
  ADD_ATTR( osg::StateAttribute::TEXMAT          , "TEXMAT"       );
  ADD_ATTR( osg::StateAttribute::BLENDFUNC       , "BLENDFUNC"    );
  ADD_ATTR( osg::StateAttribute::STENCIL         , "STENCIL"      );
  ADD_ATTR( osg::StateAttribute::COLORMASK       , "COLORMASK"    );
  ADD_ATTR( osg::StateAttribute::DEPTH           , "DEPTH"        );
  ADD_ATTR( osg::StateAttribute::VIEWPORT        , "VIEWPORT"     );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE       , "CLIPPLANE"    );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_0     , "CLIPPLANE_0"  );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_1     , "CLIPPLANE_1"  );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_2     , "CLIPPLANE_2"  );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_3     , "CLIPPLANE_3"  );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_4     , "CLIPPLANE_4"  );
  ADD_ATTR( osg::StateAttribute::CLIPPLANE_5     , "CLIPPLANE_5"  );
  ADD_ATTR( osg::StateAttribute::COLORMATRIX     , "COLORMATRIX"  );

  first_time = false;
}

const char *OSGAttrToAttrStr( osg::StateAttribute::Type attr )
{
  initOSGAttrNames();
  AttrMap::const_iterator aitr = S_attr_map.find(attr);
  if ( aitr != S_attr_map.end() )
    return aitr->second.c_str();
  else
    return 0;
}

osg::StateAttribute::Type OSGAttrStrToAttr( const char attr_str[] )
{
  initGLNames();
  for( AttrMap::const_iterator aitr = S_attr_map.begin();
       aitr != S_attr_map.end();
       ++aitr )
    if ( strcmp( attr_str, aitr->second.c_str() ) == 0 )
      return aitr->first;
  return (osg::StateAttribute::Type) -1;
}

}; // namespace dx
