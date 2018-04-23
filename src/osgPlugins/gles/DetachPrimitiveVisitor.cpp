#include "DetachPrimitiveVisitor"

void DetachPrimitiveVisitor::reparentDuplicatedGeometry(osg::Geometry& geometry, osg::Geometry& duplicated) {
    unsigned int nbParents = geometry.getNumParents();
    for(unsigned int i = 0 ; i < nbParents ; ++ i) {
        osg::Node* parent = geometry.getParent(i);
        if(parent && parent->asGeode()) {
            osg::Geode* geode = parent->asGeode();
            geode->addDrawable(&duplicated);

            if(!_inlined) {
                geode->removeDrawable(&duplicated);
            }
        }
    }
}


void DetachPrimitiveVisitor::process(osg::Geometry& geometry) {
    if(shouldDetach(geometry)) {
        osg::Geometry* detached = detachGeometry(geometry);
        reparentDuplicatedGeometry(geometry, *detached);
        setProcessed(detached);
    }
}


void DetachPrimitiveVisitor::process(osgAnimation::RigGeometry& rigGeometry) {
    return process(static_cast<osg::Geometry&>(rigGeometry));
}


bool DetachPrimitiveVisitor::shouldDetach(const osg::Geometry& geometry) const {
    if(const osgAnimation::RigGeometry* rigGeometry = dynamic_cast<const osgAnimation::RigGeometry*>(&geometry)) {
        return shouldDetach(*rigGeometry->getSourceGeometry());
    }

    bool detach = false;
    for(unsigned int i = 0 ; i < geometry.getNumPrimitiveSets() ; ++ i) {
        const osg::PrimitiveSet* primitive = geometry.getPrimitiveSet(i);
        if(primitive && primitive->getUserValue(_userValue, detach) && detach) {
            return true;
        }
    }
    return false;
}


osg::Geometry* DetachPrimitiveVisitor::detachGeometry(osg::Geometry& source) {
    // filter vertex buffers depending on geometry type
    osg::Geometry* detached = makeDetachedGeometry(source);
    detached->setUserValue(_userValue, true);
    return detached;
}


osg::Geometry* DetachPrimitiveVisitor::makeDetachedGeometry(osg::Geometry& geometry) {
    if(osgAnimation::RigGeometry* rigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(&geometry)) {
        return createDetachedGeometry(*rigGeometry);
    }
    if(osgAnimation::MorphGeometry* morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(&geometry)) {
        return createDetachedGeometry(*morphGeometry);
    }
    return createDetachedGeometry(geometry);
}


osg::Geometry* DetachPrimitiveVisitor::createDetachedGeometry(osg::Geometry& source) {
    osg::Geometry* detached = new osg::Geometry(source, osg::CopyOp::SHALLOW_COPY);
    if(!_keepGeometryAttributes) {
        // we keep only vertices and clean all other attributes and values
        detached->setNormalArray(0);
        detached->setColorArray(0);
        detached->setSecondaryColorArray(0);
        detached->setFogCoordArray(0);
        for (unsigned int i = 0 ; i < source.getNumTexCoordArrays(); ++ i) {
            detached->setTexCoordArray(i, 0);
        }
        detached->getVertexAttribArrayList().clear();
        detached->setStateSet(0);
        detached->setUserDataContainer(0);
    }

    detached->setPrimitiveSetList(createDetachedPrimitives(source));
    return detached;
}


osg::Geometry::PrimitiveSetList DetachPrimitiveVisitor::createDetachedPrimitives(osg::Geometry& source) {
    // filter primitivesets
    osg::Geometry::PrimitiveSetList detachedPrimitives;
    for(int i = source.getNumPrimitiveSets() - 1 ; i >= 0 ; -- i) {
        osg::PrimitiveSet* primitive = source.getPrimitiveSet(i);
        bool isTrue = false;
        if(primitive && primitive->getUserValue(_userValue, isTrue) && isTrue) {
            detachedPrimitives.push_back(primitive);
            source.removePrimitiveSet(i);
        }
    }
    return detachedPrimitives;
}


osgAnimation::MorphGeometry* DetachPrimitiveVisitor::createDetachedGeometry(osgAnimation::MorphGeometry& source) {
    osgAnimation::MorphGeometry* detached = new osgAnimation::MorphGeometry(*createDetachedGeometry(static_cast<osg::Geometry&>(source)));
    detached->setVertexArray(source.getVertexArray());

    osgAnimation::MorphGeometry::MorphTargetList& targets = source.getMorphTargetList();
    for(osgAnimation::MorphGeometry::MorphTargetList::iterator target = targets.begin() ; target != targets.end() ; ++ target) {
        detached->addMorphTarget(target->getGeometry(), target->getWeight());
    }
    return detached;
}


osg::Geometry* DetachPrimitiveVisitor::createDetachedGeometry(osgAnimation::RigGeometry& source) {
    osgAnimation::RigGeometry* detached;
    if(!_keepGeometryAttributes) {
        detached = new osgAnimation::RigGeometry();
        detached->setSourceGeometry(makeDetachedGeometry(*source.getSourceGeometry()));

        // Only keep vertices and Bones/Weights attrib arrays
        detached->setVertexArray(source.getVertexArray());
        for(unsigned int i = 0 ; i < source.getVertexAttribArrayList().size() ; ++ i) {
            osg::Array* attribute = source.getVertexAttribArray(i);
            if(attribute) {
                bool isBones = false;
                bool isWeights = false;
                attribute->getUserValue("bones", isBones);
                attribute->getUserValue("weights", isWeights);
                if (isBones || isWeights) {
                    detached->setVertexAttribArray(i, source.getVertexAttribArray(i));
                }
            }
        }
    }
    else {
        detached = new osgAnimation::RigGeometry(source, osg::CopyOp::SHALLOW_COPY);
    }

    return detached;
}
