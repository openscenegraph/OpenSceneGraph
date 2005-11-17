/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#include "Polygon.h"

using namespace lwosg;

Polygon::Polygon()
:    surf_(0),
    local_normals_(new VertexMap),
    weight_maps_(new VertexMap_map),
    texture_maps_(new VertexMap_map),
    rgb_maps_(new VertexMap_map),
    rgba_maps_(new VertexMap_map),
    invert_normal_(false),
    last_used_points_(0)
{
}
