#include "SubGeometry"


SubGeometry::SubGeometry(const osg::Geometry& source,
                         const std::vector<unsigned int>& triangles,
                         const std::vector<unsigned int>& lines,
                         const std::vector<unsigned int>& wireframe,
                         const std::vector<unsigned int>& points)
{
    // Create new geometry as we will modify vertex arrays and primitives (the process is
    // equivalent to deep cloning).
    // As UserValues might be changed on a per-geometry basis afterwards, we deep clone userdata
    // We do *not* want to clone statesets as they reference a UniqueID that should be unique
    // (see #83056464).
    if(dynamic_cast<const osgAnimation::MorphGeometry*>(&source)) {
        _geometry = new osgAnimation::MorphGeometry;
    }
    else {
        _geometry = new osg::Geometry;
    }

    if(source.getUserDataContainer()) {
        _geometry->setUserDataContainer(osg::clone(source.getUserDataContainer(), osg::CopyOp::DEEP_COPY_ALL));
    }

    if(source.getStateSet()) {
        _geometry->setStateSet(const_cast<osg::StateSet*>(source.getStateSet()));
    }

    addSourceBuffers(_geometry.get(), source);

    // process morph targets if needed
    if(const osgAnimation::MorphGeometry* morphSource = dynamic_cast<const osgAnimation::MorphGeometry*>(&source)) {
        osgAnimation::MorphGeometry* morph = dynamic_cast<osgAnimation::MorphGeometry*>(_geometry.get());
        if (morph)
        {
            const osgAnimation::MorphGeometry::MorphTargetList& morphTargetList = morphSource->getMorphTargetList();

            osgAnimation::MorphGeometry::MorphTargetList::const_iterator targetSource;
            for(targetSource = morphTargetList.begin() ; targetSource != morphTargetList.end() ; ++ targetSource) {
                if(targetSource->getGeometry()) {
                    osg::Geometry* target = new osg::Geometry;
                    addSourceBuffers(target, *targetSource->getGeometry());
                    morph->addMorphTarget(target, targetSource->getWeight());
                }
            }
        }
    }

    // remap primitives indices by decreasing ordering (triangles > lines > wireframe > points)
    for(unsigned int i = 0 ; i < triangles.size() ; i += 3) {
        copyTriangle(triangles[i], triangles[i + 1], triangles[i + 2]);
    }

    for(unsigned int i = 0 ; i < lines.size() ; i += 2) {
        copyEdge(lines[i], lines[i + 1], false);
    }

    for(unsigned int i = 0 ; i < wireframe.size() ; i += 2) {
        copyEdge(wireframe[i], wireframe[i + 1], true);
    }

    for(unsigned int i = 0 ; i < points.size() ; ++ i) {
        copyPoint(points[i]);
    }

    // remap vertex buffers accordingly to primitives
    for(BufferIterator it = _bufferMap.begin() ; it != _bufferMap.end() ; ++ it) {
        if(it->first) {
            copyFrom(*(it->second), *(it->first));
        }
    }
}


void SubGeometry::addSourceBuffers(osg::Geometry* geometry, const osg::Geometry& source) {
    // create necessary vertex containers
    const osg::Array* array = 0;

    geometry->setName(source.getName());

    // collect all buffers that are BIND_PER_VERTEX for eventual vertex duplication
    if( (array = vertexArray(source.getVertexArray()))  ) {
        geometry->setVertexArray(makeVertexBuffer(array));
    }

    if( (array = vertexArray(source.getNormalArray())) ){
        geometry->setNormalArray(makeVertexBuffer(array));
    }

    if( (array = vertexArray(source.getColorArray())) ){
        geometry->setColorArray(makeVertexBuffer(array));
    }

    if( (array = vertexArray(source.getSecondaryColorArray())) ){
        geometry->setSecondaryColorArray(makeVertexBuffer(array));
    }

    if( (array = vertexArray(source.getFogCoordArray())) ){
        geometry->setFogCoordArray(makeVertexBuffer(array));
    }

    for(unsigned int i = 0; i < source.getNumVertexAttribArrays(); ++ i) {
        if( (array = vertexArray(source.getVertexAttribArray(i))) ){
            geometry->setVertexAttribArray(i, makeVertexBuffer(array));
        }
    }

    for(unsigned int i = 0; i < source.getNumTexCoordArrays(); ++ i) {
        if( (array = vertexArray(source.getTexCoordArray(i))) ){
            geometry->setTexCoordArray(i, makeVertexBuffer(array));
        }
    }
}


void SubGeometry::copyTriangle(unsigned int v1, unsigned int v2, unsigned int v3) {
    osg::DrawElements* triangles = getOrCreateTriangles();
    triangles->addElement(mapVertex(v1));
    triangles->addElement(mapVertex(v2));
    triangles->addElement(mapVertex(v3));
}


void SubGeometry::copyEdge(unsigned int v1, unsigned int v2, bool wireframe) {
    osg::DrawElements* edges = getOrCreateLines(wireframe);
    edges->addElement(mapVertex(v1));
    edges->addElement(mapVertex(v2));
}


void SubGeometry::copyPoint(unsigned int v1) {
    osg::DrawElements* points = getOrCreatePoints();
    points->addElement(mapVertex(v1));
}


const osg::Array* SubGeometry::vertexArray(const osg::Array* array) {
    // filter invalid vertex buffers
    if (array && array->getNumElements() && array->getBinding() == osg::Array::BIND_PER_VERTEX) {
        return array;
    }
    else {
        return 0;
    }
}


osg::Array* SubGeometry::makeVertexBuffer(const osg::Array* array, bool copyUserData) {
    osg::Array* buffer = array ? osg::cloneType(array) : 0;
    if(buffer) {
        buffer->setBinding(osg::Array::BIND_PER_VERTEX);
        if(copyUserData && array->getUserDataContainer()) {
            buffer->setUserDataContainer(osg::clone(array->getUserDataContainer(), osg::CopyOp::DEEP_COPY_ALL));
        }
        _bufferMap[buffer] = array;
    }
    return buffer;
}


template<typename C>
void SubGeometry::copyValues(const C& src, C& dst) {
    dst.resize(_indexMap.size());
    for(IndexMapping::const_iterator remapper = _indexMap.begin() ; remapper != _indexMap.end() ; ++ remapper) {
        dst[remapper->second] = src[remapper->first];
    }
}


#define COPY_TEMPLATE(T) \
if (dynamic_cast<const T*>(&src)) \
{ copyValues<T>(dynamic_cast<const T&>(src), dynamic_cast<T&>(dst)); }

void SubGeometry::copyFrom(const osg::Array& src, osg::Array& dst) {
    COPY_TEMPLATE(osg::Vec2Array);
    COPY_TEMPLATE(osg::Vec3Array);
    COPY_TEMPLATE(osg::Vec4Array);

    COPY_TEMPLATE(osg::Vec2dArray);
    COPY_TEMPLATE(osg::Vec3dArray);
    COPY_TEMPLATE(osg::Vec4dArray);

    COPY_TEMPLATE(osg::ByteArray);
    COPY_TEMPLATE(osg::ShortArray);
    COPY_TEMPLATE(osg::IntArray);
    COPY_TEMPLATE(osg::UByteArray);
    COPY_TEMPLATE(osg::UShortArray);
    COPY_TEMPLATE(osg::UIntArray);
    COPY_TEMPLATE(osg::FloatArray);
    COPY_TEMPLATE(osg::DoubleArray);

    COPY_TEMPLATE(osg::Vec2iArray);
    COPY_TEMPLATE(osg::Vec3iArray);
    COPY_TEMPLATE(osg::Vec4iArray);

    COPY_TEMPLATE(osg::Vec2uiArray);
    COPY_TEMPLATE(osg::Vec3uiArray);
    COPY_TEMPLATE(osg::Vec4uiArray);

    COPY_TEMPLATE(osg::Vec2sArray);
    COPY_TEMPLATE(osg::Vec3sArray);
    COPY_TEMPLATE(osg::Vec4sArray);

    COPY_TEMPLATE(osg::Vec2usArray);
    COPY_TEMPLATE(osg::Vec3usArray);
    COPY_TEMPLATE(osg::Vec4usArray);

    COPY_TEMPLATE(osg::Vec2bArray);
    COPY_TEMPLATE(osg::Vec3bArray);
    COPY_TEMPLATE(osg::Vec4bArray);

    COPY_TEMPLATE(osg::Vec2ubArray);
    COPY_TEMPLATE(osg::Vec3ubArray);
    COPY_TEMPLATE(osg::Vec4ubArray);

    COPY_TEMPLATE(osg::MatrixfArray);

    COPY_TEMPLATE(osg::QuatArray);
}


unsigned int SubGeometry::mapVertex(unsigned int i) {
    if(_indexMap.find(i) == _indexMap.end()) {
        unsigned int index = _indexMap.size();
        _indexMap[i] = index;
    }
    return _indexMap[i];
}


osg::DrawElements* SubGeometry::getOrCreateTriangles() {
    if(_primitives.find("triangles") == _primitives.end()) {
        _primitives["triangles"] = new osg::DrawElementsUInt(GL_TRIANGLES);
        _geometry->addPrimitiveSet(_primitives["triangles"]);
    }
    return _primitives["triangles"];
}


osg::DrawElements* SubGeometry::getOrCreateLines(bool wireframe) {
    std::string label = wireframe ? "wireframe" : "lines";

    if(_primitives.find(label) == _primitives.end()) {
        _primitives[label] = new osg::DrawElementsUInt(GL_LINES);
        if(wireframe) {
            _primitives[label]->setUserValue("wireframe", true);
        }
        _geometry->addPrimitiveSet(_primitives[label]);
    }
    return _primitives[label];
}


osg::DrawElements* SubGeometry::getOrCreatePoints() {
    if(_primitives.find("points") == _primitives.end()) {
        _primitives["points"] = new osg::DrawElementsUInt(GL_POINTS);
        _geometry->addPrimitiveSet(_primitives["points"]);
    }
    return _primitives["points"];
}
