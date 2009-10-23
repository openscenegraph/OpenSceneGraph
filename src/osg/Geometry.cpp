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
#include <stdlib.h>

#include <osg/Geometry>
#include <osg/ArrayDispatchers>
#include <osg/Notify>

using namespace osg;

const Geometry::ArrayData Geometry::s_InvalidArrayData;

Geometry::ArrayData::ArrayData(const ArrayData& data,const CopyOp& copyop):
    array(copyop(data.array.get())),
    indices(dynamic_cast<osg::IndexArray*>(copyop(data.indices.get()))),
    binding(data.binding),
    normalize(data.normalize)
{
}

Geometry::Vec3ArrayData::Vec3ArrayData(const Vec3ArrayData& data,const CopyOp& copyop):
    array(dynamic_cast<osg::Vec3Array*>(copyop(data.array.get()))),
    indices(dynamic_cast<osg::IndexArray*>(copyop(data.indices.get()))),
    binding(data.binding),
    normalize(data.normalize)
{
}

Geometry::Geometry()
{
    // temporary test
    // setSupportsDisplayList(false);

    _fastPath = false;
    _fastPathHint = true;
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexData(geometry._vertexData,copyop),
    _normalData(geometry._normalData,copyop),
    _colorData(geometry._colorData,copyop),
    _secondaryColorData(geometry._secondaryColorData,copyop),
    _fogCoordData(geometry._fogCoordData,copyop),
    _fastPath(geometry._fastPath),
    _fastPathHint(geometry._fastPathHint)
{
    // temporary test
    // setSupportsDisplayList(false);

    for(PrimitiveSetList::const_iterator pitr=geometry._primitives.begin();
        pitr!=geometry._primitives.end();
        ++pitr)
    {
        PrimitiveSet* primitive = copyop(pitr->get());
        if (primitive) _primitives.push_back(primitive);
    }

    for(ArrayDataList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(ArrayData(*titr, copyop));
    }

    for(ArrayDataList::const_iterator vitr=geometry._vertexAttribList.begin();
        vitr!=geometry._vertexAttribList.end();
        ++vitr)
    {
        _vertexAttribList.push_back(ArrayData(*vitr, copyop));
    }
}

Geometry::~Geometry()
{
    // do dirty here to keep the getGLObjectSizeHint() estimate on the ball
    dirtyDisplayList();
    
    // no need to delete, all automatically handled by ref_ptr :-)
}

bool Geometry::empty() const
{
    if (!_primitives.empty()) return false;
    if (!_vertexData.empty()) return false;
    if (!_normalData.empty()) return false;
    if (!_colorData.empty()) return false;
    if (!_secondaryColorData.empty()) return false;
    if (!_fogCoordData.empty()) return false;
    if (!_texCoordList.empty()) return false;
    if (!_vertexAttribList.empty()) return false;
    return true;
}

void Geometry::setVertexArray(Array* array)
{
    _vertexData.array = array; 
    computeFastPathsUsed(); 
    dirtyDisplayList(); 
    dirtyBound();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setVertexIndices(IndexArray* array)
{
    _vertexData.indices = array; 
    computeFastPathsUsed(); 
    dirtyDisplayList(); 
    dirtyBound();
}

void Geometry::setVertexData(const ArrayData& arrayData)
{
    _vertexData = arrayData; 
    computeFastPathsUsed(); 
    dirtyDisplayList(); 
    dirtyBound();

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}

void Geometry::setNormalArray(Array* array)
{
    _normalData.array = array; 
    if (!_normalData.array.valid()) _normalData.binding=BIND_OFF; 
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setNormalIndices(IndexArray* array)
{
    _normalData.indices = array; 
    computeFastPathsUsed(); 
    dirtyDisplayList(); 
}

void Geometry::setNormalData(const ArrayData& arrayData)
{
    _normalData = arrayData;  
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}

void Geometry::setColorArray(Array* array)
{
    _colorData.array = array;
    if (!_colorData.array.valid()) _colorData.binding=BIND_OFF; 
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setColorIndices(IndexArray* array)
{
    _colorData.indices = array;  
    computeFastPathsUsed(); 
    dirtyDisplayList(); 
}

void Geometry::setColorData(const ArrayData& arrayData)
{
    _colorData = arrayData;  
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}


void Geometry::setSecondaryColorArray(Array* array)
{
    _secondaryColorData.array = array; 
    if (!_secondaryColorData.array.valid()) _secondaryColorData.binding=BIND_OFF; 
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setSecondaryColorIndices(IndexArray* array)
{
    _secondaryColorData.indices = array; 
    computeFastPathsUsed(); 
    dirtyDisplayList();
}

void Geometry::setSecondaryColorData(const ArrayData& arrayData)
{
    _secondaryColorData = arrayData; 
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}

void Geometry::setFogCoordArray(Array* array)
{
    _fogCoordData.array = array; 
    if (!_fogCoordData.array.valid()) _fogCoordData.binding=BIND_OFF; 
    computeFastPathsUsed(); 
    dirtyDisplayList(); 

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setFogCoordIndices(IndexArray* array)
{
    _fogCoordData.indices = array; 
    computeFastPathsUsed(); 
    dirtyDisplayList();
}

void Geometry::setFogCoordData(const ArrayData& arrayData)
{
    _fogCoordData = arrayData; 
    computeFastPathsUsed(); 
    dirtyDisplayList();

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}

void Geometry::setNormalBinding(AttributeBinding ab) 
{
    if (_normalData.binding == ab) return;
    
    _normalData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setColorBinding(AttributeBinding ab)
{
    if (_colorData.binding == ab) return;

    _colorData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setSecondaryColorBinding(AttributeBinding ab)
{
    if (_secondaryColorData.binding == ab) return;

    _secondaryColorData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setFogCoordBinding(AttributeBinding ab)
{
    if (_fogCoordData.binding == ab) return;

    _fogCoordData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setTexCoordData(unsigned int unit,const ArrayData& arrayData)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
    
    _texCoordList[unit] = arrayData;

    if (_useVertexBufferObjects && arrayData.array.valid()) addVertexBufferObjectIfRequired(arrayData.array.get());
}

Geometry::ArrayData& Geometry::getTexCoordData(unsigned int unit)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
        
    return _texCoordList[unit];
}

const Geometry::ArrayData& Geometry::getTexCoordData(unsigned int unit) const
{
    if (_texCoordList.size()<=unit)
        return s_InvalidArrayData;
        
    return _texCoordList[unit];
}

void Geometry::setTexCoordArray(unsigned int unit,Array* array)
{
    getTexCoordData(unit).binding = BIND_PER_VERTEX;
    getTexCoordData(unit).array = array;

    computeFastPathsUsed();
    dirtyDisplayList();

    if (_useVertexBufferObjects && array)
    {
        addVertexBufferObjectIfRequired(array);
    }
}

Array* Geometry::getTexCoordArray(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].array.get();
    else return 0;
}

const Array* Geometry::getTexCoordArray(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].array.get();
    else return 0;
}

void Geometry::setTexCoordIndices(unsigned int unit,IndexArray* array)
{
    getTexCoordData(unit).indices = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

IndexArray* Geometry::getTexCoordIndices(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].indices.get();
    else return 0;
}

const IndexArray* Geometry::getTexCoordIndices(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].indices.get();
    else return 0;
}


void Geometry::setVertexAttribData(unsigned int index, const Geometry::ArrayData& attrData)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);
        
    _vertexAttribList[index] = attrData;
 
    computeFastPathsUsed();
    dirtyDisplayList();

    if (_useVertexBufferObjects && attrData.array.valid()) addVertexBufferObjectIfRequired(attrData.array.get());
}

Geometry::ArrayData& Geometry::getVertexAttribData(unsigned int index)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);
        
    return _vertexAttribList[index];
}

const Geometry::ArrayData& Geometry::getVertexAttribData(unsigned int index) const
{
    if (_vertexAttribList.size()<=_vertexAttribList.size())
        return s_InvalidArrayData;
        
    return _vertexAttribList[index];
}

void Geometry::setVertexAttribArray(unsigned int index, Array* array)
{
    getVertexAttribData(index).array = array;

    computeFastPathsUsed();
    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

Array *Geometry::getVertexAttribArray(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].array.get();
    else return 0;
}

const Array *Geometry::getVertexAttribArray(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].array.get();
    else return 0;
}

void Geometry::setVertexAttribIndices(unsigned int index,IndexArray* array)
{
    getVertexAttribData(index).indices = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

IndexArray* Geometry::getVertexAttribIndices(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].indices.get();
    else return 0;
}

const IndexArray* Geometry::getVertexAttribIndices(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].indices.get();
    else return 0;
}

void Geometry::setVertexAttribBinding(unsigned int index,AttributeBinding ab)
{
    if (getVertexAttribData(index).binding == ab)
       return;
    getVertexAttribData(index).binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList();
}

Geometry::AttributeBinding Geometry::getVertexAttribBinding(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].binding;
    else return BIND_OFF;
}

void Geometry::setVertexAttribNormalize(unsigned int index,GLboolean norm)
{
    getVertexAttribData(index).normalize = norm;

    dirtyDisplayList();
}

GLboolean Geometry::getVertexAttribNormalize(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].normalize;
    else return GL_FALSE;
}

bool Geometry::addPrimitiveSet(PrimitiveSet* primitiveset)
{
    if (primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives.push_back(primitiveset);
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::addPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::setPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{
    if (i<_primitives.size() && primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives[i] = primitiveset;
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::setPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::insertPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{

    if (primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        if (i<_primitives.size())
        {
            _primitives.insert(_primitives.begin()+i,primitiveset);
            dirtyDisplayList();
            dirtyBound();
            return true;
        }
        else if (i==_primitives.size())
        {
            return addPrimitiveSet(primitiveset);
        }
        
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::insertPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::removePrimitiveSet(unsigned int i, unsigned int numElementsToRemove)
{
    if (numElementsToRemove==0) return false;

    if (i<_primitives.size())
    {
        if (i+numElementsToRemove<=_primitives.size())
        {
            _primitives.erase(_primitives.begin()+i,_primitives.begin()+i+numElementsToRemove);
        }
        else
        {
            // asking to delete too many elements, report a warning, and delete to
            // the end of the primitive list.
            notify(WARN)<<"Warning: osg::Geometry::removePrimitiveSet(i,numElementsToRemove) has been asked to remove more elements than are available,"<<std::endl;
            notify(WARN)<<"         removing on from i to the end of the list of primitive sets."<<std::endl;
            _primitives.erase(_primitives.begin()+i,_primitives.end());
        }
    
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i passed to osg::Geometry::removePrimitiveSet(i,numElementsToRemove), ignoring call."<<std::endl;
    return false;
}

unsigned int Geometry::getPrimitiveSetIndex(const PrimitiveSet* primitiveset) const
{
    for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
    {
        if (_primitives[primitiveSetIndex]==primitiveset) return primitiveSetIndex;
    }
    return _primitives.size(); // node not found.
}

bool Geometry::computeFastPathsUsed()
{
    static bool s_DisableFastPathInDisplayLists = getenv("OSG_DISABLE_FAST_PATH_IN_DISPLAY_LISTS")!=0;
    if (_useDisplayList && s_DisableFastPathInDisplayLists)
    {
        osg::notify(osg::DEBUG_INFO)<<"Geometry::computeFastPathsUsed() - Disabling fast paths in display lists"<<std::endl;
        _supportsVertexBufferObjects = _fastPath = false;
        return _fastPath;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // check to see if fast path can be used.
    //
    _fastPath = true;
    if (_vertexData.indices.valid()) _fastPath = false;
    else if (_normalData.binding==BIND_PER_PRIMITIVE || (_normalData.binding==BIND_PER_VERTEX && _normalData.indices.valid())) _fastPath = false;
    else if (_colorData.binding==BIND_PER_PRIMITIVE || (_colorData.binding==BIND_PER_VERTEX && _colorData.indices.valid())) _fastPath = false;
    else if (_secondaryColorData.binding==BIND_PER_PRIMITIVE || (_secondaryColorData.binding==BIND_PER_VERTEX && _secondaryColorData.indices.valid())) _fastPath = false;
    else if (_fogCoordData.binding==BIND_PER_PRIMITIVE || (_fogCoordData.binding==BIND_PER_VERTEX && _fogCoordData.indices.valid())) _fastPath = false;
    else 
    {
        for( unsigned int va = 0; va < _vertexAttribList.size(); ++va )
        {
            if (_vertexAttribList[va].binding==BIND_PER_PRIMITIVE)
            {
                _fastPath = false;
                break;
            }
            else
            {
                const Array * array = _vertexAttribList[va].array.get();
                const IndexArray * idxArray = _vertexAttribList[va].indices.get();

                if( _vertexAttribList[va].binding==BIND_PER_VERTEX && 
                    array && array->getNumElements()>0 &&
                    idxArray && idxArray->getNumElements()>0 ) 
                {
                    _fastPath = false;
                    break;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up tex coords if required.
    //
    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        const ArrayData& texcoordData = _texCoordList[unit];
        if (texcoordData.array.valid() && texcoordData.array->getNumElements()>0)
        {
            if (texcoordData.indices.valid())
            {
                if (texcoordData.indices->getNumElements()>0)
                {
                    _fastPath = false;         
                    break;
                }
            }
        }
    }
    
    _supportsVertexBufferObjects = _fastPath;
    
    //_supportsVertexBufferObjects = false;
    //_useVertexBufferObjects = false;

    return _fastPath;
}

unsigned int Geometry::getGLObjectSizeHint() const
{
    unsigned int totalSize = 0;
    if (_vertexData.array.valid()) totalSize += _vertexData.array->getTotalDataSize();

    if (_normalData.array.valid()) totalSize += _normalData.array->getTotalDataSize();

    if (_colorData.array.valid()) totalSize += _colorData.array->getTotalDataSize();

    if (_secondaryColorData.array.valid()) totalSize += _secondaryColorData.array->getTotalDataSize();

    if (_fogCoordData.array.valid()) totalSize += _fogCoordData.array->getTotalDataSize();


    unsigned int unit;
    for(unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].array.get();
        if (array) totalSize += array->getTotalDataSize();

    }

    bool handleVertexAttributes = true;
    if (handleVertexAttributes)
    {
        unsigned int index;
        for( index = 0; index < _vertexAttribList.size(); ++index )
        {
            const Array* array = _vertexAttribList[index].array.get();
            if (array) totalSize += array->getTotalDataSize();           
        }
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {

        totalSize += 4*(*itr)->getNumIndices();

    }


    // do a very simply mapping of display list size proportional to vertex datasize.
    return totalSize;
}

bool Geometry::getArrayList(ArrayList& arrayList) const
{
    unsigned int startSize = arrayList.size();
    
    if (_vertexData.array.valid()) arrayList.push_back(_vertexData.array.get());
    if (_normalData.array.valid()) arrayList.push_back(_normalData.array.get());
    if (_colorData.array.valid()) arrayList.push_back(_colorData.array.get());
    if (_secondaryColorData.array.valid()) arrayList.push_back(_secondaryColorData.array.get());
    if (_fogCoordData.array.valid()) arrayList.push_back(_fogCoordData.array.get());
    
    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        Array* array = _texCoordList[unit].array.get();
        if (array) arrayList.push_back(array);
    }

    for(unsigned int  index = 0; index < _vertexAttribList.size(); ++index )
    {
        Array* array = _vertexAttribList[index].array.get();
        if (array) arrayList.push_back(array);
    }

    return arrayList.size()!=startSize;
}

bool Geometry::getDrawElementsList(DrawElementsList& drawElementsList) const
{
    unsigned int startSize = drawElementsList.size();
    
    for(PrimitiveSetList::const_iterator itr = _primitives.begin();
        itr != _primitives.end();
        ++itr)
    {
        osg::DrawElements* de = (*itr)->getDrawElements();
        if (de) drawElementsList.push_back(de);
    }
    
    return drawElementsList.size()!=startSize;
}

void Geometry::addVertexBufferObjectIfRequired(osg::Array* array)
{
    if (_useVertexBufferObjects)
    {
        if (!array->getVertexBufferObject())
        {
            array->setVertexBufferObject(getOrCreateVertexBufferObject());
        }
    }
}

void Geometry::addElementBufferObjectIfRequired(osg::PrimitiveSet* primitiveSet)
{
    if (_useVertexBufferObjects)
    {
        osg::DrawElements* drawElements = primitiveSet->getDrawElements();
        if (drawElements && !drawElements->getElementBufferObject())
        {
            drawElements->setElementBufferObject(getOrCreateElementBufferObject());
        }
    }
}

osg::VertexBufferObject* Geometry::getOrCreateVertexBufferObject()
{
    ArrayList arrayList;
    getArrayList(arrayList);

    ArrayList::iterator vitr;
    for(vitr = arrayList.begin();
        vitr != arrayList.end();
        ++vitr)
    {
        osg::Array* array = *vitr;
        if (array->getVertexBufferObject()) return array->getVertexBufferObject();
    }

    return new osg::VertexBufferObject;
}

osg::ElementBufferObject* Geometry::getOrCreateElementBufferObject()
{
    DrawElementsList drawElementsList;
    getDrawElementsList(drawElementsList);

    DrawElementsList::iterator deitr;
    for(deitr = drawElementsList.begin();
        deitr != drawElementsList.end();
        ++deitr)
    {
        osg::DrawElements* elements = *deitr;
        if (elements->getElementBufferObject()) return elements->getElementBufferObject();
    }

    return new osg::ElementBufferObject;
}

void Geometry::setUseVertexBufferObjects(bool flag)
{
    // flag = true;
    
    // osg::notify(osg::NOTICE)<<"Geometry::setUseVertexBufferObjects("<<flag<<")"<<std::endl;

    if (_useVertexBufferObjects==flag) return;

    Drawable::setUseVertexBufferObjects(flag);
    
    ArrayList arrayList;
    getArrayList(arrayList);

    DrawElementsList drawElementsList;
    getDrawElementsList(drawElementsList);

    typedef std::vector<osg::VertexBufferObject*>  VertexBufferObjectList;
    typedef std::vector<osg::ElementBufferObject*>  ElementBufferObjectList;

    if (_useVertexBufferObjects)
    {
        if (!arrayList.empty()) 
        {

            VertexBufferObjectList vboList;
            
            osg::VertexBufferObject* vbo = 0;

            ArrayList::iterator vitr;
            for(vitr = arrayList.begin();
                vitr != arrayList.end() && !vbo;
                ++vitr)
            {
                osg::Array* array = *vitr;
                if (array->getVertexBufferObject()) vbo = array->getVertexBufferObject();
            }

            if (!vbo) vbo = new osg::VertexBufferObject;

            for(vitr = arrayList.begin();
                vitr != arrayList.end();
                ++vitr)
            {
                osg::Array* array = *vitr;
                if (!array->getVertexBufferObject()) array->setVertexBufferObject(vbo);
            }
        }

        if (!drawElementsList.empty()) 
        {
            ElementBufferObjectList eboList;
            
            osg::ElementBufferObject* ebo = 0;

            DrawElementsList::iterator deitr;
            for(deitr = drawElementsList.begin();
                deitr != drawElementsList.end();
                ++deitr)
            {
                osg::DrawElements* elements = *deitr;
                if (elements->getElementBufferObject()) ebo = elements->getElementBufferObject();
            }

            if (!ebo) ebo = new osg::ElementBufferObject;

            for(deitr = drawElementsList.begin();
                deitr != drawElementsList.end();
                ++deitr)
            {
                osg::DrawElements* elements = *deitr;
                if (!elements->getElementBufferObject()) elements->setElementBufferObject(ebo);
            }
        }
    }
    else
    {
        for(ArrayList::iterator vitr = arrayList.begin();
            vitr != arrayList.end();
            ++vitr)
        {
            osg::Array* array = *vitr;
            if (array->getVertexBufferObject()) array->setVertexBufferObject(0);
        }

        for(DrawElementsList::iterator deitr = drawElementsList.begin();
            deitr != drawElementsList.end();
            ++deitr)
        {
            osg::DrawElements* elements = *deitr;
            if (elements->getElementBufferObject()) elements->setElementBufferObject(0);
        }
    }
}

void Geometry::dirtyDisplayList()
{
    Drawable::dirtyDisplayList();
}

void Geometry::resizeGLObjectBuffers(unsigned int maxSize)
{
    Drawable::resizeGLObjectBuffers(maxSize);

    ArrayList arrays;
    if (getArrayList(arrays))
    {
        for(ArrayList::iterator itr = arrays.begin();
            itr != arrays.end();
            ++itr)
        {
            (*itr)->resizeGLObjectBuffers(maxSize);
        }
    }

    DrawElementsList drawElements;
    if (getDrawElementsList(drawElements))
    {
        for(DrawElementsList::iterator itr = drawElements.begin();
            itr != drawElements.end();
            ++itr)
        {
            (*itr)->resizeGLObjectBuffers(maxSize);
        }
    }
}

void Geometry::releaseGLObjects(State* state) const
{
    Drawable::releaseGLObjects(state);

    ArrayList arrays;
    if (getArrayList(arrays))
    {
        for(ArrayList::iterator itr = arrays.begin();
            itr != arrays.end();
            ++itr)
        {
            (*itr)->releaseGLObjects(state);
        }
    }

    DrawElementsList drawElements;
    if (getDrawElementsList(drawElements))
    {
        for(DrawElementsList::iterator itr = drawElements.begin();
            itr != drawElements.end();
            ++itr)
        {
            (*itr)->releaseGLObjects(state);
        }
    }

}

void Geometry::drawImplementation(RenderInfo& renderInfo) const
{
    if (_internalOptimizedGeometry.valid())
    {
        _internalOptimizedGeometry->drawImplementation(renderInfo);
        return;
    }

    State& state = *renderInfo.getState();
    Drawable::Extensions* extensions = Drawable::getExtensions(state.getContextID(),true);

    bool useFastPath = areFastPathsUsed();

    bool usingVertexBufferObjects = _useVertexBufferObjects && state.isVertexBufferObjectSupported();
    bool handleVertexAttributes = !_vertexAttribList.empty() && extensions->isVertexProgramSupported();

    ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();

    arrayDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());
    arrayDispatchers.reset();
    arrayDispatchers.setUseGLBeginEndAdapter(!useFastPath);

    arrayDispatchers.activateNormalArray(_normalData.binding, _normalData.array.get(), _normalData.indices.get());
    arrayDispatchers.activateColorArray(_colorData.binding, _colorData.array.get(), _colorData.indices.get());
    arrayDispatchers.activateSecondaryColorArray(_secondaryColorData.binding, _secondaryColorData.array.get(), _secondaryColorData.indices.get());
    arrayDispatchers.activateFogCoordArray(_fogCoordData.binding, _fogCoordData.array.get(), _fogCoordData.indices.get());

    if (handleVertexAttributes)
    {
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
        {
            arrayDispatchers.activateVertexAttribArray(_vertexAttribList[unit].binding, unit, _vertexAttribList[unit].array.get(), _vertexAttribList[unit].indices.get());
        }
    }

    // dispatch any attributes that are bound overall
    arrayDispatchers.dispatch(BIND_OVERALL,0);

    state.lazyDisablingOfVertexAttributes();

    if (useFastPath)
    {
        // set up arrays
        if( _vertexData.array.valid() )
            state.setVertexPointer(_vertexData.array.get());

        if (_normalData.binding==BIND_PER_VERTEX && _normalData.array.valid())
            state.setNormalPointer(_normalData.array.get());

        if (_colorData.binding==BIND_PER_VERTEX && _colorData.array.valid())
            state.setColorPointer(_colorData.array.get());

        if (_secondaryColorData.binding==BIND_PER_VERTEX && _secondaryColorData.array.valid())
            state.setSecondaryColorPointer(_secondaryColorData.array.get());

        if (_fogCoordData.binding==BIND_PER_VERTEX && _fogCoordData.array.valid())
            state.setFogCoordPointer(_fogCoordData.array.get());

        for(unsigned int unit=0;unit<_texCoordList.size();++unit)
        {
            const Array* array = _texCoordList[unit].array.get();
            if (array) state.setTexCoordPointer(unit,array);
        }

        if( handleVertexAttributes )
        {
            for(unsigned int index = 0; index < _vertexAttribList.size(); ++index )
            {
                const Array* array = _vertexAttribList[index].array.get();
                const AttributeBinding ab = _vertexAttribList[index].binding;
                if( ab == BIND_PER_VERTEX && array )
                {
                    state.setVertexAttribPointer( index, array, _vertexAttribList[index].normalize );
                }
            }
        }
    }
    else
    {
        for(unsigned int unit=0;unit<_texCoordList.size();++unit)
        {
            arrayDispatchers.activateTexCoordArray(BIND_PER_VERTEX, unit, _texCoordList[unit].array.get(), _texCoordList[unit].indices.get());
        }

        arrayDispatchers.activateVertexArray(BIND_PER_VERTEX, _vertexData.array.get(), _vertexData.indices.get());
    }

    state.applyDisablingOfVertexAttributes();

    bool bindPerPrimitiveSetActive = arrayDispatchers.active(BIND_PER_PRIMITIVE_SET);
    bool bindPerPrimitiveActive = arrayDispatchers.active(BIND_PER_PRIMITIVE);

    unsigned int primitiveNum = 0;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    for(unsigned int primitiveSetNum=0; primitiveSetNum!=_primitives.size(); ++primitiveSetNum)
    {

        // dispatch any attributes that are bound per primitive
        if (bindPerPrimitiveSetActive) arrayDispatchers.dispatch(BIND_PER_PRIMITIVE_SET, primitiveSetNum);

        const PrimitiveSet* primitiveset = _primitives[primitiveSetNum].get();

        if (useFastPath)
        {
            primitiveset->draw(state, usingVertexBufferObjects);
        }
        else
        {
            GLenum mode=primitiveset->getMode();

            unsigned int primLength;
            switch(mode)
            {
                case(GL_POINTS):    primLength=1; break;
                case(GL_LINES):     primLength=2; break;
                case(GL_TRIANGLES): primLength=3; break;
                case(GL_QUADS):     primLength=4; break;
                default:            primLength=0; break; // compute later when =0.
            }

            // draw primitives by the more flexible "slow" path,
            // sending OpenGL glBegin/glVertex.../glEnd().
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    arrayDispatchers.Begin(mode);

                    unsigned int primCount=0;
                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex,++primCount)
                    {
                        if (bindPerPrimitiveActive && (primCount%primLength)==0)
                        {
                            arrayDispatchers.dispatch(BIND_PER_PRIMITIVE,primitiveNum++);
                        }

                        arrayDispatchers.dispatch(BIND_PER_VERTEX, vindex);
                    }

                    arrayDispatchers.End();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {
                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {
                        unsigned int localPrimLength;
                        if (primLength==0) localPrimLength=*primItr;
                        else localPrimLength=primLength;

                        arrayDispatchers.Begin(mode);

                        for(GLsizei primCount=0;
                            primCount<*primItr;
                            ++vindex,++primCount)
                        {
                            if (bindPerPrimitiveActive && (primCount%localPrimLength)==0)
                            {
                                arrayDispatchers.dispatch(BIND_PER_PRIMITIVE, primitiveNum++);
                            }
                            arrayDispatchers.dispatch(BIND_PER_VERTEX, vindex);
                        }

                        arrayDispatchers.End();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    arrayDispatchers.Begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {

                        if (bindPerPrimitiveActive && (primCount%primLength)==0)
                        {
                            arrayDispatchers.dispatch(BIND_PER_PRIMITIVE, primitiveNum++);
                        }

                        unsigned int vindex=*primItr;
                        arrayDispatchers.dispatch(BIND_PER_VERTEX, vindex);
                    }

                    arrayDispatchers.End();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    arrayDispatchers.Begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        if (bindPerPrimitiveActive && (primCount%primLength)==0)
                        {
                            arrayDispatchers.dispatch(BIND_PER_PRIMITIVE, primitiveNum++);
                        }

                        unsigned int vindex=*primItr;
                        arrayDispatchers.dispatch(BIND_PER_VERTEX, vindex);
                    }

                    arrayDispatchers.End();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    arrayDispatchers.Begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        if (bindPerPrimitiveActive && (primCount%primLength)==0)
                        {
                            arrayDispatchers.dispatch(BIND_PER_PRIMITIVE, primitiveNum++);
                        }

                        unsigned int vindex=*primItr;
                        arrayDispatchers.dispatch(BIND_PER_VERTEX, vindex);
                    }

                    arrayDispatchers.End();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

    // unbind the VBO's if any are used.
    state.unbindVertexBufferObject();
    state.unbindElementBufferObject();
}

class AttributeFunctorArrayVisitor : public ArrayVisitor
{
    public:
    
        AttributeFunctorArrayVisitor(Drawable::AttributeFunctor& af):
            _af(af) {}
    
        virtual ~AttributeFunctorArrayVisitor() {}

        virtual void apply(ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(DoubleArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec2dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec3dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
    
    
        inline void applyArray(Drawable::AttributeType type,Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }

    protected:        
        
        AttributeFunctorArrayVisitor& operator = (const AttributeFunctorArrayVisitor&) { return *this; }
        Drawable::AttributeFunctor&   _af;
        Drawable::AttributeType       _type;
};

void Geometry::accept(AttributeFunctor& af)
{
    AttributeFunctorArrayVisitor afav(af);

    if (_vertexData.array.valid())
    {
        afav.applyArray(VERTICES,_vertexData.array.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        osg::notify(osg::INFO)<<"Geometry::accept(AttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].array.get());
    }

    afav.applyArray(NORMALS,_normalData.array.get());
    afav.applyArray(COLORS,_colorData.array.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorData.array.get());
    afav.applyArray(FOG_COORDS,_fogCoordData.array.get());
    
    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].array.get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].array.get());
    }
}

class ConstAttributeFunctorArrayVisitor : public ConstArrayVisitor
{
    public:
    
        ConstAttributeFunctorArrayVisitor(Drawable::ConstAttributeFunctor& af):
            _af(af) {}
    
        virtual ~ConstAttributeFunctorArrayVisitor() {}

        virtual void apply(const ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const DoubleArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec2dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec3dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
    
    
        inline void applyArray(Drawable::AttributeType type,const Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }
    
protected:

        ConstAttributeFunctorArrayVisitor& operator = (const ConstAttributeFunctorArrayVisitor&) { return *this; }

        Drawable::ConstAttributeFunctor&    _af;
        Drawable::AttributeType             _type;
};

void Geometry::accept(ConstAttributeFunctor& af) const
{
    ConstAttributeFunctorArrayVisitor afav(af);
    
    if (_vertexData.array.valid())
    {
        afav.applyArray(VERTICES,_vertexData.array.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        osg::notify(osg::INFO)<<"Geometry::accept(ConstAttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].array.get());
    }

    afav.applyArray(NORMALS,_normalData.array.get());
    afav.applyArray(COLORS,_colorData.array.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorData.array.get());
    afav.applyArray(FOG_COORDS,_fogCoordData.array.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].array.get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].array.get());
    }
}

void Geometry::accept(PrimitiveFunctor& functor) const
{
    const osg::Array* vertices = _vertexData.array.get();
    const osg::IndexArray* indices = _vertexData.indices.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        osg::notify(osg::INFO)<<"Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].array.get();
        indices = _vertexAttribList[0].indices.get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    if (!indices)
    {
        switch(vertices->getType())
        {
        case(Array::Vec2ArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2*>(vertices->getDataPointer()));
            break;
        case(Array::Vec3ArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3*>(vertices->getDataPointer()));
            break;
        case(Array::Vec4ArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4*>(vertices->getDataPointer()));
            break;
        case(Array::Vec2dArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2d*>(vertices->getDataPointer()));
            break;
        case(Array::Vec3dArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3d*>(vertices->getDataPointer()));
            break;
        case(Array::Vec4dArrayType): 
            functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4d*>(vertices->getDataPointer()));
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimitiveFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
            return;
        }
        
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            (*itr)->accept(functor);
        }
    }
    else
    {
        const Vec2* vec2Array = 0;
        const Vec3* vec3Array = 0;
        const Vec4* vec4Array = 0;
        const Vec2d* vec2dArray = 0;
        const Vec3d* vec3dArray = 0;
        const Vec4d* vec4dArray = 0;
        Array::Type type = vertices->getType();
        switch(type)
        {
        case(Array::Vec2ArrayType): 
            vec2Array = static_cast<const Vec2*>(vertices->getDataPointer());
            break;
        case(Array::Vec3ArrayType): 
            vec3Array = static_cast<const Vec3*>(vertices->getDataPointer());
            break;
        case(Array::Vec4ArrayType): 
            vec4Array = static_cast<const Vec4*>(vertices->getDataPointer());
            break;
        case(Array::Vec2dArrayType): 
            vec2dArray = static_cast<const Vec2d*>(vertices->getDataPointer());
            break;
        case(Array::Vec3dArrayType): 
            vec3dArray = static_cast<const Vec3d*>(vertices->getDataPointer());
            break;
        case(Array::Vec4dArrayType): 
            vec4dArray = static_cast<const Vec4d*>(vertices->getDataPointer());
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimitiveFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
            return;
        }

        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            const PrimitiveSet* primitiveset = itr->get();
            GLenum mode=primitiveset->getMode();
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    functor.begin(mode);

                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex)
                    {
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex(vec2Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex(vec3Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex(vec4Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec2dArrayType): 
                            functor.vertex(vec2dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3dArrayType): 
                            functor.vertex(vec3dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4dArrayType): 
                            functor.vertex(vec4dArray[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }

                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {

                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {

                        functor.begin(mode);

                        for(GLsizei primCount=0;primCount<*primItr;++primCount)
                        {
                            switch(type)
                            {
                            case(Array::Vec2ArrayType): 
                                functor.vertex(vec2Array[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec3ArrayType): 
                                functor.vertex(vec3Array[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec4ArrayType): 
                                functor.vertex(vec4Array[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec2dArrayType): 
                                functor.vertex(vec2dArray[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec3dArrayType): 
                                functor.vertex(vec3dArray[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec4dArrayType): 
                                functor.vertex(vec4dArray[_vertexData.indices->index(vindex)]);
                                break;
                            default:
                                break;
                            }
                            ++vindex;
                        }

                        functor.end();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    functor.begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex(vec2Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex(vec3Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex(vec4Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec2dArrayType): 
                            functor.vertex(vec2dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3dArrayType): 
                            functor.vertex(vec3dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4dArrayType): 
                            functor.vertex(vec4dArray[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex(vec2Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex(vec3Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex(vec4Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec2dArrayType): 
                            functor.vertex(vec2dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3dArrayType): 
                            functor.vertex(vec3dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4dArrayType): 
                            functor.vertex(vec4dArray[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex(vec2Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex(vec3Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex(vec4Array[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec2dArrayType): 
                            functor.vertex(vec2dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3dArrayType): 
                            functor.vertex(vec3dArray[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4dArrayType): 
                            functor.vertex(vec4dArray[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return;
}

void Geometry::accept(PrimitiveIndexFunctor& functor) const
{
    const osg::Array* vertices = _vertexData.array.get();
    const osg::IndexArray* indices = _vertexData.indices.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        osg::notify(osg::INFO)<<"Geometry::accept(PrimitiveIndexFunctor& functor): Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].array.get();
        indices = _vertexAttribList[0].indices.get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    switch(vertices->getType())
    {
    case(Array::Vec2ArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3ArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4ArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4*>(vertices->getDataPointer()));
        break;
    case(Array::Vec2dArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3dArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4dArrayType): 
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4d*>(vertices->getDataPointer()));
        break;
    default:
        notify(WARN)<<"Warning: Geometry::accept(PrimitiveIndexFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
        return;
    }

    if (!_vertexData.indices.valid())
    {
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            (*itr)->accept(functor);
        }
    }
    else
    {
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            const PrimitiveSet* primitiveset = itr->get();
            GLenum mode=primitiveset->getMode();
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    functor.begin(mode);

                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex)
                    {
                        functor.vertex(_vertexData.indices->index(vindex));
                    }
                    
                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {

                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {

                        functor.begin(mode);

                        for(GLsizei primCount=0;primCount<*primItr;++primCount)
                        {
                            functor.vertex(_vertexData.indices->index(vindex));
                            ++vindex;
                        }
                        
                        functor.end();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    functor.begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return;
}

unsigned int _computeNumberOfPrimitives(const osg::Geometry& geom)
{

    unsigned int totalNumberOfPrimitives = 0;
    
    for(Geometry::PrimitiveSetList::const_iterator itr=geom.getPrimitiveSetList().begin();
        itr!=geom.getPrimitiveSetList().end();
        ++itr)
    {
        const PrimitiveSet* primitiveset = itr->get();
        GLenum mode=primitiveset->getMode();

        unsigned int primLength;
        switch(mode)
        {
            case(GL_POINTS):    primLength=1; osg::notify(osg::INFO)<<"prim=GL_POINTS"<<std::endl; break;
            case(GL_LINES):     primLength=2; osg::notify(osg::INFO)<<"prim=GL_LINES"<<std::endl; break;
            case(GL_TRIANGLES): primLength=3; osg::notify(osg::INFO)<<"prim=GL_TRIANGLES"<<std::endl; break;
            case(GL_QUADS):     primLength=4; osg::notify(osg::INFO)<<"prim=GL_QUADS"<<std::endl; break;
            default:            primLength=0; osg::notify(osg::INFO)<<"prim="<<std::hex<<mode<<std::dec<<std::endl; break; // compute later when =0.
        }

        // draw primitives by the more flexible "slow" path,
        // sending OpenGL Begin/glVertex.../End().
        switch(primitiveset->getType())
        {
            case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {

                const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                    primItr!=drawArrayLengths->end();
                    ++primItr)
                {
                    if (primLength==0) totalNumberOfPrimitives += 1;
                    else totalNumberOfPrimitives += *primItr/primLength;
                }
                break;
            }
            default:
            {
                if (primLength==0) { totalNumberOfPrimitives += 1; osg::notify(osg::INFO)<<"   totalNumberOfPrimitives="<<totalNumberOfPrimitives<<std::endl;}
                else { totalNumberOfPrimitives += primitiveset->getNumIndices()/primLength; osg::notify(osg::INFO)<<"   primitiveset->getNumIndices()="<<primitiveset->getNumIndices()<<" totalNumberOfPrimitives="<<totalNumberOfPrimitives<<std::endl; }

            }
        }
    }

    return totalNumberOfPrimitives;
}

template<class A>
bool _verifyBindings(const osg::Geometry& geom, const A& arrayData)
{
    unsigned int numElements = arrayData.indices.valid()?arrayData.indices->getNumElements():
                               arrayData.array.valid()?arrayData.array->getNumElements():0;

    switch(arrayData.binding)
    {
        case(osg::Geometry::BIND_OFF):
            if (numElements>0) return false;
            break;
        case(osg::Geometry::BIND_OVERALL):
            if (numElements!=1) return false;
            break;
        case(osg::Geometry::BIND_PER_PRIMITIVE_SET):
            if (numElements!=geom.getPrimitiveSetList().size()) return false;
            break;
        case(osg::Geometry::BIND_PER_PRIMITIVE):
            if (numElements!=_computeNumberOfPrimitives(geom)) return false;
            break;
        case(osg::Geometry::BIND_PER_VERTEX):
        {
            unsigned int numVertices = geom.getVertexIndices()?geom.getVertexIndices()->getNumElements():
                                       geom.getVertexArray()?geom.getVertexArray()->getNumElements():0;
            if (numElements!=numVertices) return false;        
            break;
        }
    } 
    return true;
}

template<class A>
void _computeCorrectBindingsAndArraySizes(std::ostream& out, const osg::Geometry& geom, A& arrayData, const char* arrayName)
{
    unsigned int numElements = arrayData.indices.valid()?arrayData.indices->getNumElements():
                               arrayData.array.valid()?arrayData.array->getNumElements():0;

    // check to see if binding matches 0 elements required.
    if (numElements==0)
    {
        // correct binding if not correct.
        if (arrayData.binding!=osg::Geometry::BIND_OFF)
        { 
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_OFF"<<std::endl;
            arrayData.binding=osg::Geometry::BIND_OFF;
        }
        return;
    }

    // check to see if binding matches 1 elements required.
    if (numElements==1)
    {
        // correct binding if not correct.
        if (arrayData.binding!=osg::Geometry::BIND_OVERALL)
        { 
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_OVERALL"<<std::endl;
            arrayData.binding=osg::Geometry::BIND_OVERALL;
        }
        return;
    }


    unsigned int numVertices = geom.getVertexIndices()?geom.getVertexIndices()->getNumElements():
                               geom.getVertexArray()?geom.getVertexArray()->getNumElements():0;
    
    if ( numVertices==0 )
    {
        if (arrayData.binding!=osg::Geometry::BIND_OFF)
        {
            arrayData.array = 0;
            arrayData.indices = 0;
            arrayData.binding = osg::Geometry::BIND_OFF;
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() vertex array is empty but "<<std::endl
                <<"         vertex array is empty but"<<arrayName<<" is set"<<std::endl
                <<"         reseting "<<arrayName<< " binding to BIND_OFF and array & indices to 0."<<std::endl;
        }
    }
    
    if (numElements==numVertices)
    {
        // correct the binding to per vertex.
        if (arrayData.binding!=osg::Geometry::BIND_PER_VERTEX)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_VERTEX"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_VERTEX;
        }
        return;
    }



    // check to see if binding might be per primitive set    
    unsigned int numPrimitiveSets = geom.getPrimitiveSetList().size();
    
    if (numElements==numPrimitiveSets)
    {
        if (arrayData.binding != osg::Geometry::BIND_PER_PRIMITIVE_SET)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_PRIMITIVE_SET"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE_SET;
        }
        return;
    }
    
    // check to see if binding might be per primitive   
    unsigned int numPrimitives = _computeNumberOfPrimitives(geom);
    if (numElements==numPrimitives)
    {
        if (arrayData.binding != osg::Geometry::BIND_PER_PRIMITIVE)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_PRIMITIVE"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE;
        }
        return;
    }

    if (numElements>numVertices)
    {
        arrayData.binding = osg::Geometry::BIND_PER_VERTEX;
        return;
    }
    if (numElements>numPrimitives)
    {
        arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE;
        return;
    }
    if (numElements>numPrimitiveSets)
    {
        arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE_SET;
        return;
    }
    if (numElements>=1)
    {
        arrayData.binding = osg::Geometry::BIND_OVERALL;
        return;
    }
    arrayData.binding = osg::Geometry::BIND_OFF;
 
}        

bool Geometry::verifyBindings(const ArrayData& arrayData) const
{
    return _verifyBindings(*this,arrayData);
}

bool Geometry::verifyBindings(const Vec3ArrayData& arrayData) const
{
    return _verifyBindings(*this,arrayData);
}

void Geometry::computeCorrectBindingsAndArraySizes(ArrayData& arrayData, const char* arrayName)
{
    _computeCorrectBindingsAndArraySizes(osg::notify(osg::INFO),*this,arrayData,arrayName);
}

void Geometry::computeCorrectBindingsAndArraySizes(Vec3ArrayData& arrayData, const char* arrayName)
{
    _computeCorrectBindingsAndArraySizes(osg::notify(osg::INFO),*this,arrayData,arrayName);
}

bool Geometry::verifyBindings() const
{
    if (!verifyBindings(_normalData)) return false;
    if (!verifyBindings(_colorData)) return false;
    if (!verifyBindings(_secondaryColorData)) return false;
    if (!verifyBindings(_fogCoordData)) return false;

    for(ArrayDataList::const_iterator titr=_texCoordList.begin();
        titr!=_texCoordList.end();
        ++titr)
    {
        if (!verifyBindings(*titr)) return false;
    }

    for(ArrayDataList::const_iterator vitr=_vertexAttribList.begin();
        vitr!=_vertexAttribList.end();
        ++vitr)
    {
        if (!verifyBindings(*vitr)) return false;
    }

    return true;
}

void Geometry::computeCorrectBindingsAndArraySizes()
{
    // if (verifyBindings()) return;
    
    computeCorrectBindingsAndArraySizes(_normalData,"_normalData");
    computeCorrectBindingsAndArraySizes(_colorData,"_colorData");
    computeCorrectBindingsAndArraySizes(_secondaryColorData,"_secondaryColorData");
    computeCorrectBindingsAndArraySizes(_fogCoordData,"_fogCoordData");

    for(ArrayDataList::iterator titr=_texCoordList.begin();
        titr!=_texCoordList.end();
        ++titr)
    {
        computeCorrectBindingsAndArraySizes(*titr,"_texCoordList[]");
    }

    for(ArrayDataList::iterator vitr=_vertexAttribList.begin();
        vitr!=_vertexAttribList.end();
        ++vitr)
    {
        computeCorrectBindingsAndArraySizes(*vitr,"_vertAttribList[]");
    }
}

class ExpandIndexedArray : public osg::ConstArrayVisitor
{
    public:
        ExpandIndexedArray(const osg::IndexArray& indices,Array* targetArray):
            _indices(indices),
            _targetArray(targetArray) {}
        
        virtual ~ExpandIndexedArray() {}

        // Create when both of the arrays are predefined templated classes. We 
        // can do some optimizations in this case that aren't possible in the general
        // case.
        template <class T,class I>
        T* create_inline(const T& array,const I& indices)
        {
            T* newArray = 0;

            // if source array type and target array type are equal but arrays arn't equal
            if (_targetArray && _targetArray->getType()==array.getType() && _targetArray!=(osg::Array*)(&array))
            {
                // reuse exisiting target array 
                newArray = static_cast<T*>(_targetArray);
                if (newArray->size()!=indices.size())
                {
                    // make sure its the right size
                    newArray->resize(indices.size());
                }
            }
            else
            {
                //  else create a new array.
                newArray = new T(indices.size());
            }

            for(unsigned int i=0;i<indices.size();++i)
            {
                (*newArray)[i]= array[indices[i]];
            }

            return newArray;
        }

        // Create when one of the arrays isn't one of the predefined templated classes. The
        // template parameter is the type of the array that will get created. This is  always 
        // one of the predefined classes. We could call clone to get one of the same type as 
        // the input array, but the interface of the osg::Array class doesn't include a way 
        // to set an element.
        template <class T>
        osg::Array* create_noinline(const osg::Array& array, const osg::IndexArray& indices)
        {
            T* newArray = 0;
            typedef typename T::ElementDataType EDT;

            unsigned int num_indices = indices.getNumElements();
            newArray = new T(num_indices);

            const EDT* src = static_cast<const EDT*>(array.getDataPointer());

            for(unsigned int i=0;i<num_indices;++i)
            {
                (*newArray)[i]= src[indices.index(i)];
            }

            return newArray;
        }

        osg::Array* create_noinline(const osg::Array& array, const osg::IndexArray& indices)
        {
            switch (array.getType()) 
            {
                case(osg::Array::ByteArrayType):   return create_noinline<osg::ByteArray>(array,indices);
                case(osg::Array::ShortArrayType):  return create_noinline<osg::ShortArray>(array,indices);
                case(osg::Array::IntArrayType):    return create_noinline<osg::IntArray>(array,indices);
                case(osg::Array::UByteArrayType):  return create_noinline<osg::UByteArray>(array,indices);
                case(osg::Array::UShortArrayType): return create_noinline<osg::UShortArray>(array,indices);
                case(osg::Array::UIntArrayType):   return create_noinline<osg::UIntArray>(array,indices);
                case(osg::Array::Vec4ubArrayType): return create_noinline<osg::Vec4ubArray>(array,indices);
                case(osg::Array::FloatArrayType):  return create_noinline<osg::FloatArray>(array,indices);
                case(osg::Array::Vec2ArrayType):   return create_noinline<osg::Vec2Array>(array,indices);
                case(osg::Array::Vec3ArrayType):   return create_noinline<osg::Vec3Array>(array,indices);
                case(osg::Array::Vec4ArrayType):   return create_noinline<osg::Vec4Array>(array,indices);
                case(osg::Array::Vec2dArrayType):   return create_noinline<osg::Vec2dArray>(array,indices);
                case(osg::Array::Vec3dArrayType):   return create_noinline<osg::Vec3dArray>(array,indices);
                case(osg::Array::Vec4dArrayType):   return create_noinline<osg::Vec4dArray>(array,indices);
                default:
                    return NULL;
            }
        }

        template <class TA, class TI>
        osg::Array* create(const TA& array, const osg::IndexArray& indices)
        {
            // We know that indices.getType returned the same thing as TI, but 
            // we need to determine whether it is really an instance of TI, or 
            // perhaps another subclass of osg::Array that contains the same 
            // type of data.
            const TI* ba(dynamic_cast<const TI*>(&indices));
            if (ba != NULL) {
                return create_inline(array,*ba);
            }
            else {
                return create_noinline(array, _indices);
            }
        }

        template <class T>
        osg::Array* create(const T& array)
        {
            switch(_indices.getType())
            {
            case(osg::Array::ByteArrayType):   return create<T, osg::ByteArray>(array, _indices);
            case(osg::Array::ShortArrayType):  return create<T, osg::ShortArray>(array, _indices);
            case(osg::Array::IntArrayType):    return create<T, osg::IntArray>(array, _indices);
            case(osg::Array::UByteArrayType):  return create<T, osg::UByteArray>(array, _indices);
            case(osg::Array::UShortArrayType): return create<T, osg::UShortArray>(array, _indices);
            case(osg::Array::UIntArrayType):   return create<T, osg::UIntArray>(array, _indices);
            default:                           return create_noinline(array, _indices);
            }
            
        }

        // applys for the predefined classes go through 1-arg create to do indexing
        virtual void apply(const osg::ByteArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::ShortArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::IntArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UByteArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UShortArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UIntArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec4ubArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::FloatArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec2Array& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec3Array& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec4Array& array) { _targetArray = create(array); }

        // other subclasses of osg::Array end up here
        virtual void apply(const osg::Array& array) { _targetArray = create_noinline(array, _indices); }

        const osg::IndexArray& _indices;
        osg::Array* _targetArray;

    protected:

        ExpandIndexedArray& operator = (const ExpandIndexedArray&) { return *this; }

};

bool Geometry::suitableForOptimization() const
{
    bool hasIndices = false;

    if (getVertexIndices()) hasIndices = true;

    if (getNormalIndices()) hasIndices = true;

    if (getColorIndices()) hasIndices = true;

    if (getSecondaryColorIndices()) hasIndices = true;

    if (getFogCoordIndices()) hasIndices = true;

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordIndices(ti)) hasIndices = true;
    }
    
    for(unsigned int vi=0;vi<getNumVertexAttribArrays();++vi)
    {
        if (getVertexAttribIndices(vi)) hasIndices = true;
    }

    return hasIndices;
}

void Geometry::copyToAndOptimize(Geometry& target)
{
    bool copyToSelf = (this==&target);

    // copy over primitive sets.
    if (!copyToSelf) target.getPrimitiveSetList() = getPrimitiveSetList();

    // copy over attribute arrays.
    if (getVertexIndices() && getVertexArray())
    {
        ExpandIndexedArray eia(*(getVertexIndices()),target.getVertexArray());
        getVertexArray()->accept(eia);

        target.setVertexArray(eia._targetArray);
        target.setVertexIndices(0);
    }
    else if (getVertexArray())
    {
        if (!copyToSelf) target.setVertexArray(getVertexArray());
    }

    target.setNormalBinding(getNormalBinding());
    if (getNormalIndices() && getNormalArray())
    {
        ExpandIndexedArray eia(*(getNormalIndices()),target.getNormalArray());
        getNormalArray()->accept(eia);

        target.setNormalArray(dynamic_cast<osg::Vec3Array*>(eia._targetArray));
        target.setNormalIndices(0);
    }
    else if (getNormalArray())
    {
        if (!copyToSelf) target.setNormalArray(getNormalArray());
    }

    target.setColorBinding(getColorBinding());
    if (getColorIndices() && getColorArray())
    {
        ExpandIndexedArray eia(*(getColorIndices()),target.getColorArray());
        getColorArray()->accept(eia);

        target.setColorArray(eia._targetArray);
        target.setColorIndices(0);
    }
    else if (getColorArray())
    {
        if (!copyToSelf) target.setColorArray(getColorArray());
    }

    target.setSecondaryColorBinding(getSecondaryColorBinding());
    if (getSecondaryColorIndices() && getSecondaryColorArray())
    {
        ExpandIndexedArray eia(*(getSecondaryColorIndices()),target.getSecondaryColorArray());
        getSecondaryColorArray()->accept(eia);

        target.setSecondaryColorArray(eia._targetArray);
        target.setSecondaryColorIndices(0);
    }
    else if (getSecondaryColorArray())
    {
        if (!copyToSelf) target.setSecondaryColorArray(getSecondaryColorArray());
    }

    target.setFogCoordBinding(getFogCoordBinding());
    if (getFogCoordIndices() && getFogCoordArray())
    {
        ExpandIndexedArray eia(*(getFogCoordIndices()),target.getFogCoordArray());
        getFogCoordArray()->accept(eia);

        target.setFogCoordArray(eia._targetArray);
        target.setFogCoordIndices(0);
    }
    else if (getFogCoordArray())
    {
        if (!copyToSelf) target.setFogCoordArray(getFogCoordArray());
    }

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordIndices(ti) && getTexCoordArray(ti))
        {
            ExpandIndexedArray eia(*(getTexCoordIndices(ti)),target.getTexCoordArray(ti));
            
            getTexCoordArray(ti)->accept(eia);

            target.setTexCoordArray(ti,eia._targetArray);
            target.setTexCoordIndices(ti,0);
        }
        else if (getTexCoordArray(ti)) 
        {
            if (!copyToSelf) target.setTexCoordArray(ti,getTexCoordArray(ti));
        }
    }
    
    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        ArrayData& arrayData = _vertexAttribList[vi];
        if (arrayData.indices.valid())
        {
            ExpandIndexedArray eia(*arrayData.indices,target.getVertexAttribArray(vi));
            arrayData.array->accept(eia);
            target.setVertexAttribData(vi,ArrayData(eia._targetArray, 0, arrayData.binding, arrayData.normalize));
        }
        else if (arrayData.array.valid())
        {
            if (!copyToSelf) target.setVertexAttribData(vi,arrayData);
        }
    }
}

bool Geometry::containsSharedArrays() const
{
    unsigned int numSharedArrays = 0;
    
    if (getVertexArray() && getVertexArray()->referenceCount()>1) ++numSharedArrays;
    if (getNormalArray() && getNormalArray()->referenceCount()>1) ++numSharedArrays;
    if (getColorArray() && getColorArray()->referenceCount()>1) ++numSharedArrays;
    if (getSecondaryColorArray() && getSecondaryColorArray()->referenceCount()>1) ++numSharedArrays;
    if (getFogCoordArray() && getFogCoordArray()->referenceCount()>1) ++numSharedArrays;

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordArray(ti) && getTexCoordArray(ti)->referenceCount()>1) ++numSharedArrays;
    }
    
    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        const ArrayData& arrayData = _vertexAttribList[vi];
        if (arrayData.array.valid() && arrayData.array->referenceCount()>1) ++numSharedArrays;
    }
    return numSharedArrays!=0;
}

void Geometry::duplicateSharedArrays()
{
    #define DUPLICATE_IF_REQUIRED(A) \
        if (get##A() && get##A()->referenceCount()>1) \
        { \
            set##A(dynamic_cast<osg::Array*>(get##A()->clone(osg::CopyOp::DEEP_COPY_ARRAYS))); \
        }

    DUPLICATE_IF_REQUIRED(VertexArray)
    DUPLICATE_IF_REQUIRED(NormalArray)
    DUPLICATE_IF_REQUIRED(ColorArray)
    DUPLICATE_IF_REQUIRED(SecondaryColorArray)
    DUPLICATE_IF_REQUIRED(FogCoordArray)

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordArray(ti) && getTexCoordArray(ti)->referenceCount()>1)
        {
            setTexCoordArray(ti, dynamic_cast<osg::Array*>(getTexCoordArray(ti)->clone(osg::CopyOp::DEEP_COPY_ARRAYS)));
        }
    }
    
    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        ArrayData& arrayData = _vertexAttribList[vi];
        if (arrayData.array.valid() && arrayData.array->referenceCount()>1)
        {
            arrayData.array = dynamic_cast<osg::Array*>(arrayData.array->clone(osg::CopyOp::DEEP_COPY_ARRAYS));
        }
    }
}

void Geometry::computeInternalOptimizedGeometry()
{
    if (suitableForOptimization())
    {
        if (!_internalOptimizedGeometry) _internalOptimizedGeometry = new Geometry;

        copyToAndOptimize(*_internalOptimizedGeometry);
    }
}


Geometry* osg::createTexturedQuadGeometry(const Vec3& corner,const Vec3& widthVec,const Vec3& heightVec, float l, float b, float r, float t)
{
    Geometry* geom = new Geometry;

    Vec3Array* coords = new Vec3Array(4);
    (*coords)[0] = corner+heightVec;
    (*coords)[1] = corner;
    (*coords)[2] = corner+widthVec;
    (*coords)[3] = corner+widthVec+heightVec;
    geom->setVertexArray(coords);

    Vec2Array* tcoords = new Vec2Array(4);
    (*tcoords)[0].set(l,t);
    (*tcoords)[1].set(l,b);
    (*tcoords)[2].set(r,b);
    (*tcoords)[3].set(r,t);
    geom->setTexCoordArray(0,tcoords);

    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(Geometry::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    geom->setNormalArray(normals);
    geom->setNormalBinding(Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));

    return geom;
}
