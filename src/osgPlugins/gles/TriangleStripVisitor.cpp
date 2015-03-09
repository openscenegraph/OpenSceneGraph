#include <osgUtil/TriStripVisitor>
#include "TriangleStripVisitor"


void TriangleStripVisitor::apply(osg::Geometry& geometry) {
    osgUtil::TriStripVisitor tristrip;
    tristrip.setCacheSize(_cacheSize);
    tristrip.setMinStripSize(_minSize);
    tristrip.stripify(geometry);

    // merge stritrip to one using degenerated triangles as glue
    if (_merge) {
        mergeTrianglesStrip(geometry);
    }
}


void TriangleStripVisitor::mergeTrianglesStrip(osg::Geometry& geometry)
{
    int nbtristrip = 0;
    int nbtristripVertexes = 0;

    for (unsigned int i = 0; i < geometry.getNumPrimitiveSets(); i++) {
        osg::PrimitiveSet* ps = geometry.getPrimitiveSet(i);
        osg::DrawElements* de = ps->getDrawElements();
        if (de && de->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP) {
            nbtristrip++;
            nbtristripVertexes += de->getNumIndices();
        }
    }

    if (nbtristrip > 0) {
        osg::notify(osg::NOTICE) << "found " << nbtristrip << " tristrip, "
                                 << "total vertexes " << nbtristripVertexes
                                 << " should result to " << nbtristripVertexes + nbtristrip*2
                                 << " after connection" << std::endl;

        osg::DrawElementsUShort* ndw = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLE_STRIP);
        for (unsigned int i = 0 ; i < geometry.getNumPrimitiveSets() ; ++ i) {
            osg::PrimitiveSet* ps = geometry.getPrimitiveSet(i);
            if (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP) {
                osg::DrawElements* de = ps->getDrawElements();
                if (de) {
                    // if connection needed insert degenerate triangles
                    if (ndw->getNumIndices() != 0 && ndw->back() != de->getElement(0)) {
                        // duplicate last vertex
                        ndw->addElement(ndw->back());
                        // insert first vertex of next strip
                        ndw->addElement(de->getElement(0));
                    }

                    if (ndw->getNumIndices() % 2 != 0 ) {
                        // add a dummy vertex to reverse the strip
                        ndw->addElement(de->getElement(0));
                    }

                    for (unsigned int j = 0; j < de->getNumIndices(); j++) {
                        ndw->addElement(de->getElement(j));
                    }
                }
                else if (ps->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType) {
                    // trip strip can generate drawarray of 5 elements we want to merge them too
                    osg::DrawArrays* da = dynamic_cast<osg::DrawArrays*> (ps);
                    // if connection needed insert degenerate triangles
                    if (ndw->getNumIndices() != 0 && ndw->back() != da->getFirst()) {
                        // duplicate last vertex
                        ndw->addElement(ndw->back());
                        // insert first vertex of next strip
                        ndw->addElement(da->getFirst());
                    }

                    if (ndw->getNumIndices() % 2 != 0 ) {
                        // add a dummy vertex to reverse the strip
                        ndw->addElement(da->getFirst());
                    }

                    for (unsigned int j = 0; j < da->getNumIndices(); j++) {
                        ndw->addElement(da->getFirst() + j);
                    }
                }
            }
        }

        for (int i = geometry.getNumPrimitiveSets() - 1 ; i >= 0 ; -- i) {
            osg::PrimitiveSet* ps = geometry.getPrimitiveSet(i);
            // remove null primitive sets and all primitives that have been merged
            // (i.e. all TRIANGLE_STRIP DrawElements and DrawArrays)
            if (!ps || (ps && ps->getMode() == osg::PrimitiveSet::TRIANGLE_STRIP)) {
                geometry.getPrimitiveSetList().erase(geometry.getPrimitiveSetList().begin() + i);
            }
        }
        geometry.getPrimitiveSetList().insert(geometry.getPrimitiveSetList().begin(), ndw);
    }
}
