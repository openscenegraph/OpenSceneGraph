
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

void PrintFilePathList(std::ostream& stream,const FilePathList& filepath)
{
    for(FilePathList::const_iterator itr=filepath.begin();
        itr!=filepath.end();
        ++itr)
    {
        stream << "    "<< *itr<<std::endl;
    }
}

class RegistryPtr
{
    public:
        RegistryPtr() : _ptr(0L) {}
        RegistryPtr(Registry* t): _ptr(t) {}
        RegistryPtr(const RegistryPtr& rp):_ptr(rp._ptr) { }
        ~RegistryPtr() { if (_ptr) osgDelete _ptr; _ptr=0L; }

        inline Registry* get() { return _ptr; }

        Registry* _ptr;
};

Registry* Registry::instance()
{
    static RegistryPtr s_nodeFactory = osgNew Registry;
    return s_nodeFactory.get();
}


// definition of the Registry
Registry::Registry()
{
    notify(INFO) << "Constructing osg::Registry"<<std::endl;

    _createNodeFromImage = true;
    _openingLibrary = false;

    initFilePathLists();

    // register file extension alias.
    addFileExtensionAlias("sgi", "rgb");
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



void Registry::initDataFilePathList()
{
    //
    // set up data file paths
    //
    char *ptr;
    if( (ptr = getenv( "OSG_FILE_PATH" )) )
    {
        notify(DEBUG_INFO) << "OSG_FILE_PATH("<<ptr<<")"<<std::endl;
        setDataFilePathList(ptr);
    }
    else if( (ptr = getenv( "OSGFILEPATH" )) )
    {
        notify(DEBUG_INFO) << "OSGFILEPATH("<<ptr<<")"<<std::endl;
        setDataFilePathList(ptr);
    }

    osg::notify(INFO)<<"Data FilePathList"<<std::endl;
    PrintFilePathList(osg::notify(INFO),getDataFilePathList());
}

void Registry::initLibraryFilePathList()
{
    //
    // set up library paths
    //
    char* ptr;
    if( (ptr = getenv( "OSG_LIBRARY_PATH")) )
    {
        notify(DEBUG_INFO) << "OSG_LIBRARY_PATH("<<ptr<<")"<<std::endl;
        setLibraryFilePathList(ptr);
    }
    else if( (ptr = getenv( "OSG_LD_LIBRARY_PATH")) )
    {
        notify(DEBUG_INFO) << "OSG_LD_LIBRARY_PATH("<<ptr<<")"<<std::endl;
        setLibraryFilePathList(ptr);
    }

#ifdef __sgi

    convertStringPathIntoFilePathList("/usr/lib32/:/usr/local/lib32/",_libraryFilePath);

    // bloody mess see rld(1) man page
    #if (_MIPS_SIM == _MIPS_SIM_ABI32)


    if( (ptr = getenv( "LD_LIBRARY_PATH" )))
    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }

    #elif (_MIPS_SIM == _MIPS_SIM_NABI32)

    if( !(ptr = getenv( "LD_LIBRARYN32_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }

    #elif (_MIPS_SIM == _MIPS_SIM_ABI64)

    if( !(ptr = getenv( "LD_LIBRARY64_PATH" )))
        ptr = getenv( "LD_LIBRARY_PATH" );

    if( ptr )
    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }
    #endif
    
#elif defined(__CYGWIN__)


    if ((ptr = getenv( "PATH" )))
    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }

    convertStringPathIntoFilePathList("/usr/bin/:/usr/local/bin/",_libraryFilePath);
    
#elif defined(WIN32)



    if ((ptr = getenv( "PATH" )))
    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }

    convertStringPathIntoFilePathList("C:/Windows/System/",_libraryFilePath);

#else   

    if( (ptr = getenv( "LD_LIBRARY_PATH" )) )
    {
        convertStringPathIntoFilePathList(ptr,_libraryFilePath);
    }

    convertStringPathIntoFilePathList("/usr/lib/:/usr/local/lib/",_libraryFilePath);

#endif

    osg::notify(INFO)<<"Library FilePathList"<<std::endl;
    PrintFilePathList(osg::notify(INFO),getLibraryFilePathList());

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
                ++itr;
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
                ++itr;
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
        std::string libraryName = proto->libraryName();
        std::string compositeName = libraryName + "::" + name;

        _objectWrapperMap[compositeName] = wrapper;

        if (wrapper->getReadWriteMode()==DotOsgWrapper::READ_AND_WRITE) _classNameWrapperMap[proto->className()] = wrapper;

        if (dynamic_cast<const Image*>(proto))
        {
            _imageWrapperMap[name] = wrapper;
            _imageWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const Drawable*>(proto))
        {
              _drawableWrapperMap[name] = wrapper;
              _drawableWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const StateAttribute*>(proto))
        {
            _stateAttrWrapperMap[name] = wrapper;
            _stateAttrWrapperMap[compositeName] = wrapper;
        }
        if (dynamic_cast<const Node*>(proto))
        {
            _nodeWrapperMap[name] = wrapper;
            _nodeWrapperMap[compositeName] = wrapper;
        }


    }
}

// need to change to delete all instances of wrapper, since we
// now can have a wrapper entered twice with the addition of the
// library::class composite name.
void Registry::eraseWrapper(DotOsgWrapperMap& wrappermap,DotOsgWrapper* wrapper)
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

    
void Registry::removeDotOsgWrapper(DotOsgWrapper* wrapper)
{
    if (wrapper==0L) return;

    eraseWrapper(_objectWrapperMap,wrapper);
    eraseWrapper(_classNameWrapperMap,wrapper);
    eraseWrapper(_imageWrapperMap,wrapper);
    eraseWrapper(_drawableWrapperMap,wrapper);
    eraseWrapper(_stateAttrWrapperMap,wrapper);
    eraseWrapper(_nodeWrapperMap,wrapper);
}

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

#if defined(WIN32)
    // !! recheck evolving Cygwin DLL extension naming protocols !! NHV
    #ifdef __CYGWIN__
        return "cygosgdb_"+ext+".dll";
    #elif defined(__MINGW32__)
        return "libosgdb_"+ext+".dll";
    #else
        #ifdef _DEBUG
            return "osgdb_"+ext+"d.dll";
        #else
            return "osgdb_"+ext+".dll";
        #endif
    #endif
#elif macintosh
    return "osgdb_"+ext;
#else
    return "osgdb_"+ext+".so";
#endif

}

std::string Registry::createLibraryNameForNodeKit(const std::string& name)
{
#if defined(WIN32)
    // !! recheck evolving Cygwin DLL extension naming protocols !! NHV
    #ifdef __CYGWIN__ // [
	return "cyg"+name+".dll";
    #elif defined(__MINGW32__)
        return "lib"+name+".dll";
    #else
        #ifdef _DEBUG
            return name+"d.dll";
        #else
            return name+".dll";
        #endif
    #endif
#elif macintosh
    return name;
#else
    return "lib"+name+".so";
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
            if (compObj.isSameKindAs(obj))
            {
                fr+=2;
                return obj;
            }
        }
        else return NULL;

    }

    std::string name = str;
    DotOsgWrapperMap::iterator itr = _objectWrapperMap.find(name);
    if (itr==_objectWrapperMap.end())
    {
        // not found so check if a library::class composite name.
        std::string token = fr[0].getStr();
        std::string::size_type posDoubleColon = token.rfind("::");
        if (posDoubleColon != std::string::npos)
        {
            // we have a composite name so now strip off the library name
            // are try to load it, and then retry the readObject to see
            // if we can recongise the objects.
        
            std::string libraryName = std::string(token,0,posDoubleColon);

            // first try the standard nodekit library.
            std::string nodeKitLibraryName = createLibraryNameForNodeKit(libraryName);
            if (loadLibrary(nodeKitLibraryName)) return readObjectOfType(compObj,fr);
            
            // otherwise try the osgdb_ plugin library.
            std::string pluginLibraryName = createLibraryNameForExt(libraryName);
            if (loadLibrary(pluginLibraryName)) return readObjectOfType(compObj,fr);
        }
    }
    else if (fr[1].isOpenBracket())
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
        osg::Object* obj = proto->cloneType();

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
osg::Object* Registry::readObject(DotOsgWrapperMap& dowMap,Input& fr)
{
    const char *str = fr[0].getStr();
    if (str==NULL) return NULL;

    std::string name = str;
    DotOsgWrapperMap::iterator itr = dowMap.find(name);
    if (itr==dowMap.end())
    {
        // not found so check if a library::class composite name.
        std::string token = fr[0].getStr();
        std::string::size_type posDoubleColon = token.rfind("::");
        if (posDoubleColon != std::string::npos)
        {
            // we have a composite name so now strip off the library name
            // are try to load it, and then retry the readObject to see
            // if we can recongise the objects.
        
            std::string libraryName = std::string(token,0,posDoubleColon);

            // first try the standard nodekit library.
            std::string nodeKitLibraryName = createLibraryNameForNodeKit(libraryName);
            if (loadLibrary(nodeKitLibraryName)) return readObject(dowMap,fr);
            
            // otherwise try the osgdb_ plugin library.
            std::string pluginLibraryName = createLibraryNameForExt(libraryName);
            if (loadLibrary(pluginLibraryName)) return readObject(dowMap,fr);
        }
    }
    else if (fr[1].isOpenBracket())
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
        osg::Object* obj = proto->cloneType();

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

    osg::Object* obj = readObject(_imageWrapperMap,fr);
    osg::Image* image = dynamic_cast<Image*>(obj);
    if (image) return image;
    else if (obj) obj->unref();
    
    return NULL;
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

    osg::Object* obj = readObject(_drawableWrapperMap,fr);
    osg::Drawable* drawable = dynamic_cast<Drawable*>(obj);
    if (drawable) return drawable;
    else if (obj) obj->unref();
    
    return NULL;
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

    osg::Object* obj = readObject(_nodeWrapperMap,fr);
    osg::Node* node = dynamic_cast<Node*>(obj);
    if (node) return node;
    else if (obj) obj->unref();
    
    return NULL;
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

        if (strcmp(obj.libraryName(),"osg")==0)
        {
            // member of the core osg, so no need to have composite library::class name.
            fw.indent() << wrapper->getName() << " {"<< std::endl;
            fw.moveIn();
        }
        else
        {
            // member of the node kit so must use composite library::class name.
            fw.indent() << obj.libraryName()<<"::"<< wrapper->getName() << " {"<< std::endl;
            fw.moveIn();
        }


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
    std::string file = findDataFile( fileName );
    if (file.empty()) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    PushAndPopDataPath tmpfile(getFilePath(fileName));

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
    if (loadLibrary(libraryName))
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

    return results.front();
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

    return results.front();
}



ReaderWriter::ReadResult Registry::readImage(const std::string& fileName)
{
    std::string file = findDataFile( fileName );
    if (file.empty()) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    PushAndPopDataPath tmpfile(getFilePath(fileName));

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

    return results.front();
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
    
    return results.front();
}


ReaderWriter::ReadResult Registry::readNode(const std::string& fileName)
{

    std::string file = findDataFile( fileName );
    if (file.empty()) return ReaderWriter::ReadResult("Warning: file \""+fileName+"\" not found.");

    PushAndPopDataPath tmpfile(getFilePath(fileName));

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
        //else if (rr.error()) results.push_back(rr);
    }

    if (results.empty())
    {
        return ReaderWriter::ReadResult("Warning: Could not find plugin to read nodes from file \""+fileName+"\".");
    }

    return results.front();
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
    if (loadLibrary(libraryName))
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

    return results.front();
}

void Registry::convertStringPathIntoFilePathList(const std::string& paths,FilePathList& filepath)
{
#if defined(WIN32) && !defined(__CYGWIN__)
    char delimitor = ';';
#else
    char delimitor = ':';
#endif

    if (!paths.empty())
    {
        std::string::size_type start = 0;
        std::string::size_type end;
        while ((end = paths.find_first_of(delimitor,start))!=std::string::npos)
        {
            filepath.push_back(std::string(paths,start,end-start));
            start = end+1;
        }

        filepath.push_back(std::string(paths,start,std::string::npos));
    }
 
}
