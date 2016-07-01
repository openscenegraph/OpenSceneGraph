#include <vector>
#include <algorithm> // sort
#include <limits> // numeric_limits

#include <osg/Geometry>
#include <osgAnimation/MorphGeometry>
#include <osg/PrimitiveSet>
#include <osg/ValueObject>
#include <osgUtil/MeshOptimizers>

#include "glesUtil"
#include "IndexMeshVisitor"
#include "PrimitiveIndexors"


using namespace glesUtil;

void remapGeometryVertices(RemapArray ra, osg::Geometry& geom)
{
    osgAnimation::MorphGeometry *morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(&geom);
    if(morphGeometry) {
        osgAnimation::MorphGeometry::MorphTargetList targetList = morphGeometry->getMorphTargetList();
        for (osgAnimation::MorphGeometry::MorphTargetList::iterator ti = targetList.begin(); ti != targetList.end(); ++ti)  {
            osgAnimation::MorphGeometry::MorphTarget *morphTarget = &(*ti);
            osg::Geometry *target = morphTarget->getGeometry();
            VertexAttribComparitor arrayComparitor(*target);
            arrayComparitor.accept(ra);
        }
    }
}

void IndexMeshVisitor::process(osg::Geometry& geom) {
    // TODO: this is deprecated
    if (geom.getNormalBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET) return;
    if (geom.getColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET) return;
    if (geom.getSecondaryColorBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET) return;
    if (geom.getFogCoordBinding() == osg::Geometry::BIND_PER_PRIMITIVE_SET) return;

    // no point optimizing if we don't have enough vertices.
    if (!geom.getVertexArray() || geom.getVertexArray()->getNumElements() < 2) return;


    osgUtil::SharedArrayOptimizer deduplicator;
    deduplicator.findDuplicatedUVs(geom);

    // duplicate shared arrays as it isn't safe to rearrange vertices when arrays are shared.
    if (geom.containsSharedArrays()) {
        geom.duplicateSharedArrays();
    }

    osg::Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    osg::Geometry::PrimitiveSetList::iterator itr;

    osg::Geometry::PrimitiveSetList new_primitives;
    new_primitives.reserve(primitives.size());

    // compute duplicate vertices
    typedef std::vector<unsigned int> IndexList;
    unsigned int numVertices = geom.getVertexArray()->getNumElements();
    IndexList indices(numVertices);
    unsigned int i, j;
    for(i = 0 ; i < numVertices ; ++ i) {
        indices[i] = i;
    }

    VertexAttribComparitor arrayComparitor(geom);
    std::sort(indices.begin(), indices.end(), arrayComparitor);

    unsigned int lastUnique = 0;
    unsigned int numUnique = 1;
    for(i = 1 ; i < numVertices ; ++ i) {
        if (arrayComparitor.compare(indices[lastUnique], indices[i]) != 0) {
            lastUnique = i;
            ++ numUnique;
        }
    }

    IndexList remapDuplicatesToOrignals(numVertices);
    lastUnique = 0;
    for(i = 1 ; i < numVertices ; ++ i) {
        if (arrayComparitor.compare(indices[lastUnique],indices[i]) != 0) {
            // found a new vertex entry, so previous run of duplicates needs
            // to be put together.
            unsigned int min_index = indices[lastUnique];
            for(j = lastUnique + 1 ; j < i ; ++ j) {
                min_index = osg::minimum(min_index, indices[j]);
            }
            for(j = lastUnique ; j < i ; ++ j) {
                remapDuplicatesToOrignals[indices[j]] = min_index;
            }
            lastUnique = i;
        }
    }

    unsigned int min_index = indices[lastUnique];
    for(j = lastUnique + 1 ; j < i ; ++ j) {
        min_index = osg::minimum(min_index, indices[j]);
    }
    for(j = lastUnique ; j < i ; ++ j) {
        remapDuplicatesToOrignals[indices[j]] = min_index;
    }

    // copy the arrays.
    IndexList finalMapping(numVertices);
    IndexList copyMapping;
    copyMapping.reserve(numUnique);
    unsigned int currentIndex = 0;
    for(i = 0 ; i < numVertices ; ++ i) {
        if (remapDuplicatesToOrignals[i] == i) {
            finalMapping[i] = currentIndex;
            copyMapping.push_back(i);
            currentIndex++;
        }
        else {
            finalMapping[i] = finalMapping[remapDuplicatesToOrignals[i]];
        }
    }

    // remap any shared vertex attributes
    RemapArray ra(copyMapping);
    arrayComparitor.accept(ra);

    //Remap morphGeometry target
    remapGeometryVertices(ra, geom);

    // triangulate faces
    {
        TriangleIndexor ti;
        ti._maxIndex = numVertices;
        ti._remapping = finalMapping;

        for(itr = primitives.begin() ; itr != primitives.end() ; ++ itr) {
            osg::PrimitiveSet* primitive = (*itr).get();
            if(primitive && primitive->getMode() >= osg::PrimitiveSet::TRIANGLES) {
                primitive->accept(ti);
            }
        }

        addDrawElements(ti._indices, osg::PrimitiveSet::TRIANGLES, new_primitives);
    }

    // line-ify line-type primitives
    {
        LineIndexor li, wi; // lines and wireframes
        li._maxIndex = numVertices;
        wi._maxIndex = numVertices;
        li._remapping = finalMapping;
        wi._remapping = finalMapping;

        for(itr = primitives.begin() ; itr != primitives.end() ; ++ itr) {
            osg::PrimitiveSet* primitive = (*itr).get();
            if(primitive && primitive->getMode() >= osg::PrimitiveSet::LINES && primitive->getMode() <= osg::PrimitiveSet::LINE_LOOP) {
                bool isWireframe = false;
                if(primitive->getUserValue("wireframe", isWireframe) && isWireframe) {
                    primitive->accept(wi);
                }
                else {
                    primitive->accept(li);
                }
            }
        }
        addDrawElements(li._indices, osg::PrimitiveSet::LINES, new_primitives);
        addDrawElements(wi._indices, osg::PrimitiveSet::LINES, new_primitives, "wireframe");
    }

    // adds points primitives
    {
        IndexList points;
        for(itr = primitives.begin() ; itr != primitives.end() ; ++ itr) {
            osg::PrimitiveSet* primitive = (*itr).get();
            if(primitive && primitive->getMode() == osg::PrimitiveSet::POINTS) {
                for(unsigned int k = 0 ; k < primitive->getNumIndices() ; ++ k) {
                    points.push_back(finalMapping[primitive->index(k)]);
                }
            }
        }
        addDrawElements(points, osg::PrimitiveSet::POINTS, new_primitives);
    }

    geom.setPrimitiveSetList(new_primitives);
    deduplicator.deduplicateUVs(geom);
}


void IndexMeshVisitor::addDrawElements(IndexList& data,
                                       osg::PrimitiveSet::Mode mode,
                                       osg::Geometry::PrimitiveSetList& primitives,
                                       std::string userValue) {
    if(!data.empty()) {
        osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(mode,
                                                                    data.begin(),
                                                                    data.end());
        if(!userValue.empty()) {
            elements->setUserValue(userValue, true);
        }
        primitives.push_back(elements);
    }
}
