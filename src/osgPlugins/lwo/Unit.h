/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_UNIT_
#define LWOSG_UNIT_

#include "Polygon.h"
#include "VertexMap.h"

#include <osg/ref_ptr>
#include <osg/Array>

#include <vector>

namespace lwosg
{

	class Unit {
	public:

		typedef std::vector<Polygon> Polygon_list;
		typedef std::vector<int> Index_list;
		typedef std::vector<Index_list> Sharing_list;

		Unit();

		inline osg::Vec3Array *points() { return points_.get(); }
		inline const osg::Vec3Array *points() const { return points_.get(); }

		inline VertexMap *normals() { return normals_.get(); }
		inline const VertexMap *normals() const { return normals_.get(); }

		inline Polygon_list &polygons() { return polygons_; }
		inline const Polygon_list &polygons() const { return polygons_; }

		inline Sharing_list &shares() { return shares_; }
		inline const Sharing_list &shares() const { return shares_; }

		inline const VertexMap_map *weight_maps() const { return weight_maps_.get(); }
		inline VertexMap_map *weight_maps() { return weight_maps_.get(); }

		inline const VertexMap_map *subpatch_weight_maps() const { return subpatch_weight_maps_.get(); }
		inline VertexMap_map *subpatch_weight_maps() { return subpatch_weight_maps_.get(); }

		inline const VertexMap_map *texture_maps() const { return texture_maps_.get(); }
		inline VertexMap_map *texture_maps() { return texture_maps_.get(); }

		inline const VertexMap_map *rgb_maps() const { return rgb_maps_.get(); }
		inline VertexMap_map *rgb_maps() { return rgb_maps_.get(); }

		inline const VertexMap_map *rgba_maps() const { return rgba_maps_.get(); }
		inline VertexMap_map *rgba_maps() { return rgba_maps_.get(); }

		inline const VertexMap_map *displacement_maps() const { return displacement_maps_.get(); }
		inline VertexMap_map *displacement_maps() { return displacement_maps_.get(); }

		inline const VertexMap_map *spot_maps() const { return spot_maps_.get(); }
		inline VertexMap_map *spot_maps() { return spot_maps_.get(); }

		void flatten_maps();
		void generate_normals();

		void compute_vertex_remapping(const Surface *surf, Index_list &remap) const;

	protected:
		float angle_between_polygons(const Polygon &p1, const Polygon &p2) const;
		void find_shared_polygons(int vertex_index, std::vector<int> &poly_indices);
		void flatten_map(Polygon &poly, const VertexMap *local_map, VertexMap *global_map);

	private:
		osg::ref_ptr<osg::Vec3Array> points_;
		Polygon_list polygons_;
		Sharing_list shares_;

		osg::ref_ptr<VertexMap> normals_;
		osg::ref_ptr<VertexMap_map> weight_maps_;
		osg::ref_ptr<VertexMap_map> subpatch_weight_maps_;
		osg::ref_ptr<VertexMap_map> texture_maps_;
		osg::ref_ptr<VertexMap_map> rgb_maps_;
		osg::ref_ptr<VertexMap_map> rgba_maps_;
		osg::ref_ptr<VertexMap_map> displacement_maps_;
		osg::ref_ptr<VertexMap_map> spot_maps_;
	};

}

#endif
