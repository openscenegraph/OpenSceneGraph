#include <limits>

#include "OpenGLESGeometryOptimizer"
#include "glesUtil"

const unsigned glesUtil::Remapper::invalidIndex = std::numeric_limits<unsigned>::max();


osg::Node* OpenGLESGeometryOptimizer::optimize(osg::Node& node) {
    osg::ref_ptr<osg::Node> model = osg::clone(&node);

    if(_mode == "all" || _mode == "animation") {
        // animation: process bones/weights or remove all animation data if disabled
        makeAnimation(model.get());
    }

    if(_mode == "all" || _mode == "geometry") {
        // wireframe
        if (!_wireframe.empty()) {
            makeWireframe(model.get());
        }

        // bind per vertex
        makeBindPerVertex(model.get());

        // index (merge exact duplicates + uses simple triangles & lines i.e. no strip/fan/loop)
        makeIndexMesh(model.get());

        // clean (remove degenerated data)
        std::string authoringTool;
        if(model->getUserValue("authoring_tool", authoringTool) && authoringTool == "Tilt Brush") {
            makeCleanGeometry(model.get());
        }

        // smooth vertex normals (if geometry has no normal compute smooth normals)
        makeSmoothNormal(model.get());

        // tangent space
        if (_generateTangentSpace) {
            makeTangentSpace(model.get());
        }

        if(!_useDrawArray) {
            // split geometries having some primitive index > _maxIndexValue
            makeSplit(model.get());
        }

        // strip
        if(!_disableMeshOptimization) {
            makeOptimizeMesh(model.get());
        }

        if(_useDrawArray) {
            // drawelements to drawarrays
            makeDrawArray(model.get());
        }
        else if(!_disablePreTransform) {
            // pre-transform
            makePreTransform(model.get());
        }

        // unbind bones/weights from source and bind on RigGeometry
        makeBonesAndWeightOnRigGeometry(model.get());

        // detach wireframe
        makeDetach(model.get());
    }

    return model.release();
}
