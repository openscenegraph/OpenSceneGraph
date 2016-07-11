#include "RigAnimationVisitor"


struct sort_weights {
    bool operator()(const std::pair<unsigned int, float> &left, const std::pair<unsigned int, float> &right) {
        // in case weights are equal, order elements by ascending bone ids
        if(left.second == right.second) {
            return left.first < right.first;
        }
        else {
            return left.second > right.second;
        }
    }
};


void RigAnimationVisitor::apply(osg::Drawable& drawable) {
    // skip drawables already processed
    if (isProcessed(drawable)) {
        return;
    }

    apply(drawable.asGeometry());

    setProcessed(drawable);
}


void RigAnimationVisitor::apply(osg::Geometry* geometry) {
    osgAnimation::RigGeometry* rig = dynamic_cast<osgAnimation::RigGeometry*>(geometry);
    if(rig) {
        apply(*rig);
    }
}


void RigAnimationVisitor::apply(osgAnimation::RigGeometry& rigGeometry) {
    // find skeleton
    osgAnimation::UpdateRigGeometry rigUpdater;
    osg::Geometry* source = rigGeometry.getSourceGeometry();

    if(osgAnimation::MorphGeometry* morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(source)) {
        // skip normals blending when rigging a morph as targets may not have normals yet
        morphGeometry->setMorphNormals(false);
    }

    rigUpdater.update(0, &rigGeometry);

    osgAnimation::RigTransformHardware rth;
    rth(rigGeometry);
    std::vector< std::vector< std::pair<unsigned int, float> > > vertexBoneWeights(rigGeometry.getVertexArray()->getNumElements());

    // collect all bone/weight pairs for *all* vertices
    for(unsigned int i = 0 ; i < static_cast<unsigned int>(rth.getNumVertexAttrib()) ; ++ i) {
        osg::Vec4Array* weights = dynamic_cast<osg::Vec4Array*>(rth.getVertexAttrib(i));
        for(unsigned int k = 0 ; k < weights->getNumElements() ; ++ k) {
            vertexBoneWeights[k].push_back(std::pair<unsigned int, float>((*weights)[k][0], (*weights)[k][1]));
            vertexBoneWeights[k].push_back(std::pair<unsigned int, float>((*weights)[k][2], (*weights)[k][3]));
        }
    }

    osg::ref_ptr<osg::Vec4usArray> bones = new osg::Vec4usArray;
    osg::ref_ptr<osg::Vec4Array> weights = new osg::Vec4Array;

    // for each vertex a partial sort to keep only n max weights (hardcoded to 4)
    for(unsigned int i = 0 ; i < vertexBoneWeights.size() ; ++ i) {
        std::vector< std::pair<unsigned int, float> > maxVertexBoneWeight(4);
        std::partial_sort_copy(vertexBoneWeights[i].begin(), vertexBoneWeights[i].end(),
                               maxVertexBoneWeight.begin(), maxVertexBoneWeight.end(),
                               sort_weights());

        osg::Vec4 vertexWeights;
        osg::Vec4us vertexBones;
        for(unsigned int j = 0 ; j < 4 ; ++ j) {
            vertexBones[j] = maxVertexBoneWeight[j].first;
            vertexWeights[j] = maxVertexBoneWeight[j].second;
        }

        normalizeWeight(vertexWeights);

        bones->push_back(vertexBones);
        weights->push_back(vertexWeights);
    }

    RigAnimationVisitor::boneIndices geometryBoneIndices = remapGeometryBones(*bones);
    applyBoneIndicesRemap(*bones, geometryBoneIndices);
    serializeBonesUserValues(*bones, geometryBoneIndices, rth.getBoneNameToPalette());

    bones->setUserValue("bones", true);
    weights->setUserValue("weights", true);

    // attach bones & weights to source geometry during scene graph processing
    source->setVertexAttribArray(source->getNumVertexAttribArrays(), bones.get(), osg::Array::BIND_PER_VERTEX);
    source->setVertexAttribArray(source->getNumVertexAttribArrays(), weights.get(), osg::Array::BIND_PER_VERTEX);

    rigGeometry.setRigTransformImplementation(0); //Remove current implementation to force implementation re-init
}


RigAnimationVisitor::boneIndices RigAnimationVisitor::remapGeometryBones(const osg::Vec4usArray& bones) {
    RigAnimationVisitor::boneIndices remap;
    for(unsigned int i = 0 ; i < bones.getNumElements() ; ++ i) {
        for(unsigned int j = 0 ; j < 4 ; ++ j) {
            if(remap.find(bones[i][j]) == remap.end()) {
                remap[bones[i][j]] = static_cast<unsigned short>(remap.size() - 1);
            }
        }
    }
    return remap;
}


void RigAnimationVisitor::applyBoneIndicesRemap(osg::Vec4usArray& bones, const RigAnimationVisitor::boneIndices& remap) {
    for(unsigned int i = 0 ; i < bones.getNumElements() ; ++ i) {
        RigAnimationVisitor::boneIndices::const_iterator x = remap.find(bones[i][0]),
                                                         y = remap.find(bones[i][1]),
                                                         z = remap.find(bones[i][2]),
                                                         w = remap.find(bones[i][3]);
        bones[i] = osg::Vec4us(x->second,
                                y->second,
                                z->second,
                                w->second);
    }
}


void RigAnimationVisitor::serializeBonesUserValues(osg::Vec4usArray& bones,
                                                   const std::map<unsigned int, unsigned short>& oldIndexToNewIndex,
                                                   const osgAnimation::RigTransformHardware::BoneNamePaletteIndex& boneNamePaletteIndex) {

    // map 'global' palette index to bone name
    std::map<unsigned int, std::string> oldIndexToBoneName;
    for(osgAnimation::RigTransformHardware::BoneNamePaletteIndex::const_iterator it = boneNamePaletteIndex.begin() ;
        it != boneNamePaletteIndex.end() ; ++ it) {
        oldIndexToBoneName[static_cast<unsigned int>(it->second)] = it->first;
    }

    // serialize geometry 'palette index' => 'bone name' with user value using animationBone_ as
    // name prefix
    for(std::map<unsigned int, unsigned short>::const_iterator it = oldIndexToNewIndex.begin() ; it != oldIndexToNewIndex.end() ; ++ it) {
        std::ostringstream oss;
        oss << "animationBone_" << it->second;
        bones.setUserValue(oss.str(), oldIndexToBoneName[it->first]);
    }
}


bool RigAnimationVisitor::isProcessed(osg::Drawable& node) {
    return _processed.find(&node) != _processed.end();
}


void RigAnimationVisitor::setProcessed(osg::Drawable& node) {
    _processed.insert(&node);
}
