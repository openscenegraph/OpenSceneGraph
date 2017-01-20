#include "TangentSpaceVisitor"
#include "glesUtil"

void TangentSpaceVisitor::process(osgAnimation::MorphGeometry& morphGeometry) {
    process(static_cast<osg::Geometry&>(morphGeometry));

    osgAnimation::MorphGeometry::MorphTargetList& targets = morphGeometry.getMorphTargetList();
    for(osgAnimation::MorphGeometry::MorphTargetList::iterator target = targets.begin() ; target != targets.end() ; ++ target) {
        glesUtil::TargetGeometry geometry(*target, morphGeometry);
        process(*geometry);
    }
}


void TangentSpaceVisitor::process(osg::Geometry& geometry) {
    // We don't have to recompute the tangent space if we already have the data
    int tangentIndex = -1;
    if (geometry.getUserValue(std::string("tangent"), tangentIndex) && tangentIndex != -1)
    {
        if(geometry.getVertexAttribArray(tangentIndex)) {
            OSG_INFO << "[TangentSpaceVisitor::apply] Geometry '" << geometry.getName()
                    << "' The tangent space is not recomputed as it was given within the original file" << std::endl;
            geometry.getVertexAttribArray(tangentIndex)->setUserValue("tangent", true);
            return;
        }
        else {
            OSG_WARN << "Anomaly: [TangentSpaceVisitor] Missing tangent array at specificied index." << std::endl;
        }
    }

    if (!geometry.getTexCoordArray(_textureUnit)){
        int texUnit = 0;
        bool found = false;
        while(texUnit < 32){
            if (_textureUnit != texUnit && geometry.getTexCoordArray(texUnit)){
                _textureUnit = texUnit;
                found = true;
                break;
            }
            texUnit++;
        }
        if (!found)
            return;
    }

    osg::ref_ptr<osgUtil::TangentSpaceGenerator> generator = new osgUtil::TangentSpaceGenerator;
    generator->generate(&geometry, _textureUnit);

    if (generator->getTangentArray()) {
        osg::Vec4Array* normal = generator->getNormalArray();
        osg::Vec4Array* tangent = generator->getTangentArray();
        osg::Vec4Array* tangent2 = generator->getBinormalArray();
        osg::Vec4Array* finalTangent = osg::clone(generator->getTangentArray(), osg::CopyOp::DEEP_COPY_ALL);
        for (unsigned int i = 0; i < tangent->size(); i++) {
            osg::Vec3 n = osg::Vec3((*normal)[i][0],
                                    (*normal)[i][1],
                                    (*normal)[i][2]);
            osg::Vec3 t = osg::Vec3((*tangent)[i][0],
                                    (*tangent)[i][1],
                                    (*tangent)[i][2]);
            osg::Vec3 t2 = osg::Vec3((*tangent2)[i][0],
                                        (*tangent2)[i][1],
                                        (*tangent2)[i][2]);

            // Gram-Schmidt orthogonalize
            osg::Vec3 t3 = (t - n * (n * t));
            t3.normalize();
            (*finalTangent)[i] = osg::Vec4(t3, 0.0);

            // Calculate handedness
            (*finalTangent)[i][3] = (((n ^ t) * t2) < 0.0) ? -1.0 : 1.0;
            // The bitangent vector B is then given by B = (N × T) · Tw
        }
        finalTangent->setUserValue("tangent", true);
        tangentIndex = (tangentIndex >= 0 ? tangentIndex : geometry.getNumVertexAttribArrays()) ;
        geometry.setVertexAttribArray(tangentIndex, finalTangent, osg::Array::BIND_PER_VERTEX);
    }
}
