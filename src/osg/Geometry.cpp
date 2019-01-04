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
#include <osg/Notify>

using namespace osg;


Geometry::Geometry():
    _containsDeprecatedData(false)
{
#if 0
    _supportsVertexBufferObjects = true;
    // temporary test
    // setSupportsDisplayList(false);
#else
    _supportsVertexBufferObjects = true;
    _useVertexBufferObjects = false;
#endif
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(copyop(geometry._vertexArray.get())),
    _normalArray(copyop(geometry._normalArray.get())),
    _colorArray(copyop(geometry._colorArray.get())),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _fogCoordArray(copyop(geometry._fogCoordArray.get())),
    _containsDeprecatedData(geometry._containsDeprecatedData)
{
    _supportsVertexBufferObjects = true;
    // temporary test
    // setSupportsDisplayList(false);

    for(PrimitiveSetList::const_iterator pitr=geometry._primitives.begin();
        pitr!=geometry._primitives.end();
        ++pitr)
    {
        PrimitiveSet* primitive = copyop(pitr->get());
        if (primitive) _primitives.push_back(primitive);
    }

    for(ArrayList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(copyop(titr->get()));
    }

    for(ArrayList::const_iterator vitr=geometry._vertexAttribList.begin();
        vitr!=geometry._vertexAttribList.end();
        ++vitr)
    {
        _vertexAttribList.push_back(copyop(vitr->get()));
    }

    if ((copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_ARRAYS) || (copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_PRIMITIVES))
    {
        /*if (_useVertexBufferObjects)*/
        {
            // copying of arrays doesn't set up buffer objects so we'll need to force
            // Geometry to assign these, we'll do this by changing the cached value to false then re-enabling.
            // note do not use setUseVertexBufferObjects(false) as it might modify Arrays that we have not deep-copied.
            _useVertexBufferObjects = false;
            setUseVertexBufferObjects(true);
        }
    }

}

Geometry::~Geometry()
{
    // clean up display lists if assigned, and use the GLObjectSizeHint() while prior to it being invalidated by the automatic clean up of arrays that will invalidate the getGLObjectSizeHint() value.
    #ifdef OSG_GL_DISPLAYLISTS_AVAILABLE
    for(unsigned int i=0;i<_globjList.size();++i)
    {
        if (_globjList[i] != 0)
        {
            Drawable::deleteDisplayList(i,_globjList[i], getGLObjectSizeHint());
            _globjList[i] = 0;
        }
    }
    #endif
}

#define ARRAY_NOT_EMPTY(array) (array!=0 && array->getNumElements()!=0)

bool Geometry::empty() const
{
    if (!_primitives.empty()) return false;
    if (ARRAY_NOT_EMPTY(_vertexArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_normalArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_colorArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_secondaryColorArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_fogCoordArray.get())) return false;
    if (!_texCoordList.empty()) return false;
    if (!_vertexAttribList.empty()) return false;
    return true;
}

void Geometry::configureBufferObjects()
{
    osg::Array* vertices = getVertexArray();
    if (!vertices) return;

    osg::BufferObject* vbo = vertices->getBufferObject();
    unsigned int numVertices = vertices->getNumElements();

    typedef std::vector< osg::ref_ptr<osg::Array> > Arrays;
    Arrays arrays;

    if (getNormalArray()) arrays.push_back(getNormalArray());
    if (getColorArray()) arrays.push_back(getColorArray());
    if (getSecondaryColorArray()) arrays.push_back(getSecondaryColorArray());
    if (getFogCoordArray()) arrays.push_back(getFogCoordArray());

    for(unsigned int i=0; i<getNumTexCoordArrays(); ++i)
    {
        if (getTexCoordArray(i)) arrays.push_back(getTexCoordArray(i));
    }

    for(unsigned int i=0; i<getNumVertexAttribArrays(); ++i)
    {
        if (getVertexAttribArray(i)) arrays.push_back(getVertexAttribArray(i));
    }

    for(Arrays::iterator itr = arrays.begin();
        itr != arrays.end();
        ++itr)
    {
        osg::Array* array = itr->get();
        if (array->getBinding()==osg::Array::BIND_PER_VERTEX)
        {
            if (array->getNumElements()==numVertices)
            {
                if (!array->getBufferObject()) array->setBufferObject(vbo);
            }
            else if (array->getNumElements()>=1)
            {
                array->setBinding(osg::Array::BIND_OVERALL);
            }
            else
            {
                array->setBinding(osg::Array::BIND_OFF);
            }
        }
    }
}


void Geometry::setVertexArray(Array* array)
{
    if (array && array->getBinding()==osg::Array::BIND_UNDEFINED) array->setBinding(osg::Array::BIND_PER_VERTEX);

    _vertexArray = array;

    dirtyGLObjects();
    dirtyBound();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignVertexArrayDispatcher();

        addVertexBufferObjectIfRequired(array);
    }
}

void Geometry::setNormalArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _normalArray = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignNormalArrayDispatcher();

        addVertexBufferObjectIfRequired(array);
    }
}

void Geometry::setColorArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _colorArray = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignColorArrayDispatcher();

        addVertexBufferObjectIfRequired(array);
    }
}

void Geometry::setSecondaryColorArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _secondaryColorArray = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignSecondaryColorArrayDispatcher();

        addVertexBufferObjectIfRequired(array);
    }
}

void Geometry::setFogCoordArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _fogCoordArray = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignFogCoordArrayDispatcher();

        addVertexBufferObjectIfRequired(array);
    }
}



void Geometry::setTexCoordArray(unsigned int index,Array* array, osg::Array::Binding binding)
{
    if (_texCoordList.size()<=index)
        _texCoordList.resize(index+1);

    if (array)
    {
        if (binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);
        else array->setBinding(osg::Array::BIND_PER_VERTEX);
    }

    _texCoordList[index] = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignTexCoordArrayDispatcher(_texCoordList.size());

        addVertexBufferObjectIfRequired(array);
    }
}

Array* Geometry::getTexCoordArray(unsigned int index)
{
    if (index<_texCoordList.size()) return _texCoordList[index].get();
    else return 0;
}

const Array* Geometry::getTexCoordArray(unsigned int index) const
{
    if (index<_texCoordList.size()) return _texCoordList[index].get();
    else return 0;
}

void Geometry::setTexCoordArrayList(const ArrayList& arrayList)
{
    _texCoordList = arrayList;

    dirtyGLObjects();

    if (!_texCoordList.empty())
    {
        _vertexArrayStateList.assignTexCoordArrayDispatcher(_texCoordList.size());

        for(ArrayList::iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr)
        {
           if (itr->get()) addVertexBufferObjectIfRequired(itr->get());
        }
    }
}

void Geometry::setVertexAttribArray(unsigned int index, Array* array, osg::Array::Binding binding)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);

    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _vertexAttribList[index] = array;

    dirtyGLObjects();

    if (/*_useVertexBufferObjects && */array)
    {
        _vertexArrayStateList.assignVertexAttribArrayDispatcher(_vertexAttribList.size());

        addVertexBufferObjectIfRequired(array);
    }
}

Array *Geometry::getVertexAttribArray(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].get();
    else return 0;
}

const Array *Geometry::getVertexAttribArray(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].get();
    else return 0;
}

void Geometry::setVertexAttribArrayList(const ArrayList& arrayList)
{
    _vertexAttribList = arrayList;

    dirtyGLObjects();

    if (!_vertexAttribList.empty())
    {
        _vertexArrayStateList.assignVertexAttribArrayDispatcher(_vertexAttribList.size());

        for(ArrayList::iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            if (itr->get()) addVertexBufferObjectIfRequired(itr->get());
        }
    }
}


bool Geometry::addPrimitiveSet(PrimitiveSet* primitiveset)
{
    if (primitiveset)
    {
        /*if (_useVertexBufferObjects)*/ addElementBufferObjectIfRequired(primitiveset);

        _primitives.push_back(primitiveset);
        dirtyGLObjects();
        dirtyBound();
        return true;
    }

    OSG_WARN<<"Warning: invalid primitiveset passed to osg::Geometry::addPrimitiveSet(i, primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::setPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{
    if (i<_primitives.size() && primitiveset)
    {
        /*if (_useVertexBufferObjects)*/ addElementBufferObjectIfRequired(primitiveset);

        _primitives[i] = primitiveset;
        dirtyGLObjects();
        dirtyBound();
        return true;
    }
    OSG_WARN<<"Warning: invalid index i or primitiveset passed to osg::Geometry::setPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::insertPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{

    if (primitiveset)
    {
        /*if (_useVertexBufferObjects)*/ addElementBufferObjectIfRequired(primitiveset);

        if (i<_primitives.size())
        {
            _primitives.insert(_primitives.begin()+i,primitiveset);
            dirtyGLObjects();
            dirtyBound();
            return true;
        }
        else if (i==_primitives.size())
        {
            return addPrimitiveSet(primitiveset);
        }

    }
    OSG_WARN<<"Warning: invalid index i or primitiveset passed to osg::Geometry::insertPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

void Geometry::setPrimitiveSetList(const PrimitiveSetList& primitives)
{
    _primitives = primitives;
    /*if (_useVertexBufferObjects)*/
    {
        for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
        {
            addElementBufferObjectIfRequired(_primitives[primitiveSetIndex].get());
        }

    }
    dirtyGLObjects(); dirtyBound();
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
            OSG_WARN<<"Warning: osg::Geometry::removePrimitiveSet(i,numElementsToRemove) has been asked to remove more elements than are available,"<<std::endl;
            OSG_WARN<<"         removing on from i to the end of the list of primitive sets."<<std::endl;
            _primitives.erase(_primitives.begin()+i,_primitives.end());
        }

        dirtyGLObjects();
        dirtyBound();
        return true;
    }
    OSG_WARN<<"Warning: invalid index i passed to osg::Geometry::removePrimitiveSet(i,numElementsToRemove), ignoring call."<<std::endl;
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

unsigned int Geometry::getGLObjectSizeHint() const
{
    unsigned int totalSize = 0;
    if (_vertexArray.valid()) totalSize += _vertexArray->getTotalDataSize();

    if (_normalArray.valid()) totalSize += _normalArray->getTotalDataSize();

    if (_colorArray.valid()) totalSize += _colorArray->getTotalDataSize();

    if (_secondaryColorArray.valid()) totalSize += _secondaryColorArray->getTotalDataSize();

    if (_fogCoordArray.valid()) totalSize += _fogCoordArray->getTotalDataSize();


    unsigned int unit;
    for(unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].get();
        if (array) totalSize += array->getTotalDataSize();

    }

    bool handleVertexAttributes = true;
    if (handleVertexAttributes)
    {
        unsigned int index;
        for( index = 0; index < _vertexAttribList.size(); ++index )
        {
            const Array* array = _vertexAttribList[index].get();
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

    if (_vertexArray.valid()) arrayList.push_back(_vertexArray.get());
    if (_normalArray.valid()) arrayList.push_back(_normalArray.get());
    if (_colorArray.valid()) arrayList.push_back(_colorArray.get());
    if (_secondaryColorArray.valid()) arrayList.push_back(_secondaryColorArray.get());
    if (_fogCoordArray.valid()) arrayList.push_back(_fogCoordArray.get());

    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        Array* array = _texCoordList[unit].get();
        if (array) arrayList.push_back(array);
    }

    for(unsigned int  index = 0; index < _vertexAttribList.size(); ++index )
    {
        Array* array = _vertexAttribList[index].get();
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
    if (/*_useVertexBufferObjects &&*/ array->getBinding()==Array::BIND_PER_VERTEX || array->getBinding()==Array::BIND_UNDEFINED)
    {
        if (!array->getVertexBufferObject())
        {
            array->setVertexBufferObject(getOrCreateVertexBufferObject());
        }
    }
}

void Geometry::addElementBufferObjectIfRequired(osg::PrimitiveSet* primitiveSet)
{
    /*if (_useVertexBufferObjects)*/
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
        osg::Array* array = vitr->get();
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

    // OSG_NOTICE<<"Geometry::setUseVertexBufferObjects("<<flag<<")"<<std::endl;

    if (_useVertexBufferObjects==flag) return;

    Drawable::setUseVertexBufferObjects(flag);

    ArrayList arrayList;
    getArrayList(arrayList);

    DrawElementsList drawElementsList;
    getDrawElementsList(drawElementsList);

    /*if (_useVertexBufferObjects)*/
    {
        if (!arrayList.empty())
        {

            osg::ref_ptr<osg::VertexBufferObject> vbo;

            ArrayList::iterator vitr;
            for(vitr = arrayList.begin();
                vitr != arrayList.end() && !vbo;
                ++vitr)
            {
                osg::Array* array = vitr->get();
                if (array->getVertexBufferObject()) vbo = array->getVertexBufferObject();
            }

            if (!vbo) vbo = new osg::VertexBufferObject;

            for(vitr = arrayList.begin();
                vitr != arrayList.end();
                ++vitr)
            {
                osg::Array* array = vitr->get();
                if (!array->getVertexBufferObject()) array->setVertexBufferObject(vbo.get());
            }
        }

        if (!drawElementsList.empty())
        {
            osg::ref_ptr<osg::ElementBufferObject> ebo;

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
                if (!elements->getElementBufferObject()) elements->setElementBufferObject(ebo.get());
            }
        }
    }
    /*
    else
    {
        for(ArrayList::iterator vitr = arrayList.begin();
            vitr != arrayList.end();
            ++vitr)
        {
            osg::Array* array = vitr->get();
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
    */
}

void Geometry::dirtyGLObjects()
{
    Drawable::dirtyGLObjects();
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

    if (state)
    {
        if (_vertexArrayStateList[state->getContextID()].valid())
        {
            _vertexArrayStateList[state->getContextID()]->release();
            _vertexArrayStateList[state->getContextID()] = 0;
        }
    }
    else _vertexArrayStateList.clear();

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

VertexArrayState* Geometry::createVertexArrayStateImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();

    VertexArrayState* vas = new osg::VertexArrayState(&state);

    // OSG_NOTICE<<"Creating new osg::VertexArrayState "<< vas<<std::endl;

    if (_vertexArray.valid()) vas->assignVertexArrayDispatcher();
    if (_colorArray.valid()) vas->assignColorArrayDispatcher();
    if (_normalArray.valid()) vas->assignNormalArrayDispatcher();
    if (_secondaryColorArray.valid()) vas->assignSecondaryColorArrayDispatcher();
    if (_fogCoordArray.valid()) vas->assignFogCoordArrayDispatcher();

    if (!_texCoordList.empty()) vas->assignTexCoordArrayDispatcher(_texCoordList.size());
    if (!_vertexAttribList.empty()) vas->assignVertexAttribArrayDispatcher(_vertexAttribList.size());

    if (state.useVertexArrayObject(_useVertexArrayObject))
    {
        // OSG_NOTICE<<"  Setup VertexArrayState to use VAO "<<vas<<std::endl;

        vas->generateVertexArrayObject();
    }
    else
    {
        // OSG_NOTICE<<"  Setup VertexArrayState to without using VAO "<<vas<<std::endl;
    }

    return vas;
}

void Geometry::compileGLObjects(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();
    if (renderInfo.getState()->useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects))
    {
        unsigned int contextID = state.getContextID();
        GLExtensions* extensions = state.get<GLExtensions>();
        if (!extensions) return;

        typedef std::set<BufferObject*> BufferObjects;
        BufferObjects bufferObjects;

        // first collect all the active unique BufferObjects
        if (_vertexArray.valid() && _vertexArray->getBufferObject()) bufferObjects.insert(_vertexArray->getBufferObject());
        if (_normalArray.valid() && _normalArray->getBufferObject()) bufferObjects.insert(_normalArray->getBufferObject());
        if (_colorArray.valid() && _colorArray->getBufferObject()) bufferObjects.insert(_colorArray->getBufferObject());
        if (_secondaryColorArray.valid() && _secondaryColorArray->getBufferObject()) bufferObjects.insert(_secondaryColorArray->getBufferObject());
        if (_fogCoordArray.valid() && _fogCoordArray->getBufferObject()) bufferObjects.insert(_fogCoordArray->getBufferObject());

        for(ArrayList::const_iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        for(ArrayList::const_iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        for(PrimitiveSetList::const_iterator itr = _primitives.begin();
            itr != _primitives.end();
            ++itr)
        {
            if ((*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        if (bufferObjects.empty())
            return; // no buffers, nothing to compile

        //osg::ElapsedTime timer;

        // now compile any buffer objects that require it.
        for(BufferObjects::iterator itr = bufferObjects.begin();
            itr != bufferObjects.end();
            ++itr)
        {
            GLBufferObject* glBufferObject = (*itr)->getOrCreateGLBufferObject(contextID);
            if (glBufferObject && glBufferObject->isDirty())
            {
                // OSG_NOTICE<<"Compile buffer "<<glBufferObject<<std::endl;
                glBufferObject->compileBuffer();
            }
        }

        // OSG_NOTICE<<"Time to compile "<<timer.elapsedTime_m()<<"ms"<<std::endl;

        if (state.useVertexArrayObject(_useVertexArrayObject))
        {
            VertexArrayState* vas = 0;

            _vertexArrayStateList[contextID] = vas = createVertexArrayState(renderInfo);

            State::SetCurrentVertexArrayStateProxy setVASProxy(state, vas);

            state.bindVertexArrayObject(vas);

            drawVertexArraysImplementation(renderInfo);

            state.unbindVertexArrayObject();
        }

        // unbind the BufferObjects
        extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
    }
    else
    {
        Drawable::compileGLObjects(renderInfo);
    }
}

void Geometry::drawImplementation(RenderInfo& renderInfo) const
{
    // OSG_NOTICE<<"Geometry::drawImplementation() "<<this<<std::endl;

    if (_containsDeprecatedData)
    {
        OSG_WARN<<"Geometry::drawImplementation() unable to render due to deprecated data, call geometry->fixDeprecatedData();"<<std::endl;
        return;
    }

    State& state = *renderInfo.getState();

    bool usingVertexBufferObjects = state.useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects);
    bool usingVertexArrayObjects = usingVertexBufferObjects && state.useVertexArrayObject(_useVertexArrayObject);

    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();
    vas->setVertexBufferObjectSupported(usingVertexBufferObjects);

    bool checkForGLErrors = state.getCheckForGLErrors()==osg::State::ONCE_PER_ATTRIBUTE;
    if (checkForGLErrors) state.checkGLErrors("start of Geometry::drawImplementation()");


    drawVertexArraysImplementation(renderInfo);

    if (checkForGLErrors) state.checkGLErrors("Geometry::drawImplementation() after vertex arrays setup.");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //

    drawPrimitivesImplementation(renderInfo);

    if (usingVertexBufferObjects && !usingVertexArrayObjects)
    {
        // unbind the VBO's if any are used.
        vas->unbindVertexBufferObject();
        vas->unbindElementBufferObject();
    }

    if (checkForGLErrors) state.checkGLErrors("end of Geometry::drawImplementation().");
}

void Geometry::drawVertexArraysImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();
    VertexArrayState* vas = state.getCurrentVertexArrayState();

    bool handleVertexAttributes = !_vertexAttribList.empty();

    AttributeDispatchers& attributeDispatchers = state.getAttributeDispatchers();

    attributeDispatchers.reset();
    attributeDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());

    if (handleVertexAttributes)
    {
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
        {
            attributeDispatchers.activateVertexAttribArray(unit, _vertexAttribList[unit].get());
        }
    }

    // activate or dispatch any attributes that are bound overall
    attributeDispatchers.activateNormalArray(_normalArray.get());
    attributeDispatchers.activateColorArray(_colorArray.get());
    attributeDispatchers.activateSecondaryColorArray(_secondaryColorArray.get());
    attributeDispatchers.activateFogCoordArray(_fogCoordArray.get());

    if (state.useVertexArrayObject(_useVertexArrayObject))
    {
        if (!vas->getRequiresSetArrays()) return;
    }

    vas->lazyDisablingOfVertexAttributes();

    // set up arrays
    if( _vertexArray.valid() )
        vas->setVertexArray(state, _vertexArray.get());

    if (_normalArray.valid() && _normalArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        vas->setNormalArray(state, _normalArray.get());

    if (_colorArray.valid() && _colorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        vas->setColorArray(state, _colorArray.get());

    if (_secondaryColorArray.valid() && _secondaryColorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        vas->setSecondaryColorArray(state, _secondaryColorArray.get());

    if (_fogCoordArray.valid() && _fogCoordArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        vas->setFogCoordArray(state, _fogCoordArray.get());

    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].get();
        if (array)
        {
            vas->setTexCoordArray(state, unit,array);
        }
    }

    if ( handleVertexAttributes )
    {
        for(unsigned int index = 0; index < _vertexAttribList.size(); ++index)
        {
            const Array* array = _vertexAttribList[index].get();
            if (array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
            {
                vas->setVertexAttribArray(state, index, array);
            }
        }
    }

    vas->applyDisablingOfVertexAttributes(state);
}

void Geometry::drawPrimitivesImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();
    AttributeDispatchers& attributeDispatchers = state.getAttributeDispatchers();
    bool usingVertexBufferObjects = state.useVertexBufferObject(_supportsVertexBufferObjects && _useVertexBufferObjects);

    bool bindPerPrimitiveSetActive = attributeDispatchers.active();
    for(unsigned int primitiveSetNum=0; primitiveSetNum!=_primitives.size(); ++primitiveSetNum)
    {
        // dispatch any attributes that are bound per primitive
        if (bindPerPrimitiveSetActive) attributeDispatchers.dispatch(primitiveSetNum);

        const PrimitiveSet* primitiveset = _primitives[primitiveSetNum].get();

        primitiveset->draw(state, usingVertexBufferObjects);
    }
}


void Geometry::accept(AttributeFunctor& af)
{
    AttributeFunctorArrayVisitor afav(af);

    if (_vertexArray.valid())
    {
        afav.applyArray(VERTICES,_vertexArray.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        OSG_INFO<<"Geometry::accept(AttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].get());
    }

    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorArray.get());
    afav.applyArray(FOG_COORDS,_fogCoordArray.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].get());
    }
}


void Geometry::accept(ConstAttributeFunctor& af) const
{
    ConstAttributeFunctorArrayVisitor afav(af);

    if (_vertexArray.valid())
    {
        afav.applyArray(VERTICES,_vertexArray.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        OSG_INFO<<"Geometry::accept(ConstAttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].get());
    }

    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorArray.get());
    afav.applyArray(FOG_COORDS,_fogCoordArray.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].get());
    }
}

void Geometry::accept(PrimitiveFunctor& functor) const
{
    const osg::Array* vertices = _vertexArray.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        OSG_INFO<<"Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    if (_containsDeprecatedData && dynamic_cast<const osg::IndexArray*>(vertices->getUserData())!=0)
    {
        OSG_WARN<<"Geometry::accept(PrimitiveFunctor& functor) unable to work due to deprecated data, call geometry->fixDeprecatedData();"<<std::endl;
        return;
    }

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
        OSG_WARN<<"Warning: Geometry::accept(PrimitiveFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
        return;
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
    }
}

void Geometry::accept(PrimitiveIndexFunctor& functor) const
{
    const osg::Array* vertices = _vertexArray.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        OSG_INFO<<"Geometry::accept(PrimitiveIndexFunctor& functor): Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    if (_containsDeprecatedData && dynamic_cast<const osg::IndexArray*>(vertices->getUserData())!=0)
    {
        OSG_WARN<<"Geometry::accept(PrimitiveIndexFunctor& functor) unable to work due to deprecated data, call geometry->fixDeprecatedData();"<<std::endl;
        return;
    }

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
        OSG_WARN<<"Warning: Geometry::accept(PrimitiveIndexFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
        return;
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
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
    geom->setColorArray(colours, osg::Array::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    geom->setNormalArray(normals, osg::Array::BIND_OVERALL);

    DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
    elems->push_back(0);
    elems->push_back(1);
    elems->push_back(2);

    elems->push_back(2);
    elems->push_back(3);
    elems->push_back(0);
    geom->addPrimitiveSet(elems);

    return geom;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Deprecated methods.
//
#define SET_BINDING(array, ab)\
    osg::Array::Binding binding = static_cast<osg::Array::Binding>(ab); \
    if (!array) \
    { \
        if (binding==osg::Array::BIND_OFF) return; \
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl; \
        return; \
    } \
    if (array->getBinding() == binding) return; \
    array->setBinding(binding);\
    if (binding==osg::Array::BIND_PER_VERTEX) addVertexBufferObjectIfRequired(array); \
    if (ab==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) _containsDeprecatedData = true; \
    dirtyGLObjects();


#define GET_BINDING(array) (array!=0 ? static_cast<AttributeBinding>(array->getBinding()) : BIND_OFF)

#if defined(OSG_DEPRECATED_GEOMETRY_BINDING)
void Geometry::setNormalBinding(AttributeBinding ab)
{
    SET_BINDING(_normalArray.get(), ab)
}

void Geometry::setColorBinding(AttributeBinding ab)
{
    SET_BINDING(_colorArray.get(), ab)
}


void Geometry::setSecondaryColorBinding(AttributeBinding ab)
{
    SET_BINDING(_secondaryColorArray.get(), ab)
}


void Geometry::setFogCoordBinding(AttributeBinding ab)
{
    SET_BINDING(_fogCoordArray.get(), ab)
}

void Geometry::setVertexAttribBinding(unsigned int index,AttributeBinding ab)
{
    osg::Array::Binding binding = static_cast<osg::Array::Binding>(ab);
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        if (_vertexAttribList[index]->getBinding()==binding) return;

        _vertexAttribList[index]->setBinding(binding);

        dirtyGLObjects();
    }
    else
    {
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl;
    }
}

void Geometry::setVertexAttribNormalize(unsigned int index,GLboolean norm)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        _vertexAttribList[index]->setNormalize(norm!=GL_FALSE);

        dirtyGLObjects();
    }
}

Geometry::AttributeBinding Geometry::getNormalBinding() const
{
    return GET_BINDING(_normalArray.get());
}

Geometry::AttributeBinding Geometry::getColorBinding() const
{
    return GET_BINDING(_colorArray.get());
}
Geometry::AttributeBinding Geometry::getSecondaryColorBinding() const
{
    return GET_BINDING(_secondaryColorArray.get());
}

Geometry::AttributeBinding Geometry::getFogCoordBinding() const
{
    return GET_BINDING(_fogCoordArray.get());
}

Geometry::AttributeBinding Geometry::getVertexAttribBinding(unsigned int index) const
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) return static_cast<AttributeBinding>(_vertexAttribList[index]->getBinding());
    else return BIND_OFF;
}


GLboolean Geometry::getVertexAttribNormalize(unsigned int index) const
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) return _vertexAttribList[index]->getNormalize();
    else return GL_FALSE;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Helper methods
//
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
        if (getVertexAttribArray(vi) && getVertexAttribArray(vi)->referenceCount()>1) ++numSharedArrays;
    }
    return numSharedArrays!=0;
}

void Geometry::duplicateSharedArrays()
{
    #define DUPLICATE_IF_REQUIRED(A) \
        if (get##A() && get##A()->referenceCount()>1) \
        { \
            set##A(osg::clone(get##A(), osg::CopyOp::DEEP_COPY_ARRAYS)); \
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
            setTexCoordArray(ti, osg::clone(getTexCoordArray(ti),osg::CopyOp::DEEP_COPY_ARRAYS));
        }
    }

    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        if (getVertexAttribArray(vi) && getVertexAttribArray(vi)->referenceCount()>1)
        {
            setVertexAttribArray(vi, osg::clone(getVertexAttribArray(vi),osg::CopyOp::DEEP_COPY_ARRAYS));
        }
    }
}

namespace GeometryUtilFunctions
{

    inline osg::IndexArray* getIndexArray(osg::Array* array)
    {
        return array ? dynamic_cast<osg::IndexArray*>(array->getUserData()) : 0;
    }

    inline bool containsDeprecatedUsage(osg::Array* array)
    {
        if (array)
        {
            if (array->getBinding()==3 /* BIND_PER_PRIMITIVE */) return true;
            if (dynamic_cast<osg::IndexArray*>(array->getUserData())!=0) return true;
        }
        return false;
    }

    static osg::Array* expandIndexArray(const osg::Array* sourceArray, const osg::IndexArray* indices)
    {
        osg::ref_ptr<Array> targetArray = osg::cloneType(sourceArray);
        targetArray->setBinding(sourceArray->getBinding());
        targetArray->setNormalize(sourceArray->getNormalize());
        targetArray->setPreserveDataType(sourceArray->getPreserveDataType());
        targetArray->resizeArray(indices->getNumElements());

        unsigned int elementSize = sourceArray->getElementSize();
        const char* sourcePtr = static_cast<const char*>(sourceArray->getDataPointer());
        char* targetPtr = const_cast<char*>(static_cast<const char*>(targetArray->getDataPointer()));
        for(unsigned int i=0; i<indices->getNumElements(); ++i)
        {
            unsigned int vi = indices->index(i);
            const char* sourceElementPtr = sourcePtr + elementSize*vi;
            char* targetElementPtr = targetPtr + elementSize*i;
            for(unsigned int j=0; j<elementSize; ++j)
            {
                *targetElementPtr++ = *sourceElementPtr++;
            }
        }
        return targetArray.release();
    }

    typedef std::pair< osg::ref_ptr<osg::Array>, osg::ref_ptr<osg::Array> > ArrayPair;
    typedef std::vector< ArrayPair > ArrayPairs;
    static void duplicateArray(ArrayPairs& pairs, osg::ref_ptr<osg::Array>& sourceArray, unsigned int numVertices)
    {
        osg::Array* targetArray = osg::cloneType(sourceArray.get());
        targetArray->setBinding(osg::Array::BIND_PER_VERTEX);
        targetArray->setNormalize(sourceArray->getNormalize());
        targetArray->setPreserveDataType(sourceArray->getPreserveDataType());
        targetArray->resizeArray(numVertices);
        pairs.push_back(ArrayPair(sourceArray, targetArray));
        sourceArray = targetArray;
    }

    struct PtrData
    {
        char* source;
        char* target;
        unsigned int elementSize;

        PtrData():source(0),target(0),elementSize(0) {}

        PtrData(osg::Array* s, osg::Array* t):
            source(const_cast<char*>(static_cast<const char*>(s->getDataPointer()))),
            target(const_cast<char*>(static_cast<const char*>(t->getDataPointer()))),
            elementSize(s->getElementSize()) {}

        PtrData(const PtrData& rhs):
            source(rhs.source),
            target(rhs.target),
            elementSize(rhs.elementSize) {}

        PtrData& operator = (const PtrData& rhs)
        {
            source = rhs.source;
            target = rhs.target;
            elementSize = rhs.elementSize;
            return *this;
        }
    };

}

bool Geometry::checkForDeprecatedData()
{
    _containsDeprecatedData = false;

    if (GeometryUtilFunctions::containsDeprecatedUsage(_vertexArray.get())) _containsDeprecatedData = true;

    if (GeometryUtilFunctions::containsDeprecatedUsage(_normalArray.get())) _containsDeprecatedData = true;

    if (GeometryUtilFunctions::containsDeprecatedUsage(_colorArray.get())) _containsDeprecatedData = true;

    if (GeometryUtilFunctions::containsDeprecatedUsage(_secondaryColorArray.get())) _containsDeprecatedData = true;

    if (GeometryUtilFunctions::containsDeprecatedUsage(_fogCoordArray.get())) _containsDeprecatedData = true;

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (GeometryUtilFunctions::containsDeprecatedUsage(_texCoordList[ti].get())) _containsDeprecatedData = true;
    }

    for(unsigned int vi=0;vi<getNumVertexAttribArrays();++vi)
    {
        if (GeometryUtilFunctions::containsDeprecatedUsage(_vertexAttribList[vi].get())) _containsDeprecatedData = true;
    }

    return _containsDeprecatedData;
}


void Geometry::fixDeprecatedData()
{
    if (!_containsDeprecatedData) return;

    bool containsBindPerPrimitive = false;

    // copy over attribute arrays.
    osg::IndexArray* indices = GeometryUtilFunctions::getIndexArray(_vertexArray.get());

    if (indices) setVertexArray(GeometryUtilFunctions::expandIndexArray(_vertexArray.get(), indices));

    if (osg::getBinding(_normalArray.get())==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) containsBindPerPrimitive = true;
    indices = GeometryUtilFunctions::getIndexArray(_normalArray.get());
    if (indices) setNormalArray(GeometryUtilFunctions::expandIndexArray(getNormalArray(), indices));

    if (osg::getBinding(getColorArray())==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) containsBindPerPrimitive = true;
    indices = GeometryUtilFunctions::getIndexArray(_colorArray.get());
    if (indices) setColorArray(GeometryUtilFunctions::expandIndexArray(getColorArray(), indices));

    if (osg::getBinding(getSecondaryColorArray())==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) containsBindPerPrimitive = true;
    indices = GeometryUtilFunctions::getIndexArray(_secondaryColorArray.get());
    if (indices) setSecondaryColorArray(GeometryUtilFunctions::expandIndexArray(getSecondaryColorArray(), indices));

    if (osg::getBinding(getFogCoordArray())==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) containsBindPerPrimitive = true;
    indices = GeometryUtilFunctions::getIndexArray(_fogCoordArray.get());
    if (indices) setFogCoordArray(GeometryUtilFunctions::expandIndexArray(getFogCoordArray(), indices));

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        indices = GeometryUtilFunctions::getIndexArray(_texCoordList[ti].get());
        if (indices) setTexCoordArray(ti, GeometryUtilFunctions::expandIndexArray(getTexCoordArray(ti), indices));
    }

    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        if (osg::getBinding(getVertexAttribArray(vi))==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) containsBindPerPrimitive = true;
        indices = GeometryUtilFunctions::getIndexArray(_vertexAttribList[vi].get());
        if (indices) setVertexAttribArray(vi, GeometryUtilFunctions::expandIndexArray(getVertexAttribArray(vi), indices));
    }

    // if none of the arrays are bind per primitive our job is done!
    if (!containsBindPerPrimitive)
    {
        _containsDeprecatedData = false;
        return;
    }

    // need to expand bind per primitive entries.

    // count how many vertices are required
    unsigned int numVertices = 0;
    for(PrimitiveSetList::iterator itr = _primitives.begin();
        itr != _primitives.end();
        ++itr)
    {
        osg::PrimitiveSet* primitiveset = itr->get();
        switch(primitiveset->getType())
        {
            case(PrimitiveSet::DrawArraysPrimitiveType):
            {
                const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                numVertices += drawArray->getCount();
                break;
            }
            case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {
                const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                    primItr!=drawArrayLengths->end();
                    ++primItr)
                {
                    unsigned int localNumVertices = *primItr;
                    numVertices += localNumVertices;
                }
                break;
            }
            case(PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                numVertices += drawElements->getNumIndices();
                break;
            }
            case(PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                numVertices += drawElements->getNumIndices();
                break;
            }
            case(PrimitiveSet::DrawElementsUIntPrimitiveType):
            {
                const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                numVertices += drawElements->getNumIndices();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    // allocate the arrays.
    GeometryUtilFunctions::ArrayPairs perVertexArrays;
    GeometryUtilFunctions::ArrayPairs perPrimitiveArrays;
    if (_vertexArray.valid()) GeometryUtilFunctions::duplicateArray(perVertexArrays, _vertexArray, numVertices);

    if (_normalArray.valid())
    {
        if (_normalArray->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, _normalArray, numVertices);
        else if (_normalArray->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, _normalArray, numVertices);
    }

    if (_colorArray.valid())
    {
        if (_colorArray->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, _colorArray, numVertices);
        else if (_colorArray->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, _colorArray, numVertices);
    }

    if (_secondaryColorArray.valid())
    {
        if (_secondaryColorArray->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, _secondaryColorArray, numVertices);
        else if (_secondaryColorArray->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, _secondaryColorArray, numVertices);
    }

    if (_fogCoordArray.valid())
    {
        if (_fogCoordArray->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, _fogCoordArray, numVertices);
        else if (_fogCoordArray->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, _fogCoordArray, numVertices);
    }

    for(ArrayList::iterator itr = _texCoordList.begin();
        itr != _texCoordList.end();
        ++itr)
    {
        if (itr->valid())
        {
            if ((*itr)->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, *itr, numVertices);
            else if ((*itr)->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, *itr, numVertices);
        }
    }

    for(ArrayList::iterator itr = _vertexAttribList.begin();
        itr != _vertexAttribList.end();
        ++itr)
    {
        if (itr->valid())
        {
            if ((*itr)->getBinding()==osg::Array::BIND_PER_VERTEX) GeometryUtilFunctions::duplicateArray(perVertexArrays, *itr, numVertices);
            else if ((*itr)->getBinding()==3 /*osg::Array::BIND_PER_PRIMITIVE*/) GeometryUtilFunctions::duplicateArray(perPrimitiveArrays, *itr, numVertices);
        }
    }

    typedef std::vector<GeometryUtilFunctions::PtrData> PtrList;
    PtrList perVertexPtrs;
    PtrList perPrimitivePtrs;

    for(GeometryUtilFunctions::ArrayPairs::iterator itr = perVertexArrays.begin();
        itr != perVertexArrays.end();
        ++itr)
    {
        perVertexPtrs.push_back(GeometryUtilFunctions::PtrData(itr->first.get(), itr->second.get()));
    }

    for(GeometryUtilFunctions::ArrayPairs::iterator itr = perPrimitiveArrays.begin();
        itr != perPrimitiveArrays.end();
        ++itr)
    {
        perPrimitivePtrs.push_back(GeometryUtilFunctions::PtrData(itr->first.get(), itr->second.get()));
    }


    // start the primitiveNum at -1 as we increment it the first time through when
    // we start processing the primitive sets.
    int target_vindex = 0;
    int source_pindex = -1; // equals primitiveNum
    for(PrimitiveSetList::iterator prim_itr = _primitives.begin();
        prim_itr != _primitives.end();
        ++prim_itr)
    {
        osg::PrimitiveSet* primitiveset = prim_itr->get();
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

        // copy the vertex data across to the new arays
        switch(primitiveset->getType())
        {
            case(PrimitiveSet::DrawArraysPrimitiveType):
            {
                if (primLength==0) primLength=primitiveset->getNumIndices();

                DrawArrays* drawArray = static_cast<DrawArrays*>(primitiveset);

                if (primLength==0) primLength = drawArray->getCount();

                unsigned int primCount=0;
                unsigned int startindex=drawArray->getFirst();
                drawArray->setFirst(target_vindex);
                unsigned int indexEnd = startindex+drawArray->getCount();
                for(unsigned int vindex=startindex;
                    vindex<indexEnd;
                    ++vindex, ++target_vindex, ++primCount)
                {
                    if ((primCount%primLength)==0) ++source_pindex;

                    // copy bind per vertex from vindex
                    for(PtrList::iterator itr = perVertexPtrs.begin();
                        itr != perVertexPtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + vindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }

                    // copy bind per primitive from source_pindex
                    for(PtrList::iterator itr = perPrimitivePtrs.begin();
                        itr != perPrimitivePtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + source_pindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }
                }
                break;
            }
            case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {
                DrawArrayLengths* drawArrayLengths = static_cast<DrawArrayLengths*>(primitiveset);
                unsigned int vindex=drawArrayLengths->getFirst();
                for(DrawArrayLengths::iterator primItr=drawArrayLengths->begin();
                    primItr!=drawArrayLengths->end();
                    ++primItr)
                {
                    unsigned int localPrimLength;
                    if (primLength==0) localPrimLength=*primItr;
                    else localPrimLength=primLength;
                    drawArrayLengths->setFirst(target_vindex);
                    for(GLsizei primCount=0;
                        primCount<*primItr;
                        ++vindex, ++target_vindex, ++primCount)
                    {
                        if ((primCount%localPrimLength)==0) ++source_pindex;
                        // copy bind per vertex from vindex
                        for(PtrList::iterator itr = perVertexPtrs.begin();
                            itr != perVertexPtrs.end();
                            ++itr)
                        {
                            GeometryUtilFunctions::PtrData& ptrs = *itr;
                            char* source = ptrs.source + vindex*ptrs.elementSize;
                            char* target = ptrs.target + target_vindex*ptrs.elementSize;
                            for(unsigned int c=0; c<ptrs.elementSize; ++c)
                            {
                                *target++ = *source++;
                            }
                        }

                        // copy bind per primitive from source_pindex
                        for(PtrList::iterator itr = perPrimitivePtrs.begin();
                            itr != perPrimitivePtrs.end();
                            ++itr)
                        {
                            GeometryUtilFunctions::PtrData& ptrs = *itr;
                            char* source = ptrs.source + source_pindex*ptrs.elementSize;
                            char* target = ptrs.target + target_vindex*ptrs.elementSize;
                            for(unsigned int c=0; c<ptrs.elementSize; ++c)
                            {
                                *target++ = *source++;
                            }
                        }
                    }
                }
                break;
            }
            case(PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                if (primLength==0) primLength=primitiveset->getNumIndices();

                DrawElementsUByte* drawElements = static_cast<DrawElementsUByte*>(primitiveset);
                unsigned int primCount=0;
                for(DrawElementsUByte::iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primCount, ++target_vindex, ++primItr)
                {
                    if ((primCount%primLength)==0) ++source_pindex;
                    unsigned int vindex=*primItr;
                    *primItr=target_vindex;

                    // copy bind per vertex from vindex
                    for(PtrList::iterator itr = perVertexPtrs.begin();
                        itr != perVertexPtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + vindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }

                    // copy bind per primitive from source_pindex
                    for(PtrList::iterator itr = perPrimitivePtrs.begin();
                        itr != perPrimitivePtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + source_pindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }
                }
                break;
            }
            case(PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                if (primLength==0) primLength=primitiveset->getNumIndices();

                DrawElementsUShort* drawElements = static_cast<DrawElementsUShort*>(primitiveset);
                unsigned int primCount=0;
                for(DrawElementsUShort::iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primCount, ++target_vindex, ++primItr)
                {
                    if ((primCount%primLength)==0) ++source_pindex;
                    unsigned int vindex=*primItr;
                    *primItr=target_vindex;

                    // copy bind per vertex from vindex
                    for(PtrList::iterator itr = perVertexPtrs.begin();
                        itr != perVertexPtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + vindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }
                    // copy bind per primitive from source_pindex
                    for(PtrList::iterator itr = perPrimitivePtrs.begin();
                        itr != perPrimitivePtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + source_pindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }
                }
                break;
            }
            case(PrimitiveSet::DrawElementsUIntPrimitiveType):
            {
                if (primLength==0) primLength=primitiveset->getNumIndices();

                DrawElementsUInt* drawElements = static_cast<DrawElementsUInt*>(primitiveset);
                unsigned int primCount=0;
                for(DrawElementsUInt::iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primCount, ++target_vindex, ++primItr)
                {
                    if ((primCount%primLength)==0) ++source_pindex;
                    unsigned int vindex=*primItr;
                    *primItr=target_vindex;

                    // copy bind per vertex from vindex
                    for(PtrList::iterator itr = perVertexPtrs.begin();
                        itr != perVertexPtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + vindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }

                    // copy bind per primitive from source_pindex
                    for(PtrList::iterator itr = perPrimitivePtrs.begin();
                        itr != perPrimitivePtrs.end();
                        ++itr)
                    {
                        GeometryUtilFunctions::PtrData& ptrs = *itr;
                        char* source = ptrs.source + source_pindex*ptrs.elementSize;
                        char* target = ptrs.target + target_vindex*ptrs.elementSize;
                        for(unsigned int c=0; c<ptrs.elementSize; ++c)
                        {
                            *target++ = *source++;
                        }
                    }
                }
                break;
            }
            default:
            {
                break;
            }
        }
    }

    _containsDeprecatedData = false;
}
