#include "DeprecatedGeometry.h"

void deprecated_osg::Geometry::setNormalBinding(AttributeBinding ab) { SET_BINDING(_normalArray.get(), ab) }
deprecated_osg::Geometry::AttributeBinding deprecated_osg::Geometry::getNormalBinding() const { return static_cast<AttributeBinding>(osg::getBinding(getNormalArray())); }

void deprecated_osg::Geometry::setColorBinding(deprecated_osg::Geometry::AttributeBinding ab) { SET_BINDING(_colorArray.get(), ab) }
deprecated_osg::Geometry::AttributeBinding deprecated_osg::Geometry::getColorBinding() const { return static_cast<AttributeBinding>(osg::getBinding(getColorArray())); }

void deprecated_osg::Geometry::setSecondaryColorBinding(deprecated_osg::Geometry::AttributeBinding ab) { SET_BINDING(_secondaryColorArray.get(), ab) }
deprecated_osg::Geometry::AttributeBinding deprecated_osg::Geometry::getSecondaryColorBinding() const { return static_cast<AttributeBinding>(osg::getBinding(getSecondaryColorArray())); }

void deprecated_osg::Geometry::setFogCoordBinding(deprecated_osg::Geometry::AttributeBinding ab) { SET_BINDING(_fogCoordArray.get(), ab) }
deprecated_osg::Geometry::AttributeBinding deprecated_osg::Geometry::getFogCoordBinding() const { return static_cast<AttributeBinding>(osg::getBinding(getFogCoordArray())); }


void deprecated_osg::Geometry::setVertexAttribBinding(unsigned int index,AttributeBinding ab)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        if (_vertexAttribList[index]->getBinding()==static_cast<osg::Array::Binding>(ab)) return;

        _vertexAttribList[index]->setBinding(static_cast<osg::Array::Binding>(ab));

        dirtyGLObjects();
    }
    else
    {
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl;
    }
}

deprecated_osg::Geometry::AttributeBinding deprecated_osg::Geometry::getVertexAttribBinding(unsigned int index) const { return static_cast<AttributeBinding>(osg::getBinding(getVertexAttribArray(index))); }

void deprecated_osg::Geometry::setVertexAttribNormalize(unsigned int index,GLboolean norm)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        _vertexAttribList[index]->setNormalize(norm!=GL_FALSE);

        dirtyGLObjects();
    }
}

GLboolean deprecated_osg::Geometry::getVertexAttribNormalize(unsigned int index) const { return osg::getNormalize(getVertexAttribArray(index)); }

void deprecated_osg::Geometry::setVertexIndices(osg::IndexArray* array)
{
    if (_vertexArray.valid()) { _vertexArray->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setVertexIndicies(..) function failed as there is no vertex array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getVertexIndices() const
{
    if (_vertexArray.valid()) return dynamic_cast<osg::IndexArray*>(_vertexArray->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setNormalIndices(osg::IndexArray* array)
{
    if (_normalArray.valid()) { _normalArray->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setNormalIndicies(..) function failed as there is no normal array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getNormalIndices() const
{
    if (_normalArray.valid()) return dynamic_cast<osg::IndexArray*>(_normalArray->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setColorIndices(osg::IndexArray* array)
{
    if (_colorArray.valid()) { _colorArray->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setColorIndicies(..) function failed as there is no color array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getColorIndices() const
{
    if (_colorArray.valid()) return dynamic_cast<osg::IndexArray*>(_colorArray->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setSecondaryColorIndices(osg::IndexArray* array)
{
    if (_secondaryColorArray.valid()) { _secondaryColorArray->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setSecondaryColorArray(..) function failed as there is no secondary color array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getSecondaryColorIndices() const
{
    if (_secondaryColorArray.valid()) return dynamic_cast<osg::IndexArray*>(_secondaryColorArray->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setFogCoordIndices(osg::IndexArray* array)
{
    if (_fogCoordArray.valid()) { _fogCoordArray->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setFogCoordIndicies(..) function failed as there is no fog coord array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getFogCoordIndices() const
{
    if (_fogCoordArray.valid()) return dynamic_cast<osg::IndexArray*>(_fogCoordArray->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setTexCoordIndices(unsigned int unit,osg::IndexArray* array)
{
    if (unit<_texCoordList.size() && _texCoordList[unit].valid()) { _texCoordList[unit]->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setTexCoordIndices(..) function failed as there is no texcoord array to associate inidices with."<<std::endl; }
}

const osg::IndexArray* deprecated_osg::Geometry::getTexCoordIndices(unsigned int unit) const
{
    if (unit<_texCoordList.size() && _texCoordList[unit].valid()) return dynamic_cast<osg::IndexArray*>(_texCoordList[unit]->getUserData());
    else return 0;
}

void deprecated_osg::Geometry::setVertexAttribIndices(unsigned int index,osg::IndexArray* array)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) { _vertexAttribList[index]->setUserData(array); if (array)  _containsDeprecatedData = true; }
    else { OSG_WARN<<"Geometry::setVertexAttribIndices(..) function failed as there is no vertex attrib array to associate inidices with."<<std::endl; }
}
const osg::IndexArray* deprecated_osg::Geometry::getVertexAttribIndices(unsigned int index) const
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) return dynamic_cast<osg::IndexArray*>(_vertexAttribList[index]->getUserData());
    else return 0;
}


