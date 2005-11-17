/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Clip.h"

using namespace lwosg;

Clip::Clip(const lwo2::FORM::CLIP *clip)
{
    if (clip) {
        compile(clip);
    }
}

void Clip::compile(const lwo2::FORM::CLIP *clip)
{
    for (iff::Chunk_list::const_iterator j=clip->attributes.begin(); j!=clip->attributes.end(); ++j) {
        const lwo2::FORM::CLIP::STIL *stil = dynamic_cast<const lwo2::FORM::CLIP::STIL *>(*j);
        if (stil) still_filename_ = stil->name.name;
    }
}
