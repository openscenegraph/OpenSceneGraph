/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_SURFACE_
#define LWOSG_SURFACE_

#include "lwo2chunks.h"

#include "VertexMap.h"
#include "Block.h"
#include "Clip.h"

#include <osg/ref_ptr>
#include <osg/Vec3>
#include <osg/StateSet>
#include <osg/Geometry>

#include <osgDB/ReaderWriter>

#include <string>
#include <map>

namespace lwosg
{

    class Surface {
    public:

        enum Sidedness {
            NONE           = 0,
            FRONT_ONLY     = 1,
            BACK_ONLY      = 2,
            FRONT_AND_BACK = 3
        };

        typedef std::multimap<std::string, Block> Block_map;

        Surface();
        Surface(const lwo2::FORM::SURF *surf, const Clip_map &clips);

        void compile(const lwo2::FORM::SURF *surf, const Clip_map &clips);

        osg::Group *apply(osg::Geometry *geo, const VertexMap_map *texture_maps, const VertexMap_map *rgb_maps, const VertexMap_map *rgba_maps, int max_tex_units, bool use_osgfx, bool force_arb_compression, const VertexMap_binding_map &texmap_bindings, const osgDB::ReaderWriter::Options *db_options) const;

        void generate_stateset(unsigned int max_tex_units, bool force_arb_compression, const osgDB::ReaderWriter::Options* options) const;

        inline const std::string &get_name() const { return name_; }
        inline void set_name(const std::string &n) { name_ = n; }

        inline const osg::Vec3 &get_base_color() const { return base_color_; }

        inline float get_diffuse() const { return diffuse_; }
        inline float get_luminosity() const { return luminosity_; }
        inline float get_specularity() const { return specularity_; }
        inline float get_reflection() const { return reflection_; }
        inline float get_transparency() const { return transparency_; }
        inline float get_translucency() const { return translucency_; }
        inline float get_glossiness() const { return glossiness_; }

        inline Sidedness get_sidedness() const { return sidedness_; }

        inline float get_max_smoothing_angle() const { return max_smoothing_angle_; }

        inline const std::string &get_color_map_type() const { return color_map_type_; }
        inline const std::string &get_color_map_name() const { return color_map_name_; }
        inline float get_color_map_intensity() const { return color_map_intensity_; }

        inline Block_map &blocks() { return blocks_; }
        inline const Block_map &blocks() const { return blocks_; }
/*
        inline const std::string &get_uv_map_name() const { return uv_map_name_; }

        inline bool has_clip() const { return clip_ != 0; }
        inline const Clip *get_clip() const { return clip_; }
        inline void set_clip(const Clip *c) { clip_ = c; }
*/
    private:
        std::string name_;
        osg::Vec3 base_color_;
        float diffuse_;
        float luminosity_;
        float specularity_;
        float reflection_;
        float transparency_;
        float translucency_;
        float glossiness_;
        Sidedness sidedness_;
        float max_smoothing_angle_;
        std::string color_map_type_;
        std::string color_map_name_;
        float color_map_intensity_;

        Block_map blocks_;
/*
        std::string uv_map_name_;
        const Clip *clip_;
*/
        mutable osg::ref_ptr<osg::StateSet> stateset_;
    };

}

#endif
