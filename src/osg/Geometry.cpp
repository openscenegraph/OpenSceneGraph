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
#include <osg/ContextData>

using namespace osg;

namespace osg{

    class VAOBufferObjectAffector{

        typedef std::vector< osg::ref_ptr<osg::BufferObject> > BuffSet;

        typedef struct VecBuffSetAndLastNotEmpty{
            VecBuffSetAndLastNotEmpty():lastempty(0) {}
            std::vector<BuffSet> vecBuffSet;
            unsigned int lastempty;
        } VecBuffSetLastnotempty;

        std::map<unsigned int,VecBuffSetLastnotempty > _store;

        unsigned int _hardMaxbuffsize;///prohibit bufferdata concatenation in bufferobject
        unsigned int _softMaxbuffsize;///hint a bufferobject is full (and increment lastempty)
        OpenThreads::Mutex _muter;

    public:
        VAOBufferObjectAffector()
        {
            _hardMaxbuffsize=1200000;
            _softMaxbuffsize=1000000;
        }
        //return Hash
#define MAX_TEX_COORD 8
#define MAX_VERTEX_ATTRIB 8
        unsigned int getArrayList(Geometry*g,Geometry::ArrayList &arrayList){
            unsigned int hash=0;

            if (g->getVertexArray()){ hash++;arrayList.push_back(g->getVertexArray());}
            hash<<=1;
            if (g->getNormalArray()){ hash++;arrayList.push_back(g->getNormalArray());}
            hash<<=1;
            if (g->getColorArray()){ hash++;arrayList.push_back(g->getColorArray());}
            hash<<=1;
            if (g->getSecondaryColorArray()){hash++; arrayList.push_back(g->getSecondaryColorArray());}
            hash<<=1;
            if (g->getFogCoordArray()){hash++; arrayList.push_back(g->getFogCoordArray());}
hash<<=1;
            for(unsigned int unit=0;unit<g->getNumTexCoordArrays();++unit)
            {
                Array* array = g->getTexCoordArray(unit);
                if (array){ hash++;arrayList.push_back(array);}
            hash<<=1;
            }
hash<<=MAX_TEX_COORD-g->getNumTexCoordArrays();
            for(unsigned int  index = 0; index <g->getNumVertexAttribArrays(); ++index )
            {
                Array* array =g->getVertexAttribArray(index);
                if (array){ hash++;arrayList.push_back(array);}
                hash<<=1;
            }
            hash<<=MAX_VERTEX_ATTRIB-g->getNumVertexAttribArrays();
return hash;
        }

        void  unpopulateBufferObjects(Geometry* g)
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_muter);
            Geometry::ArrayList  bdlist;
           unsigned int hash= getArrayList(g,bdlist);
            BuffSet buffsettofind;
            for(    Geometry::ArrayList::iterator arit=bdlist.begin(); arit!=bdlist.end(); arit++)
            {
                buffsettofind.push_back((*arit)->getBufferObject());
                (*arit)->setBufferObject(0);
            }
            unsigned int nbborequired=bdlist.size();
            for(unsigned index=0; index< g->getNumPrimitiveSets(); index++)
            {
                if(dynamic_cast<DrawElements*>(g->getPrimitiveSet(index)))
                {
                   if(bdlist.size()==nbborequired) {
                    nbborequired++;hash++;
                    buffsettofind.push_back(g->getPrimitiveSet(index)->getBufferObject());
                    }

                    g->getPrimitiveSet(index)->setBufferObject(0);
                }
            }

            VecBuffSetLastnotempty& vecBuffSet=_store[hash];

            std::vector<BuffSet>::iterator found;
            unsigned int cpt;
            for(found=vecBuffSet.vecBuffSet.begin(); found!=vecBuffSet.vecBuffSet.end(); found++,cpt++)
            {

                if(*found==buffsettofind)
                {
                    bool todelete=true;
                    for(BuffSet::iterator boit=found->begin();boit!=found->end();boit++)
                        if( (*boit)->getNumBufferData()>0)todelete=false;

                    if(todelete)
                    {
                        vecBuffSet.vecBuffSet.erase(found);if(vecBuffSet.lastempty!=0)vecBuffSet.lastempty--;
                    }
                    else if(vecBuffSet.lastempty>cpt)
                        vecBuffSet.lastempty=cpt;
                    return;
                }
            }
            OSG_WARN<<"unpopulateBufferObjects::Can't happens"<<std::endl;


        }
        ///check bufferobject already guesting the bd
        bool isGuesting(const BufferObject&bo,const BufferData*bd){
                            for(unsigned int i=0;i<bo.getNumBufferData();i++)
                               if(bo.getBufferData(i)==bd)return true;
                            return false;
        }


        void  populateBufferObjects(Geometry* g)
        {

            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_muter);
            Geometry::ArrayList  bdlist;

            unsigned int hash= getArrayList(g,bdlist);
            unsigned int nbborequired=bdlist.size(),nbdrawelmt=0;
            for(unsigned index=0; index< g->getNumPrimitiveSets(); index++)
            {
                if(dynamic_cast<DrawElements*>(g->getPrimitiveSet(index)))
                {
                    if(nbborequired==bdlist.size()){nbborequired++;hash++;}
                    nbdrawelmt++;
                }
            }

            VecBuffSetLastnotempty& vecBuffSet=_store[hash];

            OSG_WARN<<vecBuffSet.lastempty<<std::endl;
            std::vector<BuffSet>::iterator itbuffset=  vecBuffSet.vecBuffSet.begin()+vecBuffSet.lastempty;

            unsigned int * buffSize=new unsigned int [nbborequired+nbdrawelmt],*ptrbuffsize=buffSize;

            //while !itbuffset.canGuest(bdlist)
            bool canGuest=false;bool alreadyGuesting=true;
            Geometry::ArrayList::iterator arit;
            while(!canGuest && itbuffset!=vecBuffSet.vecBuffSet.end())
            {
                canGuest=true;alreadyGuesting=true;

                BuffSet::iterator itbo=(*itbuffset).begin();
                for(arit=bdlist.begin(); arit!=bdlist.end(); itbo++,arit++,ptrbuffsize++)
                {
                    *ptrbuffsize=(*itbo)->computeRequiredBufferSize()+(*arit)->getTotalDataSize();
                    if(*ptrbuffsize>_hardMaxbuffsize)canGuest=false;

                    ///check bufferobject already guesting the bd
                    if(!isGuesting(*itbo->get(),(*arit)))alreadyGuesting=false;

                }
                for(unsigned index=0; index< g->getNumPrimitiveSets(); index++)
                {
                    if(dynamic_cast<DrawElements*>(g->getPrimitiveSet(index)))
                    {
                        *ptrbuffsize=(*itbo)->computeRequiredBufferSize()+g->getPrimitiveSet(index)->getTotalDataSize();
                        if(*ptrbuffsize++>_hardMaxbuffsize)canGuest=false;
                        ///check bufferobject already guesting the bd
                        if(!isGuesting(*itbo->get(),g->getPrimitiveSet(index)))alreadyGuesting=false;
                    }
                }

                ptrbuffsize=buffSize;
                if(alreadyGuesting)
                    return;
                if(!canGuest)itbuffset++;

            }

            if(itbuffset!=vecBuffSet.vecBuffSet.end())
            {
                unsigned buffSetMaxSize=0;
                BuffSet::iterator itbo= itbuffset->begin();
                ptrbuffsize=buffSize;
                for( arit=bdlist.begin(); arit!=bdlist.end(); itbo++,arit++,ptrbuffsize++)
                {
                    (*arit)->setBufferObject(*itbo);
                    buffSetMaxSize=buffSetMaxSize<*ptrbuffsize?*ptrbuffsize:buffSetMaxSize;
                }
                for(unsigned index=0; index< g->getNumPrimitiveSets(); index++)
                {
                    if(dynamic_cast<DrawElements*>(g->getPrimitiveSet(index)))
                    {
                        g->getPrimitiveSet(index)->setBufferObject(*itbo);
                        buffSetMaxSize=buffSetMaxSize<*ptrbuffsize?*ptrbuffsize:buffSetMaxSize;
                        ptrbuffsize++;
                    }
                }
                ///check if bufferobject is full
                if(++itbuffset!=vecBuffSet.vecBuffSet.end())
                {
                    ///can increment
                    if( buffSetMaxSize>_softMaxbuffsize)vecBuffSet.lastempty++;
                }
                delete [] buffSize;
                return;
            }


            ///new BuffSetis required
            BuffSet newBuffSet;
            ElementBufferObject *ebo=0;
            for( arit=bdlist.begin(); arit!=bdlist.end(); arit++)
            {

                {
                    newBuffSet.push_back(new VertexBufferObject());
                    (*arit)->setBufferObject(newBuffSet.back());
                }

            }
            for(unsigned index=0; index< g->getNumPrimitiveSets(); index++)
            {
                if(dynamic_cast<DrawElements*>(g->getPrimitiveSet(index) ))
                {
                    if(!ebo)
                    {
                        ebo=new ElementBufferObject();
                        newBuffSet.push_back(ebo);
                    }
                    g->getPrimitiveSet(index)->setBufferObject(ebo);
                }
            }
            ///if not a newone increment lastempty
            if(!vecBuffSet.vecBuffSet.empty()) vecBuffSet.lastempty++;

            vecBuffSet.vecBuffSet.push_back(newBuffSet);
            delete [] buffSize;
        }
    };
    static VAOBufferObjectAffector VaoBufferObjectAffector;

    class VertexArrayObjectManager : public GraphicsObjectManager
    {
    public:
        VertexArrayObjectManager(unsigned int contextID):
            GraphicsObjectManager("VertexArrayObjectManager", contextID)
        {
        }

        virtual void flushDeletedGLObjects(double, double& availableTime)
        {
        #ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
             flushAllDeletedGLObjects();
        #else
             OSG_INFO<<"VertexArrayObjectManager::flushAllDeletedGLObjects(double,double) Not supported"<<std::endl;
        #endif
        }

        virtual void flushAllDeletedGLObjects()
        {
        #ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_VertexArrayObject);

            for(std::vector<GLuint>::iterator ditr=_tobedeleted.begin();
                ditr!=_tobedeleted.end();
                ++ditr)
            {
                 osg::get<osg::GLExtensions>(_contextID)->glDeleteVertexArrays(1,&*ditr);
            }

            _tobedeleted.clear();
        #else
             OSG_INFO<<"VertexArrayObjectManager::flushAllDeletedGLObjects() Not supported"<<std::endl;
        #endif
             }

        virtual void deleteAllGLObjects()
        {
            flushAllDeletedGLObjects();
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_VertexArrayObject);

                for(Geometry::VertexArrayObjectMap::iterator itr =_vertexArrayObjectMap.begin();itr!= _vertexArrayObjectMap.end();itr++)
                    osg::get<osg::GLExtensions>(_contextID)->glDeleteVertexArrays(1,&itr->second->_GLID);

            }
        }

        virtual void discardAllGLObjects()
        {
            OSG_INFO<<"VertexArrayObjectManager::discardAllGLObjects() Not currently implementated"<<std::endl;
        }

        void deleteVertexArrayObject(Geometry::PerContextVertexArrayObject* globj, Geometry::VertexArrayObjectMap::iterator vaokeyit)
        {
        #ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
            if (globj!=0
                    ///Geometry deref is done after destruction of VertexArrayObjectManager
                    ///so ~VertexArrayObject can recreate an empty one
                    /// (not a problem because glDeleteVertexArrays is already done at previous destruction in deleteAllGLObjects)
                    && !_vertexArrayObjectMap.empty()
                    )
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_VertexArrayObject);
                _tobedeleted.push_back(globj->_GLID);

                _vertexArrayObjectMap.erase(vaokeyit);
            }
        #else
            OSG_INFO<<"Warning: Geometry::deleteVertexArrayObject(..) - not supported."<<std::endl;
        #endif
        }
        Geometry::PerContextVertexArrayObject* getVertexArrayObject(const Geometry::VAOKey & vaokey)
        {
        #ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_VertexArrayObject);
            Geometry::VertexArrayObjectMap::iterator itr = _vertexArrayObjectMap.find(vaokey);// _vertexArrayObjectListMap.lower_bound(sizeHint);
            if (itr!=_vertexArrayObjectMap.end())return itr->second;
        #endif
            return NULL;
        }
        Geometry::PerContextVertexArrayObject* generateVertexArrayObject(const Geometry::VAOKey & vaokey)
        {
        #ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex_VertexArrayObject);

            Geometry::VertexArrayObjectMap::iterator itr = _vertexArrayObjectMap.find(vaokey);// _vertexArrayObjectListMap.lower_bound(sizeHint);
            if (itr!=_vertexArrayObjectMap.end())
            {
                return itr->second;
            }
            else
            {
                GLuint newvao;
                osg::get<osg::GLExtensions>(_contextID)->glGenVertexArrays(1,&newvao);
                Geometry::PerContextVertexArrayObject* ret=new Geometry::PerContextVertexArrayObject(_contextID,newvao);
                _vertexArrayObjectMap[vaokey]=ret;
                ret->_itvao=_vertexArrayObjectMap.insert(std::pair<Geometry::VAOKey,Geometry::PerContextVertexArrayObject*>(vaokey,ret)).first;
                return ret;
            }

        #else
            OSG_INFO<<"Warning: Geometry::generateVertexArrayObject(..) - not supported."<<std::endl;
            return 0;
        #endif
        }

    protected:
        OpenThreads::Mutex _mutex_VertexArrayObject;
        Geometry::VertexArrayObjectMap _vertexArrayObjectMap;
        std::vector<GLuint> _tobedeleted;
    };
}
//////////////////////////////////////////////////////////////////
//////////////////// VertexArrayObject  //////////////////////////
//////////////////////////////////////////////////////////////////

Geometry::PerContextVertexArrayObject::~PerContextVertexArrayObject(){
   osg::get<VertexArrayObjectManager>(_contextID)->deleteVertexArrayObject(const_cast<Geometry::PerContextVertexArrayObject*>(this),_itvao);
}

//////////////////////////////////////////////////////////////////
////////////////////  Geometry  //////////////////////////////////
//////////////////////////////////////////////////////////////////

Geometry::Geometry():
    _containsDeprecatedData(false)
{
    _supportsVertexBufferObjects = true;
    _useVertexArrayObject=false;
    dirtyVertexArrayObject();
    // temporary test
   //  setUseDisplayList(false);
    // setUseVertexArrayObject(true);
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(copyop(geometry._vertexArray.get())),
    _normalArray(copyop(geometry._normalArray.get())),
    _colorArray(copyop(geometry._colorArray.get())),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _fogCoordArray(copyop(geometry._fogCoordArray.get())),
    _containsDeprecatedData(geometry._containsDeprecatedData),
    _useVertexArrayObject(geometry._useVertexArrayObject)
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
        if (_useVertexBufferObjects)
        {
            // copying of arrays doesn't set up buffer objects so we'll need to force
            // Geometry to assign these, we'll do this by changing the cached value to false then re-enabling.
            // note do not use setUseVertexBufferObjects(false) as it might modify Arrays that we have not deep-copied.
            _useVertexBufferObjects = false;
            setUseVertexBufferObjects(true);
        }
    }
    dirtyVertexArrayObject();
}

Geometry::~Geometry()
{
    // do dirty here to keep the getGLObjectSizeHint() estimate on the ball
    dirtyDisplayList();
    setUseVertexArrayObject(false);
    // no need to delete, all automatically handled by ref_ptr :-)
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

void Geometry::setVertexArray(Array* array)
{
    if (array && array->getBinding()==osg::Array::BIND_UNDEFINED) array->setBinding(osg::Array::BIND_PER_VERTEX);

    _vertexArray = array;

    dirtyDisplayList();
    dirtyVertexArrayObject();
    dirtyBound();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setNormalArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _normalArray = array;

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setColorArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _colorArray = array;

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setSecondaryColorArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _secondaryColorArray = array;

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void Geometry::setFogCoordArray(Array* array, osg::Array::Binding binding)
{
    if (array && binding!=osg::Array::BIND_UNDEFINED) array->setBinding(binding);

    _fogCoordArray = array;

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
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

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array)
    {
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

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects)
    {
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

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
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

    dirtyDisplayList();
    dirtyVertexArrayObject();

    if (_useVertexBufferObjects)
    {
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
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives.push_back(primitiveset);
        dirtyDisplayList();
        dirtyVertexArrayObject();
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
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives[i] = primitiveset;
        dirtyDisplayList();
        dirtyVertexArrayObject();
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
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        if (i<_primitives.size())
        {
            _primitives.insert(_primitives.begin()+i,primitiveset);
            dirtyDisplayList();
            dirtyVertexArrayObject();
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
    if (_useVertexBufferObjects)
    {
        for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
        {
            addElementBufferObjectIfRequired(_primitives[primitiveSetIndex].get());
        }

    }
    dirtyDisplayList();
    dirtyVertexArrayObject();
    dirtyBound();
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

        dirtyDisplayList();
        dirtyVertexArrayObject();
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


void Geometry::setUseDisplayList(bool flag){
    if(flag == _useDisplayList)return;
    if(flag&&_useVertexArrayObject)
    {
        setUseVertexArrayObject(false);
        setUseVertexBufferObjects(false);
    }

}

void Geometry::setUseVertexArrayObject(bool flag){
#ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
    if (_useVertexArrayObject == flag) return;

if(_useVertexArrayObject)
    VaoBufferObjectAffector.unpopulateBufferObjects(this);

    _useVertexArrayObject=flag;

    if(flag){ _useVertexBufferObjects=true;
    VaoBufferObjectAffector.populateBufferObjects(this);
}else  setUseVertexBufferObjects(_useVertexBufferObjects);
    dirtyVertexArrayObject();
#else
    OSG_INFO<<"Warning: Geometry::setUseVertexArrayObject(..) - not supported."<<std::endl;
#endif
}

void Geometry::setUseVertexBufferObjects(bool flag)
{

    // OSG_NOTICE<<"Geometry::setUseVertexBufferObjects("<<flag<<")"<<std::endl;

    if(_useVertexArrayObject&&!flag)setUseVertexArrayObject(false);
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
            ElementBufferObjectList eboList;

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

}

void Geometry::dirtyDisplayList()
{
    Drawable::dirtyDisplayList();
}
void Geometry::dirtyVertexArrayObject()
{
#ifdef OSG_GL_VERTEX_ARRAY_OBJECTS_AVAILABLE
    _vao.setAllElementsTo(NULL);
#endif
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
    _vao.setAllElementsTo(NULL);

}

void Geometry::compileGLObjects(RenderInfo& renderInfo) const
{
    bool useVertexArrays = _supportsVertexBufferObjects &&
                           _useVertexBufferObjects &&
                           renderInfo.getState()->isVertexBufferObjectSupported();
    if (useVertexArrays)
    {
        State& state = *renderInfo.getState();
        unsigned int contextID = state.getContextID();
        GLExtensions* extensions = state.get<GLExtensions>();
        if (!extensions) return;

        typedef std::set<BufferObject*> BufferObjects;
        BufferObjects bufferObjects;

        VAOKey VAOkey;
        {///suboptimal stuffing
        ArrayList dummy;
        DrawElementsList drawelmt;
        getDrawElementsList(drawelmt);
        VAOkey.first=VaoBufferObjectAffector.getArrayList(const_cast<Geometry*>(this),dummy);
        if(!drawelmt.empty())VAOkey.first++;
        }

        #define ADDINbufferObjectsANDVAOkey(ARRAY)  \
            {\
            bufferObjects.insert(ARRAY->getBufferObject());\
            VAOkey.second.push_back(ARRAY->getBufferObject());\
            }
        // first collect all the active unique BufferObjects
        if (_vertexArray.valid() && _vertexArray->getBufferObject())                ADDINbufferObjectsANDVAOkey(_vertexArray);
        if (_normalArray.valid() && _normalArray->getBufferObject())                ADDINbufferObjectsANDVAOkey(_normalArray);
        if (_colorArray.valid() && _colorArray->getBufferObject())                  ADDINbufferObjectsANDVAOkey(_colorArray);
        if (_secondaryColorArray.valid() && _secondaryColorArray->getBufferObject())ADDINbufferObjectsANDVAOkey(_secondaryColorArray);
        if (_fogCoordArray.valid() && _fogCoordArray->getBufferObject())            ADDINbufferObjectsANDVAOkey(_fogCoordArray);
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
            if ( _vertexAttribList[unit].valid() &&  _vertexAttribList[unit]->getBufferObject())ADDINbufferObjectsANDVAOkey(_vertexAttribList[unit])

        for(ArrayList::const_iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject())                          ADDINbufferObjectsANDVAOkey(((*itr)));
        }

        for(ArrayList::const_iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject())                          ADDINbufferObjectsANDVAOkey(((*itr)));
        }
        #undef ADDINbufferObjectsANDVAOkey

        unsigned int numebo=0;
        for(PrimitiveSetList::const_iterator itr = _primitives.begin();
            itr != _primitives.end();
            ++itr)
        {
            if ((*itr)->getBufferObject()) {
                bufferObjects.insert((*itr)->getBufferObject());
                if(numebo++==0)VAOkey.second.push_back((*itr)->getBufferObject());
            }
        }

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


        if(_useVertexArrayObject){

            PerContextVertexArrayObject* vao=  osg::get<VertexArrayObjectManager>(contextID)->getVertexArrayObject(VAOkey);
            if(vao)
                _vao[contextID]=vao;
            else
            {

                const BufferObject * ebo;   GLBufferObject *glebo,*uniqueebo; numebo=0;
                ///VAO setup
                state.disableAllVertexArrays();
                state.unbindElementBufferObject();
                state.unbindVertexBufferObject();

                _vao[contextID]=osg::get<VertexArrayObjectManager>(contextID)->generateVertexArrayObject(VAOkey);
                extensions->glBindVertexArray(_vao[contextID]->getGLID());
                ///it would be too easy
                 //drawVertexArraysImplementation(renderInfo);
    ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();

    arrayDispatchers.reset();
    arrayDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());

    arrayDispatchers.activateNormalArray(_normalArray.get());
    arrayDispatchers.activateColorArray(_colorArray.get());
    arrayDispatchers.activateSecondaryColorArray(_secondaryColorArray.get());
    arrayDispatchers.activateFogCoordArray(_fogCoordArray.get());


        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
        {
            arrayDispatchers.activateVertexAttribArray(unit, _vertexAttribList[unit].get());
        }


    // dispatch any attributes that are bound overall
    arrayDispatchers.dispatch(osg::Array::BIND_OVERALL,0);

    state.lazyDisablingOfVertexAttributes();
                 ///check if all arrays have the same offset and activate attribpointer

             #define BASEVERTEXCHECK(ARRAYNAME)  if(basevertex==0)basevertex=vbo->getOffset(array->getBufferIndex());\
                if( basevertex != vbo->getOffset(array->getBufferIndex())){\
                OSG_WARN<<"Geometry::  "<<#ARRAYNAME<<" basevertex is different from the others buffers Object : make sure you've properly set bufferobjects... continuing anyway assuming vertexdivisor is setted"<<std::endl;\
                }

                 Array* array;GLBufferObject* vbo;GLint basevertex=0;
                 array=_vertexArray;
                 if(array){
                     vbo=array->getOrCreateGLBufferObject(contextID);
                    state.bindVertexBufferObject(vbo);
                    state. setVertexPointer( array->getDataSize(),array->getDataType(),0,(const GLvoid *)0,array->getNormalize());

                    BASEVERTEXCHECK(VertexArray)
                }

                 array=_normalArray;
                 if(array){
                     vbo=array->getOrCreateGLBufferObject(contextID);
                    state.bindVertexBufferObject(vbo);
                    state. setNormalPointer( array->getDataType(),0,(const GLvoid *)0,array->getNormalize());
                     BASEVERTEXCHECK(NormalArray)
                }
                 array=_colorArray;
                 if(array){
                     vbo=array->getOrCreateGLBufferObject(contextID);
                    state.bindVertexBufferObject(vbo);
                    state. setNormalPointer( array->getDataType(),0,(const GLvoid *)0,array->getNormalize());
                     BASEVERTEXCHECK(ColorArray)
                }
                 array=_secondaryColorArray;
                 if(array){
                     vbo=array->getOrCreateGLBufferObject(contextID);
                    state.bindVertexBufferObject(vbo);
                    state. setNormalPointer( array->getDataType(),0,(const GLvoid *)0,array->getNormalize());
                     BASEVERTEXCHECK(SecondaryColorArray)
                }
                 array=_fogCoordArray;
                 if(array){
                     vbo=array->getOrCreateGLBufferObject(contextID);
                    state.bindVertexBufferObject(vbo);
                    state. setNormalPointer( array->getDataType(),0,(const GLvoid *)0,array->getNormalize());
                     BASEVERTEXCHECK(FogCoordArray)
                }
           int itex=0;
                for(ArrayList::const_iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr,++itex)
        {
                 array=*itr;
                 vbo=array->getOrCreateGLBufferObject(contextID);
                state.bindVertexBufferObject(vbo);
                state. setTexCoordPointer(itex, array->getDataSize(),array->getDataType(),0,(const GLvoid *)0,array->getNormalize());//(vbo->getOffset(array->getBufferIndex())));
 BASEVERTEXCHECK(TextureCoordinate)
}
itex=0;
  for(ArrayList::const_iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            array=*itr;
                 vbo=array->getOrCreateGLBufferObject(contextID);
                state.bindVertexBufferObject(vbo);
                state. setVertexAttribPointer(itex, array->getDataSize(),array->getDataType(),array->getNormalize(),0,(const GLvoid *)0);//(vbo->getOffset(array->getBufferIndex())));
 BASEVERTEXCHECK(VertexAttribs)

        }
    state.applyDisablingOfVertexAttributes();
       // if (_normalArray.valid() && _normalArray->getBufferObject())
                   /* GLBufferObject* vbo = isVertexBufferObjectSupported() ? array->getOrCreateGLBufferObject(_contextID) : 0;
                if (vbo)
                {
                    state.bindVertexBufferObject(vbo);
                   state. setVertexAttribPointer(unit, array->getDataSize(),array->getDataType(),array->getNormalize(),0,(const GLvoid *)(vbo->getOffset(array->getBufferIndex())));
                }
                else
                {
                   state. unbindVertexBufferObject();
                    state.setVertexAttribPointer(unit, array->getDataSize(),array->getDataType(),array->getNormalize(),0,array->getDataPointer());
                }*/


                for (unsigned int primitiveSetNum = 0; primitiveSetNum != _primitives.size(); ++primitiveSetNum)
                {
                    if ( (ebo = getPrimitiveSet(primitiveSetNum)->getBufferObject()) != NULL){
                        glebo = ebo->getGLBufferObject(contextID);
                        if (glebo){
                            glebo->bindBuffer();
                            if(numebo++!=0 && uniqueebo!=glebo)
                            {
                                OSG_WARN<<"Warning Geometry::compileGLObjects: Trying to compile VAO for a Geometry with multiple ElementBufferObjects"<<std::endl;
                            }
                            else uniqueebo=glebo;
                        }
                    }
                }

                extensions->glBindVertexArray(0);

                state.disableAllVertexArrays();
                state.unbindElementBufferObject();
                state.unbindVertexBufferObject();
            }
        }

        // OSG_NOTICE<<"Time to compile "<<timer.elapsedTime_m()<<"ms"<<std::endl;

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
    if (_containsDeprecatedData)
    {
        OSG_WARN<<"Geometry::drawImplementation() unable to render due to deprecated data, call geometry->fixDeprecatedData();"<<std::endl;
        return;
    }

    State& state = *renderInfo.getState();
    GLuint contextID=state.getContextID();

    bool checkForGLErrors = state.getCheckForGLErrors()==osg::State::ONCE_PER_ATTRIBUTE;
    if (checkForGLErrors) state.checkGLErrors("start of Geometry::drawImplementation()");


    if(!_useVertexArrayObject)
        drawVertexArraysImplementation(renderInfo);
    else
    {
        if(!_vao[contextID].valid())compileGLObjects(renderInfo);
        ///check if BufferObjects have changed
        GLBufferObject* glBufferObject=NULL;BufferObject*bo=NULL;
        #define RECOMPILEIFREQUIRED(XXX) {  bo=(XXX)->getBufferObject();glBufferObject= bo->getOrCreateGLBufferObject(contextID);\
            if (glBufferObject && glBufferObject->isDirty()) glBufferObject->compileBuffer(); }
        if (_vertexArray.valid() && _vertexArray->getBufferObject())                RECOMPILEIFREQUIRED(_vertexArray);
        if (_normalArray.valid() && _normalArray->getBufferObject())                RECOMPILEIFREQUIRED(_normalArray);
        if (_colorArray.valid() && _colorArray->getBufferObject())                  RECOMPILEIFREQUIRED(_colorArray);
        if (_secondaryColorArray.valid() && _secondaryColorArray->getBufferObject())RECOMPILEIFREQUIRED(_secondaryColorArray);
        if (_fogCoordArray.valid() && _fogCoordArray->getBufferObject())            RECOMPILEIFREQUIRED(_fogCoordArray);
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
            if ( _vertexAttribList[unit].valid() &&  _vertexAttribList[unit]->getBufferObject())RECOMPILEIFREQUIRED(_vertexAttribList[unit])

        for(ArrayList::const_iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject())                          RECOMPILEIFREQUIRED(((*itr)));
        }

        for(ArrayList::const_iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject())                          RECOMPILEIFREQUIRED(((*itr)));
        }

        for(PrimitiveSetList::const_iterator itr = _primitives.begin();
            itr != _primitives.end();
            ++itr)
        {
            if ((*itr)->getBufferObject())                                          RECOMPILEIFREQUIRED(*itr);

        }
        #undef RECOMPILEIFREQUIRED
        // unbind the BufferObjects
        state.get<GLExtensions>()->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        state.get<GLExtensions>()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
                        state.unbindElementBufferObject();
                state.unbindVertexBufferObject();
        state.get<GLExtensions>()->glBindVertexArray(_vao[contextID]->getGLID());
    }

    if (checkForGLErrors) state.checkGLErrors("Geometry::drawImplementation() after vertex arrays setup.");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    if(!_useVertexArrayObject){

        drawPrimitivesImplementation(renderInfo);
        // unbind the VBO's if any are used.
        state.unbindVertexBufferObject();
        state.unbindElementBufferObject();
    }
    else{
        /// drawPrimitivesImplementation(renderInfo) not used because there is no such all vertex attribute buffer binding in core profile are assumed to be BIND_PER_VERTEX
        const Array * array=_vertexArray.get();
        if(!array)array=getVertexAttribArray(0);
        GLBufferObject* globj=array->getOrCreateGLBufferObject(contextID);

        GLint vertexbase=
        globj->getOffset(array->getBufferIndex())/
        (array->getElementSize());

        //OSG_WARN<<vertexbase<<std::endl;
        for (unsigned int primitiveSetNum = 0; primitiveSetNum != _primitives.size(); ++primitiveSetNum)
            _primitives[primitiveSetNum]->draw(state, true, vertexbase,true);
        state.get<GLExtensions>()->glBindVertexArray(0);
    }

    if (checkForGLErrors) state.checkGLErrors("end of Geometry::drawImplementation().");
}

void Geometry::drawVertexArraysImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();

    bool handleVertexAttributes = !_vertexAttribList.empty();

    ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();

    arrayDispatchers.reset();
    arrayDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());

    arrayDispatchers.activateNormalArray(_normalArray.get());
    arrayDispatchers.activateColorArray(_colorArray.get());
    arrayDispatchers.activateSecondaryColorArray(_secondaryColorArray.get());
    arrayDispatchers.activateFogCoordArray(_fogCoordArray.get());

    if (handleVertexAttributes)
    {
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
        {
            arrayDispatchers.activateVertexAttribArray(unit, _vertexAttribList[unit].get());
        }
    }

    // dispatch any attributes that are bound overall
    arrayDispatchers.dispatch(osg::Array::BIND_OVERALL,0);

    state.lazyDisablingOfVertexAttributes();

    // set up arrays
    if( _vertexArray.valid() )
        state.setVertexPointer(_vertexArray.get());

    if (_normalArray.valid() && _normalArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setNormalPointer(_normalArray.get());

    if (_colorArray.valid() && _colorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setColorPointer(_colorArray.get());

    if (_secondaryColorArray.valid() && _secondaryColorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setSecondaryColorPointer(_secondaryColorArray.get());

    if (_fogCoordArray.valid() && _fogCoordArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setFogCoordPointer(_fogCoordArray.get());

    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].get();
        if (array)
        {
            state.setTexCoordPointer(unit,array);
        }
    }

    if ( handleVertexAttributes )
    {
        for(unsigned int index = 0; index < _vertexAttribList.size(); ++index)
        {
            const Array* array = _vertexAttribList[index].get();
            if (array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
            {
                if (array->getPreserveDataType())
                {
                    GLenum dataType = array->getDataType();
                    if (dataType==GL_FLOAT) state.setVertexAttribPointer( index, array );
                    else if (dataType==GL_DOUBLE) state.setVertexAttribLPointer( index, array );
                    else state.setVertexAttribIPointer( index, array );
                }
                else
                {
                    state.setVertexAttribPointer( index, array );
                }
            }
        }
    }

    state.applyDisablingOfVertexAttributes();
}

void Geometry::drawPrimitivesImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();
    ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();
    bool usingVertexBufferObjects = _useVertexBufferObjects && state.isVertexBufferObjectSupported();

    bool bindPerPrimitiveSetActive = arrayDispatchers.active(osg::Array::BIND_PER_PRIMITIVE_SET);
    for(unsigned int primitiveSetNum=0; primitiveSetNum!=_primitives.size(); ++primitiveSetNum)
    {
        // dispatch any attributes that are bound per primitive
        if (bindPerPrimitiveSetActive) arrayDispatchers.dispatch(osg::Array::BIND_PER_PRIMITIVE_SET, primitiveSetNum);

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


#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE)
    DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
    elems->push_back(0);
    elems->push_back(1);
    elems->push_back(2);

    elems->push_back(2);
    elems->push_back(3);
    elems->push_back(0);
    geom->addPrimitiveSet(elems);
#else
    geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));
#endif

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
    if (ab==3 /*osg::Geometry::BIND_PER_PRIMITIVE*/) _containsDeprecatedData = true; \
        dirtyDisplayList();\
        dirtyVertexArrayObject();


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

        dirtyDisplayList();
        dirtyVertexArrayObject();
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

        dirtyDisplayList();
        dirtyVertexArrayObject();
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
