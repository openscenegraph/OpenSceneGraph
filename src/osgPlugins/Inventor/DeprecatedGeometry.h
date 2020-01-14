/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSG_DEPRECATED_GEOMETRY
#define OSG_DEPRECATED_GEOMETRY 1

#include <osg/Geometry>

/** Contains deprecated features of namespace osg. */
namespace deprecated_osg {


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Deprecated methods.
//
#define SET_BINDING(array, ab)\
    if (!array) \
    { \
        if (ab==BIND_OFF) return; \
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl; \
        return; \
    } \
    if (array->getBinding() == static_cast<osg::Array::Binding>(ab)) return; \
    array->setBinding(static_cast<osg::Array::Binding>(ab));\
    if (ab==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) _containsDeprecatedData = true; \
    dirtyGLObjects();


#define GET_BINDING(array) (array!=0 ? static_cast<AttributeBinding>(array->getBinding()) : BIND_OFF)


/** Geometry class containing deprecated features.
 * Users should only use deprecated_osg::Geometry when absolutely necessary for keeping things compiling,
 * it is recommended that you should migrate your code to work just with osg::Geometry as existing
 * deprecated_osg::Geometry will be removed in future release.
*/
class Geometry : public osg::Geometry
{
    public:
        Geometry() : osg::Geometry() {}
        Geometry(const Geometry& geometry,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) : osg::Geometry(geometry, copyop) {}

        /** Same values as Array::Binding.*/
        enum AttributeBinding
        {
            BIND_OFF=0,
            BIND_OVERALL=1,
            BIND_PER_PRIMITIVE_SET=2,
            BIND_PER_PRIMITIVE=3,
            BIND_PER_VERTEX=4
        };

        void setNormalBinding(AttributeBinding ab);
        AttributeBinding getNormalBinding() const;

        void setColorBinding(AttributeBinding ab);
        AttributeBinding getColorBinding() const;

        void setSecondaryColorBinding(AttributeBinding ab);
        AttributeBinding getSecondaryColorBinding() const;

        void setFogCoordBinding(AttributeBinding ab);
        AttributeBinding getFogCoordBinding() const;

        void setVertexAttribBinding(unsigned int index,AttributeBinding ab);
        AttributeBinding getVertexAttribBinding(unsigned int index) const;

        void setVertexAttribNormalize(unsigned int index,GLboolean norm);
        GLboolean getVertexAttribNormalize(unsigned int index) const;

        void setVertexIndices(osg::IndexArray* array);
        const osg::IndexArray* getVertexIndices() const;

        void setNormalIndices(osg::IndexArray* array);
        const osg::IndexArray* getNormalIndices() const;

        void setColorIndices(osg::IndexArray* array);
        const osg::IndexArray* getColorIndices() const;

        void setSecondaryColorIndices(osg::IndexArray* array);
        const osg::IndexArray* getSecondaryColorIndices() const;

        void setFogCoordIndices(osg::IndexArray* array);
        const osg::IndexArray* getFogCoordIndices() const;

        void setTexCoordIndices(unsigned int unit,osg::IndexArray* array);
        const osg::IndexArray* getTexCoordIndices(unsigned int unit) const;

        void setVertexAttribIndices(unsigned int index,osg::IndexArray* array);
        const osg::IndexArray* getVertexAttribIndices(unsigned int index) const;

};

} // namespace deprecated_osg

#endif

