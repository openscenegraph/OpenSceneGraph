/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Unit.h"

using namespace lwosg;

Unit::Unit()
:    points_(new osg::Vec3Array),
    normals_(new VertexMap),
    weight_maps_(new VertexMap_map),
    subpatch_weight_maps_(new VertexMap_map),
    texture_maps_(new VertexMap_map),
    rgb_maps_(new VertexMap_map),
    rgba_maps_(new VertexMap_map),
    displacement_maps_(new VertexMap_map),
    spot_maps_(new VertexMap_map)
{
}

float Unit::angle_between_polygons(const Polygon &p1, const Polygon &p2) const
{
    float a = p1.face_normal(points_.get()) * p2.face_normal(points_.get());
    if (a > 1) return 0;
    if (a < -1) return osg::PI;
    return acosf(a);
}

void Unit::find_shared_polygons(int vertex_index, std::vector<int> &poly_indices)
{
    int k = 0;
    for (Polygon_list::const_iterator i=polygons_.begin(); i!=polygons_.end(); ++i, ++k) {
        for (Polygon::Index_list::const_iterator j=i->indices().begin(); j!=i->indices().end(); ++j) {
            if (*j == vertex_index) {
                poly_indices.push_back(k);
                break;
            }
        }
    }
}

void Unit::generate_normals()
{
    // create smoothed normals
    for (Polygon_list::iterator i=polygons_.begin(); i!=polygons_.end(); ++i) {
        osg::Vec4 N = osg::Vec4(i->face_normal(points_.get()), 0);
        for (Polygon::Index_list::iterator j=i->indices().begin(); j!=i->indices().end(); ++j) {
            (*normals_.get())[*j] += N;
        }
    }

    // normalize smoothed normals
    for (VertexMap::iterator ni=normals_->begin(); ni!=normals_->end(); ++ni) {
        float l = ni->second.length();
        if (l != 0) ni->second /= l;
    }

    // compute per-polygon normals
    int pn = 0;
    for (Polygon_list::iterator pi=polygons_.begin(); pi!=polygons_.end(); ++pi, ++pn) {

        Polygon &poly = *pi;

        float max_smoothing_angle = 0;
        if (poly.has_surface()) {
            max_smoothing_angle = poly.get_surface()->get_max_smoothing_angle();
        }

        for (Polygon::Index_list::const_iterator j=poly.indices().begin(); j!=poly.indices().end(); ++j) {
            
            osg::Vec4 N(poly.face_normal(points_.get()), 0);
            unsigned num_smoothed = 1;

            const Index_list &shared_polys = shares_.at(*j);

            for (unsigned k=0; k<shared_polys.size(); ++k) {
                if (shared_polys[k] != pn) {
                    const Polygon &shared_poly = polygons_.at(shared_polys[k]);
                    float angle = angle_between_polygons(poly, shared_poly);
                    if (angle <= max_smoothing_angle && (poly.get_smoothing_group() == shared_poly.get_smoothing_group())) {
                        N += osg::Vec4(shared_poly.face_normal(points_.get()), 0);
                        ++num_smoothed;
                    }
                }
            }

            if (num_smoothed != shared_polys.size()) {
                float l = N.length();
                if (l != 0) N /= l;
                (*poly.local_normals())[*j] = N;
            }
        }
    }
}

void Unit::flatten_maps()
{
    for (Polygon_list::iterator i=polygons().begin(); i!=polygons().end(); ++i) {

        // flatten normal map
        flatten_map(*i, i->local_normals(), normals_.get());
        i->local_normals()->clear();

        VertexMap_map::const_iterator j;
        
        // flatten weight maps
        while (!i->weight_maps()->empty()) {
            VertexMap_map::iterator j = i->weight_maps()->begin();
            flatten_map(*i, j->second.get(), weight_maps_->getOrCreate(j->first));
            i->weight_maps()->erase(j);
        }

        // flatten texture maps
        while (!i->texture_maps()->empty()) {
            VertexMap_map::iterator j = i->texture_maps()->begin();
            flatten_map(*i, j->second.get(), texture_maps_->getOrCreate(j->first));
            i->texture_maps()->erase(j);
        }

        // flatten rgb maps
        while (!i->rgb_maps()->empty()) {
            VertexMap_map::iterator j = i->rgb_maps()->begin();
            flatten_map(*i, j->second.get(), rgb_maps_->getOrCreate(j->first));
            i->rgb_maps()->erase(j);
        }

        // flatten rgba maps
        while (!i->rgba_maps()->empty()) {
            VertexMap_map::iterator j = i->rgba_maps()->begin();
            flatten_map(*i, j->second.get(), rgba_maps_->getOrCreate(j->first));
            i->rgba_maps()->erase(j);
        }

    }
}

void Unit::flatten_map(Polygon &poly, const VertexMap *local_map, VertexMap *global_map)
{
    int j = 0;
    for (Polygon::Index_list::iterator i=poly.indices().begin(); i!=poly.indices().end(); ++i, ++j) {

        // try original vertex index
        VertexMap::const_iterator k = local_map->find(*i);

        // try duplicated vertex index
        if (k == local_map->end()) {
            k = local_map->find(poly.dup_vertices()[j]);
        }

        if (k != local_map->end()) {

            // duplication may be needed!
            if (poly.dup_vertices()[j] == 0) {

                // duplicate point
                points_->push_back(points_->at(*i));

                int new_index = static_cast<int>(points_->size())-1;

                // duplicate normal
                (*normals_.get())[new_index] = (*normals_.get())[*i];

                // duplicate share
                shares_.push_back(shares_.at(*i));

                VertexMap_map::iterator vm;

                // duplicate weights
                for (vm=weight_maps()->begin(); vm!=weight_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }
                
                // duplicate subpatch weights
                for (vm=subpatch_weight_maps()->begin(); vm!=subpatch_weight_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }

                // duplicate texture UVs
                for (vm=texture_maps()->begin(); vm!=texture_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }
                
                // duplicate RGBs
                for (vm=rgb_maps()->begin(); vm!=rgb_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }
                
                // duplicate RGBAs
                for (vm=rgba_maps()->begin(); vm!=rgba_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }
                
                // duplicate displacements
                for (vm=displacement_maps()->begin(); vm!=displacement_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }
                
                // duplicate spots
                for (vm=spot_maps()->begin(); vm!=spot_maps()->end(); ++vm) {
                    if (vm->second->find(*i) != vm->second->end())
                        (*vm->second.get())[new_index] = (*vm->second.get())[*i];
                }                
                
                // update vertex index                
                poly.dup_vertices()[j] = *i;
                *i = new_index;
            }

            (*global_map)[*i] = k->second;
        }
    }
}

void Unit::compute_vertex_remapping(const Surface *surf, Index_list &remap) const
{
    remap.assign(points_->size(), -1);
    for (Polygon_list::const_iterator i=polygons_.begin(); i!=polygons_.end(); ++i) {
        if (i->get_surface() == surf) {
            for (Polygon::Index_list::const_iterator j=i->indices().begin(); j!=i->indices().end(); ++j) {
                remap[*j] = *j;
            }
        }
    }
    int offset = 0;
    for (Index_list::iterator j=remap.begin(); j!=remap.end(); ++j) {
        if (*j == -1) {
            ++offset;
        } else {
            *j -= offset;
        }
    }
}
