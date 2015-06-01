/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Object.h"

#include <osg/Notify>

#include <vector>
#include <string>
#include <sstream>

using namespace lwosg;

namespace
{

#if 0
    bool triangle_is_clockwise(const osg::Vec3Array *points, int a, int b, int c)
    {
        const osg::Vec3 &A = (*points)[a];
        const osg::Vec3 &B = (*points)[b];
        const osg::Vec3 &C = (*points)[c];
        float area2 = 0;
        area2 += A.x() * B.y() - B.x() * A.y();
        area2 += B.x() * C.y() - C.x() * B.y();
        area2 += C.x() * A.y() - A.x() * C.y();
        return area2 < 0;
    }
#endif

    float cylindrical_angle(float x, float y)
    {
        float r = sqrtf(x*x+y*y);
        if (r == 0) return 0;
        x /= r;
        float a;
        if (x < 0 && y >= 0) a = osg::PI_2 - acosf(-x);
        else if (x < 0 && y < 0) a = acosf(-x) + osg::PI_2;
        else if (x >= 0 && y >= 0) a = acosf(x) + 3 * osg::PI_2;
        else if (x >= 0 && y < 0) a = 3 * osg::PI_2 - acosf(x);
                else a = 0.0f;
        return a/osg::PI/2;
    }

}

Object::Object()
:    csf_(new LwoCoordFixer)
{
}

Object::Object(const iff::Chunk_list &data)
:    csf_(new LwoCoordFixer)
{
    build(data);
}

void Object::build(const iff::Chunk_list &data)
{
    clips_.clear();
    surfaces_.clear();
    layers_.clear();
    comment_ = "";
    description_ = "";

    OSG_INFO << "INFO: lwosg::Object: scanning clips\n";
    scan_clips(data);
    OSG_INFO << "INFO: lwosg::Object: scanning surfaces\n";
    scan_surfaces(data);
    OSG_INFO << "INFO: lwosg::Object: parsing LWO2 chunks and building object\n";
    parse(data);
    OSG_INFO << "INFO: lwosg::Object: generating normals\n";
    generate_normals();
    OSG_INFO << "INFO: lwosg::Object: generating automatic texture maps\n";
    generate_auto_texture_maps();
}

void Object::scan_clips(const iff::Chunk_list &data)
{
    for (iff::Chunk_list::const_iterator i=data.begin(); i!=data.end(); ++i) {
        const lwo2::FORM::CLIP *clip = dynamic_cast<const lwo2::FORM::CLIP *>(*i);
        if (clip) {
            clips_[clip->index] = Clip(clip);
        }
    }
}

void Object::scan_surfaces(const iff::Chunk_list &data)
{
    for (iff::Chunk_list::const_iterator i=data.begin(); i!=data.end(); ++i) {
        const lwo2::FORM::SURF *surf = dynamic_cast<const lwo2::FORM::SURF *>(*i);
        if (surf) {
            surfaces_[surf->name] = Surface(surf, clips_);
        }
    }
}

void Object::parse(const iff::Chunk_list &data)
{
    typedef std::vector<std::string> String_list;
    String_list tag_strings;

    Layer current_layer;

    for (iff::Chunk_list::const_iterator i=data.begin(); i!=data.end(); ++i) {

        const lwo2::FORM::LAYR *layr = dynamic_cast<const lwo2::FORM::LAYR *>(*i);
        if (layr) {
            if (!current_layer.units().empty() || current_layer.get_layer_chunk()) {
                layers_[current_layer.number()] = current_layer;
            }
            current_layer.set_layer_chunk(layr);
            current_layer.units().clear();
        }

        const lwo2::FORM::PNTS *pnts = dynamic_cast<const lwo2::FORM::PNTS *>(*i);
        if (pnts) {
            Unit new_unit;
            for (lwo2::FORM::PNTS::Point_list::const_iterator i=pnts->point_location.begin(); i!=pnts->point_location.end(); ++i) {
                new_unit.points()->push_back(csf_->fix_point(osg::Vec3(i->X, i->Y, i->Z) /*+ current_layer.pivot()*/));
            }
            new_unit.shares().assign(new_unit.points()->size(), Unit::Index_list());
            current_layer.units().push_back(new_unit);
        }

        const lwo2::FORM::VMAP *vmap = dynamic_cast<const lwo2::FORM::VMAP *>(*i);
        if (vmap && !current_layer.units().empty()) {
            std::string type(vmap->type.id, 4);
            if (type == "WGHT") {
                if (vmap->dimension != 1) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().weight_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), 0, 0, 0);
                }
            }
            if (type == "MNVW") {
                if (vmap->dimension != 1) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().subpatch_weight_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), 0, 0, 0);
                }
            }
            if (type == "TXUV") {
                if (vmap->dimension != 2) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().texture_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), 0, 0);
                }
            }
            if (type == "RGB ") {
                if (vmap->dimension != 3) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().rgb_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), i->value.at(2), 1);
                }
            }
            if (type == "RGBA") {
                if (vmap->dimension != 4) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().rgba_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), i->value.at(2), i->value.at(3));
                }
            }
            if (type == "MORF") {
                if (vmap->dimension != 3) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().displacement_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), i->value.at(2), 0);
                }
            }
            if (type == "SPOT") {
                if (vmap->dimension != 3) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " vertex map dimension: " << vmap->dimension << std::endl;
                    continue;
                }
                VertexMap *new_map = current_layer.units().back().spot_maps()->getOrCreate(vmap->name);
                for (lwo2::FORM::VMAP::Mapping_list::const_iterator i=vmap->mapping.begin(); i!=vmap->mapping.end(); ++i) {
                    (*new_map)[i->vert.index] = osg::Vec4(csf_->fix_point(osg::Vec3(i->value.at(0), i->value.at(1), i->value.at(2))), 0);
                }
            }
        }

        const lwo2::FORM::POLS *pols = dynamic_cast<const lwo2::FORM::POLS *>(*i);
        if (pols && !current_layer.units().empty()) {
            std::string type(pols->type.id, 4);
            if (type != "FACE") {
                OSG_INFO << "INFO: Lwo2Object: polygon list of type " << type << " not supported, rendering may be inaccurate" << std::endl;
            }
            for (lwo2::FORM::POLS::Polygon_list::const_iterator i=pols->polygons.begin(); i!=pols->polygons.end(); ++i) {
                Polygon polygon;
                bool must_invert_winding = csf_->invert_winding();

                // FIX FOR A LIGHTWAVE BUG? MAYBE IT IS A FEATURE, I DON'T KNOW...
                // if the first vertex is at a concave corner, we must invert the winding of the polygon
                // because it appears as flipped in Lighwave. Also, we tell the polygon to invert its normal.
                // (not implemented yet)
                /*if (i->vert.size() >= 4) {
                    if (must_invert_winding == triangle_is_clockwise(current_layer.units().back().points(), i->vert.front().index, i->vert.back().index, i->vert[1].index)) {
                        must_invert_winding = !must_invert_winding;
                        polygon.set_invert_normal(true);
                    }
                }*/

                if (must_invert_winding) {
                    for (unsigned j=0; j<i->numvert; ++j) {
                        int index = i->vert.at((i->numvert-j)%i->numvert).index;
                        polygon.indices().push_back(index);
                        current_layer.units().back().shares().at(index).push_back(current_layer.units().back().polygons().size());
                    }
                } else {
                    for (unsigned j=0; j<i->numvert; ++j) {
                        int index = i->vert.at(j).index;
                        polygon.indices().push_back(index);
                        current_layer.units().back().shares().at(index).push_back(current_layer.units().back().polygons().size());
                    }
                }
                current_layer.units().back().polygons().push_back(polygon);
            }
        }

        const lwo2::FORM::TAGS *tags = dynamic_cast<const lwo2::FORM::TAGS *>(*i);
        if (tags) {
            tag_strings = tags->tag_string;
        }

        const lwo2::FORM::PTAG *ptag = dynamic_cast<const lwo2::FORM::PTAG *>(*i);
        if (ptag && !current_layer.units().empty()) {
            std::string type(ptag->type.id, 4);
            if (type == "SURF") {
                for (lwo2::FORM::PTAG::Mapping_list::const_iterator i=ptag->mapping.begin(); i!=ptag->mapping.end(); ++i) {
                    current_layer.units().back().polygons().at(i->poly.index).set_surface(&surfaces_[tag_strings.at(i->tag)]);
                }
            }
            if (type == "PART") {
                for (lwo2::FORM::PTAG::Mapping_list::const_iterator i=ptag->mapping.begin(); i!=ptag->mapping.end(); ++i) {
                    current_layer.units().back().polygons().at(i->poly.index).set_part_name(tag_strings.at(i->tag));
                }
            }
            if (type == "SMGP") {
                for (lwo2::FORM::PTAG::Mapping_list::const_iterator i=ptag->mapping.begin(); i!=ptag->mapping.end(); ++i) {
                    current_layer.units().back().polygons().at(i->poly.index).set_smoothing_group(tag_strings.at(i->tag));
                }
            }
        }

        const lwo2::FORM::VMAD *vmad = dynamic_cast<const lwo2::FORM::VMAD *>(*i);
        if (vmad && !current_layer.units().empty()) {
            std::string type(vmad->type.id, 4);
            if (type == "WGHT") {
                if (vmad->dimension != 1) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " discontinuous vertex map dimension: " << vmad->dimension << std::endl;
                    continue;
                }
                for (lwo2::FORM::VMAD::Mapping_list::const_iterator i=vmad->mapping.begin(); i!=vmad->mapping.end(); ++i) {
                    VertexMap *this_map = current_layer.units().back().polygons().at(i->poly.index).weight_maps()->getOrCreate(vmad->name);
                    (*this_map)[i->vert.index] = osg::Vec4(i->value.at(0), 0, 0, 0);
                }
            }
            if (type == "TXUV") {
                if (vmad->dimension != 2) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " discontinuous vertex map dimension: " << vmad->dimension << std::endl;
                    continue;
                }
                for (lwo2::FORM::VMAD::Mapping_list::const_iterator i=vmad->mapping.begin(); i!=vmad->mapping.end(); ++i) {
                    VertexMap *this_map = current_layer.units().back().polygons().at(i->poly.index).texture_maps()->getOrCreate(vmad->name);
                    (*this_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), 0, 0);
                }
            }
            if (type == "RGB ") {
                if (vmad->dimension != 3) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " discontinuous vertex map dimension: " << vmad->dimension << std::endl;
                    continue;
                }
                for (lwo2::FORM::VMAD::Mapping_list::const_iterator i=vmad->mapping.begin(); i!=vmad->mapping.end(); ++i) {
                    VertexMap *this_map = current_layer.units().back().polygons().at(i->poly.index).rgb_maps()->getOrCreate(vmad->name);
                    (*this_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), i->value.at(2), 1);
                }
            }
            if (type == "RGBA") {
                if (vmad->dimension != 4) {
                    OSG_WARN << "Warning: Lwo2Object: invalid " << type << " discontinuous vertex map dimension: " << vmad->dimension << std::endl;
                    continue;
                }
                for (lwo2::FORM::VMAD::Mapping_list::const_iterator i=vmad->mapping.begin(); i!=vmad->mapping.end(); ++i) {
                    VertexMap *this_map = current_layer.units().back().polygons().at(i->poly.index).rgba_maps()->getOrCreate(vmad->name);
                    (*this_map)[i->vert.index] = osg::Vec4(i->value.at(0), i->value.at(1), i->value.at(2), i->value.at(3));
                }
            }
        }

        const lwo2::FORM::DESC *desc = dynamic_cast<const lwo2::FORM::DESC *>(*i);
        if (desc) {
            description_ = desc->description_line;
        }

        const lwo2::FORM::TEXT *text = dynamic_cast<const lwo2::FORM::TEXT *>(*i);
        if (text) {
            comment_ = text->comment;
        }

    }

    if (!current_layer.units().empty() || current_layer.get_layer_chunk()) {
        layers_[current_layer.number()] = current_layer;
    }
}

void Object::generate_normals()
{
    for (Layer_map::iterator i=layers_.begin(); i!=layers_.end(); ++i) {
        for (Layer::Unit_list::iterator j=i->second.units().begin(); j!=i->second.units().end(); ++j) {
            j->generate_normals();
        }
    }
}

void Object::generate_auto_texture_maps()
{
    for (Surface_map::iterator i=surfaces_.begin(); i!=surfaces_.end(); ++i) {
        for (Surface::Block_map::iterator j=i->second.blocks().begin(); j!=i->second.blocks().end(); ++j) {
            Block &block = j->second;
            if (block.get_type() == "IMAP") {
                if (block.get_image_map().projection == Image_map::UV) continue;

                Image_map::Axis_type axis = block.get_image_map().axis;

                std::ostringstream oss;
                oss << "Auto_map_" << &block;
                std::string map_name = oss.str();

                OSG_DEBUG << "DEBUG INFO: lwosg::Object: creating automatic texture map '" << map_name << "'\n";

                for (Layer_map::iterator k=layers_.begin(); k!=layers_.end(); ++k) {
                    for (Layer::Unit_list::iterator h=k->second.units().begin(); h!=k->second.units().end(); ++h) {

                        osg::ref_ptr<VertexMap> new_map = new VertexMap;
                        (*h->texture_maps())[map_name] = new_map.get();

                        if (block.get_image_map().projection == Image_map::FRONT_PROJECTION) {
                            OSG_WARN << "Warning: lwosg::Object: front projection is not supported" << std::endl;
                        }

                        if (block.get_image_map().projection == Image_map::CUBIC) {

                            for (Unit::Polygon_list::iterator p=h->polygons().begin(); p!=h->polygons().end(); ++p) {

                                Polygon &poly = *p;
                                osg::ref_ptr<VertexMap> local_uv_map = poly.texture_maps()->getOrCreate(map_name);

                                osg::Vec3 N = csf_->fix_vector(poly.face_normal(h->points()));

                                Image_map::Axis_type axis = Image_map::X;
                                if (N.y() > N.x() && N.y() > N.z()) axis = Image_map::Y;
                                if (-N.y() > N.x() && -N.y() > N.z()) axis = Image_map::Y;
                                if (N.z() > N.x() && N.z() > N.y()) axis = Image_map::Z;
                                if (-N.z() > N.x() && -N.z() > N.y()) axis = Image_map::Z;

                                for (Polygon::Index_list::iterator i=poly.indices().begin(); i!=poly.indices().end(); ++i) {

                                    // fetch vertex
                                    osg::Vec3 P = csf_->fix_point((*h->points())[*i]);

                                    // setup scale/translation/rotation
                                    P = block.setup_texture_point(P);

                                    osg::Vec2 uv;
                                    switch (axis) {
                                        case Image_map::X: uv.set(P.z(), P.y()); break;
                                        case Image_map::Y: uv.set(P.x(), P.z()); break;
                                        case Image_map::Z: uv.set(P.x(), P.y()); break;
                                        default: ;
                                    }
                                    uv += osg::Vec2(0.5f, 0.5f);

                                    osg::Vec4 map_value(uv.x(), uv.y(), 0, 0);

                                    if ((new_map->find(*i) != new_map->end()) && (map_value != (*new_map.get())[*i])) {
                                        (*local_uv_map.get())[*i] = map_value;
                                    } else {
                                        (*new_map.get())[*i] = map_value;
                                    }
                                }
                            }
                        } else {

                            for (unsigned p=0; p<h->points()->size(); ++p) {

                                // fetch vertex
                                osg::Vec3 P = csf_->fix_point((*h->points())[p]);

                                // setup scale/translation/rotation
                                P = block.setup_texture_point(P);

                                osg::Vec2 uv;

                                if (block.get_image_map().projection == Image_map::PLANAR) {
                                    switch (axis) {
                                        case Image_map::X: uv.set(P.z(), P.y()); break;
                                        case Image_map::Y: uv.set(P.x(), P.z()); break;
                                        case Image_map::Z: uv.set(P.x(), P.y()); break;
                                        default: ;
                                    }
                                    uv += osg::Vec2(0.5f, 0.5f);
                                }

                                if (block.get_image_map().projection == Image_map::CYLINDRICAL) {
                                    switch (axis) {
                                        case Image_map::X: uv.set(cylindrical_angle(-P.z(), -P.y()), P.x()); break;
                                        case Image_map::Y: uv.set(cylindrical_angle(P.x(), P.z()), P.y()); break;
                                        case Image_map::Z: uv.set(cylindrical_angle(P.x(), -P.y()), P.z()); break;
                                        default: ;
                                    }
                                    uv.x() *= block.get_image_map().wrap_amount_w;
                                    uv += osg::Vec2(0, 0.5f);
                                }

                                if (block.get_image_map().projection == Image_map::SPHERICAL) {
                                    float r = P.length();
                                    if (r != 0) {
                                        switch (axis) {
                                            case Image_map::X: uv.set(cylindrical_angle(-P.z(), -P.y()), (asinf(P.x()/r) + osg::PI_2) / osg::PI); break;
                                            case Image_map::Y: uv.set(cylindrical_angle(P.x(), P.z()), (asinf(P.y()/r) + osg::PI_2) / osg::PI); break;
                                            case Image_map::Z: uv.set(cylindrical_angle(P.x(), -P.y()), (asinf(P.z()/r) + osg::PI_2) / osg::PI); break;
                                            default: ;
                                        }
                                    }
                                    uv.x() *= block.get_image_map().wrap_amount_w;
                                    uv.y() *= block.get_image_map().wrap_amount_h;
                                }

                                (*new_map.get())[p] = osg::Vec4(uv.x(), uv.y(), 0, 0);
                            }
                        }
                    }
                }

                block.get_image_map().uv_map = map_name;
                block.get_image_map().projection = Image_map::UV;
            }
        }
    }
}
