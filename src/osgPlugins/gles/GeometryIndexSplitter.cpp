#include "GeometryIndexSplitter"


bool GeometryIndexSplitter::split(osg::Geometry& geometry) {
    if(!needToSplit(geometry)) {
        _geometryList.push_back(&geometry);
        return false;
    }

    // keep bounding box data as user value if needed in subsequent processing
    attachBufferBoundingBox(geometry);

    osg::DrawElements *wire_primitive = 0,
                      *line_primitive = 0,
                      *point_primitive = 0;
    for(unsigned int i = 0 ; i < geometry.getNumPrimitiveSets() ; ++ i) {
        osg::DrawElements* primitive = (geometry.getPrimitiveSet(i) ? geometry.getPrimitiveSet(i)->getDrawElements() : 0);
        if(primitive) {
            if(primitive->getMode() == osg::PrimitiveSet::LINES) {
                bool isWireframe = false;
                if(primitive->getUserValue("wireframe", isWireframe) && isWireframe) {
                    wire_primitive = primitive;
                }
                else {
                    line_primitive = primitive;
                }
            }
            else if(primitive->getMode() == osg::PrimitiveSet::POINTS) {
                point_primitive = primitive;
            }
        }
    }

    TriangleMeshGraph graph(geometry, false);

    // only wireframe can be processed directly as they simply "duplicate" triangle or edge data;
    // lines/points may reference points not used for triangles so we keep a set of primitives
    // that remain to process
    IndexSet triangles;
    LineSet lines, wires;
    IndexSet points;

    for(unsigned int i = 0 ; i < graph.getNumTriangles() ; ++ i) {
        triangles.insert(i);
    }
    if(line_primitive) {
        for(unsigned int i = 0 ; i < line_primitive->getNumIndices() ; i += 2) {
            lines.insert(Line(line_primitive->index(i), line_primitive->index(i + 1)));
        }
    }
    if(wire_primitive) {
        for(unsigned int i = 0 ; i < wire_primitive->getNumIndices() ; i += 2) {
            wires.insert(Line(wire_primitive->index(i), wire_primitive->index(i + 1)));
        }
    }
    if(point_primitive) {
        for(unsigned int i = 0 ; i < point_primitive->getNumIndices() ; ++ i) {
            points.insert(point_primitive->index(i));
        }
    }

    // assign a cluster id for each triangle
    // 1. bootstrap cluster by selecting first remaining triangle
    // 2. while cluster size is < max cluster size
    //   2.1 look for a triangle that is a neighbor of one cluster triangle (considering N last cluster triangles from cache)
    //   2.2 if 2.1 was not successful and there still is room for 3 new vertices (i.e. a full new triangle) then
    //       add any (might share an edge/vertex/nothing with existing cluster) triangle
    //   2.3 if we still have lines/points, add anything we can 'naively'
    // 3. insert wireframe edges corresponding to selected triangles
    // 4. extract subgeometry

    while(triangles.size() || lines.size() || points.size()) {
        Cluster cluster(_maxAllowedIndex);
        IndexCache cache;
        unsigned int candidate = std::numeric_limits<unsigned int>::max();

        // let's consider that every insert needs the place for a full new primitive for simplicity
        while(!cluster.fullOfTriangles() &&
              (candidate = findCandidate(triangles, cache, graph)) != std::numeric_limits<unsigned int>::max()) {
            cache.push_back(candidate);
            Triangle t = graph.triangle(candidate);
            cluster.addTriangle(t.v1(), t.v2(), t.v3());
        }

        while(!cluster.fullOfLines() && lines.size()) {
            Line line = getNext(lines, Line(std::numeric_limits<unsigned int>::max(), std::numeric_limits<unsigned int>::max()));
            cluster.addLine(line._a, line._b);
        }

        while(!cluster.full() && points.size()) {
            unsigned int point = getNext(points, std::numeric_limits<unsigned int>::max());
            cluster.addPoint(point);
        }

        // update lines/points: if all vertices referenced by a point/line primitive are
        // already extracted, let's insert it in the subgeometry and update the set of
        // primitives still remaining Lines may e.g. reference one vertex in cluster A and
        // the other in cluster B hence need specific care
        if(line_primitive) {
            for(LineSet::iterator line = lines.begin() ; line != lines.end() ; ) {
                if(cluster.contains(line->_a, line->_b)) {
                    cluster.addLine(line->_a, line->_b);
                    lines.erase(line ++);
                }
                else {
                    ++ line;
                }
            }
        }

        if(point_primitive) {
            // find all cluster vertices that should also have a point primitive
            for(IndexSet::iterator subvertex = cluster.subvertices.begin() ; subvertex != cluster.subvertices.end() ; ++ subvertex) {
                unsigned int index = *subvertex;
                if(points.find(index) != points.end()) {
                    cluster.addPoint(index);
                    points.erase(index);
                }
            }
        }

        // finally extract wireframe (may originate from triangles or lines but necessarily have
        // to reference vertices that are *all* in the geometry)
        if(wire_primitive) {
            for(LineSet::iterator wire = wires.begin() ; wire != wires.end() ; ) {
                if(cluster.contains(wire->_a, wire->_b)) {
                    cluster.addWire(wire->_a, wire->_b);
                    wires.erase(wire ++);
                }
                else {
                    ++ wire;
                }
            }
        }

        _geometryList.push_back(SubGeometry(geometry,
                                            cluster.subtriangles,
                                            cluster.sublines,
                                            cluster.subwireframe,
                                            cluster.subpoints).geometry());
    }

    osg::notify(osg::NOTICE) << "geometry " << &geometry << " " << geometry.getName()
                                << " vertices (" << geometry.getVertexArray()->getNumElements()
                                << ") has DrawElements index > " << _maxAllowedIndex << ", split to "
                                << _geometryList.size() << " geometry" << std::endl;

    return true;
}


unsigned int GeometryIndexSplitter::findCandidate(IndexSet& triangles, const IndexCache& cache, const TriangleMeshGraph& graph) {
    // look for unclustered neighboring triangles
    for(IndexCache::const_reverse_iterator cached = cache.rbegin() ; cached != cache.rend() ; ++ cached) {
        IndexVector candidates = graph.triangleNeighbors(*cached);
        for(IndexVector::const_iterator candidate = candidates.begin() ; candidate != candidates.end() ; ++ candidate) {
            if(triangles.count(*candidate)) {
                triangles.erase(*candidate);
                return *candidate;
            }
        }
    }

    //fallback on any unclustered triangle
    return getNext(triangles, std::numeric_limits<unsigned int>::max());
}


bool GeometryIndexSplitter::needToSplit(const osg::Geometry& geometry) const {
    for(unsigned int i = 0; i < geometry.getNumPrimitiveSets(); ++ i) {
        const osg::DrawElements* primitive = geometry.getPrimitiveSet(i)->getDrawElements();
        if (primitive && needToSplit(*primitive)) {
            return true;
        }
    }
    return false;
}


bool GeometryIndexSplitter::needToSplit(const osg::DrawElements& primitive) const {
    for(unsigned int j = 0; j < primitive.getNumIndices(); j++) {
        if (primitive.index(j) > _maxAllowedIndex){
            return true;
        }
    }
    return false;
}


void GeometryIndexSplitter::attachBufferBoundingBox(osg::Geometry& geometry) const {
    // positions
    setBufferBoundingBox(dynamic_cast<osg::Vec3Array*>(geometry.getVertexArray()));
    // uvs
    for(unsigned int i = 0 ; i < geometry.getNumTexCoordArrays() ; ++ i) {
        setBufferBoundingBox(dynamic_cast<osg::Vec2Array*>(geometry.getTexCoordArray(i)));
    }
}


template<typename T>
void GeometryIndexSplitter::setBufferBoundingBox(T* buffer) const {
    if(!buffer) return;

    typename T::ElementDataType bbl;
    typename T::ElementDataType ufr;
    const unsigned int dimension = buffer->getDataSize();

    if(buffer->getNumElements()) {
        for(unsigned int i = 0 ; i < dimension ; ++i) {
            bbl[i] = ufr[i] = (*buffer->begin())[i];
        }

        for(typename T::const_iterator it = buffer->begin() + 1 ; it != buffer->end() ; ++ it) {
            for(unsigned int i = 0 ; i < dimension ; ++ i) {
                bbl[i] = std::min(bbl[i], (*it)[i]);
                ufr[i] = std::max(ufr[i], (*it)[i]);
            }
        }

        buffer->setUserValue("bbl", bbl);
        buffer->setUserValue("ufr", ufr);
    }
}
