/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Tessellator.h"

using namespace lwosg;

namespace lwosg
{

    void CALLBACK cb_begin_data(GLenum type, void *data)
    {
        Tessellator *tess = static_cast<Tessellator *>(data);
        tess->prim_type_ = type;
        tess->incoming_.clear();
    }

    void CALLBACK cb_vertex_data(void *vertex_data, void *data)
    {
        Tessellator *tess = static_cast<Tessellator *>(data);
        tess->incoming_.push_back(*static_cast<int *>(vertex_data));
    }

    void CALLBACK cb_end_data(void *data)
    {
        Tessellator *tess = static_cast<Tessellator *>(data);
        tess->finalize_primitive();
    }

    void CALLBACK cb_error_data(GLenum error, void *data)
    {
        Tessellator *tess = static_cast<Tessellator *>(data);
        tess->last_error_ = error;
    }

}

bool Tessellator::tessellate(const Polygon &poly, const osg::Vec3Array *points, osg::DrawElementsUInt *out, const std::vector<int> *remap)
{
    out_ = out;
    last_error_ = 0;

    osg::GLUtesselator *tess = osg::gluNewTess();

    osg::gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (osg::GLU_TESS_CALLBACK) (cb_begin_data));
    osg::gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (osg::GLU_TESS_CALLBACK) (cb_vertex_data));
    osg::gluTessCallback(tess, GLU_TESS_END_DATA, (osg::GLU_TESS_CALLBACK) (cb_end_data));
    osg::gluTessCallback(tess, GLU_TESS_ERROR_DATA, (osg::GLU_TESS_CALLBACK) (cb_error_data));

    osg::gluTessBeginPolygon(tess, this);
    osg::gluTessBeginContour(tess);

    double *vertices = new double[poly.indices().size() * 3];
    int *indices = new int[poly.indices().size()];

    double *v = vertices;
    int *x = indices;
    for (Polygon::Index_list::const_iterator i=poly.indices().begin(); i!=poly.indices().end(); ++i, v+=3, ++x) {
        const osg::Vec3 &P = (*points)[*i];
        v[0] = P.x();
        v[1] = P.y();
        v[2] = P.z();
        if (remap) {
            *x = (*remap)[*i];
        } else {
            *x = *i;
        }
        gluTessVertex(tess, v, x);
    }

    osg::gluTessEndContour(tess);
    osg::gluTessEndPolygon(tess);
    osg::gluDeleteTess(tess);

    delete[] vertices;
    delete[] indices;

    return last_error_ == 0;
}

Tessellator::~Tessellator()
{
}

void Tessellator::finalize_primitive()
{
    if (incoming_.size() < 3) return;

    if (prim_type_ == GL_TRIANGLES) {
        for (Index_list::const_iterator i=incoming_.begin(); i!=incoming_.end(); ++i) {
            out_->push_back(*i);
        }
    }

    if (prim_type_ == GL_TRIANGLE_FAN) {
        for (Index_list::const_iterator i=incoming_.begin()+1; (i+1)!=incoming_.end(); ++i) {
            out_->push_back(incoming_.front());
            out_->push_back(*i);
            out_->push_back(*(i+1));
        }
    }

    if (prim_type_ == GL_TRIANGLE_STRIP) {
        int j = 0;
        for (Index_list::const_iterator i=incoming_.begin(); (i+2)!=incoming_.end(); ++i, ++j) {
            if ((j % 2) == 0) {
                out_->push_back(*i);
                out_->push_back(*(i+1));
                out_->push_back(*(i+2));
            } else {
                out_->push_back(*i);
                out_->push_back(*(i+2));
                out_->push_back(*(i+1));
            }
        }
    }
}
