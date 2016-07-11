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

    // only wireframe can be processed directly as they simply "duplicate" triangle or edge data;
    // lines/points may reference points not used for triangles so we keep a set of primitives
    // that remain to process
    LineSet source_lines;
    IndexSet source_points;

    if(line_primitive) {
        for(unsigned int i = 0 ; i < line_primitive->getNumIndices() ; i += 2) {
            source_lines.insert(Line(line_primitive->index(i), line_primitive->index(i + 1)));
        }
    }
    if(point_primitive) {
        for(unsigned int i = 0 ; i < point_primitive->getNumIndices() ; ++ i) {
            source_points.insert(point_primitive->index(i));
        }
    }

    TriangleMeshGraph graph(geometry, false);
    unsigned int remaining_triangles = graph.getNumTriangles(),
                    cluster = 0;
    IndexVector clusters(remaining_triangles, 0);
    IndexCache cache;

    // assign a cluster id for each triangle
    // 1. bootstrap cluster by selecting first remaining triangle
    // 2. while cluster size is < max cluster size
    //   2.1 look for a triangle that is a neighbor of one cluster triangle (considering N last cluster triangles from cache)
    //   2.2 if 2.1 was not successful and there still is room for 3 new vertices (i.e. a full new triangle) then
    //       add any (might share an edge/vertex/nothing with existing cluster) triangle
    //   2.3 if we still have lines/points, add anything we can 'naively'
    // 3. insert wireframe edges corresponding to selected triangles
    // 4. extract subgeometry

    while(remaining_triangles || !source_lines.empty() || !source_points.empty()) {
        IndexVector subtriangles, subwireframe, sublines, subpoints;
        IndexSet cluster_vertices;

        ++ cluster;

        if(remaining_triangles) {
            // find first unmarked triangle (as remaining_triangles > 0 there *must* be at least one)
            cache.push_back(findCandidate(clusters));
            setTriangleCluster(graph, cache.back(), cluster, clusters, cluster_vertices, remaining_triangles);

            while(remaining_triangles && cluster_vertices.size() < _maxAllowedIndex) {
                unsigned int candidate = std::numeric_limits<unsigned int>::max();

                for(IndexCache::const_reverse_iterator cached = cache.rbegin() ; cached != cache.rend() ; ++ cached) {
                    candidate = findCandidate(graph.triangleNeighbors(*cached), clusters);
                    if(candidate != std::numeric_limits<unsigned int>::max()) break;
                }

                if(candidate == std::numeric_limits<unsigned int>::max()) {
                    // do we have room for a triangle having all vertices not in the cluster?
                    if(!(cluster_vertices.size() + 2 < _maxAllowedIndex)) {
                        break;
                    }

                    candidate = findCandidate(clusters);
                }

                cache.push_back(candidate);
                setTriangleCluster(graph, candidate, cluster, clusters, cluster_vertices, remaining_triangles);
            }

            // build list of cluster triangles
            for(unsigned int triangle = 0 ; triangle < clusters.size() ; ++ triangle) {
                if(clusters[triangle] == cluster) {
                    const Triangle& t = graph.triangle(triangle);
                    subtriangles.push_back(t.v1());
                    subtriangles.push_back(t.v2());
                    subtriangles.push_back(t.v3());
                }
            }

            // update lines/points: if all vertices referenced by a point/line primitive are
            // already extracted, let's insert it in the subgeometry and update the set of
            // primitives still remaining Lines may e.g. reference one vertex in cluster A and
            // the other in cluster B hence need specific care
            if(line_primitive) {
                extract_primitives(cluster_vertices, line_primitive, sublines, 2);
                for(unsigned int i = 0 ; i < sublines.size() / 2 ; i += 2) {
                    source_lines.erase(Line(sublines[i], sublines[i + 1]));
                }
            }

            if(point_primitive) {
                extract_primitives(cluster_vertices, point_primitive, subpoints, 1);
                for(unsigned int i = 0 ; i < subpoints.size() ; ++ i) {
                    source_points.erase(subpoints[i]);
                }
            }
        }

        // let's consider that every new lines adds 2 vertices for simplicity
        while(!source_lines.empty() && cluster_vertices.size() - 1 < _maxAllowedIndex) {
            Line line = *source_lines.begin();
            source_lines.erase(source_lines.begin());
            cluster_vertices.insert(line._a);
            cluster_vertices.insert(line._b);
            sublines.push_back(line._a);
            sublines.push_back(line._b);
        }

        while(!source_points.empty() && cluster_vertices.size() < _maxAllowedIndex) {
            unsigned int point = *source_points.begin();
            source_points.erase(source_points.begin());
            cluster_vertices.insert(point);
            subpoints.push_back(point);
        }

        // finally extract wireframe (may originate from triangles or lines but necessarily have
        // to reference vertices that are *all* in the geometry)
        if(wire_primitive) {
            extract_primitives(cluster_vertices, wire_primitive, subwireframe, 2);
        }

        _geometryList.push_back(SubGeometry(geometry,
                                            subtriangles,
                                            sublines,
                                            subwireframe,
                                            subpoints).geometry());
    }

    osg::notify(osg::NOTICE) << "geometry " << &geometry << " " << geometry.getName()
                                << " vertexes (" << geometry.getVertexArray()->getNumElements()
                                << ") has DrawElements index > " << _maxAllowedIndex << ", splitted to "
                                << _geometryList.size() << " geometry" << std::endl;

    return true;
}


unsigned int GeometryIndexSplitter::findCandidate(const IndexVector& clusters) {
    for(unsigned int i = 0 ; i < clusters.size() ; ++ i) {
        if(!clusters[i]) {
            return i;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}


unsigned int GeometryIndexSplitter::findCandidate(const IndexVector& candidates, const IndexVector& clusters) {
    for(IndexVector::const_iterator candidate = candidates.begin() ; candidate != candidates.end() ; ++ candidate) {
        if(!clusters[*candidate]) {
            return *candidate;
        }
    }
    return std::numeric_limits<unsigned int>::max();
}


void GeometryIndexSplitter::setTriangleCluster(const TriangleMeshGraph& graph,
                                               unsigned int triangle,
                                               unsigned int cluster,
                                               IndexVector& clusters,
                                               IndexSet& cluster_vertices,
                                               unsigned int& remaining) {
    clusters[triangle] = cluster;
    const Triangle& t = graph.triangle(triangle);
    cluster_vertices.insert(t.v1());
    cluster_vertices.insert(t.v2());
    cluster_vertices.insert(t.v3());
    remaining --;
}


void GeometryIndexSplitter::extract_primitives(const IndexSet& vertices,
                                               const osg::DrawElements* elements,
                                               IndexVector& indices,
                                               unsigned int primitive_size) {
    for(unsigned int i = 0 ; i < elements->getNumIndices() ; i += primitive_size) {
        bool is_included = true;
        for(unsigned int j = 0 ; j < primitive_size ; ++ j) {
            if(!vertices.count(elements->index(i + j))) {
                is_included = false;
                break;
            }
        }

        if(is_included) {
            for(unsigned int j = 0 ; j < primitive_size ; ++ j) {
                indices.push_back(elements->index(i + j));
            }
        }
    }
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


void GeometryIndexSplitter::setValidIndices(std::set<unsigned int>& indices, const osg::DrawElements* primitive) const {
    for(unsigned int j = 0 ; j < primitive->getNumIndices() ; ++ j) {
        indices.insert(primitive->index(j));
    }
}
