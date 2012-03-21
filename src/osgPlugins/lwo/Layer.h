/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_LAYER_
#define LWOSG_LAYER_

#include "lwo2chunks.h"

#include "Unit.h"

namespace lwosg
{

	class Layer {
	public:
		typedef std::vector<Unit> Unit_list;

		inline Layer();

		inline const lwo2::FORM::LAYR *get_layer_chunk() const;
		inline void set_layer_chunk(const lwo2::FORM::LAYR *layr);

		inline int number() const;

		inline Unit_list &units();
		inline const Unit_list &units() const;
		inline osg::Vec3 pivot() const;

	private:
		const lwo2::FORM::LAYR *layr_;
		Unit_list units_;
	};

	// INLINE METHODS

	inline Layer::Layer()
		: layr_(0)
	{
	}

	inline Layer::Unit_list &Layer::units()
	{
		return units_;
	}

	inline const Layer::Unit_list &Layer::units() const
	{
		return units_;
	}

	inline osg::Vec3 Layer::pivot() const
	{
		if (layr_) {
			return osg::Vec3(layr_->pivot.X, layr_->pivot.Y, layr_->pivot.Z);
		}
		return osg::Vec3(0, 0, 0);
	}

	inline const lwo2::FORM::LAYR *Layer::get_layer_chunk() const
	{
		return layr_;
	}

	inline void Layer::set_layer_chunk(const lwo2::FORM::LAYR *layr)
	{
		layr_ = layr;
	}

	inline int Layer::number() const
	{
		if (layr_) {
			return layr_->number;
		}
		return -1;
	}


}

#endif
