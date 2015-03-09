#include <limits>

#include "OpenGLESGeometryOptimizer"
#include "glesUtil"

const unsigned glesUtil::Remapper::invalidIndex = std::numeric_limits<unsigned>::max();


osg::Node* OpenGLESGeometryOptimizer::optimize(osg::Node& node) {
    osg::ref_ptr<osg::Node> model = osg::clone(&node);

    // animation: create regular Geometry if RigGeometry
    makeAnimation(model.get());

    // wireframe
    if (!_wireframe.empty()) {
        makeWireframe(model.get());
    }

    // bind per vertex
    makeBindPerVertex(model.get());

    // index (merge exact duplicates + uses simple triangles & lines i.e. no strip/fan/loop)
    makeIndexMesh(model.get());

    // tangent space
    if (_generateTangentSpace) {
        makeTangentSpace(model.get());
    }

    if(!_useDrawArray) {
        // split geometries having some primitive index > _maxIndexValue
        makeSplit(model.get());
    }

    // strip
    if(!_disableTriStrip) {
        makeTriStrip(model.get());
    }

    if(_useDrawArray) {
        // drawelements to drawarrays
        makeDrawArray(model.get());
    }
    else if(!_disablePreTransform) {
        // pre-transform
        makePreTransform(model.get());
    }

    // detach wireframe
    makeDetach(model.get());

    return model.release();
}
