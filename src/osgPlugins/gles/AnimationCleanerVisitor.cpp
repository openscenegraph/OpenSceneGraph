#include "AnimationCleanerVisitor"



void AnimationCleanerVisitor::apply(osg::Node& node) {
    osgAnimation::BasicAnimationManager* manager = getCallbackType<osgAnimation::BasicAnimationManager>(node.getUpdateCallback());
    if(manager) {
        _managers[manager] = osg::ref_ptr<osg::Node>(&node);
        collectAnimationChannels(*manager);
    }

    collectUpdateCallback(node);

    traverse(node);
}


void AnimationCleanerVisitor::apply(osg::MatrixTransform& transform) {
    HasGeometryVisitor hasData;
    transform.traverse(hasData);

    if(!hasData.geometry) {
        // if animation transforms have no child geometry they are cleanable
        osgAnimation::Skeleton* skeleton = dynamic_cast<osgAnimation::Skeleton*>(&transform);
        osgAnimation::Bone* bone = dynamic_cast<osgAnimation::Bone*>(&transform);
        if(skeleton) {
            _transforms.push_back(osg::ref_ptr<osgAnimation::Skeleton>(skeleton));
        }
        if(bone) {
            _transforms.push_back(osg::ref_ptr<osgAnimation::Bone>(bone));
        }
    }

    osgAnimation::UpdateMatrixTransform* update = getCallbackType<osgAnimation::UpdateMatrixTransform>(transform.getUpdateCallback());
    if(update) {
        _updates[update] = osg::ref_ptr<osg::Node>(&transform);
    }

    traverse(transform);
}


void AnimationCleanerVisitor::apply(osg::Geometry& geometry) {
    osgAnimation::MorphGeometry* morphGeometry = 0;
    osgAnimation::RigGeometry* rigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(&geometry);

    if(rigGeometry) {
        if(std::find(_rigGeometries.begin(), _rigGeometries.end(), rigGeometry) == _rigGeometries.end()) {
            _rigGeometries.push_back(rigGeometry);
        }
        morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(rigGeometry->getSourceGeometry());
        if(morphGeometry) {
            _morphGeometries[morphGeometry] = rigGeometry;
        }
    }
    else {
        morphGeometry = dynamic_cast<osgAnimation::MorphGeometry*>(&geometry);
        if(morphGeometry && _morphGeometries.count(morphGeometry) == 0) {
            _morphGeometries[morphGeometry] = 0;
        }
    }

    if(morphGeometry) {
        typedef osgAnimation::MorphGeometry::MorphTargetList MorphTargetList;
        MorphTargetList morphTargetList = morphGeometry->getMorphTargetList();
        for(MorphTargetList::iterator morphTarget = morphTargetList.begin(); morphTarget != morphTargetList.end(); ++morphTarget) {
            osgAnimation::MorphGeometry::MorphTarget& target = *morphTarget;
            _morphTargets[target.getGeometry()->getName()] = morphGeometry;
        }
    }
}


void AnimationCleanerVisitor::collectUpdateCallback(osg::Node& node) {
    osg::Callback *callBack = node.getUpdateCallback();
    while(callBack) {
        BaseAnimationUpdateCallback* update = getCallbackType<BaseAnimationUpdateCallback>(callBack);
        if(update)  {
            _updates[update] = osg::ref_ptr<osg::Node>(&node);
        }
        callBack = callBack->getNestedCallback();
    }
}


void AnimationCleanerVisitor::collectAnimationChannels(osgAnimation::BasicAnimationManager& manager) {
    osgAnimation::AnimationList& animations = manager.getAnimationList();
    for(osgAnimation::AnimationList::iterator animation = animations.begin() ; animation != animations.end() ; ++ animation) {
        if(animation->valid()) {
            osgAnimation::ChannelList& channels = (*animation)->getChannels();
            for(osgAnimation::ChannelList::iterator channel = channels.begin() ; channel != channels.end() ; ++ channel) {
                if(channel->valid()) {
                    _channels.push_back(std::pair<std::string, osgAnimation::Channel*>((*channel)->getTargetName(), channel->get()));
                }
            }
        }
    }
}


void AnimationCleanerVisitor::clean() {
    // 1. clean scene graph to only keep 'valid' objects (top to bottom):
    //     * BasicAnimationManager
    //     * Animation
    //     * Target
    //         * deduplicate successive identical KeyFrames
    // 2. check if there is still valid animation data in the scene graph.
    //    If no valid BasicAnimationManager is left, then clean all collected animation data.
    if(_managers.size() == 0) {
        OSG_WARN << "Monitor: animation.no_animation_manager" << std::endl;
    }
    else if(_managers.size() > 1) {
        OSG_WARN << "Monitor: animation.multiple_animation_manager" << std::endl;
    }
    else {
        OSG_WARN << "Monitor: animation.single_animation_manager" << std::endl;
    }

    // only keep animations if we have a single animation manager
    bool keepAnimations = _managers.size() == 1;

    cleanUnusedMorphTarget();
    cleanInvalidUpdateMorph();

    for(BasicAnimationManagerMap::iterator manager = _managers.begin() ; keepAnimations && manager != _managers.end() ; ++ manager) {
        cleanAnimations(*manager->first);
        if(!isValidAnimationManager(*manager->first)) {
            if(manager->second.valid()) {
                manager->second->removeUpdateCallback(manager->first.get());
            }
            keepAnimations = false;
            OSG_WARN << "No valid animation data found. Removing all animation objects" << std::endl;
            OSG_WARN << "Monitor: animation.disable_animation" << std::endl;
        }
    }

    if(!keepAnimations) {
        removeAnimation();
    }
    else
    {
        cleanInvalidMorphGeometries();
        cleanInvalidRigGeometries();
    }
}


void AnimationCleanerVisitor::removeAnimation() {
    // * bake rig
    // * replace animation geometries by static geometries
    // * remove animation callbacks
    // * remove animation transforms
    bakeRigInitialPose();
    removeAnimatedGeometries();
    removeAnimationUpdateCallbacks();
    removeAnimationTransforms();
}


void AnimationCleanerVisitor::cleanInvalidMorphGeometries() {
    // Replace morph geometries by static geometries if:
    //   * empty morph target list
    for(MorphGeometryMap::iterator morphGeometry = _morphGeometries.begin() ; morphGeometry != _morphGeometries.end() ; ) {
        if(morphGeometry->first.valid()) {
            if(morphGeometry->first.get()->getMorphTargetList().size() == 0) {
                OSG_WARN << "Monitor: animation.invalid_morphgeometry" << std::endl;
                replaceMorphGeometryByGeometry(*morphGeometry->first.get(), morphGeometry->second);
                _morphGeometries.erase(morphGeometry ++);
            }
            else {
                ++ morphGeometry;
            }
        }
    }
}


void AnimationCleanerVisitor::cleanInvalidRigGeometries() {
    // Replace rig geometries by static geometries if:
    //   * empty or inexistant vertex influence map
    //   * no *strictly* positive influence coefficient
    for(RigGeometryList::iterator iterator = _rigGeometries.begin() ; iterator != _rigGeometries.end() ; ) {
        osg::ref_ptr<osgAnimation::RigGeometry> rigGeometry = *iterator;
        if(rigGeometry.valid() && !glesUtil::hasPositiveWeights(rigGeometry->getSourceGeometry())) {
            OSG_WARN << "Monitor: animation.invalid_riggeometry" << std::endl;
            replaceRigGeometryBySource(*rigGeometry.get());
            iterator = _rigGeometries.erase(iterator);
        }
        else {
            ++ iterator;
        }
    }
}


void AnimationCleanerVisitor::cleanUnusedMorphTarget() {
    // Removes MorphGeometry targets not updated by any channel
    std::set<std::string> kept, removed;

    for(NameMorphMap::iterator targetMorph = _morphTargets.begin() ; targetMorph != _morphTargets.end() ; ) {
        const std::string& target = targetMorph->first;
        unsigned int count = 0;
        for(TargetChannelList::const_iterator targetChannel = _channels.begin() ; targetChannel != _channels.end() ; ++ targetChannel) {
            if(targetChannel->first == target) {
                ++ count;
            }
        }

        if(count == 0) {
            removed.insert(target);
            targetMorph->second->removeMorphTarget(target);
            _morphTargets.erase(targetMorph ++);
        }
        else {
            kept.insert(target);
            ++ targetMorph;
        }
    }

    if(!removed.empty()) {
        OSG_WARN << "Monitor: animation.unused_morphtarget" << std::endl;
        for(TargetChannelList::iterator targetChannel = _channels.begin() ; targetChannel != _channels.end() ; ) {
            std::string target = targetChannel->first;

            if(removed.find(target) != removed.end()) {
                _channels.erase(targetChannel ++);
            }
            else {
                if(kept.find(target) != kept.end()) {
                    // update target channel names with the (possibly) new target index in MophTargetList
                    osgAnimation::MorphGeometry& morphGeometry = *_morphTargets[target];
                    const osgAnimation::MorphGeometry::MorphTargetList& targets = morphGeometry.getMorphTargetList();
                    for(unsigned int i = 0 ; i < targets.size() ; ++ i) {
                        if(targets[i].getGeometry()->getName() == target) {
                            std::ostringstream oss;
                            oss << i;
                            targetChannel->second->setName(oss.str());
                        }
                    }
                }

                ++ targetChannel;
            }
        }
    }
}


void AnimationCleanerVisitor::cleanInvalidUpdateMorph() {
    // Removes unused UpdateMorph targets (i.e. name does not match any MorphGeometry target)
    for(AnimationUpdateCallBackMap::iterator update = _updates.begin() ; update != _updates.end() ; ++ update) {
        osgAnimation::UpdateMorph *updateMorph = dynamic_cast<osgAnimation::UpdateMorph*>(update->first.get());
        if(!updateMorph) continue;

        NameSet toRemove;
        for(unsigned int i = 0, numTarget = updateMorph->getNumTarget(); i < numTarget; ++i) {
            const std::string& name = updateMorph->getTargetName(i);
            if(_morphTargets.count(name) == 0) {
                toRemove.insert(name);
            }
        }

        for(NameSet::iterator targetName = toRemove.begin(); targetName != toRemove.end(); ++targetName) {
            updateMorph->removeTarget(*targetName);
        }
    }

    // Removes empty UpdateMorphCallback
    for(AnimationUpdateCallBackMap::iterator update = _updates.begin() ; update != _updates.end() ; ) {
        osgAnimation::UpdateMorph *updateMorph = dynamic_cast<osgAnimation::UpdateMorph*>(update->first.get());
        if(!updateMorph || updateMorph->getNumTarget() != 0) {
            ++ update;
        }
        else {
            osg::Callback *callBack = update->second.get()->getUpdateCallback();
            if(callBack) {
                if(callBack == updateMorph)
                    update->second.get()->setUpdateCallback(callBack->getNestedCallback());
                else
                    callBack->removeNestedCallback(updateMorph);
            }
            _updates.erase(update ++);
        }
    }
}


void AnimationCleanerVisitor::warn(const std::string& method,
                                   const std::string& label,
                                   const osgAnimation::Channel& channel,
                                   const std::string& message) const {
    OSG_WARN << std::flush << "Warning: " << "[" << method << "] " << "[[" << label << "]] "
                << "Channel '" << channel.getName() << "' with target '"  << channel.getTargetName()
                << " '" << message << std::endl;
}


template<typename T>
T* AnimationCleanerVisitor::getCallbackType(osg::Callback* callback) {
    if(!callback) return 0;

    T* callback_type = dynamic_cast<T*>(callback);
    if(callback_type) {
        return callback_type;
    }

    return getCallbackType<T>(callback->getNestedCallback());
}


void AnimationCleanerVisitor::cleanAnimations(osgAnimation::BasicAnimationManager& manager)  {
    // remove manager's invalid animations
    osgAnimation::AnimationList& animations = manager.getAnimationList();

    std::vector<osgAnimation::Animation*> invalids;
    for(osgAnimation::AnimationList::iterator animation = animations.begin() ; animation != animations.end() ; ++ animation) {
        if(animation->valid()) {
            cleanAnimation(*animation->get());
        }

        if(!(animation->valid()) || !isValidAnimation(*animation->get())) {
            invalids.push_back(animation->get());
        }
    }

    for(std::vector<osgAnimation::Animation*>::iterator invalid = invalids.begin() ; invalid != invalids.end() ; ++ invalid) {
        manager.unregisterAnimation(*invalid);
    }
}


void AnimationCleanerVisitor::cleanAnimation(osgAnimation::Animation& animation) {
    // remove animation's invalid channels
    osgAnimation::ChannelList& channels = animation.getChannels();
    osgAnimation::ChannelList invalids;

    for(osgAnimation::ChannelList::iterator channel = channels.begin() ; channel != channels.end() ; ++ channel) {
        if(channel->valid()) {
            cleanChannel(*channel->get());
        }

        if(!channel->valid() || !isValidChannel(*channel->get())) {
            invalids.push_back(channel->get());
        }
    }

    for(osgAnimation::ChannelList::iterator invalid = invalids.begin() ; invalid != invalids.end() ; ++ invalid) {
        animation.removeChannel(invalid->get());
    }
}


void AnimationCleanerVisitor::cleanChannel(osgAnimation::Channel& channel) {
    // deduplicate successive KeyFrames that are identical
    osgAnimation::Sampler* sampler = channel.getSampler();
    if(sampler) {
        osgAnimation::KeyframeContainer* container = sampler->getKeyframeContainer();
        if(container && container->size()) {
            unsigned int deduplicated = container->linearInterpolationDeduplicate();
            if(deduplicated) {
                OSG_WARN << "Deduplicated " << deduplicated << " keyframes on channel " << channel.getName() << std::endl;
            }
        }
    }
}


bool AnimationCleanerVisitor::isValidAnimationManager(const osgAnimation::BasicAnimationManager& manager) const {
    // a valid manager has at least one animation and all animations must be valid
    const osgAnimation::AnimationList& animations = manager.getAnimationList();

    for(osgAnimation::AnimationList::const_iterator animation = animations.begin() ; animation != animations.end() ; ++ animation) {
        if(!animation->valid() || !isValidAnimation(*animation->get())) {
            return false;
        }
    }
    return manager.getAnimationList().size() > 0;
}


bool AnimationCleanerVisitor::isValidAnimation(const osgAnimation::Animation& animation) const {
    // a valid animation has at least one channel and all channels must be valid
    const osgAnimation::ChannelList& channels = animation.getChannels();

    for(osgAnimation::ChannelList::const_iterator channel = channels.begin() ; channel != channels.end() ; ++ channel) {
        if(!channel->valid() || !isValidChannel(*channel->get())) {
            return false;
        }
    }

    return channels.size() > 0;
}


bool AnimationCleanerVisitor::isValidChannel(const osgAnimation::Channel& channel) const {
    // a valid channel has valid target i.e.
    // 1. there exists some UpdateMatrixTransform with channel's target name
    // 2. the channel does not simply mimic the UpdateMatrixTransform content
    std::string target = channel.getTargetName();
    for(AnimationUpdateCallBackMap::const_iterator update = _updates.begin() ; update != _updates.end() ; ++ update) {
        osgAnimation::UpdateMorph *updateMorph = dynamic_cast<osgAnimation::UpdateMorph*>(update->first.get());
        if(updateMorph) {
            for(int i = 0, num = updateMorph->getNumTarget(); i < num; ++i) {
                if(updateMorph->getTargetName(i) == target) {
                    return true;
                }
            }
        }
        else {
            if(update->first->getName() == target) {
                // check if channel contains necessary data or just mimic the UpdateCallback's StackedTransform
                bool channelMimickingTransform = isChannelEqualToStackedTransform(channel,
                                                                                    dynamic_cast<osgAnimation::UpdateMatrixTransform*>(update->first.get()));
                if(channelMimickingTransform) {
                    warn("isChannelEqualToStackedTransform", "animation", channel, "seems redundant with stacked transform and has been removed.");
                }
                return !channelMimickingTransform;
            }
        }
    }
    return false;
}


const osgAnimation::StackedTransformElement* AnimationCleanerVisitor::getStackedElement(const osgAnimation::StackedTransform& transforms,
                                                                                        const std::string& name) const {
    for(osgAnimation::StackedTransform::const_iterator transform = transforms.begin() ; transform != transforms.end() ; ++ transform) {
        if(transform->valid() && transform->get()->getName() == name) {
            return transform->get();
        }
    }
    return 0;
}


bool AnimationCleanerVisitor::isChannelEqualToStackedTransform(const osgAnimation::Channel& channel,
                                                               const osgAnimation::UpdateMatrixTransform* matrixTransform) const {
    // if the channel has no 'StackedTransform' we compare the KeyFrame with the 'no-op' transform
    const osgAnimation::StackedTransformElement* element = getStackedElement(matrixTransform->getStackedTransforms(), channel.getName());

    if(channel.getName() == "translate") {
        const osgAnimation::StackedTranslateElement* translation = dynamic_cast<const osgAnimation::StackedTranslateElement*>(element);
        osg::Vec3 value(0., 0., 0.);
        if(translation) {
            value = translation->getTranslate();
        }
        return isChannelEqualToStackedTransform(dynamic_cast<const osgAnimation::Vec3LinearChannel*>(&channel), value);
    }
    else if(channel.getName() == "scale") {
        const osgAnimation::StackedScaleElement* scale = dynamic_cast<const osgAnimation::StackedScaleElement*>(element);
        osg::Vec3 value(1., 1., 1.);
        if(scale) {
            value = scale->getScale();
        }
        return isChannelEqualToStackedTransform(dynamic_cast<const osgAnimation::Vec3LinearChannel*>(&channel), value);
    }
    else if(channel.getName() == "rotate") {
        const osgAnimation::StackedQuaternionElement* rotation = dynamic_cast<const osgAnimation::StackedQuaternionElement*>(element);
        osg::Quat value(0., 0., 0., 1.);
        if(rotation) {
            value = rotation->getQuaternion();
        }
        return isChannelEqualToStackedTransform(dynamic_cast<const osgAnimation::QuatSphericalLinearChannel*>(&channel), value);
    }
    return false;
}


template<typename T, typename V>
bool AnimationCleanerVisitor::isChannelEqualToStackedTransform(const T* channel, const V& value) const {
    if(!channel) {
        return false;
    }

    const typename T::KeyframeContainerType* keys = channel->getSamplerTyped()->getKeyframeContainerTyped();
    if(keys->size() == 0) {
        // channel with no keyframe is equal to anything
        return true;
    }
    if(keys->size() == 1) {
        return (*keys)[0].getValue() == value;
    }
    return false;
}


void AnimationCleanerVisitor::removeAnimationUpdateCallbacks() {
    removeUpdateCallbacksTemplate<AnimationUpdateCallBackMap, osg::NodeCallback>(_updates);
    removeUpdateCallbacksTemplate<BasicAnimationManagerMap, osgAnimation::BasicAnimationManager>(_managers);
}


template<typename C, typename T>
void AnimationCleanerVisitor::removeUpdateCallbacksTemplate(C& callbackNodes) {
    for(typename C::iterator callbackNode = callbackNodes.begin() ; callbackNode != callbackNodes.end() ; ++ callbackNode) {
        if(callbackNode->first && callbackNode->second.valid()) {
            osg::Callback* callback = callbackNode->first.get();
            osg::Node* node = callbackNode->second.get();
            do {
                node->removeUpdateCallback(callback);
                callback = getCallbackType<T>(node->getUpdateCallback());
            }
            while(callback);
        }
    }
}


void AnimationCleanerVisitor::removeAnimationTransforms() {
    for(MatrixTransformList::iterator transform = _transforms.begin() ; transform != _transforms.end() ; ++ transform) {
        if(transform->valid()) {
            removeFromParents(transform->get());
        }
    }
}


void AnimationCleanerVisitor::removeAnimatedGeometries() {
    for(MorphGeometryMap::iterator morphGeometry = _morphGeometries.begin() ; morphGeometry != _morphGeometries.end() ; ++ morphGeometry) {
        if(morphGeometry->first.valid()) {
            replaceMorphGeometryByGeometry(*morphGeometry->first.get(), morphGeometry->second);
        }
    }

    for(RigGeometryList::iterator rigGeometry = _rigGeometries.begin() ; rigGeometry != _rigGeometries.end() ; ++ rigGeometry) {
        if(rigGeometry->valid()) {
            replaceRigGeometryBySource(*(rigGeometry->get()));
        }
    }
}


void AnimationCleanerVisitor::removeFromParents(osg::Node* node) {
    osg::Node::ParentList parents = node->getParents();
    for(osg::Node::ParentList::iterator parent = parents.begin() ; parent != parents.end() ; ++ parent) {
        if(*parent) {
            (*parent)->removeChild(node);
        }
    }
}


void AnimationCleanerVisitor::replaceRigGeometryBySource(osgAnimation::RigGeometry& rigGeometry) const {
    if(osgAnimation::MorphGeometry* source = dynamic_cast<osgAnimation::MorphGeometry*>(rigGeometry.getSourceGeometry())) {
        osgAnimation::MorphGeometry* morph = new osgAnimation::MorphGeometry(*source);
        replaceAnimatedGeometryByStaticGeometry(&rigGeometry, morph);
    }
    else {
        replaceAnimatedGeometryByStaticGeometry(&rigGeometry,
                                                new osg::Geometry(*rigGeometry.getSourceGeometry()));
    }
}


void AnimationCleanerVisitor::replaceMorphGeometryByGeometry(osgAnimation::MorphGeometry& morphGeometry,
                                                             osgAnimation::RigGeometry* rigGeometry) const {
    osg::Geometry* geometry = new osg::Geometry(morphGeometry);
    if(!rigGeometry) {
        replaceAnimatedGeometryByStaticGeometry(&morphGeometry, geometry);
    }
    else {
        rigGeometry->setSourceGeometry(geometry);
    }
}


void AnimationCleanerVisitor::replaceAnimatedGeometryByStaticGeometry(osg::Geometry* animatedGeometry,
                                                                      osg::Geometry* staticGeometry) const {
    for(unsigned int i = 0 ; i < animatedGeometry->getNumParents() ; ++ i) {
        osg::Geode* parent = (animatedGeometry->getParent(i) ? animatedGeometry->getParent(i)->asGeode() : 0);
        if(parent) {
            parent->addDrawable(staticGeometry);
            parent->removeDrawable(animatedGeometry);
        }
    }
}


void AnimationCleanerVisitor::bakeRigInitialPose() {
    // use RigTransformSoftware to compute T-pose and replace rig source by computed geometry
    for(RigGeometryList::iterator rigiterator = _rigGeometries.begin() ; rigiterator != _rigGeometries.end() ; ++ rigiterator) {
        osgAnimation::RigGeometry* rigGeometry = (*rigiterator).get();
        rigGeometry->setRigTransformImplementation(new osgAnimation::RigTransformSoftware);
        rigGeometry->update();

        osg::Geometry* baked = static_cast<osg::Geometry*>(rigGeometry->clone(osg::CopyOp::DEEP_COPY_ALL));
        rigGeometry->setSourceGeometry(baked);
    }
}
