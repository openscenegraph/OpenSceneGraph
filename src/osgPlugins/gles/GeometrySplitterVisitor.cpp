#include "GeometrySplitterVisitor"
#include "GeometryIndexSplitter"


void GeometrySplitterVisitor::apply(osg::Geode& geode) {
    GeometryUniqueVisitor::apply(geode);
    GeometryList remappedGeometries;
    DrawableList nonGeometryDrawables;

    for(unsigned int i = 0 ; i < geode.getNumDrawables() ; ++ i) {
        osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
        if(geometry) {
            if(osgAnimation::RigGeometry* rigGeometry = dynamic_cast<osgAnimation::RigGeometry*>(geometry)) {
                SplitMap::iterator lookup = _split.find(rigGeometry->getSourceGeometry());
                if(lookup != _split.end() && !lookup->second.empty()) {
                    for(GeometryList::iterator splittedSource = lookup->second.begin() ; splittedSource != lookup->second.end() ; ++ splittedSource) {
                        if(glesUtil::hasPositiveWeights(splittedSource->get())) {
                            osgAnimation::RigGeometry* splittedRig = new osgAnimation::RigGeometry(*rigGeometry);
                            splittedRig->setSourceGeometry(splittedSource->get());
                            remappedGeometries.push_back(splittedRig);
                        }
                        else {
                            remappedGeometries.push_back(splittedSource->get());
                        }
                    }
                }
            }
            else {
                SplitMap::iterator lookup = _split.find(geometry);
                if(lookup != _split.end() && !lookup->second.empty()) {
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


void GeometrySplitterVisitor::process(osg::Geometry& geometry) {
    GeometryIndexSplitter splitter(_maxAllowedIndex);
    splitter.split(geometry);
    setProcessed(&geometry, splitter._geometryList);
}


bool GeometrySplitterVisitor::isProcessed(osg::Geometry* node) {
    return _split.find(node) != _split.end();
}


void GeometrySplitterVisitor::setProcessed(osg::Geometry* node, const GeometryList& list) {
    _split.insert(std::pair<osg::Geometry*, GeometryList>(node, GeometryList(list)));
}
