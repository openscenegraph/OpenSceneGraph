#include "AABBonBoneVisitor"


void ComputeAABBOnBoneVisitor::apply(osg::Transform& node) {
    if(!_root)
        _root = dynamic_cast<osgAnimation::Skeleton*>(&node);

    osgAnimation::Bone * b = dynamic_cast<osgAnimation::Bone*>(&node);
    if(b) apply(*b);
    traverse(node);
}


void ComputeAABBOnBoneVisitor::apply(osg::Geometry& geometry) {
    if(osgAnimation::RigGeometry *rigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(&geometry)) {
        apply(*rigGeometry);
    }
}


void ComputeAABBOnBoneVisitor::apply(osgAnimation::Bone &bone) {
    _bones.push_back(&bone);
}


void ComputeAABBOnBoneVisitor::apply(osgAnimation::RigGeometry &rig) {
    _rigGeometries.push_back(&rig);
}


void ComputeAABBOnBoneVisitor::computeBoundingBoxOnBones() {
    //Perform Updates
    updateBones();
    updateRigGeometries();

    //We have our T pose, we can compute an AABB for each bone
    for (BoneList::iterator bone = _bones.begin(); bone != _bones.end(); ++ bone) {
        osg::BoundingBox bb;
        //For each geometry
        for (RigGeometryList::iterator iterator = _rigGeometries.begin(); iterator != _rigGeometries.end(); ++ iterator) {
            osgAnimation::RigGeometry* rigGeometry = *iterator;
            if(!rigGeometry) continue;

            osg::Matrix mtxLocalToSkl = rigGeometry->getWorldMatrices(_root).at(0);

            //For each Vertex influence
            osgAnimation::VertexInfluenceMap * infMap = rigGeometry->getInfluenceMap();
            osgAnimation::VertexInfluenceMap::iterator itMap = infMap->find((*bone)->getName());
            if(itMap == infMap->end()) continue;

            osg::Vec3Array *vertices = dynamic_cast<osg::Vec3Array*>(rigGeometry->getVertexArray());
            if(!vertices) continue;

            osgAnimation::VertexInfluence vxtInf = (*itMap).second;

            //Expand the boundingBox with each vertex
            for(unsigned int j = 0; j < vxtInf.size(); j++) {
                if(vxtInf.at(j).second < 10e-2) continue; //Skip vertex if low weight
                osg::Vec3 vx = vertices->at(vxtInf.at(j).first);
                vx = vx * mtxLocalToSkl;
                bb.expandBy(vx);
            }

            // Compare initial and actual boundingBox if (no change) => no box on bone
            if(bb == osg::BoundingBox() || (bb._min.x() == bb._max.x() && bb._min.y() == bb._max.y() && bb._min.z() == bb._max.z())) {
                continue;
            }

            osg::Matrix worldToBone = osg::Matrix::inverse((*bone)->getWorldMatrices(_root).at(0));

            if(_createGeometry) {
                osg::Geode *g = new osg::Geode;
                g->setName("AABB_for_bone_" + (*bone)->getName());
                g->addDrawable(createBox(bb, worldToBone));
                (*bone)->addChild(g);
            }

            serializeBoundingBox(bb, worldToBone, *(*bone));
        }
    }

    //Clear geometries
    for (RigGeometryList::iterator iterator = _rigGeometries.begin(); iterator != _rigGeometries.end(); ++ iterator) {
        osgAnimation::RigGeometry* rigGeometry = *iterator;
        if(rigGeometry) {
            rigGeometry->copyFrom(*rigGeometry->getSourceGeometry());
            rigGeometry->setRigTransformImplementation(0);
        }
    }
}


osg::Geometry* ComputeAABBOnBoneVisitor::createBox(const osg::BoundingBox &bb,
                                                   const osg::Matrix &transform,
                                                   float ratio,
                                                   osg::Vec4 color) {
    osg::Geometry *cube = new osg::Geometry;

    osg::Vec3 center = bb.center();
    double halfLenghtX = (bb._max.x() - bb._min.x()) * 0.50;
    double halfLenghtY = (bb._max.y() - bb._min.y()) * 0.50;
    double halfLenghtZ = (bb._max.z() - bb._min.z()) * 0.50;

    halfLenghtX *= ratio;
    halfLenghtY *= ratio;
    halfLenghtZ *= ratio;

    osg::Vec3Array *cubeVertices = new osg::Vec3Array;
    cubeVertices->push_back(osg::Vec3(center.x() - halfLenghtX, center.y() + halfLenghtY, center.z() + halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() - halfLenghtX, center.y() + halfLenghtY, center.z() - halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() - halfLenghtX, center.y() - halfLenghtY, center.z() - halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() - halfLenghtX, center.y() - halfLenghtY, center.z() + halfLenghtZ) * transform);

    cubeVertices->push_back(osg::Vec3(center.x() + halfLenghtX, center.y() + halfLenghtY, center.z() + halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() + halfLenghtX, center.y() + halfLenghtY, center.z() - halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() + halfLenghtX, center.y() - halfLenghtY, center.z() - halfLenghtZ) * transform);
    cubeVertices->push_back(osg::Vec3(center.x() + halfLenghtX, center.y() - halfLenghtY, center.z() + halfLenghtZ) * transform);

    cube->setVertexArray(cubeVertices);

    {
        osg::DrawElementsUInt* up = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        up->push_back(4);
        up->push_back(5);
        up->push_back(1);
        up->push_back(0);
        cube->addPrimitiveSet(up);
    }

    {
        osg::DrawElementsUInt* down = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        down->push_back(2);
        down->push_back(6);
        down->push_back(7);
        down->push_back(3);
        cube->addPrimitiveSet(down);
    }

    {
        osg::DrawElementsUInt* left = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        left->push_back(2);
        left->push_back(3);
        left->push_back(0);
        left->push_back(1);
        cube->addPrimitiveSet(left);
    }

    {
        osg::DrawElementsUInt* right = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        right->push_back(7);
        right->push_back(6);
        right->push_back(5);
        right->push_back(4);
        cube->addPrimitiveSet(right);
    }

    {
        osg::DrawElementsUInt* front = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        front->push_back(3);
        front->push_back(7);
        front->push_back(4);
        front->push_back(0);
        cube->addPrimitiveSet(front);
    }

    {
        osg::DrawElementsUInt* back = new osg::DrawElementsUInt(osg::PrimitiveSet::QUADS, 0);
        back->push_back(6);
        back->push_back(2);
        back->push_back(1);
        back->push_back(5);
        cube->addPrimitiveSet(back);
    }

    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);
    colors->push_back(color);

    cube->setColorArray(colors);
    cube->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

    return cube;
}


void ComputeAABBOnBoneVisitor::serializeBoundingBox(const osg::BoundingBox &bb,
                                                    const osg::Matrix &transform,
                                                    osgAnimation::Bone &b,
                                                    float ratio) {
    osg::Vec3 center = bb.center();
    double halfLenghtX = (bb._max.x() - bb._min.x()) * 0.50;
    double halfLenghtY = (bb._max.y() - bb._min.y()) * 0.50;
    double halfLenghtZ = (bb._max.z() - bb._min.z()) * 0.50;

    halfLenghtX *= ratio;
    halfLenghtY *= ratio;
    halfLenghtZ *= ratio;

    osg::BoundingBox serializedBB;

    serializedBB.expandBy(osg::Vec3(center.x() - halfLenghtX, center.y() + halfLenghtY, center.z() + halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() - halfLenghtX, center.y() + halfLenghtY, center.z() - halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() - halfLenghtX, center.y() - halfLenghtY, center.z() - halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() - halfLenghtX, center.y() - halfLenghtY, center.z() + halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() + halfLenghtX, center.y() + halfLenghtY, center.z() + halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() + halfLenghtX, center.y() + halfLenghtY, center.z() - halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() + halfLenghtX, center.y() - halfLenghtY, center.z() - halfLenghtZ) * transform);
    serializedBB.expandBy(osg::Vec3(center.x() + halfLenghtX, center.y() - halfLenghtY, center.z() + halfLenghtZ) * transform);

    b.setUserValue("AABBonBone_min", serializedBB._min);
    b.setUserValue("AABBonBone_max", serializedBB._max);
}


void ComputeAABBOnBoneVisitor::updateBones() {
    osgUtil::UpdateVisitor update;
    _root->accept(update);
}


void ComputeAABBOnBoneVisitor::updateRigGeometries() {
    for (unsigned int i = 0, size = _rigGeometries.size(); i < size; i++) {
        osgAnimation::RigGeometry * rig = _rigGeometries.at(i);
        osg::Drawable::UpdateCallback * callback = dynamic_cast<osg::Drawable::UpdateCallback*>(rig->getUpdateCallback());
        if(callback) {
            callback->update(0, rig);
        }
    }
}
