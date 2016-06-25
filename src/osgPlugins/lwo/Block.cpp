/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Block.h"

#include <osg/Notify>
#include <osg/Matrix>

using namespace lwosg;

Block::Block(const lwo2::FORM::SURF::BLOK *blok)
:    enabled_(true),
    opacity_type_(ADDITIVE),
    opacity_amount_(1),
    displacement_axis_(X)
{
    if (blok) {
        compile(blok);
    }
}

void Block::read_common_attributes(const iff::Chunk_list &subchunks)
{
    for (iff::Chunk_list::const_iterator i=subchunks.begin(); i!=subchunks.end(); ++i) {
        const lwo2::FORM::SURF::BLOK::CHAN *chan = dynamic_cast<const lwo2::FORM::SURF::BLOK::CHAN *>(*i);
        if (chan) {
            channel_ = std::string(chan->texture_channel.id, 4);
        }
        const lwo2::FORM::SURF::BLOK::ENAB *enab = dynamic_cast<const lwo2::FORM::SURF::BLOK::ENAB *>(*i);
        if (enab) {
            enabled_ = enab->enable != 0;
        }
        const lwo2::FORM::SURF::BLOK::OPAC *opac = dynamic_cast<const lwo2::FORM::SURF::BLOK::OPAC *>(*i);
        if (opac) {
            opacity_type_ = static_cast<Opacity_type>(opac->type);
            opacity_amount_ = opac->opacity.fraction;
        }
        const lwo2::FORM::SURF::BLOK::AXIS *axis = dynamic_cast<const lwo2::FORM::SURF::BLOK::AXIS *>(*i);
        if (axis) {
            displacement_axis_ = static_cast<Axis_type>(axis->displacement_axis);
        }
    }
}

void Block::compile(const lwo2::FORM::SURF::BLOK *blok)
{
    const lwo2::FORM::SURF::BLOK::IMAP *imap = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP *>(blok->header);
    if (imap) {
        type_ = "IMAP";
        ordinal_ = imap->ordinal;

        // read common parameters
        read_common_attributes(imap->block_attributes);

        // read imagemap-related attributes
        for (iff::Chunk_list::const_iterator bi=blok->attributes.begin(); bi!=blok->attributes.end(); ++bi) {
            const lwo2::FORM::SURF::BLOK::IMAP::TMAP *tmap = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TMAP *>(*bi);
            if (tmap) {
                Texture_mapping mapping;
                for (iff::Chunk_list::const_iterator i=tmap->attributes.begin(); i!=tmap->attributes.end(); ++i) {
                    const lwo2::FORM::SURF::BLOK::IMAP::TMAP::CNTR *cntr = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TMAP::CNTR *>(*i);
                    if (cntr) {
                        mapping.center_ = osg::Vec3(cntr->vector.X, cntr->vector.Y, cntr->vector.Z);
                    }
                    const lwo2::FORM::SURF::BLOK::IMAP::TMAP::SIZE *size = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TMAP::SIZE *>(*i);
                    if (size) {
                        mapping.size_ = osg::Vec3(size->vector.X, size->vector.Y, size->vector.Z);
                    }
                    const lwo2::FORM::SURF::BLOK::IMAP::TMAP::ROTA *rota = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TMAP::ROTA *>(*i);
                    if (rota) {
                        mapping.rotation_ = osg::Vec3(rota->vector.X, rota->vector.Y, rota->vector.Z);
                    }
                    const lwo2::FORM::SURF::BLOK::IMAP::TMAP::CSYS *csys = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TMAP::CSYS *>(*i);
                    if (csys) {
                        mapping.csys_ = static_cast<Texture_mapping::Coordinate_system_type>(csys->type);
                    }
                }
                imap_.mapping = mapping;
            }

            const lwo2::FORM::SURF::BLOK::IMAP::PROJ *proj = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::PROJ *>(*bi);
            if (proj) {
                imap_.projection = static_cast<Image_map::Projection_mode>(proj->projection_mode);
            }

            const lwo2::FORM::SURF::BLOK::IMAP::AXIS *axis = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::AXIS *>(*bi);
            if (axis) {
                imap_.axis = static_cast<Image_map::Axis_type>(axis->texture_axis);
            }

            const lwo2::FORM::SURF::BLOK::IMAP::IMAG *imag = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::IMAG *>(*bi);
            if (imag) {
                imap_.image_map = imag->texture_image.index;
            }

            const lwo2::FORM::SURF::BLOK::IMAP::WRAP *wrap = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::WRAP *>(*bi);
            if (wrap) {
                imap_.width_wrap = static_cast<Image_map::Wrap_type>(wrap->width_wrap);
                imap_.height_wrap = static_cast<Image_map::Wrap_type>(wrap->height_wrap);
            }

            const lwo2::FORM::SURF::BLOK::IMAP::WRPW *wrpw = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::WRPW *>(*bi);
            if (wrpw) {
                imap_.wrap_amount_w = wrpw->cycles.fraction;
            }

            const lwo2::FORM::SURF::BLOK::IMAP::WRPH *wrph = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::WRPH *>(*bi);
            if (wrph) {
                imap_.wrap_amount_h = wrph->cycles.fraction;
            }

            const lwo2::FORM::SURF::BLOK::IMAP::VMAP *vmap = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::VMAP *>(*bi);
            if (vmap) {
                imap_.uv_map = vmap->txuv_map_name;
            }

            const lwo2::FORM::SURF::BLOK::IMAP::TAMP *tamp = dynamic_cast<const lwo2::FORM::SURF::BLOK::IMAP::TAMP *>(*bi);
            if (tamp) {
                imap_.texture_amplitude = tamp->amplitude.fraction;
            }
        }

    } else {
        OSG_WARN << "Warning: lwosg::Block: only IMAP (image map) block types are supported, this block will be ignored" << std::endl;
    }
}

osg::Vec3 Block::setup_texture_point(const osg::Vec3 &P) const
{
    osg::Vec3 Q = P;

    const osg::Vec3 &ypr = imap_.mapping.rotation_;
    Q -= imap_.mapping.center_;
    Q = Q * osg::Matrix::rotate(ypr.z(), osg::Vec3(0, 0, -1));
    Q = Q * osg::Matrix::rotate(ypr.x(), osg::Vec3(0, 1, 0));
    Q = Q * osg::Matrix::rotate(ypr.y(), osg::Vec3(-1, 0, 0));
    if (imap_.projection != Image_map::SPHERICAL) {
        Q.x() *= 1/imap_.mapping.size_.x();
        Q.y() *= 1/imap_.mapping.size_.y();
        Q.z() *= 1/imap_.mapping.size_.z();
    }

    return Q;
}
