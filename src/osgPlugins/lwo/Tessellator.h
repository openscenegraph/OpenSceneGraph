/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_TESSELLATOR_
#define LWOSG_TESSELLATOR_

#include "Polygon.h"

#include <osg/ref_ptr>
#include <osg/Array>
#include <osg/PrimitiveSet>
#include <osg/GLU>

#include <vector>

#ifndef CALLBACK
#define CALLBACK
#endif

namespace lwosg
{

	class Tessellator {
	public:
		bool tessellate(const Polygon &poly, const osg::Vec3Array *points, osg::DrawElementsUInt *out, const std::vector<int> *remap = 0);

		~Tessellator();

	protected:
		void finalize_primitive();

	private:
		friend void CALLBACK cb_begin_data(GLenum, void *);
		friend void CALLBACK cb_vertex_data(void *, void *);
		friend void CALLBACK cb_end_data(void *);
		friend void CALLBACK cb_error_data(GLenum, void *);

		osg::ref_ptr<osg::DrawElementsUInt> out_;

		GLenum prim_type_;
		GLenum last_error_;

		typedef std::vector<int> Index_list;
		Index_list incoming_;
	};

}

#endif
