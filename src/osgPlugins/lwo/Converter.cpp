/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Converter.h"
#include "Tessellator.h"

#include <osg/Notify>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/CullFace>
#include <osg/LightModel>

#include <osgDB/FileUtils>
#include <osgDB/fstream>

#include "lwo2parser.h"

using namespace lwosg;

namespace
{

    struct GeometryBin {
        osg::ref_ptr<osg::DrawElementsUInt> deui_points;
        osg::ref_ptr<osg::DrawElementsUInt> deui_lines;
        osg::ref_ptr<osg::DrawElementsUInt> deui_triangles;

        GeometryBin()
        :    deui_points(new osg::DrawElementsUInt(GL_POINTS)),
            deui_lines(new osg::DrawElementsUInt(GL_LINES)),
            deui_triangles(new osg::DrawElementsUInt(GL_TRIANGLES))
        {}
    };

}

Converter::Converter()
:    root_(new osg::Group)
{
}

Converter::Converter(const Options &options, const osgDB::ReaderWriter::Options* db_options)
:    root_(new osg::Group),
    options_(options),
    db_options_(db_options)
{
}

osg::Group *Converter::convert(Object &obj)
{
    if (root_->getNumChildren() > 0) {
        root_->removeChildren(0, root_->getNumChildren());
    }

    osg::notify(osg::INFO) << "INFO: lwosg::Converter: flattening per-polygon vertex maps\n";
    for (Object::Layer_map::iterator i=obj.layers().begin(); i!=obj.layers().end(); ++i) {
        for (Layer::Unit_list::iterator j=i->second.units().begin(); j!=i->second.units().end(); ++j) {
            j->flatten_maps();
        }
    }

    osg::notify(osg::INFO) << "INFO: lwosg::Converter: creating scene graph\n";
    build_scene_graph(obj);

    return root_.get();
}

void Converter::build_scene_graph(Object &obj)
{
    // generate layer structure
    typedef std::map<int, osg::ref_ptr<osg::Group> > Layer_group_map;
    Layer_group_map lymap;

    osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter: creating layer structure\n";

    // create a flat layer structure, no parenting since it's handled in scene files
    for (Object::Layer_map::const_iterator i=obj.layers().begin(); i!=obj.layers().end(); ++i) {
        const Layer &layer = i->second;
        osg::ref_ptr<osg::Group> new_group = new osg::Group;
        lymap[layer.number()] = new_group.get();
        if (layer.get_layer_chunk()) {
            new_group->setName(layer.get_layer_chunk()->name);
            if (layer.get_layer_chunk()->flags & 1) {
                new_group->setNodeMask(0);
            }
        } else {
            new_group->setName("Default_layer");
        }
        root_->addChild(new_group.get());
    }

    for (Object::Layer_map::iterator li=obj.layers().begin(); li!=obj.layers().end(); ++li) {
        Layer &layer = li->second;

        osg::Group *layer_group = lymap[layer.number()].get();

        osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter: processing layer '" << layer_group->getName() << "'\n";

        for (Layer::Unit_list::iterator j=layer.units().begin(); j!=layer.units().end(); ++j) {

            osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter: \tcreating primitives\n";

            int tess_success = 0;
            int tess_fail = 0;

            typedef std::map<const Surface *, GeometryBin> GeometryBin_map;
            GeometryBin_map bins;

            typedef std::map<const Surface *, Unit::Index_list> Remapping_map;
            Remapping_map remappings;

            // compute remapping map for default surface
            j->compute_vertex_remapping(0, remappings[0]);

            // compute remapping maps for other surfaces            
            for (Object::Surface_map::const_iterator h=obj.surfaces().begin(); h!=obj.surfaces().end(); ++h) {
                j->compute_vertex_remapping(&h->second, remappings[&h->second]);
            }

            // create primitive sets, taking into account remapping maps
            for (unsigned k=0; k<j->polygons().size(); ++k) {
                const Polygon &poly = j->polygons()[k];
                GeometryBin &bin = bins[poly.get_surface()];
                const Unit::Index_list &remapping = remappings[poly.get_surface()];

                if (poly.indices().size() == 1) {
                    bin.deui_points->push_back(remapping[poly.indices()[0]]);
                }
                if (poly.indices().size() == 2) {
                    bin.deui_lines->push_back(remapping[poly.indices()[0]]);
                    bin.deui_lines->push_back(remapping[poly.indices()[1]]);
                }
                if (poly.indices().size() == 3) {
                    bin.deui_triangles->push_back(remapping[poly.indices()[0]]);
                    bin.deui_triangles->push_back(remapping[poly.indices()[1]]);
                    bin.deui_triangles->push_back(remapping[poly.indices()[2]]);
                }
                if (poly.indices().size() >= 4) {
                    Tessellator tess;
                    if (tess.tessellate(poly, j->points(), bin.deui_triangles.get(), &remapping)) {
                        ++tess_success;
                    } else {
                        ++tess_fail;
                    }
                }
            }

            if (tess_success > 0) {
                osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter:   " << tess_success << " polygons have been tessellated correctly\n";
            }

            if (tess_fail > 0) {
                osg::notify(osg::WARN) << "Warning: lwosg::Converter:   could not tessellate " << tess_fail << " polygons correctly. This is probably due to self-intersecting polygons being used, try to Triple them in Lightwave and restart the conversion" << std::endl;
            }

            // create normal array
            osg::ref_ptr<osg::Vec3Array> normals = j->normals()->asVec3Array(j->points()->size());

            // create first geode
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;

            for (GeometryBin_map::iterator i=bins.begin(); i!=bins.end(); ++i) {
                const Surface *surface = i->first;
                GeometryBin &bin = i->second;

                const Unit::Index_list &remapping = remappings[surface];

                // clean up points and normals according to remapping map
                osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter: \tcleaning up redundant vertices and vertex attributes for surface '" << (surface ? surface->get_name() : std::string("anonymous")) << "'\n";                
                osg::ref_ptr<osg::Vec3Array> new_points = new osg::Vec3Array;
                osg::ref_ptr<osg::Vec3Array> new_normals = new osg::Vec3Array;
                for (unsigned pi=0; pi<j->points()->size(); ++pi) {
                    if (remapping[pi] != -1) {
                        new_points->push_back((*j->points())[pi]);
                        new_normals->push_back((*normals)[pi]);
                    }
                }
                
                osg::notify(osg::DEBUG_INFO) << "DEBUG INFO: lwosg::Converter: \tcreating geometry for surface '" << (surface ? surface->get_name() : std::string("anonymous")) << "'\n";

                osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
                geo->setVertexArray(new_points.get());
                geo->setNormalArray(new_normals.get());
                geo->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);                

                bool group_used = false;

                if (surface) {
                    if (!options_.combine_geodes)
                    {
                        geode->setName(surface->get_name());
                    }

                    // apply surface parameters and texture/color maps according to remapping map
                    osg::ref_ptr<VertexMap_map> rm_texture_maps = j->texture_maps()->remap(remapping);
                    osg::ref_ptr<VertexMap_map> rm_rgb_maps = j->rgb_maps()->remap(remapping);
                    osg::ref_ptr<VertexMap_map> rm_rgba_maps = j->rgba_maps()->remap(remapping);
                    osg::Group *sgrp = surface->apply(geo.get(),
                        rm_texture_maps.get(),
                        rm_rgb_maps.get(),
                        rm_rgba_maps.get(),
                        options_.max_tex_units,
                        options_.use_osgfx,
                        options_.force_arb_compression,
                        options_.texturemap_bindings,
                        db_options_.get());
                    if (sgrp) 
                    {
                        group_used = true;
                        osg::ref_ptr<osg::Geode> grp_geode = new osg::Geode;
                        grp_geode->setName(surface->get_name());
                        grp_geode->addDrawable(geo.get());
                        sgrp->addChild(grp_geode.get());
                        layer_group->addChild(sgrp);
                    }
                }

                if (!group_used)
                {
                    geode->addDrawable(geo.get());
                    if (geode->getNumParents() == 0)
                    {
                        layer_group->addChild(geode.get());
                    }
                }

                if (!options_.combine_geodes)
                {
                    geode = new osg::Geode;
                }

                // add primitive sets to geometry
                if (!bin.deui_points->empty()) geo->addPrimitiveSet(bin.deui_points.get());
                if (!bin.deui_lines->empty()) geo->addPrimitiveSet(bin.deui_lines.get());
                if (!bin.deui_triangles->empty()) geo->addPrimitiveSet(bin.deui_triangles.get());
            }
        }

        osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
        cf->setMode(osg::CullFace::BACK);
        root_->getOrCreateStateSet()->setAttributeAndModes(cf.get());

        if (options_.apply_light_model) {
            osg::ref_ptr<osg::LightModel> lm = new osg::LightModel;
            lm->setTwoSided(true);
            lm->setColorControl(osg::LightModel::SEPARATE_SPECULAR_COLOR);
            lm->setAmbientIntensity(osg::Vec4(0, 0, 0, 0));
            lm->setLocalViewer(true);
            root_->getOrCreateStateSet()->setAttributeAndModes(lm.get());
        }
    }
}

osg::Group *Converter::convert(const iff::Chunk_list &data)
{
    Object obj(data);
    obj.set_coordinate_system_fixer(options_.csf.get());
    return convert(obj);
}

osg::Group *Converter::convert(const std::string &filename)
{
    std::string file = osgDB::findDataFile(filename, db_options_.get());
    if (file.empty()) return 0;

    osgDB::ifstream ifs(file.c_str(), std::ios_base::in | std::ios_base::binary);
    if (!ifs.is_open()) return 0;

    std::vector<char> buffer;
    char c;
    while (ifs.get(c)) buffer.push_back(c);

    lwo2::Parser<std::vector<char>::const_iterator > parser(osg::notify(osg::DEBUG_INFO));

    try
    {
        parser.parse(buffer.begin(), buffer.end());
    }
    catch(lwo2::parser_error &e)
    {
        std::cerr << e.what() << std::endl;
        return 0;
    }

    for (iff::Chunk_list::const_iterator i=parser.chunks().begin(); i!=parser.chunks().end(); ++i) {
        const lwo2::FORM *form = dynamic_cast<const lwo2::FORM *>(*i);
        if (form) {
            Object obj(form->data);
            obj.set_coordinate_system_fixer(options_.csf.get());
            if (!convert(obj)) {
                return 0;
            }
            root_->setName(file);
            return root_.get();
        }
    }


    return 0;
}
