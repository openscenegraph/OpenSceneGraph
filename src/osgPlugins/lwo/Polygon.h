/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_POLYGON_
#define LWOSG_POLYGON_

#include "lwo2chunks.h"
#include "Surface.h"
#include "VertexMap.h"

#include <osg/Vec3>
#include <osg/Array>

#include <string>
#include <map>

namespace lwosg
{

	class Polygon {
	public:
		typedef std::vector<int> Index_list;
		typedef std::map<int, int> Duplication_map;

		Polygon();

		inline void set_invert_normal(bool v = true) { invert_normal_ = v; }

		inline const Index_list &indices() const { return indices_; }
		inline Index_list &indices() { dirty_normal(); return indices_; }

		inline const Duplication_map &dup_vertices() const { return dup_vertices_; }
		inline Duplication_map &dup_vertices() { return dup_vertices_; }

		inline const VertexMap *local_normals() const { return local_normals_.get(); }
		inline VertexMap *local_normals() { return local_normals_.get(); }

		inline const VertexMap_map *weight_maps() const { return weight_maps_.get(); }
		inline VertexMap_map *weight_maps() { return weight_maps_.get(); }

		inline const VertexMap_map *texture_maps() const { return texture_maps_.get(); }
		inline VertexMap_map *texture_maps() { return texture_maps_.get(); }

		inline const VertexMap_map *rgb_maps() const { return rgb_maps_.get(); }
		inline VertexMap_map *rgb_maps() { return rgb_maps_.get(); }

		inline const VertexMap_map *rgba_maps() const { return rgba_maps_.get(); }
		inline VertexMap_map *rgba_maps() { return rgba_maps_.get(); }

		inline bool has_surface() const { return surf_ != 0; }
		inline const Surface *get_surface() const { return surf_; }
		inline void set_surface(const Surface *s) { surf_ = s; }

		inline bool has_part() const { return !part_.empty(); }
		inline const std::string &get_part_name() const { return part_; }
		inline void set_part_name(const std::string &n) { part_ = n; }

		inline bool has_smoothing_group() const { return !smoothing_group_.empty(); }
		inline const std::string &get_smoothing_group() const { return smoothing_group_; }
		inline void set_smoothing_group(const std::string &n) { smoothing_group_ = n; }

		inline const osg::Vec3 &face_normal(const osg::Vec3Array *points) const;

	protected:
		inline void dirty_normal() { last_used_points_ = 0; }

	private:
		Index_list indices_;
		Duplication_map dup_vertices_;

		const Surface *surf_;
		std::string part_;
		std::string smoothing_group_;

		osg::ref_ptr<VertexMap> local_normals_;
		osg::ref_ptr<VertexMap_map> weight_maps_;
		osg::ref_ptr<VertexMap_map> texture_maps_;
		osg::ref_ptr<VertexMap_map> rgb_maps_;
		osg::ref_ptr<VertexMap_map> rgba_maps_;

		bool invert_normal_;

		mutable const osg::Vec3Array *last_used_points_;
		mutable osg::Vec3 normal_;
	};

	// INLINE METHODS

	inline const osg::Vec3 &Polygon::face_normal(const osg::Vec3Array *points) const
	{
		if (last_used_points_ != points) {
			normal_.set(0, 0, 0);
			if (indices_.size() >= 3) {
				const osg::Vec3 &A = points->at(indices_.front());
				const osg::Vec3 &B = points->at(indices_[1]);
				const osg::Vec3 &C = points->at(indices_.back());
				if (invert_normal_) {
					normal_ = (C - A) ^ (B - A);
				} else {
					normal_ = (B - A) ^ (C - A);
				}
				float len = normal_.length();
				if (len != 0) {
					normal_ /= len;
				}
			}
			last_used_points_ = points;
		}
		return normal_;
	}

}

#endif
