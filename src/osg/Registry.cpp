#include "osg/Registry"

#include "osg/Notify"
#include "osg/Object"
#include "osg/Image"
#include "osg/Node"
#include "osg/Group"

#include "osg/Input"
#include "osg/Output"
#include "osg/FileNameUtils"

#include <stdio.h>

#include <algorithm>
#include <set>

using namespace osg;

Object* osg::loadObjectFile(const char *name)
{
    return Registry::instance()->readObject(name);
}
Image*  osg::loadImageFile(const char *name)
{
    return Registry::instance()->readImage(name);
}

Node*   osg::loadNodeFile(const char *name)
{
    return Registry::instance()->readNode(name);
}

bool osg::saveObjectFile(Object& object,const char *name)
{
    return Registry::instance()->writeObject(object,name);
}

bool osg::saveImageFile(Image& image,const char *name)
{
    return Registry::instance()->writeImage(image,name);
}

bool osg::saveNodeFile(Node& node,const char *name)
{
    return Registry::instance()->writeNode(node,name);
}

// definition of the Registry
Registry::Registry()
{
    notify(INFO) << "Constructing osg::Registry"<<endl;
    _openingLibrary = false;
    osg::Init();
}


Registry::~Registry()
{
    // Don't write to notify() in here. Destruction order is indeterminate
    // (I think) and the notify stream may have been destroyed before this
    // is called.

    // note, do not need to unregister attached prototype and reader/writers
    // as they will be automatically unreferenced by the std::vector 
    // destructor & ref_ptr combination.

}

Registry* Registry::instance()
{
    static Registry s_nodeFactory;
    return &s_nodeFactory;
}


void Registry::addPrototype(Object* obj)
{
    if (obj==0L) return;

    // works under win32, good old C :-)
    if (_openingLibrary) notify(INFO) << "Opening Library : ";
    notify(INFO) << "osg::Registry::addPrototype("<<obj->className()<<")"<<endl;

    int newPos = _nodeProtoList.size();

    _protoList.push_back(obj);

    if (dynamic_cast<Node*>(obj))
    {
        _nodeProtoList.push_back(newPos);
    }
    if (dynamic_cast<Image*>(obj))
    {
        _imageProtoList.push_back(newPos);
    }
}

void Registry::removePrototype(Object* obj)
{
    if (obj==0L) return;

    notify(INFO) << "osg::Registry::removePrototype()"<<endl;

    PrototypeList::iterator pitr = std::find(_protoList.begin(),_protoList.end(),obj);
    if (pitr!=_protoList.end())
    {
        _protoList.erase(pitr);
        
        // note need to handle the node and image lists,
        // to be implemented...
    }

}

void Registry::addReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

    if (_openingLibrary) notify(INFO) << "Opening Library : "<<endl;

    notify(INFO) << "osg::Registry::addReaderWriter("<<rw->className()<<")"<<endl;

    _rwList.push_back(rw);

}

void Registry::removeReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

    notify(INFO) << "osg::Registry::removeReaderWriter();"<<endl;

    ReaderWriterList::iterator rwitr = std::find(_rwList.begin(),_rwList.end(),rw);
    if (rwitr!=_rwList.end())
    {
        _rwList.erase(rwitr);
    }

}

std::string Registry::createLibraryNameForFile(const std::string& fileName)
{
    std::string ext = getLowerCaseFileExtension(fileName);
    return createLibraryNameForExt(ext);
}

std::string Registry::createLibraryNameForExt(const std::string& ext)
{

#ifdef WIN32
#   ifdef _DEBUG
    return "osgdb_"+ext+"d.dll";
#   else
    return "osgdb_"+ext+".dll";
#   endif
#else
    return "osgdb_"+ext+".so";
#endif

}

bool Registry::loadLibrary(const std::string& fileName)
{
    DynamicLibrary* dl = getLibrary(fileName);
    if (dl) return false;
    
    _openingLibrary=true;
    
    
    dl = DynamicLibrary::loadLibrary(fileName);
    _openingLibrary=false;

    if (dl)
    {
        dl->ref();
        _dlList.push_back(dl);
        return true;
    }
    return false;
}

bool Registry::closeLibrary(const std::string& fileName)
{
    DynamicLibraryList::iterator ditr = getLibraryItr(fileName);
    if (ditr!=_dlList.end()) 
    {
        (*ditr)->unref();
        _dlList.erase(ditr);
        return true;
    }
    return false;
}

Registry::DynamicLibraryList::iterator Registry::getLibraryItr(const std::string& fileName)
{
    DynamicLibraryList::iterator ditr = _dlList.begin();
    for(;ditr!=_dlList.end();++ditr)
    {
        if ((*ditr)->getName()==fileName) return ditr;
    }
    return _dlList.end();
}

DynamicLibrary* Registry::getLibrary(const std::string& fileName)
{
    DynamicLibraryList::iterator ditr = getLibraryItr(fileName);
    if (ditr!=_dlList.end()) return ditr->get();
    else return NULL;
}


Object* Registry::readObject(Input& fr)
{

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString()) {
            Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (obj) fr+=2;
            return obj;
        }
        else return NULL;
        
    }

    for(unsigned int i=0;i<_protoList.size();++i)
    {
        Object* obj = _protoList[i]->readClone(fr);
        if (obj) return obj;
    }
    return 0L;
}


Object* Registry::readObject(const std::string& fileName)
{
    char *file = FindFile( fileName.c_str() );
    if (file==NULL) return NULL;

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        Object* obj = (*itr)->readObject(file);
        if (obj) return obj;
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (Registry::instance()->loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                Object* obj = (*itr)->readObject(file);
                if (obj) return obj;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to read file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to read file "<<fileName<<endl;


    return NULL;
}


bool Registry::writeObject(Object& obj,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        if ((*itr)->writeObject(obj,fileName)) return true;
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                if ((*itr)->writeObject(obj,fileName)) return true;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to write file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to write file "<<fileName<<endl;

    return false;
}


Image* Registry::readImage(Input& fr)
{

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString()) {
            Image* image = dynamic_cast<Image*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (image) fr+=2;
            return image;
        }
        else return NULL;
        
    }

    for(std::vector<int>::iterator itr=_imageProtoList.begin();
        itr!=_imageProtoList.end();
        ++itr)
    {
        int i=*itr;
        Object* obj = _protoList[i]->readClone(fr);
        if (obj)
        {
            Image* image = static_cast<Image*>(obj);
            return image;
        }
    }
    return 0L;
}


Image* Registry::readImage(const std::string& fileName)
{
    char *file = FindFile( fileName.c_str() );
    if (file==NULL) return NULL;

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        Image* image = (*itr)->readImage(file);
        if (image) return image;
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                Image* image = (*itr)->readImage(file);
                if (image) return image;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to read file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to read file "<<fileName<<endl;

    return NULL;
}


bool Registry::writeImage(Image& image,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        if ((*itr)->writeImage(image,fileName)) return true;
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                if ((*itr)->writeImage(image,fileName)) return true;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to write file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to write file "<<fileName<<endl;

    return false;
}


Node* Registry::readNode(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString()) {
            Node* node = dynamic_cast<Node*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (node) fr+=2;
            return node;
        }
        else return NULL;
        
    }

    std::vector<int>::iterator itr=_nodeProtoList.begin();
    for(;itr!=_nodeProtoList.end();++itr)
    {
        int i=*itr;
        Object* obj = _protoList[i]->readClone(fr);
        if (obj)
        {
            Node* node = static_cast<Node*>(obj);
            return node;
        }
    }
    return 0L;
}


Node* Registry::readNode(const std::string& fileName)
{

    char *file = FindFile( fileName.c_str() );
    if (file==NULL) return NULL;

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        Node* node = (*itr)->readNode(file);
        if (node) return node;
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    notify(INFO) << "Now checking for plug-in "<<libraryName<<endl;
    if (loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                Node* node = (*itr)->readNode(file);
                if (node) return node;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to read file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to read file "<<fileName<<endl;

    return NULL;
}


bool Registry::writeNode(Node& node,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        if ((*itr)->writeNode(node,fileName)) return true;
    }

    // now look for a plug-in to save the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    if (Registry::instance()->loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                if ((*itr)->writeNode(node,fileName)) return true;
            }
        }
    }
    else
    {
        notify(NOTICE)<<"Warning: Could not find plugin to write file with extension ."
                      <<getLowerCaseFileExtension(fileName)<<endl;
    }
    notify(NOTICE)<<"Warning: Unable to write file "<<fileName<<endl;

    return false;
}
