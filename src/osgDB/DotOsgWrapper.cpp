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
#include <osgDB/DotOsgWrapper>
#include <osgDB/Registry>

using namespace osgDB;

DotOsgWrapper::DotOsgWrapper(osg::Object* proto,
              const std::string& name,
              const std::string& associates,
              ReadFunc readFunc,
              WriteFunc writeFunc,
              ReadWriteMode readWriteMode)
{


    _prototype = proto;
    _name = name;


    // copy the names in the space delimited associates input into
    // a vector of separated names.
    std::string::size_type start_of_name = associates.find_first_not_of(' ');
    while (start_of_name!=std::string::npos)
    {
        std::string::size_type end_of_name = associates.find_first_of(' ',start_of_name);
        if (end_of_name!=std::string::npos)
        {
            _associates.push_back(std::string(associates,start_of_name,end_of_name-start_of_name));
            start_of_name = associates.find_first_not_of(' ',end_of_name);
        }
        else
        {
            _associates.push_back(std::string(associates,start_of_name,associates.size()-start_of_name));
            start_of_name = end_of_name;
        }
    }

    _readFunc = readFunc;
    _writeFunc = writeFunc;

    _readWriteMode = readWriteMode;
}


RegisterDotOsgWrapperProxy::RegisterDotOsgWrapperProxy(osg::Object* proto,
                            const std::string& name,
                            const std::string& associates,
                            DotOsgWrapper::ReadFunc readFunc,
                            DotOsgWrapper::WriteFunc writeFunc,
                            DotOsgWrapper::ReadWriteMode readWriteMode)
{
    if (Registry::instance())
    {
        _wrapper = new DotOsgWrapper(proto,name,associates,readFunc,writeFunc,readWriteMode);
        Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->addDotOsgWrapper(_wrapper.get());
    }
}

RegisterDotOsgWrapperProxy::~RegisterDotOsgWrapperProxy()
{
    if (Registry::instance())
    {
        Registry::instance()->getDeprecatedDotOsgObjectWrapperManager()->removeDotOsgWrapper(_wrapper.get());
    }
}



void DeprecatedDotOsgWrapperManager::addDotOsgWrapper(DotOsgWrapper* wrapper)
{
    if (wrapper==0L) return;

    //OSG_INFO << "osg::Registry::addDotOsgWrapper("<<wrapper->getName()<<")"<< std::endl;
    const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();

    for(DotOsgWrapper::Associates::const_iterator itr=assoc.begin();
                                                  itr!=assoc.end();
                                                  ++itr)
    {
        //OSG_INFO << "    ("<<*itr<<")"<< std::endl;
    }

    const std::string& name = wrapper->getName();
    const osg::Object* proto = wrapper->getPrototype();

    _objectWrapperMap[name] = wrapper;
    if (wrapper->getReadWriteMode()==DotOsgWrapper::READ_AND_WRITE) _classNameWrapperMap[name] = wrapper;

    if (proto)
    {
        std::string libraryName = proto->libraryName();
        std::string compositeName = libraryName + "::" + name;

        _objectWrapperMap[compositeName] = wrapper;
        if (wrapper->getReadWriteMode()==DotOsgWrapper::READ_AND_WRITE) _classNameWrapperMap[compositeName] = wrapper;

        if (dynamic_cast<const osg::Image*>(proto))
        {
            _imageWrapperMap[name] = wrapper;
            _imageWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const osg::Drawable*>(proto))
        {
              _drawableWrapperMap[name] = wrapper;
              _drawableWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const osg::StateAttribute*>(proto))
        {
            _stateAttrWrapperMap[name] = wrapper;
            _stateAttrWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const osg::Uniform*>(proto))
        {
            _uniformWrapperMap[name] = wrapper;
            _uniformWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const osg::Node*>(proto))
        {
            _nodeWrapperMap[name] = wrapper;
            _nodeWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const osg::Shader*>(proto))
        {
            _shaderWrapperMap[name] = wrapper;
            _shaderWrapperMap[compositeName] = wrapper;
        }


    }
}

// need to change to delete all instances of wrapper, since we
// now can have a wrapper entered twice with the addition of the
// library::class composite name.
void DeprecatedDotOsgWrapperManager::eraseWrapper(DotOsgWrapperMap& wrappermap,DotOsgWrapper* wrapper)
{
    typedef std::vector<DotOsgWrapperMap::iterator> EraseList;
    EraseList eraseList;
    for(DotOsgWrapperMap::iterator witr=wrappermap.begin();
        witr!=wrappermap.end();
        ++witr)
    {
        if (witr->second==wrapper) eraseList.push_back(witr);
    }
    for(EraseList::iterator eitr=eraseList.begin();
        eitr!=eraseList.end();
        ++eitr)
    {
        wrappermap.erase(*eitr);
    }
}

void DeprecatedDotOsgWrapperManager::removeDotOsgWrapper(DotOsgWrapper* wrapper)
{
    if (wrapper==0L) return;

    eraseWrapper(_objectWrapperMap,wrapper);
    eraseWrapper(_classNameWrapperMap,wrapper);
    eraseWrapper(_imageWrapperMap,wrapper);
    eraseWrapper(_drawableWrapperMap,wrapper);
    eraseWrapper(_uniformWrapperMap,wrapper);
    eraseWrapper(_stateAttrWrapperMap,wrapper);
    eraseWrapper(_nodeWrapperMap,wrapper);
    eraseWrapper(_shaderWrapperMap,wrapper);
}

struct concrete_wrapper: basic_type_wrapper
{
    virtual ~concrete_wrapper() {}
    concrete_wrapper(const osg::Object *myobj) : myobj_(myobj) {}
    bool matches(const osg::Object *proto) const
    {
        return myobj_->isSameKindAs(proto);
    }
    const osg::Object *myobj_;
};


osg::Object* DeprecatedDotOsgWrapperManager::readObjectOfType(const osg::Object& compObj,Input& fr)
{
    return readObjectOfType(concrete_wrapper(&compObj), fr);
}

bool DeprecatedDotOsgWrapperManager::getLibraryFileNamesToTry(const std::string& name, FileNames& fileNames)
{
    FileNames::size_type sizeBefore = fileNames.size();

    std::string libraryName = osgDB::Registry::instance()->createLibraryNameForNodeKit(name);
    if (!libraryName.empty()) fileNames.push_back(libraryName);

    libraryName = osgDB::Registry::instance()->createLibraryNameForExtension(std::string("deprecated_")+name);
    if (!libraryName.empty()) fileNames.push_back(libraryName);

    libraryName = osgDB::Registry::instance()->createLibraryNameForExtension(name);
    if (!libraryName.empty()) fileNames.push_back(libraryName);

    return fileNames.size() != sizeBefore;
}

osg::Object* DeprecatedDotOsgWrapperManager::readObjectOfType(const basic_type_wrapper &btw,Input& fr)
{
    const char *str = fr[0].getStr();
    if (str==NULL) return NULL;

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (obj && btw.matches(obj))
            {
                fr+=2;
                return obj;
            }
        }
        else return NULL;

    }

    std::string name = str;
    DotOsgWrapperMap::iterator ow_itr = _objectWrapperMap.find(name);
    if (ow_itr==_objectWrapperMap.end())
    {
        // not found so check if a library::class composite name.
        std::string token = fr[0].getStr();
        std::string::size_type posDoubleColon = token.rfind("::");
        if (posDoubleColon != std::string::npos)
        {
            // we have a composite name so now strip off the library name
            // are try to load it, and then retry the readObject to see
            // if we can recognize the objects.
            std::string libraryName = std::string(token,0,posDoubleColon);

            FileNames fileNames;
            if (getLibraryFileNamesToTry(libraryName, fileNames))
            {
                for(FileNames::iterator itr = fileNames.begin();
                    itr != fileNames.end();
                    ++itr)
                {
                    if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED) return readObjectOfType(btw,fr);
                }
            }
        }
    }
    else if (fr[1].isOpenBracket())
    {
        DotOsgWrapper* wrapper = ow_itr->second.get();
        const osg::Object* proto = wrapper->getPrototype();
        if (proto==NULL)
        {
            OSG_WARN<<"Token "<<fr[0].getStr()<<" read, but has no prototype, cannot load."<< std::endl;
            return NULL;
        }

        if (!btw.matches(proto))
        {
            return NULL;
        }

        // record the number of nested brackets move the input iterator
        // over the name { tokens.
        int entry = fr[0].getNoNestedBrackets();
        fr+=2;

        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();
        osg::Object* obj = proto->cloneType();

        while(!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool iteratorAdvanced = false;
            if (fr[0].matchWord("UniqueID") && fr[1].isString())
            {
                fr.registerUniqueIDForObject(fr[1].getStr(),obj);
                fr += 2;
                iteratorAdvanced = true;
            }

            // read the local data by iterating through the associate
            // list, mapping the associate names to DotOsgWrapper's which
            // in turn have the appropriate functions.
            for(DotOsgWrapper::Associates::const_iterator aitr=assoc.begin();
                                                          aitr!=assoc.end();
                                                          ++aitr)
            {
                DotOsgWrapperMap::iterator mitr = _objectWrapperMap.find(*aitr);
                if (mitr==_objectWrapperMap.end())
                {
                    // not found so check if a library::class composite name.
                    std::string token = *aitr;
                    std::string::size_type posDoubleColon = token.rfind("::");
                    if (posDoubleColon != std::string::npos)
                    {
                        // we have a composite name so now strip off the library name
                        // and try to load it, and then retry the find to see
                        // if we can recognize the objects.
                        std::string libraryName = std::string(token,0,posDoubleColon);

                        FileNames fileNames;
                        if (getLibraryFileNamesToTry(libraryName, fileNames))
                        {
                            for(FileNames::iterator itr = fileNames.begin();
                                itr != fileNames.end() && mitr==_objectWrapperMap.end();
                                ++itr)
                            {
                                if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED)
                                {
                                    mitr = _objectWrapperMap.find(*aitr);
                                }
                            }
                        }
                    }
                }

                if (mitr!=_objectWrapperMap.end())
                {
                    // get the function to read the data...
                    DotOsgWrapper::ReadFunc rf = mitr->second->getReadFunc();
                    if (rf && (*rf)(*obj,fr)) iteratorAdvanced = true;
                }

            }

            if (!iteratorAdvanced) fr.advanceOverCurrentFieldOrBlock();
        }
        ++fr;                        // step over trailing '}'

        return obj;

    }
    return 0L;
}

//
// read object from input iterator.
//
osg::Object* DeprecatedDotOsgWrapperManager::readObject(DotOsgWrapperMap& dowMap,Input& fr)
{
    const char *str = fr[0].getStr();
    if (str==NULL) return NULL;

    std::string name = str;
    DotOsgWrapperMap::iterator dow_itr = dowMap.find(name);
    if (dow_itr==dowMap.end())
    {
        // not found so check if a library::class composite name.
        std::string token = fr[0].getStr();
        std::string::size_type posDoubleColon = token.rfind("::");
        if (posDoubleColon != std::string::npos)
        {
            // we have a composite name so now strip off the library name
            // are try to load it, and then retry the readObject to see
            // if we can recognize the objects.

            std::string libraryName = std::string(token,0,posDoubleColon);

            FileNames fileNames;
            if (getLibraryFileNamesToTry(libraryName, fileNames))
            {
                for(FileNames::iterator itr = fileNames.begin();
                    itr != fileNames.end();
                    ++itr)
                {
                    if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED) return readObject(dowMap,fr);
                }
            }
        }
    }
    else if (fr[1].isOpenBracket())
    {

        DotOsgWrapper* wrapper = dow_itr->second.get();
        const osg::Object* proto = wrapper->getPrototype();
        if (proto==NULL)
        {
            OSG_WARN<<"Token "<<fr[0].getStr()<<" read, but has no prototype, cannot load."<< std::endl;
            return NULL;
        }

        // record the number of nested brackets move the input iterator
        // over the name { tokens.
        int entry = fr[0].getNoNestedBrackets();
        fr+=2;

        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();
        osg::Object* obj = proto->cloneType();

        while(!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool iteratorAdvanced = false;
            if (fr[0].matchWord("UniqueID") && fr[1].isString())
            {
                fr.registerUniqueIDForObject(fr[1].getStr(),obj);
                fr += 2;
                iteratorAdvanced = true;
            }

            // read the local data by iterating through the associate
            // list, mapping the associate names to DotOsgWrapper's which
            // in turn have the appropriate functions.
            for(DotOsgWrapper::Associates::const_iterator aitr=assoc.begin();
                                                          aitr!=assoc.end();
                                                          ++aitr)
            {
                DotOsgWrapperMap::iterator mitr = _objectWrapperMap.find(*aitr);
                if (mitr==_objectWrapperMap.end())
                {
                    // not found so check if a library::class composite name.
                    std::string token = *aitr;
                    std::string::size_type posDoubleColon = token.rfind("::");
                    if (posDoubleColon != std::string::npos)
                    {

                        // we have a composite name so now strip off the library name
                        // are try to load it, and then retry the find to see
                        // if we can recognize the objects.

                        std::string libraryName = std::string(token,0,posDoubleColon);

                        FileNames fileNames;
                        if (getLibraryFileNamesToTry(libraryName, fileNames))
                        {
                            for(FileNames::iterator itr = fileNames.begin();
                                itr != fileNames.end() && mitr==_objectWrapperMap.end();
                                ++itr)
                            {
                                if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED)
                                {
                                    mitr = _objectWrapperMap.find(*aitr);
                                }
                            }
                        }
                    }
                }

                if (mitr!=_objectWrapperMap.end())
                {
                    // get the function to read the data...
                    DotOsgWrapper::ReadFunc rf = mitr->second->getReadFunc();
                    if (rf && (*rf)(*obj,fr)) iteratorAdvanced = true;
                }

            }

            if (!iteratorAdvanced) fr.advanceOverCurrentFieldOrBlock();
        }
        ++fr;                        // step over trailing '}'

        return obj;

    }

    return 0L;
}

//
// read object from input iterator.
//
osg::Object* DeprecatedDotOsgWrapperManager::readObject(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (obj) fr+=2;
            return obj;
        }
        else return NULL;

    }

    return readObject(_objectWrapperMap,fr);
}


//
// read image from input iterator.
//
osg::Image* DeprecatedDotOsgWrapperManager::readImage(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Image* image = dynamic_cast<osg::Image*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (image) fr+=2;
            return image;
        }
        else return NULL;

    }

    osg::Object* obj = readObject(_imageWrapperMap,fr);
    osg::Image* image = dynamic_cast<osg::Image*>(obj);
    if (image) return image;
    else if (obj) obj->unref();

    return NULL;
}


//
// read drawable from input iterator.
//
osg::Drawable* DeprecatedDotOsgWrapperManager::readDrawable(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (drawable) fr+=2;
            return drawable;
        }
        else return NULL;

    }

    osg::Object* obj = readObject(_drawableWrapperMap,fr);
    osg::Drawable* drawable = dynamic_cast<osg::Drawable*>(obj);
    if (drawable) return drawable;
    else if (obj) obj->unref();

    return NULL;
}

//
// read drawable from input iterator.
//
osg::StateAttribute* DeprecatedDotOsgWrapperManager::readStateAttribute(Input& fr)
{

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::StateAttribute* attribute = dynamic_cast<osg::StateAttribute*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (attribute) fr+=2;
            return attribute;
        }
        else return NULL;

    }

    return dynamic_cast<osg::StateAttribute*>(readObject(_stateAttrWrapperMap,fr));
}

//
// read drawable from input iterator.
//
osg::Uniform* DeprecatedDotOsgWrapperManager::readUniform(Input& fr)
{

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Uniform* attribute = dynamic_cast<osg::Uniform*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (attribute) fr+=2;
            return attribute;
        }
        else return NULL;

    }

    return dynamic_cast<osg::Uniform*>(readObject(_uniformWrapperMap,fr));
}

//
// read node from input iterator.
//
osg::Node* DeprecatedDotOsgWrapperManager::readNode(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Node* node = dynamic_cast<osg::Node*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (node) fr+=2;
            return node;
        }
        else return NULL;

    }

    osg::Object* obj = readObject(_nodeWrapperMap,fr);
    osg::Node* node = dynamic_cast<osg::Node*>(obj);
    if (node) return node;
    else if (obj) obj->unref();

    return NULL;
}

//
// read image from input iterator.
//
osg::Shader* DeprecatedDotOsgWrapperManager::readShader(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            osg::Shader* shader = dynamic_cast<osg::Shader*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (shader) fr+=2;
            return shader;
        }
        else return NULL;

    }

    osg::Object* obj = readObject(_shaderWrapperMap,fr);
    osg::Shader* shader = dynamic_cast<osg::Shader*>(obj);
    if (shader) return shader;
    else if (obj) obj->unref();

    return NULL;
}

//
// Write object to output
//
bool DeprecatedDotOsgWrapperManager::writeObject(const osg::Object& obj,Output& fw)
{

    if (obj.referenceCount()>1)
    {
        std::string uniqueID;
        if (fw.getUniqueIDForObject(&obj,uniqueID))
        {
            fw.writeUseID( uniqueID );
            return true;
        }
    }

    const std::string classname( obj.className() );
    const std::string libraryName( obj.libraryName() );
    const std::string compositeName( libraryName + "::" + classname );

    // try composite name first
    DotOsgWrapperMap::iterator cnw_itr = _classNameWrapperMap.find(compositeName);

    if (cnw_itr==_classNameWrapperMap.end())
    {
        FileNames fileNames;
        if (getLibraryFileNamesToTry(libraryName, fileNames))
        {
            for(FileNames::iterator itr = fileNames.begin();
                itr != fileNames.end();
                ++itr)
            {
                if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED) return writeObject(obj,fw);
            }
        }

        // otherwise try simple class name
        if (cnw_itr == _classNameWrapperMap.end())
            cnw_itr = _classNameWrapperMap.find(classname);
    }

    if (cnw_itr!=_classNameWrapperMap.end())
    {
        DotOsgWrapper* wrapper = cnw_itr->second.get();
        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();

        if (libraryName=="osg")
        {
            // member of the core osg, so no need to have composite library::class name.
            fw.writeBeginObject( wrapper->getName() );
        }
        else
        {
            // member of the node kit so must use composite library::class name.
            std::string::size_type posDoubleColon = wrapper->getName().find("::");
            if (posDoubleColon != std::string::npos)
            {
                fw.writeBeginObject( wrapper->getName() );
            }
            else
            {
                fw.writeBeginObject( libraryName + "::" + wrapper->getName() );
            }
        }
        fw.moveIn();


        // write out the unique ID if required.
        if (obj.referenceCount()>1)
        {
            std::string uniqueID;
            fw.createUniqueIDForObject(&obj,uniqueID);
            fw.registerUniqueIDForObject(&obj,uniqueID);
            fw.writeUniqueID( uniqueID );
        }

        // read the local data by iterating through the associate
        // list, mapping the associate names to DotOsgWrapper's which
        // in turn have the appropriate functions.
        for(DotOsgWrapper::Associates::const_iterator aitr=assoc.begin();
                                                      aitr!=assoc.end();
                                                      ++aitr)
        {
            DotOsgWrapperMap::iterator mitr = _objectWrapperMap.find(*aitr);
            if (mitr==_objectWrapperMap.end())
            {
                // not found so check if a library::class composite name.
                std::string token = *aitr;
                std::string::size_type posDoubleColon = token.rfind("::");
                if (posDoubleColon != std::string::npos)
                {

                    // we have a composite name so now strip off the library name
                    // are try to load it, and then retry the find to see
                    // if we can recognize the objects.

                    std::string assoc_libraryName = std::string(token,0,posDoubleColon);

                    FileNames fileNames;
                    if (getLibraryFileNamesToTry(assoc_libraryName, fileNames))
                    {
                        for(FileNames::iterator itr = fileNames.begin();
                            itr != fileNames.end() && mitr==_objectWrapperMap.end();
                            ++itr)
                        {
                            if (osgDB::Registry::instance()->loadLibrary(*itr)==osgDB::Registry::LOADED)
                            {
                                mitr = _objectWrapperMap.find(*aitr);
                            }
                        }
                    }
                }
            }
            if (mitr!=_objectWrapperMap.end())
            {
                // get the function to read the data...
                DotOsgWrapper::WriteFunc wf = mitr->second->getWriteFunc();
                if (wf) (*wf)(obj,fw);
            }

        }

        fw.moveOut();
        fw.writeEndObject();

        return true;
    }

    return false;
}

