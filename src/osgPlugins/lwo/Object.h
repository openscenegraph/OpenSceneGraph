/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_OBJECT_
#define LWOSG_OBJECT_

#include "Layer.h"
#include "Surface.h"
#include "Clip.h"
#include "Unit.h"

#include "iffparser.h"

#include <osg/Referenced>
#include <osg/ref_ptr>

#include <string>
#include <vector>
#include <map>

namespace lwosg
{

	class CoordinateSystemFixer: public osg::Referenced {
	public:
		virtual osg::Vec3 fix_point(const osg::Vec3 &P) const = 0;
		virtual osg::Vec4 fix_point(const osg::Vec4 &P) const = 0;
		virtual osg::Vec3 fix_vector(const osg::Vec3 &V) const = 0;
		virtual osg::Vec4 fix_vector(const osg::Vec4 &V) const = 0;
		virtual inline bool invert_winding() const { return false; }

	protected:
		virtual ~CoordinateSystemFixer() {}
		CoordinateSystemFixer &operator=(const CoordinateSystemFixer &) { return *this; }
	};

	class LwoCoordFixer: public CoordinateSystemFixer {
	public:
		inline osg::Vec3 fix_point(const osg::Vec3 &P) const;
		inline osg::Vec4 fix_point(const osg::Vec4 &P) const;
		inline osg::Vec3 fix_vector(const osg::Vec3 &V) const;
		inline osg::Vec4 fix_vector(const osg::Vec4 &V) const;
		inline bool invert_winding() const { return true; }

	protected:
		virtual ~LwoCoordFixer() {}
		LwoCoordFixer &operator=(const LwoCoordFixer &) { return *this; }
	};



	class Object {
	public:

		typedef std::map<int, Layer> Layer_map;
		typedef std::map<std::string, Surface> Surface_map;

		Object();
		Object(const iff::Chunk_list &data);

		inline CoordinateSystemFixer *get_coordinate_system_fixer() { return csf_.get(); }
		inline const CoordinateSystemFixer *get_coordinate_system_fixer() const { return csf_.get(); }
		inline void set_coordinate_system_fixer(CoordinateSystemFixer *csf) { csf_ = csf; }

		void build(const iff::Chunk_list &data);

		inline Layer_map &layers() { return layers_; }
		inline const Layer_map &layers() const { return layers_; }

		inline Surface_map &surfaces() { return surfaces_; }
		inline const Surface_map &surfaces() const { return surfaces_; }

		inline const std::string &get_comment() const { return comment_; }
		inline const std::string &get_description() const { return description_; }

	protected:
		void scan_clips(const iff::Chunk_list &data);
		void scan_surfaces(const iff::Chunk_list &data);
		void parse(const iff::Chunk_list &data);

		void generate_normals();
		void generate_auto_texture_maps();

	private:
		Layer_map layers_;

		typedef std::map<int, Clip> Clip_map;
		Clip_map clips_;

		Surface_map surfaces_;

		std::string comment_;
		std::string description_;

		osg::ref_ptr<CoordinateSystemFixer> csf_;
	};

	// INLINE METHODS

	inline osg::Vec3 LwoCoordFixer::fix_point(const osg::Vec3 &P) const
	{
		return osg::Vec3(P.x(), P.z(), P.y());
	}

	inline osg::Vec4 LwoCoordFixer::fix_point(const osg::Vec4 &P) const
	{
		return osg::Vec4(fix_point(osg::Vec3(P.x(), P.y(), P.z())), P.w());
	}

	inline osg::Vec3 LwoCoordFixer::fix_vector(const osg::Vec3 &V) const
	{
		return fix_point(V);
	}

	inline osg::Vec4 LwoCoordFixer::fix_vector(const osg::Vec4 &V) const
	{
		return fix_point(V);
	}

}

#endif
