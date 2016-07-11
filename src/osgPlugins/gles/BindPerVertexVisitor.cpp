#include "BindPerVertexVisitor"


void BindPerVertexVisitor::process(osg::Geometry& geometry) {
    if (geometry.getNormalArray() && geometry.getNormalBinding() != osg::Geometry::BIND_PER_VERTEX) {
        bindPerVertex(geometry.getNormalArray(),
                      geometry.getNormalBinding(),
                      geometry.getPrimitiveSetList());
        geometry.setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    if (geometry.getColorArray() && geometry.getColorBinding() != osg::Geometry::BIND_PER_VERTEX) {
        bindPerVertex(geometry.getColorArray(),
                      geometry.getColorBinding(),
                      geometry.getPrimitiveSetList());
        geometry.setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    if (geometry.getSecondaryColorArray() && geometry.getSecondaryColorBinding() != osg::Geometry::BIND_PER_VERTEX) {
        bindPerVertex(geometry.getSecondaryColorArray(),
                      geometry.getSecondaryColorBinding(),
                      geometry.getPrimitiveSetList());
        geometry.setSecondaryColorBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    if (geometry.getFogCoordArray() && geometry.getFogCoordBinding() != osg::Geometry::BIND_PER_VERTEX) {
        bindPerVertex(geometry.getFogCoordArray(),
                      geometry.getFogCoordBinding(),
                      geometry.getPrimitiveSetList());
        geometry.setFogCoordBinding(osg::Geometry::BIND_PER_VERTEX);
    }
}


void BindPerVertexVisitor::bindPerVertex(osg::Array* src,
                                         osg::Geometry::AttributeBinding fromBinding,
                                         osg::Geometry::PrimitiveSetList& primitives) {
    if (doConvert<osg::ByteArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::ShortArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::IntArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::UByteArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::UShortArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::UIntArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::FloatArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::DoubleArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2Array>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3Array>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4Array>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2bArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3bArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4bArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2sArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3sArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4sArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2iArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3iArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4iArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2dArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3dArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4dArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2ubArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3ubArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4ubArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2usArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3usArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4usArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::Vec2uiArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec3uiArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::Vec4uiArray>(src, fromBinding, primitives))
        return;

    if (doConvert<osg::MatrixfArray>(src, fromBinding, primitives))
        return;
    if (doConvert<osg::MatrixdArray>(src, fromBinding, primitives))
        return;
}


template <class T>
bool BindPerVertexVisitor::doConvert(osg::Array* src,
                                     osg::Geometry::AttributeBinding fromBinding,
                                     osg::Geometry::PrimitiveSetList& primitives) {
    T* array= dynamic_cast<T*>(src);
    if (array) {
        convert(*array, fromBinding, primitives);
        return true;
    }
    return false;
}


template <class T>
void BindPerVertexVisitor::convert(T& array,
                                   osg::Geometry::AttributeBinding fromBinding,
                                   osg::Geometry::PrimitiveSetList& primitives) {
    osg::ref_ptr<T> result = new T();
    for (unsigned int p = 0; p < primitives.size(); p++) {
        switch ( primitives[p]->getMode() ) {
        case osg::PrimitiveSet::POINTS:
            osg::notify(osg::WARN) << "ConvertToBindPerVertex not supported for POINTS" << std::endl;
            break;

        case osg::PrimitiveSet::LINE_STRIP:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                unsigned int nb = primitives[p]->getNumIndices();
                for (unsigned int i = 0; i < nb; i++)
                    result->push_back(array[p]);
            }
            break;
            }
            break;

        case osg::PrimitiveSet::LINES:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                unsigned int nb = primitives[p]->getNumIndices();
                for (unsigned int i = 0; i < nb; i++)
                    result->push_back(array[p]);
            }
            break;
            }
            break;

        case osg::PrimitiveSet::TRIANGLES:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                unsigned int nb = primitives[p]->getNumIndices();
                for (unsigned int i = 0; i < nb; i++)
                    result->push_back(array[p]);
            }
            break;
            }
            break;

        case osg::PrimitiveSet::TRIANGLE_STRIP:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                osg::notify(osg::FATAL) << "Can't convert Array from BIND_PER_PRIMITIVE_SET to BIND_PER_VERTEX, for TRIANGLE_STRIP" << std::endl;
            }
            break;
            }
            break;

        case osg::PrimitiveSet::TRIANGLE_FAN:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                osg::notify(osg::FATAL) << "Can't convert Array from BIND_PER_PRIMITIVE_SET to BIND_PER_VERTEX, for TRIANGLE_FAN" << std::endl;
            }
            break;
            }
            break;

        case osg::PrimitiveSet::QUADS:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                osg::notify(osg::FATAL) << "Can't convert Array from BIND_PER_PRIMITIVE_SET to BIND_PER_VERTEX, for QUADS" << std::endl;
            }
            break;
            }
            break;

        case osg::PrimitiveSet::QUAD_STRIP:
            switch(fromBinding) {
            case osg::Geometry::BIND_OFF:
            case osg::Geometry::BIND_PER_VERTEX:
                break;
            case osg::Geometry::BIND_OVERALL:
            {
                for (unsigned int i = 0; i < primitives[p]->getNumIndices(); i++)
                    result->push_back(array[0]);
            }
            break;
            case osg::Geometry::BIND_PER_PRIMITIVE_SET:
            {
                osg::notify(osg::FATAL) << "Can't convert Array from BIND_PER_PRIMITIVE_SET to BIND_PER_VERTEX, for QUAD_STRIP" << std::endl;
            }
            break;
            }
            break;
        }
    }
    array = *result;
}
