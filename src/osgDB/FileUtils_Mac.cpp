/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
// FileUtil_Mac asses that defined(TARGET_API_MAC_CARBON) is valid.
#include <CoreServices/CoreServices.h>

#include <stdio.h>
#include <stdlib.h>

#include <osg/Notify>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>

#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

using namespace osg;
using namespace osgDB;
using namespace std;

// follows is definition of strdup for compatibility under mac,
// However, I'd prefer to migrate all the findFindInPath tools to use
// std::string's and then be able to remove the following definition.
// My objective is to minimize the number of platform #ifdef's as they
// are source of potential bugs and developer confusion.

#define FILEUTILS_MAX_PATH_LENGTH 2048

char *PathDelimitor = " ";
static const char *s_default_file_path = ":";
static const char *s_default_dso_path = ":";
static char *s_filePath = ":";

static bool s_filePathInitialized = false;

// MACOS utilities

 /** MAC Carbon only : get an FSSpec for the named file system object (file or folder) contained within the specified folder
    ridiculous that this kind of thing doesn't seem to be built into carbon. or manybe I'm just not finding it ?
    give me back my cocoa.
    TODO : we should probably do a version of this routine that searches subfolders too
 */
static OSErr getObjectContainedInFolder(const std::string& objectName,FSSpecPtr containingFolder,FSSpecPtr theFSSpecPtr)
 {
    FSRef	theObjectRef;
    FSRef	containingFolderRef;
    CFStringRef unicodeObjectName;
    OSErr	errCode;
    UniChar	objectNameChars[ 255 ];
    CFIndex	nameLength;
    
 
    errCode=FSpMakeFSRef(containingFolder,&containingFolderRef); // make FSREf to containing folder
    if(errCode==noErr)
    {
        unicodeObjectName=CFStringCreateWithCString (NULL,objectName.c_str(),kCFStringEncodingASCII);
        nameLength=CFStringGetLength( unicodeObjectName );
        CFStringGetCharacters( unicodeObjectName, CFRangeMake( 0, nameLength ), &objectNameChars[0] );
        errCode=FSMakeFSRefUnicode (&containingFolderRef,nameLength,objectNameChars,CFStringGetSystemEncoding(),&theObjectRef);
        CFRelease(unicodeObjectName);
        if(errCode==noErr)
        {// the object exists, so we just have to make an FSSpec from the FSRef.
            return FSGetCatalogInfo (&theObjectRef, kFSCatInfoNone, NULL,NULL,theFSSpecPtr,NULL);
        }
    }
    return errCode;
 }
 
 /** MAC Carbon only : get an FSSpec for the named file system object (file or folder) contained within the specified folder
    this just makes an FSSpec and calls the getObjectContainedInFolder
*/

 static OSErr getObjectContainedInFolder(const std::string& objectName,SInt16 containingFolderVolRef, SInt32 containingFolderDirID,FSSpecPtr theFSSpecPtr)
 {
    FSSpec container;
    OSErr errCode;
    errCode=FSMakeFSSpec(containingFolderVolRef,containingFolderDirID,(ConstStr255Param)"",&container);
    if(errCode==noErr)
    {
        errCode=getObjectContainedInFolder(objectName,&container,theFSSpecPtr);
    }
    return errCode;
}
/** MAC Carbon only : get an FSSpec for the current application package.
    this is mostly useful for finding the Plug-ins directory if the application chooses to store plugins in a folder next
    to the application in the "old fashioned" way.
    This code directly taken from Apple technote TN2015 (locating application support files under OSX)
    TODO - this currently only supports applications that are built as a bundle, not simple unix style exectutables.
    The technote above describes how to support this situation and we should probably add support for that at some point. But I need to sleep now.
*/
static OSErr getApplicationFSSpec(FSSpecPtr theFSSpecPtr)
 {
    OSErr err = fnfErr;
    CFBundleRef myAppsBundle = CFBundleGetMainBundle();
    if (myAppsBundle == NULL) return err;
    CFURLRef myBundleURL = CFBundleCopyBundleURL(myAppsBundle);
    if (myBundleURL == NULL) return err;
    
    FSRef myBundleRef;
    Boolean ok = CFURLGetFSRef(myBundleURL, &myBundleRef);
    CFRelease(myBundleURL);
    if (!ok) return err;
    
    return FSGetCatalogInfo(&myBundleRef, kFSCatInfoNone, NULL, NULL, theFSSpecPtr, NULL);
 }
 
/** MAC Carbon only : returns an FSSpec for the folder containing the specified file system object
*/
 static OSErr getParentFolder(FSSpecPtr theFSSpec, FSSpecPtr theParentSpec)
 {
    FSRef theSubjectRef;
    FSRef theParentRef;
    OSErr theErr;
    
    theErr=FSpMakeFSRef(theFSSpec,&theSubjectRef);
    if(theErr==noErr)
    {
        theErr=FSGetCatalogInfo(&theSubjectRef, kFSCatInfoNone, NULL, NULL, NULL, &theParentRef);
        if(theErr==noErr)
        {
            theErr=FSGetCatalogInfo(&theParentRef, kFSCatInfoNone, NULL, NULL, theParentSpec, NULL);
        }
    }
    return theErr;
 }
 
  /** MAC Carbon only : get an FSSpec for the folder containing the current application package.
    this is mostly useful for finding the Plug-ins directory if the application chooses to store plugins in a folder next
    to the application in the "old fashioned" way.
*/
static OSErr getApplicationParentFolderFSSpec(FSSpecPtr theFolder)
{
    FSSpec	theApp;
    OSErr	theErr;
    
    theErr=getApplicationFSSpec(&theApp);
    if(theErr==noErr)
    {
       theErr=getParentFolder(&theApp,theFolder);
    }
    return theErr;
}
 
 
/** MAC Carbon only : get an FSSpect for a sub folder of the parent folder of the current application.
    this is mostly useful for finding things like "plug-in" directories. Note that the proper place to put this kind of
    thing is now supposed to be the "Application Support" folder.
*/
static OSErr getApplicationPeerFolderFSSpec(const std::string& folderName,FSSpecPtr theFSSpecPtr)
 {
    FSSpec applicationFolder;
    OSErr errCode;
    
    errCode=getApplicationParentFolderFSSpec(&applicationFolder);
    if(errCode==noErr)
    {
        return getObjectContainedInFolder(folderName,&applicationFolder,theFSSpecPtr);
    }
    return errCode; // file not found error
 }
 
 /** MAC carbon only : returns the posix style path to a file specified in an FSSpec
    returns NULL if the file object pointed to by the FSSpec doesn't exist
 */
static char * pathFromFSSpec(FSSpecPtr theFSSpec)
 {
    FSRef	theObjectRef;
    char	pathTmp[1024];
    char	*path=NULL;
    CFURLRef	theObjectURL;
    OSErr	errCode;
    
    errCode=FSpMakeFSRef(theFSSpec,&theObjectRef);
    if(errCode==noErr)
    {
        theObjectURL=CFURLCreateFromFSRef(NULL,&theObjectRef);
        if(theObjectURL!=NULL)
        {
            CFStringRef thePath=CFURLCopyFileSystemPath(theObjectURL,kCFURLPOSIXPathStyle); // maybe should be HFS style on OS9 ?
            CFStringGetCString(thePath,pathTmp,1024,kCFStringEncodingASCII);
            path=strdup( pathTmp );
            CFRelease(theObjectURL);CFRelease(thePath);
        }
    }
    return path;
 }
 
 /** MAC Carbon only : checks for the existance of a file (replaces unix access() for OS8/9 platform)
*/
static bool checkFileExists(const char *filePath)
{
    CFURLRef	fileURL;
    FSRef	fileRef;
    bool	fileExists;
    //if(gestalt(gestaltFSAttr)==noErr)
   //{
    //    if(gestaltReply & gestaltFSUsesPOSIXPathsForConversion)
    //CFURLCreateWithFileSystemPath(ilePath, kCFURLPOSIXPathStyle, Boolean isDirectory);
    // FSPathMakeRef is only implemented on OSX, so can't use that.
    
    fileURL=CFURLCreateFromFileSystemRepresentation(NULL,(const UInt8 *)filePath,strlen(filePath),false); // hopefully this isolates us from dealing with what form the path is in - i think it should assume the native path format for the platfrom we're running on.
    fileExists=CFURLGetFSRef(fileURL,&fileRef);
    CFRelease(fileURL);
    return fileExists;
}
 
 /** MAC carbon only : returns the file path of a CFURLRef as a C string.
    you're responsible for disposing of the string when you're done
*/
static char *pathFromCFURL(CFURLRef theURL)
{
    UInt8 pathTmp[1024]; // temporary buffer for path
    char *path=NULL;
    if(theURL!=NULL)
    {
        CFURLGetFileSystemRepresentation(theURL,false,pathTmp,1024); // TODO this can fail somehow - not sure what to do if it does
        path=strdup( (char*)pathTmp );
    }
    return path; // phew
}

/** MAC carbon only : searches the given bundle for a named resource and returns the path to it if it exists, otherwise NULL
*/
static char *getPathOfResourceInBundle(const std::string& resourceName,CFBundleRef theBundle)
{
    CFStringRef fileNameString=CFStringCreateWithCString (NULL,resourceName.c_str(),kCFStringEncodingASCII);
    CFURLRef fileURL=CFBundleCopyResourceURL( theBundle, fileNameString, NULL, NULL); // look for a resource with the specified name 
    CFRelease(fileNameString);
    if(fileURL!=NULL)
    {
        notify( DEBUG_INFO ) << "found file:" << resourceName << " as a bundle resource" << endl;
        char *thePath=pathFromCFURL(fileURL);
        CFRelease(fileURL);
        return thePath;
    }
    return NULL;
}

/** MAC carbon only : returns the path to a resource in the Application bundle as a string
    returns NULL if the resource doesn't exist
*/
static char *getPathOfApplicationResource(const std::string& resourceName)
{
    CFBundleRef theSearchBundle=CFBundleGetMainBundle();
    char *thePath=getPathOfResourceInBundle(resourceName,theSearchBundle);
    return thePath;
}
 
// end mac utilites

void osgDB::initFilePath( void )
{
    char *ptr;
    if( (ptr = getenv( "OSG_FILE_PATH" ))  )
    {
        notify(DEBUG_INFO) << "osgDB::Init("<<ptr<<")"<<std::endl;
        setFilePath( ptr );
    }
    else if( (ptr = getenv( "OSGFILEPATH" ))  )
    {
        notify(DEBUG_INFO) << "osgDB::Init("<<ptr<<")"<<std::endl;
        setFilePath( ptr );
    }
    else
    {
        notify(DEBUG_INFO) << "osgDB::Init(NULL)"<<std::endl;
    }
    s_filePathInitialized = true;
}


void osgDB::setFilePath( const char *_path )
{
    char buff[FILEUTILS_MAX_PATH_LENGTH];

    notify(DEBUG_INFO) << "In osgDB::setFilePath("<<_path<<")"<<std::endl;

    buff[0] = 0;

    if( s_filePath != s_default_file_path )
    {
        strcpy( buff, s_filePath );
    }

    strcat( buff, PathDelimitor );
    strcat( buff, _path );

    s_filePath = strdup( buff );

    s_filePathInitialized = true;
}


const char* osgDB::getFilePath()
{
    return s_filePath;
}


char *osgDB::findFileInPath( const char *_file, const char * filePath )
{
    // the mac version now works, but it doesn't do much yet, since paths don't really mean much on the mac.
    #define CHECK_FILE_EXISTS(theFile) (checkFileExists(theFile) ? 0 : 1)

    notify(DEBUG_INFO) << "FindFileInPath() : trying " << _file << " ...\n";
    if( CHECK_FILE_EXISTS(_file) == 0 ) return (char *)_file;

    char pathbuff[FILEUTILS_MAX_PATH_LENGTH];
    char *tptr, *tmppath;
    char *path = 0L;

    notify(DEBUG_INFO) << "FindFileInPath() : trying " << _file << " ...\n";
    if( CHECK_FILE_EXISTS( _file ) == 0 ) 
    {
        return strdup(_file);
    }

    tptr    = strdup( filePath );
    tmppath = strtok(  tptr, PathDelimitor );

    do
    {
        sprintf( pathbuff, "%s/%s", tmppath, _file );
        notify(DEBUG_INFO) << "FindFileInPath() : trying " << pathbuff << " ...\n";
        if( CHECK_FILE_EXISTS(pathbuff) == 0 ) break;
    } while( (tmppath = strtok( 0, PathDelimitor )) );

    if( tmppath != (char *)0L )
        path = strdup( pathbuff );

    ::free(tptr);

    if (path) notify( DEBUG_INFO ) << "FindFileInPath() : returning " << path << endl;
	else notify( DEBUG_INFO ) << "FindFileInPath() : returning NULL" << endl;
    
    return path;
//#endif
}


char *osgDB::findFile( const char *file )
{
    if (!file) return NULL;
    
    if (!s_filePathInitialized) initFilePath();

    char* newFileName = findFileInPath( file, s_filePath );
    if (newFileName) return newFileName;
    

    // need to check here to see if file has a path on it.
    
    // now strip the file of an previous path if one exists.
    std::string simpleFileName = getSimpleFileName(file);
    newFileName = findFileInPath( simpleFileName.c_str(), s_filePath );
    if (newFileName) return newFileName;
    
    // on the mac we look inside the application's bundle for a resource with the correct name after we've tried the explicitly defined paths
    newFileName=getPathOfApplicationResource(simpleFileName);
    return newFileName;
}

/*

Order of precedence for

Under UNIX.

  ./
  OSG_LD_LIBRARY_PATH
  s_default_dso_path
  LD_LIBRARY*_PATH

Under Windows

  ./
  OSG_LD_LIBRARY_PATH
  s_default_dso_path
  PATH

Under MAC Carbon
    things are a little complex. The normal place to put plug-ins is supposed to be your apps "application support" folder.
    However, i've tried to support other places that you might want to put them.
    At the moment a plugin found next to the application will take precedence over all others - 
    this is so that during development you can just build plugins to your deffault build folder and they'll work.
    TODO : I should proabably conditionally compile this so that this behaviour deosn't happen when doing a deployment build.
    As a fallback the appliction will search it's Application bundle and then the OSG framework for plug-ins.
    This means that an application (or even just the OSG frameowrk itself) can be delivered as a single bundle "file" which
    will Just Work straight off, but that the plug ins contained there can be supersceded by newer versions explicitly provided by the user.
    TODO : Currently I don't searh OSG_LD_LIBRARY_PATH, though it shouldn't be difficult to implement.

  Application folder
  Applictaion Folder/Plug-ins
  User domain/Application Support/OSG/Plug-ins
  Local domain/Application Support/OSG/Plug-ins
  Network domain/Application Support/OSG/Plug-ins
  Current Application Bundle/Plug-ins
  OSG Framework

*/

char *osgDB::findDSO( const char *name )
{
    // mac version - wow the ifdefs have got pretty wild and crazy here - couldn't it be sorted out a little ?

    // On the mac we look in the mac default locations.
    // ie. in the users application support folder, the local application support folder, the network application support folder and a "Plug-ins" folder in the same directory as the application and finally in the application bundle itself
    // this should give us the correct behaviour for loading plug-ins, I think roughly matching the strategyusde by PlugIn Services.
    // BUT we're not doing any of the more fancy plug-in services stuff like dealing with different version of plaug-ins, we just use the first one we find.
    // note that I've tried to avoid using path strings as much as possible for compatibility with OS8/9
    // TODO - ideally we would make the Registry class a bit more abstract so that it could be implemented differently on different platforms.
    // the MacOS has a "PlugIn Services" API that duplicates virtually all this functionality and would be a lot simpler to deal with
    // on the mac.
    SInt16 foundVRefNum;
    SInt32 foundDirID;
    FSSpec foundFileSpec;
    FSSpec appPlugInsFolder;
    CFBundleRef theSearchBundle; // reference to a bundle that is being seacrched for resources
    
    notify( DEBUG_INFO ) << "searching for plug-in:" << name << endl;
    
    // to make development easier we'll look for plugins in the same folder as the application first. This means that the plug-ins and application can easily be built
    // in the same folder and the appl will use them first. This should probably only happen during development, and when building a deployment version this behaviour should be
    // removed, but I can't be bothered at the moment.
    if(getApplicationParentFolderFSSpec(&appPlugInsFolder)==noErr)
    {
        if(getObjectContainedInFolder(name,&appPlugInsFolder,&foundFileSpec)==noErr)
        {
            notify( DEBUG_INFO ) << "found plugin:" << name << " next to Application" << endl;
            return pathFromFSSpec(&foundFileSpec);
        }
    }
    
    // next look in Plug-ins driectory next to current application. This isn't the "standard" carbon way to do things, but very common so we should support it, i think.
    if(getApplicationPeerFolderFSSpec("Plug-ins",&appPlugInsFolder)==noErr)
    {
        if(getObjectContainedInFolder(name,&appPlugInsFolder,&foundFileSpec)==noErr)
        {
            notify( DEBUG_INFO ) << "found plugin:" << name << " in Plug-ins folder next to Application" << endl;
            return pathFromFSSpec(&foundFileSpec);
        }
    }

    // next check in the normal places to find plugins.
    if(FindFolder( kUserDomain, kApplicationSupportFolderType, kDontCreateFolder,&foundVRefNum, &foundDirID)==noErr)
    {
        if(getObjectContainedInFolder("Open Scene Graph",foundVRefNum, foundDirID, &appPlugInsFolder)==noErr)
        {
            if(getObjectContainedInFolder(name,&appPlugInsFolder,&foundFileSpec)==noErr)
            {
                notify( DEBUG_INFO ) << "found plugin:" << name << " in user domain appliaction support folder" << endl;
                return pathFromFSSpec(&foundFileSpec);
            }
        }
    }
    if(FindFolder( kLocalDomain, kApplicationSupportFolderType, kDontCreateFolder,&foundVRefNum, &foundDirID)==noErr)
    {
        if(getObjectContainedInFolder("Open Scene Graph",foundVRefNum, foundDirID, &appPlugInsFolder)==noErr)
        {
            if(getObjectContainedInFolder(name,&appPlugInsFolder,&foundFileSpec)==noErr)
            {
                notify( DEBUG_INFO ) << "found plugin:" << name << " in local domain appliaction support folder" << endl;
                return pathFromFSSpec(&foundFileSpec);
            }
        }
    }
    if(FindFolder( kNetworkDomain, kApplicationSupportFolderType, kDontCreateFolder,&foundVRefNum, &foundDirID)==noErr)
    {
        if(getObjectContainedInFolder("Open Scene Graph",foundVRefNum, foundDirID, &appPlugInsFolder)==noErr)
        {
            if(getObjectContainedInFolder(name,&appPlugInsFolder,&foundFileSpec)==noErr)
            {
                notify( DEBUG_INFO ) << "found plugin:" << name << " in network domain appliaction support folder" << endl;
                return pathFromFSSpec(&foundFileSpec);
            }
        }
    }
    
    // next we'll check if the Application has a resource with the correct name. This will allow plug-ins to be packaged inside the application itself.
    char *thePath=getPathOfApplicationResource(name);
    if(thePath!=NULL)
    {
        notify( DEBUG_INFO ) << "found plugin:" << name << " as an application resource" << endl;
        return thePath;
    }
    
    // as a last resort we look inside the "OSG" framework to see if we can find anything there.
    theSearchBundle = CFBundleGetBundleWithIdentifier(CFSTR("org.OpenSceneGraph.Core"));
    if (theSearchBundle != NULL)
    {
        thePath=getPathOfResourceInBundle(name,theSearchBundle);
        CFRelease(theSearchBundle);
    }
    if(thePath==NULL)
        notify( DEBUG_INFO ) << "couldn't find plugin:"<< name << endl;
    else
        notify( DEBUG_INFO ) << "found plugin:" << name << " as an OSG framework resource" << endl;
    return thePath;
}


/***************************************************************************/

// TODO
osgDB::DirectoryContents osgDB::getDirectoryContents(const std::string& dirName)
{
    osgDB::DirectoryContents contents;
    notify(WARN)<<"Warning : MAC implementation doesn't implement getDirectoryContents yet"<<endl;
    return contents;
}

std::string osg::findFileInPath(const std::string& filename, const FilePath& filePath)
{
    notify(WARN)<<"Warning : MAC implementation doesn't findFileInPath yet"<<endl;
    return std::string;
}

std::string osg::findLibraryInPath(const std::string& filename, const FilePath& filePath)
{
    notify(WARN)<<"Warning : MAC implementation doesn't findLibraryInPath yet"<<endl;
    return std::string;
}

bool osgDB::fileExists(const std::string& filename)
{
    CFURLRef	fileURL;
    FSRef	fileRef;
    bool	fileExists;

    fileURL=CFURLCreateFromFileSystemRepresentation(NULL,(const UInt8 *)filename.c_str(),filename.size(),false); // hopefully this isolates us from dealing with what form the path is in - i think it should assume the native path format for the platfrom we're running on.
    fileExists=CFURLGetFSRef(fileURL,&fileRef);
    CFRelease(fileURL);
    return fileExists;
}

