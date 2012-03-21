/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_CONVERTER_
#define LWOSG_CONVERTER_

#include "Object.h"
#include "iffparser.h"

#include <osg/ref_ptr>
#include <osg/Group>

#include <osgDB/ReaderWriter>

#include <string>

namespace lwosg
{

    class Converter {
    public:

        struct Options {
            osg::ref_ptr<CoordinateSystemFixer> csf;
            int max_tex_units;
            bool apply_light_model;
            bool use_osgfx;
            bool force_arb_compression;
            bool combine_geodes;
            VertexMap_binding_map texturemap_bindings;

            Options()
            :    csf(new LwoCoordFixer),
                max_tex_units(0),
                apply_light_model(true),
                use_osgfx(false),
                force_arb_compression(false),
                combine_geodes(false)
            {
            }
        };

        Converter();
        Converter(const Options &options, const osgDB::ReaderWriter::Options* db_options);

        osg::Group *convert(Object &obj);
        osg::Group *convert(const iff::Chunk_list &data);
        osg::Group *convert(const std::string &filename);

        inline osg::Group *get_root() { return root_.get(); }
        inline const osg::Group *get_root() const { return root_.get(); }

        inline const Options &get_options() const { return options_; }
        inline Options &get_options() { return options_; }
        inline void set_options(const Options &options) { options_ = options; }

    protected:
        void build_scene_graph(Object &obj);

    private:
        osg::ref_ptr<osg::Group> root_;
        Options options_;
        osg::ref_ptr<const osgDB::ReaderWriter::Options> db_options_;
    };

}

#endif
