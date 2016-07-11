#include "RigAttributesVisitor"


void RigAttributesVisitor::process(osgAnimation::RigGeometry& rigGeometry) {
    osg::Geometry* source = rigGeometry.getSourceGeometry();
    if(source) {
        int sourceBones = getPropertyIndex(*source, std::string("bones"));
        int rigBones = getPropertyIndex(rigGeometry, std::string("bones"));
        if(sourceBones >= 0) {
            rigBones = (rigBones >= 0 ? rigBones : rigGeometry.getNumVertexAttribArrays());
            rigGeometry.setVertexAttribArray(rigBones, source->getVertexAttribArray(sourceBones));
            source->setVertexAttribArray(sourceBones, 0);
        }

        int sourceWeights = getPropertyIndex(*source, std::string("weights"));
        int rigWeights = getPropertyIndex(rigGeometry, std::string("weights"));
        if(sourceWeights >= 0) {
            rigWeights = (rigWeights >= 0 ? rigWeights : rigGeometry.getNumVertexAttribArrays());
            rigGeometry.setVertexAttribArray(rigWeights, source->getVertexAttribArray(sourceWeights));
            source->setVertexAttribArray(sourceWeights, 0);
        }
    }
}


int RigAttributesVisitor::getPropertyIndex(const osg::Geometry& geometry, const std::string& property) {
    for(unsigned int i = 0 ; i < geometry.getNumVertexAttribArrays() ; ++ i) {
        const osg::Array* attribute = geometry.getVertexAttribArray(i);
        bool isProperty = false;
        if(attribute && attribute->getUserValue(property, isProperty) && isProperty) {
            return i;
        }
    }
    return -1;
}
