/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWOSG_CLIP_
#define LWOSG_CLIP_

#include "lwo2chunks.h"

#include <string>
#include <map>

namespace lwosg
{

	class Clip {
	public:
		Clip(const lwo2::FORM::CLIP *clip = 0);

		void compile(const lwo2::FORM::CLIP *clip);

		inline const std::string &get_still_filename() const { return still_filename_; }

	private:
		std::string still_filename_;
	};

	typedef std::map<int, Clip> Clip_map;

}

#endif
