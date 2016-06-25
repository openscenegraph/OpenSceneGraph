/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_BLOCK_
#define LWOSG_BLOCK_

#include "lwo2chunks.h"

#include <osg/Vec3>

#ifdef DIFFERENCE
#undef DIFFERENCE
#endif

namespace lwosg
{

	class Clip;

	struct Texture_mapping {

		enum Coordinate_system_type {
			OBJECT = 0,
			WORLD = 1
		};

		osg::Vec3 center_;
		osg::Vec3 size_;
		osg::Vec3 rotation_;
		// missing: OREF, FALL
		Coordinate_system_type csys_;

		Texture_mapping()
			: size_(1, 1, 1),
			  csys_(OBJECT)
		{}
	};

	struct Image_map {

		enum Axis_type {
			X = 0,
			Y = 1,
			Z = 2
		};

		enum Projection_mode {
			PLANAR = 0,
			CYLINDRICAL = 1,
			SPHERICAL = 2,
			CUBIC = 3,
			FRONT_PROJECTION = 4,
			UV = 5
		};

		enum Wrap_type {
			RESET = 0,
			REPEAT = 1,
			MIRROR = 2,
			EDGE = 3
		};

		Texture_mapping mapping;
		Projection_mode projection;
		Axis_type axis;
		int image_map;
		const Clip *clip;		// is filled by Surface::compile()
		Wrap_type width_wrap;
		Wrap_type height_wrap;
		float wrap_amount_w;
		float wrap_amount_h;
		std::string uv_map;
		// missing: AAST, PIXB, STCK
		float texture_amplitude;

		Image_map()
			:
			  projection(PLANAR),
			  axis(X),
			  image_map(-1),
			  clip(0),
			  width_wrap(REPEAT),
			  height_wrap(REPEAT),
			  wrap_amount_w(1),
			  wrap_amount_h(1),
			  texture_amplitude(1)
		{}
	};

	class Block {
	public:

		enum Axis_type {
			X = 0,
			Y = 1,
			Z = 2
		};

		enum Opacity_type {
			NORMAL = 0,
			SUBTRACTIVE = 1,
			DIFFERENCE = 2,
			MULTIPLY = 3,
			DIVIDE = 4,
			ALPHA = 5,
			TEXTURE_DISPLACEMENT = 6,
			ADDITIVE = 7
		};

		Block(const lwo2::FORM::SURF::BLOK *blok = 0);

		void compile(const lwo2::FORM::SURF::BLOK *blok = 0);

		inline const std::string &get_type() const { return type_; }
		inline const std::string &get_ordinal() const { return ordinal_; }
		inline const std::string &get_channel() const { return channel_; }
		inline bool enabled() const { return enabled_; }
		inline Opacity_type get_opacity_type() const { return opacity_type_; }
		inline float get_opacity_amount() const { return opacity_amount_; }
		inline Axis_type get_displacement_axis() const { return displacement_axis_; }
		inline const Image_map &get_image_map() const { return imap_; }
		inline Image_map &get_image_map() { return imap_; }

		osg::Vec3 setup_texture_point(const osg::Vec3 &P) const;

	protected:
		void read_common_attributes(const iff::Chunk_list &subchunks);

	private:
		std::string type_;
		std::string ordinal_;
		std::string channel_;
		bool enabled_;
		Opacity_type opacity_type_;
		float opacity_amount_;
		Axis_type displacement_axis_;

		Image_map imap_;
	};

}

#endif
