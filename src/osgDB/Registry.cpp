
#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <stdio.h>

#include <algorithm>
#include <set>

using namespace osg;
using namespace osgDB;


class RegistryPtr
{
    public:
        RegistryPtr() : _ptr(0L) {}
        RegistryPtr(Registry* t): _ptr(t) {}
        RegistryPtr(const RegistryPtr& rp):_ptr(rp._ptr) { }
        ~RegistryPtr() { if (_ptr) delete _ptr; _ptr=0L; }

        inline Registry* get() { return _ptr; }

        Registry* _ptr;
};

// definition of the Registry
Registry::Registry()
{
    notify(INFO) << "Constructing osg::Registry"<<std::endl;

    _createNodeFromImage = true;
    _openingLibrary = false;

    osgDB::initFilePath();

    // register file extension alias.
    addFileExtensionAlias("rgba", "rgb");
    addFileExtensionAlias("int",  "rgb");
    addFileExtensionAlias("inta", "rgb");
    addFileExtensionAlias("bw",   "rgb");
    
    addFileExtensionAlias("jpg",  "jpeg");
    addFileExtensionAlias("jpe",  "jpeg");

    addFileExtensionAlias("tif",  "tiff");

    addFileExtensionAlias("geo",  "lwo");
    addFileExtensionAlias("lw",   "lwo");
}


Registry::~Registry()
{
}


Registry* Registry::instance()
{
    static RegistryPtr s_nodeFactory = new Registry();
    return s_nodeFactory.get();
}

void Registry::readCommandLine(std::vector<std::string>& commandLine)
{

    bool found = true;
    while (found)
    {
        found = false;

        // load library option.
        std::vector<std::string>::iterator itr = commandLine.begin();
        for(;itr!=commandLine.end();++itr)
        {
            if (*itr=="-l") break;
        }

        if (itr!=commandLine.end())
        {
            std::vector<std::string>::iterator start = itr; 
            ++itr;
            if (itr!=commandLine.end())
            {
                loadLibrary(*itr);
            }
            commandLine.erase(start,itr);
            found = true;
        }
        

        // load library for extension 
        itr = commandLine.begin();
        for(;itr!=commandLine.end();++itr)
        {
            if (*itr=="-e") break;
        }

        if (itr!=commandLine.end())
        {
            std::vector<std::string>::iterator start = itr; 
            ++itr;
            if (itr!=commandLine.end())
            {
                std::string libName = osgDB::Registry::instance()->createLibraryNameForExt(*itr);
                loadLibrary(libName);
            }
            commandLine.erase(start,itr);
            found = true;
        }
        
        
    }    
}

void Registry::addDotOsgWrapper(DotOsgWrapper* wrapper)
{
    if (wrapper==0L) return;

    if (_openingLibrary) notify(INFO) << "Opening Library : "<< std::endl;

    notify(INFO) << "osg::Registry::addDotOsgWrapper("<<wrapper->getName()<<")"<< std::endl;
    const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();
    
    for(DotOsgWrapper::Associates::const_iterator itr=assoc.begin();
                                                  itr!=assoc.end();
                                                  ++itr)
    {
        notify(INFO) << "    ("<<*itr<<")"<< std::endl;
    }

    const std::string& name = wrapper->getName();
    const osg::Object* proto = wrapper->getPrototype();

    _objectWrapperMap[name] = wrapper;
   
    if (proto)
    {
        if (wrapper->getReadWriteMode()==DotOsgWrapper::READ_AND_WRITE) _classNameWrapperMap[proto->className()] = wrapper;

       if (dynamic_cast<const Image*>(proto))          _imageWrapperMap[name] = wrapper;
       if (dynamic_cast<const Drawable*>(proto))       _drawableWrapperMap[name] = wrapper;
       if (dynamic_cast<const StateAttribute*>(proto)) _stateAttrWrapperMap[name] = wrapper;
       if (dynamic_cast<const Node*>(proto))           _nodeWrapperMap[name] = wrapper;
    }
}

#define EraseMacro(WL,W) \
{ \
    DotOsgWrapperMap::iterator itr = WL.find(W->getName()); \
    if (itr!=WL.end() && itr->second.get()==W) WL.erase(itr); \
}
    
void Registry::removeDotOsgWrapper(DotOsgWrapper* wrapper)
{
    if (wrapper==0L) return;

////    notify(INFO) << "osg::Registry::removeReaderWriter();"<< std::endl;

    EraseMacro(_objectWrapperMap,wrapper);
    EraseMacro(_classNameWrapperMap,wrapper);
    EraseMacro(_imageWrapperMap,wrapper);
    EraseMacro(_drawableWrapperMap,wrapper);
    EraseMacro(_stateAttrWrapperMap,wrapper);
    EraseMacro(_nodeWrapperMap,wrapper);
}

#undef EraseMaroc

void Registry::addReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

    if (_openingLibrary) notify(INFO) << "Opening Library : "<< std::endl;

    notify(INFO) << "osg::Registry::addReaderWriter("<<rw->className()<<")"<< std::endl;

    _rwList.push_back(rw);

}


void Registry::removeReaderWriter(ReaderWriter* rw)
{
    if (rw==0L) return;

//    notify(INFO) << "osg::Registry::removeReaderWriter();"<< std::endl;

    ReaderWriterList::iterator rwitr = std::find(_rwList.begin(),_rwList.end(),rw);
    if (rwitr!=_rwList.end())
    {
        _rwList.erase(rwitr);
    }

}


void Registry::addFileExtensionAlias(const std::string mapExt, const std::string toExt)
{
    if (mapExt!=toExt) _extAliasMap[mapExt] = toExt;
}

std::string Registry::createLibraryNameForFile(const std::string& fileName)
{
    std::string ext = getLowerCaseFileExtension(fileName);
    return createLibraryNameForExt(ext);
}


std::string Registry::createLibraryNameForExt(const std::string& ext)
{

    ExtensionAliasMap::iterator itr=_extAliasMap.find(ext);
    if (itr!=_extAliasMap.end()) return createLibraryNameForExt(itr->second);

#ifdef WIN32
#   ifdef _DEBUG
    return "osgdb_"+ext+"d.dll";
#   else
    return "osgdb_"+ext+".dll";
#   endif
#elif macintosh
    return "osgdb_"+ext;
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

osg::Object* Registry::readObjectOfType(const osg::Object& compObj,Input& fr)
{
    const char *str = fr[0].getStr();
    if (str==NULL) return NULL;

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
            if (compObj.isSameKindAs(obj)) fr+=2;
            return obj;
        }
        else return NULL;

    }

    std::string name = str;
    DotOsgWrapperMap::iterator itr = _objectWrapperMap.find(name);
    if (itr!=_objectWrapperMap.end() && fr[1].isOpenBracket())
    {
    
        DotOsgWrapper* wrapper = itr->second.get();
        const osg::Object* proto = wrapper->getPrototype();
        if (proto==NULL)
        {
            osg::notify(osg::WARN)<<"Token "<<fr[0].getStr()<<" read, but has no prototype, cannot load."<< std::endl;
            return NULL;
        }
        
        if (!compObj.isSameKindAs(proto))
        {
            return NULL;
        }

        // record the number of nested brackets move the input iterator
        // over the name { tokens.
        int entry = fr[0].getNoNestedBrackets();
        fr+=2;

        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();
        osg::Object* obj = proto->clone();

        while(!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool iteratorAdvanced = false;
            if (fr[0].matchWord("UniqueID") && fr[1].isString())
            {
                fr.regisiterUniqueIDForObject(fr[1].getStr(),obj);
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
                if (mitr!=_objectWrapperMap.end())
                {
                    // get the function to read the data...
                    DotOsgWrapper::ReadFunc rf = mitr->second->getReadFunc();
                    if (rf && (*rf)(*obj,fr)) iteratorAdvanced = true;
                }

            }

            if (!iteratorAdvanced) ++fr;
        }
        ++fr;                        // step over trailing '}'
        
        return obj;
        
    }

    return 0L;
    
}

//
// read object from input iterator.
//
osg::Object* Registry::readObject(DotOsgWrapperMap& dowMap,Input& fr)
{
    const char *str = fr[0].getStr();
    if (str==NULL) return NULL;

    std::string name = str;
    DotOsgWrapperMap::iterator itr = dowMap.find(name);
    if (itr!=dowMap.end() && fr[1].isOpenBracket())
    {
    
        DotOsgWrapper* wrapper = itr->second.get();
        const osg::Object* proto = wrapper->getPrototype();
        if (proto==NULL)
        {
            osg::notify(osg::WARN)<<"Token "<<fr[0].getStr()<<" read, but has no prototype, cannot load."<< std::endl;
            return NULL;
        }

        // record the number of nested brackets move the input iterator
        // over the name { tokens.
        int entry = fr[0].getNoNestedBrackets();
        fr+=2;

        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();
        osg::Object* obj = proto->clone();

        while(!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool iteratorAdvanced = false;
            if (fr[0].matchWord("UniqueID") && fr[1].isString())
            {
                fr.regisiterUniqueIDForObject(fr[1].getStr(),obj);
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
                if (mitr!=_objectWrapperMap.end())
                {
                    // get the function to read the data...
                    DotOsgWrapper::ReadFunc rf = mitr->second->getReadFunc();
                    if (rf && (*rf)(*obj,fr)) iteratorAdvanced = true;
                }

            }

            if (!iteratorAdvanced) ++fr;
        }
        ++fr;                        // step over trailing '}'
        
        return obj;
        
    }

    return 0L;
}

//
// read object from input iterator.
//
Object* Registry::readObject(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Object* obj = fr.getObjectForUniqueID(fr[1].getStr());
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
Image* Registry::readImage(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Image* image = dynamic_cast<Image*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (image) fr+=2;
            return image;
        }
        else return NULL;

    }

    return dynamic_cast<Image*>(readObject(_imageWrapperMap,fr));
}


//
// read drawable from input iterator.
//
Drawable* Registry::readDrawable(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Drawable* drawable = dynamic_cast<Drawable*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (drawable) fr+=2;
            return drawable;
        }
        else return NULL;

    }

    return dynamic_cast<Drawable*>(readObject(_drawableWrapperMap,fr));
}

//
// read drawable from input iterator.
//
StateAttribute* Registry::readStateAttribute(Input& fr)
{

    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            StateAttribute* attribute = dynamic_cast<StateAttribute*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (attribute) fr+=2;
            return attribute;
        }
        else return NULL;

    }

    return dynamic_cast<StateAttribute*>(readObject(_stateAttrWrapperMap,fr));
}

//
// read node from input iterator.
//
Node* Registry::readNode(Input& fr)
{
    if (fr[0].matchWord("Use"))
    {
        if (fr[1].isString())
        {
            Node* node = dynamic_cast<Node*>(fr.getObjectForUniqueID(fr[1].getStr()));
            if (node) fr+=2;
            return node;
        }
        else return NULL;

    }

    return dynamic_cast<Node*>(readObject(_nodeWrapperMap,fr));
}

//
// Write object to output 
//
bool Registry::writeObject(const osg::Object& obj,Output& fw)
{

    if (obj.referenceCount()>1)
    {
        std::string uniqueID;
        if (fw.getUniqueIDForObject(&obj,uniqueID))
        {
            fw.indent() << "Use " << uniqueID << std::endl;
            return true;
        }
    }

    std::string classname = obj.className();
    DotOsgWrapperMap::iterator itr = _classNameWrapperMap.find(classname);
    if (itr!=_classNameWrapperMap.end())
    {
    
    
        DotOsgWrapper* wrapper = itr->second.get();
        const DotOsgWrapper::Associates& assoc = wrapper->getAssociates();

        fw.indent() << wrapper->getName() << " {"<< std::endl;
        fw.moveIn();


        // write out the unique ID if required.
        if (obj.referenceCount()>1)
        {
            std::string uniqueID;
            fw.createUniqueIDForObject(&obj,uniqueID);
            fw.registerUniqueIDForObject(&obj,uniqueID);
            fw.indent() << "UniqueID " << uniqueID << std::endl;
        }

        // read the local data by iterating through the associate
        // list, mapping the associate names to DotOsgWrapper's which
        // in turn have the appropriate functions.
        for(DotOsgWrapper::Associates::const_iterator aitr=assoc.begin();
                                                      aitr!=assoc.end();
                                                      ++aitr)
        {
            DotOsgWrapperMap::iterator mitr = _objectWrapperMap.find(*aitr);
            if (mitr!=_objectWrapperMap.end())
            {
                // get the function to read the data...
                DotOsgWrapper::WriteFunc wf = mitr->second->getWriteFunc();
                if (wf) (*wf)(obj,fw);
            }

        }

        fw.moveOut();
        fw.indent() << "}"<< std::endl;

        return true;
    }

    return false;
}

//
// read object from specified file.
//
ReaderWriter::ReadResult Registry::readObject(const std::string& fileName)
{
    char *file = findFile( fileName.c_str() );
    if (file==NULL) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;
    
    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::ReadResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::ReadResult rr = (*itr)->readObject(file,_options.get());
        if (rr.validObject()) return rr;
        else if (rr.error()) results.push_back(rr);
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
                ReaderWriter::ReadResult rr = (*itr)->readObject(file,_options.get());
                if (rr.validObject()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }
    
    if (results.empty())
    {
        return ReaderWriter::ReadResult("Warning: Could not find plugin to read objects from file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}


ReaderWriter::WriteResult Registry::writeObject(const Object& obj,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::WriteResult rr = (*itr)->writeObject(obj,fileName,_options.get());
        if (rr.success()) return rr;
        else if (rr.error()) results.push_back(rr);
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
                ReaderWriter::WriteResult rr = (*itr)->writeObject(obj,fileName,_options.get());
                if (rr.success()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Warning: Could not find plugin to write objects to file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}



ReaderWriter::ReadResult Registry::readImage(const std::string& fileName)
{
    char *file = findFile( fileName.c_str() );
    if (file==NULL) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::ReadResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::ReadResult rr = (*itr)->readImage(file,_options.get());
        if (rr.validImage()) return rr;
        else if (rr.error()) results.push_back(rr);
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
                ReaderWriter::ReadResult rr = (*itr)->readImage(file,_options.get());
                if (rr.validImage()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }

    if (results.empty())
    {
        return ReaderWriter::ReadResult("Warning: Could not find plugin to read image from file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}


ReaderWriter::WriteResult Registry::writeImage(const Image& image,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::WriteResult rr = (*itr)->writeImage(image,fileName,_options.get());
        if (rr.success()) return rr;
        else if (rr.error()) results.push_back(rr);
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
                ReaderWriter::WriteResult rr = (*itr)->writeImage(image,fileName,_options.get());
                if (rr.success()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Warning: Could not find plugin to write image to file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}


ReaderWriter::ReadResult Registry::readNode(const std::string& fileName)
{

    char *file = findFile( fileName.c_str() );
    if (file==NULL) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::ReadResult> Results;
    Results results;

    // first attempt to load the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::ReadResult rr = (*itr)->readNode(file,_options.get());
        if (rr.validNode()) return rr;
        else if (rr.error()) results.push_back(rr);
    }

    // now look for a plug-in to load the file.
    std::string libraryName = createLibraryNameForFile(fileName);
    notify(INFO) << "Now checking for plug-in "<<libraryName<< std::endl;
    if (loadLibrary(libraryName))
    {
        for(ReaderWriterList::iterator itr=_rwList.begin();
            itr!=_rwList.end();
            ++itr)
        {
            if (rwOriginal.find(itr->get())==rwOriginal.end())
            {
                ReaderWriter::ReadResult rr = (*itr)->readNode(file,_options.get());
                if (rr.validNode()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }


    if (_createNodeFromImage)
    {
        ReaderWriter::ReadResult rr = readImage(file);
        if (rr.validImage()) return createGeodeForImage(rr.takeImage());
        else if (rr.error()) results.push_back(rr);
    }

    if (results.empty())
    {
        return ReaderWriter::ReadResult("Warning: Could not find plugin to read nodes from file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}


ReaderWriter::WriteResult Registry::writeNode(const Node& node,const std::string& fileName)
{
    // record the existing reader writer.
    std::set<ReaderWriter*> rwOriginal;

    // record the errors reported by readerwriters.
    typedef std::vector<ReaderWriter::WriteResult> Results;
    Results results;

    // first attempt to write the file from existing ReaderWriter's
    for(ReaderWriterList::iterator itr=_rwList.begin();
        itr!=_rwList.end();
        ++itr)
    {
        rwOriginal.insert(itr->get());
        ReaderWriter::WriteResult rr = (*itr)->writeNode(node,fileName,_options.get());
        if (rr.success()) return rr;
        else if (rr.error()) results.push_back(rr);
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
                ReaderWriter::WriteResult rr = (*itr)->writeNode(node,fileName,_options.get());
                if (rr.success()) return rr;
                else if (rr.error()) results.push_back(rr);
            }
        }
    }

    if (results.empty())
    {
        return ReaderWriter::WriteResult("Warning: Could not find plugin to write nodes to file \""+fileName+"\".");
    }
    else
    {
        return results.front();
    }
}
