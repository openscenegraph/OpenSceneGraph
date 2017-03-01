#include "TriangleMeshSmoother"


TriangleMeshSmoother::TriangleMeshSmoother(osg::Geometry& geometry, float creaseAngle, bool comparePosition, int mode):
    _geometry(geometry),
    _creaseAngle(creaseAngle),
    _graph(0),
    _mode(mode)
{
    if(!_geometry.getVertexArray() || !_geometry.getVertexArray()->getNumElements()) {
        return;
    }

    osgUtil::SharedArrayOptimizer deduplicator;
    deduplicator.findDuplicatedUVs(geometry);

    // duplicate shared arrays as it isn't safe to duplicate vertices when arrays are shared.
    if (geometry.containsSharedArrays()) {
        geometry.duplicateSharedArrays();
    }

    if(!_geometry.getNormalArray() || _geometry.getNormalArray()->getNumElements() != _geometry.getVertexArray()->getNumElements()) {
        _geometry.setNormalArray(new osg::Vec3Array(_geometry.getVertexArray()->getNumElements()), osg::Array::BIND_PER_VERTEX);
    }

    // build a unifier to consider deduplicated vertex indices
    _graph = new TriangleMeshGraph(_geometry, comparePosition);

    unsigned int nbTriangles = 0;
    for(unsigned int i = 0 ; i < _geometry.getNumPrimitiveSets() ; ++ i) {
        osg::PrimitiveSet* primitive = _geometry.getPrimitiveSet(i);

        if(!primitive || !primitive->getNumIndices()) {
            continue;
        }
        else if(primitive->getMode() > osg::PrimitiveSet::TRIANGLES) {
            OSG_INFO << "[smoother] Cannot smooth geometry '" << _geometry.getName()
                        << "' due to not tessellated primitives" << std::endl;
            return;
        }
        else if(primitive->getMode() == osg::PrimitiveSet::TRIANGLES) {
            nbTriangles += primitive->getNumIndices() / 3;
        }
    }
    _triangles.reserve(nbTriangles);

    // collect all buffers that are BIND_PER_VERTEX for eventual vertex duplication
    addArray(_geometry.getVertexArray());
    addArray(_geometry.getColorArray());
    addArray(_geometry.getSecondaryColorArray());
    addArray(_geometry.getFogCoordArray());
    for(unsigned int i = 0; i < _geometry.getNumTexCoordArrays(); ++ i) {
        addArray(_geometry.getTexCoordArray(i));
    }
    for(unsigned int i = 0; i < _geometry.getNumVertexAttribArrays(); ++ i) {
        addArray(_geometry.getVertexAttribArray(i));
    }

    switch(_mode) {
        case recompute:
            computeVertexNormals();
            break;
        case smooth_all:
            smoothVertexNormals(true, true);
            break;
        case smooth_flipped:
            smoothVertexNormals(true, false);
            break;
        case diagnose:
            smoothVertexNormals(false, false);
            break;
    };

    // deduplicate UVs array that were only shared within the geometry
    deduplicator.deduplicateUVs(geometry);
}


unsigned int TriangleMeshSmoother::duplicateVertex(unsigned int index) {
    DuplicateVertex duplicate(index);
    for(ArrayVector::iterator array = _vertexArrays.begin(); array != _vertexArrays.end(); ++ array) {
        (*array)->accept(duplicate);
    }

    _graph->add(duplicate._end, index);
    return duplicate._end;
}


void TriangleMeshSmoother::smoothVertexNormals(bool fix, bool force) {
    _vertexArrays.clear(); // make sure we do not change vertex count
    bool flipped = false;

    osg::Vec3Array* normals = dynamic_cast<osg::Vec3Array*>(_geometry.getNormalArray());
    osg::Vec3Array* positions = dynamic_cast<osg::Vec3Array*>(_geometry.getVertexArray());

    if(!positions || !normals || normals->getNumElements() != positions->getNumElements()) {
        OSG_WARN << std::endl
                    << "Warning: [smoothVertexNormals] [[normals]] Geometry '" << _geometry.getName()
                    << "' has invalid positions/normals";
        return;
    }

    for(unsigned int index = 0 ; index < positions->getNumElements() ; ++ index) {
        std::vector<IndexVector> oneRing = _graph->vertexOneRing(_graph->unify(index), _creaseAngle);
        osg::Vec3f smoothedNormal(0.f, 0.f, 0.f);

        // sum normals for each cluster in the one-ring
        for(std::vector<IndexVector>::iterator cluster = oneRing.begin() ; cluster != oneRing.end() ; ++ cluster) {
            smoothedNormal += cumulateTriangleNormals(*cluster);
        }

        float length = smoothedNormal.normalize();
        if(length > 0.) {
            if(force || smoothedNormal * normals->at(index) < 1.e-6) {
                flipped = true;
                if(fix) {
                    (*normals)[index] = smoothedNormal;
                }
            }
        }
    }

    if(flipped) {
        OSG_WARN << std::endl << "Warning: [smoothVertexNormals] [[normals]] Geometry '" << _geometry.getName() << "' ";
        switch(_mode) {
            case diagnose:
                OSG_WARN << "has some flipped normals; please check that the shading is correct" << std::endl;
                OSG_WARN << "Monitor: normal.invalid" << std::endl;
                break;
            case smooth_flipped:
                OSG_WARN << "has some flipped normals that have been fixed" << std::endl;
                OSG_WARN << "Monitor: normal.smooth_flipped" << std::endl;
                break;
            case smooth_all:
                OSG_WARN << "normals have all been smoothed" << std::endl;
                OSG_WARN << "Monitor: normal.smooth_all" << std::endl;
                break;
        }
    }
}


void TriangleMeshSmoother::computeVertexNormals() {
    osg::Vec3Array* normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX,
                                                    _geometry.getVertexArray()->getNumElements());
    addArray(normals);

    for(unsigned int i = 0 ; i < normals->getNumElements() ; ++ i) {
        (*normals)[i].set(0.f, 0.f, 0.f);
    }

    for(VertexIterator uniqueIndex = _graph->begin() ; uniqueIndex != _graph->end() ; ++ uniqueIndex) {
        unsigned int index = uniqueIndex->_index;
        std::set<unsigned int> processed;

        std::vector<IndexVector> oneRing = _graph->vertexOneRing(index, _creaseAngle);
        for(std::vector<IndexVector>::iterator cluster = oneRing.begin() ; cluster != oneRing.end() ; ++ cluster) {
            osg::Vec3f clusterNormal = cumulateTriangleNormals(*cluster);
            clusterNormal.normalize();

            std::set<unsigned int> duplicates;
            for(IndexVector::const_iterator tri = cluster->begin() ; tri != cluster->end() ; ++ tri) {
                const Triangle& triangle = _graph->triangle(*tri);

                if(_graph->unify(triangle.v1()) == index) {
                    duplicates.insert(triangle.v1());
                }
                else if(_graph->unify(triangle.v2()) == index) {
                    duplicates.insert(triangle.v2());
                }
                else if(_graph->unify(triangle.v3()) == index) {
                    duplicates.insert(triangle.v3());
                }
            }

            for(std::set<unsigned int>::iterator vertex = duplicates.begin() ; vertex != duplicates.end() ; ++ vertex) {
                if(processed.find(*vertex) == processed.end()) {
                    // vertex not yet processed
                    (*normals)[*vertex] = clusterNormal;
                    processed.insert(*vertex);
                }
                else {
                    // vertex already processed in a previous cluster: need to duplicate
                    unsigned int duplicate = duplicateVertex(*vertex);
                    replaceVertexIndexInTriangles(*cluster, *vertex, duplicate);
                    (*normals)[duplicate] = clusterNormal;

                    processed.insert(duplicate);
                }
            }
        }
    }

    _geometry.setNormalArray(normals, osg::Array::BIND_PER_VERTEX);
    updateGeometryPrimitives();

    OSG_WARN << std::endl <<"Warning: [computeVertexNormals] [[normals]] Geometry '" << _geometry.getName()
                << "' normals have been recomputed" << std::endl;
    OSG_WARN << "Monitor: normal.recompute" << std::endl;
}


osg::Vec3f TriangleMeshSmoother::cumulateTriangleNormals(const IndexVector& triangles) const {
    osg::Vec3f normal;
    normal.set(0.f, 0.f, 0.f);
    for(IndexVector::const_iterator triangle = triangles.begin() ; triangle != triangles.end() ; ++ triangle) {
        const Triangle& t = _graph->triangle(*triangle);
        normal += (t._normal * t._area);
    }
    return normal;
}


void TriangleMeshSmoother::replaceVertexIndexInTriangles(const IndexVector& triangles, unsigned int oldIndex, unsigned int newIndex) {
    for(IndexVector::const_iterator tri = triangles.begin() ; tri != triangles.end() ; ++ tri) {
        Triangle& triangle = _graph->triangle(*tri);
        if(triangle.v1() == oldIndex) {
            triangle.v1() = newIndex;
        }
        else if(triangle.v2() == oldIndex) {
            triangle.v2() = newIndex;
        }
        else if(triangle.v3() == oldIndex) {
            triangle.v3() = newIndex;
        }
    }
}


void TriangleMeshSmoother::addArray(osg::Array* array) {
    if (array && array->getBinding() == osg::Array::BIND_PER_VERTEX) {
        _vertexArrays.push_back(array);
    }
}


void TriangleMeshSmoother::updateGeometryPrimitives() {
    osg::Geometry::PrimitiveSetList primitives;
    for(unsigned int i = 0 ; i < _geometry.getNumPrimitiveSets() ; ++ i) {
        osg::PrimitiveSet* primitive = _geometry.getPrimitiveSet(i);
        if(primitive && primitive->getMode() < osg::PrimitiveSet::TRIANGLES) {
            primitives.push_back(primitive);
        }
    }

    if(_graph->getNumTriangles()) {
        osg::DrawElementsUInt* triangles = new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES);
        for(unsigned int i = 0 ; i < _graph->getNumTriangles() ; ++ i) {
            const Triangle& triangle = _graph->triangle(i);
            triangles->push_back(triangle.v1());
            triangles->push_back(triangle.v2());
            triangles->push_back(triangle.v3());
        }
        primitives.push_back(triangles);
    }

    _geometry.setPrimitiveSetList(primitives);
}
