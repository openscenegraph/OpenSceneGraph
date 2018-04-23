#include "RemapGeometryVisitor"
#include "glesUtil"


void RemapGeometryVisitor::apply(osg::Geode& geode) {
    GeometryUniqueVisitor::apply(geode);
    GeometryList remappedGeometries;
    DrawableList nonGeometryDrawables;

    for(unsigned int i = 0 ; i < geode.getNumDrawables() ; ++ i) {
        osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
        if(geometry) {
            if(osgAnimation::RigGeometry* rigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(geometry)) {
                GeometryMap::iterator lookup = _remap.find(rigGeometry->getSourceGeometry());
                if(lookup != _remap.end() && !lookup->second.empty()) {

                    // convert now static rigGeometry into simple Geometry
                    for(GeometryList::iterator mapped = lookup->second.begin() ; mapped != lookup->second.end() ; ++ mapped) {
                        if(glesUtil::hasPositiveWeights(mapped->get())) {
                            osgAnimation::RigGeometry* mappedRig = new osgAnimation::RigGeometry(*rigGeometry);
                            mappedRig->setSourceGeometry(mapped->get());
                            remappedGeometries.push_back(mappedRig);
                        }
                        else {
                            remappedGeometries.push_back(mapped->get());
                        }
                    }

                }
            }
            else {
                GeometryMap::iterator lookup = _remap.find(geometry);
                if(lookup != _remap.end() && !lookup->second.empty()) {
                    remappedGeometries.insert(remappedGeometries.end(), lookup->second.begin(), lookup->second.end());
                }
            }
        }
        else {
            nonGeometryDrawables.push_back(geode.getDrawable(i));
        }
    }

    // remove all drawables
    geode.removeDrawables(0, geode.getNumDrawables());
    // insert split geometries
    for(unsigned int i = 0 ; i < remappedGeometries.size() ; ++ i) {
        geode.addDrawable(remappedGeometries[i].get());
    }

    if(_exportNonGeometryDrawables) {
        // insert other drawables (e.g. osgText)
        for(unsigned int i = 0 ; i < nonGeometryDrawables.size() ; ++ i) {
            geode.addDrawable(nonGeometryDrawables[i].get());
        }
    }
}


void RemapGeometryVisitor::process(osg::Geometry& geometry) {
    const GeometryList& mapped = _mapper.process(geometry);
    setProcessed(&geometry, mapped);
}


bool RemapGeometryVisitor::isProcessed(osg::Geometry* node) {
    return _remap.find(node) != _remap.end();
}


void RemapGeometryVisitor::setProcessed(osg::Geometry* node, const GeometryList& list) {
    _remap.insert(std::pair<osg::Geometry*, GeometryList>(node, GeometryList(list)));
}
