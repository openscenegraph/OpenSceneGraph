SET(TARGET_SRC 
  ShapeToGeometry.cpp
  DrawIndirectPrimitiveSet.cpp
  AggregateGeometryVisitor.cpp
  osggpucull.cpp
)

SET(TARGET_H
  ShapeToGeometry.h
  DrawIndirectPrimitiveSet.h
  AggregateGeometryVisitor.h
  GpuCullShaders.h
)

#### end var setup  ###
SETUP_EXAMPLE(osggpucull)
