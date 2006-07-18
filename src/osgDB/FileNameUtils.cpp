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
#include <osgDB/FileNameUtils>

#if defined(__sgi)
    #include <ctype.h>
#elif defined(__GNUC__) || !defined(WIN32) || defined(__MWERKS__)
    #include <cctype>
    using std::tolower;
    using std::strlen;    
#endif

using namespace std;

std::string osgDB::getFilePath(const std::string& fileName)
{
    std::string::size_type slash1 = fileName.find_last_of('/');
    std::string::size_type slash2 = fileName.find_last_of('\\');
    if (slash1==std::string::npos) 
    {
        if (slash2==std::string::npos) return std::string();
        return std::string(fileName,0,slash2);
    }
    if (slash2==std::string::npos) return std::string(fileName,0,slash1);
    return std::string(fileName, 0, slash1>slash2 ?  slash1 : slash2);
}


std::string osgDB::getSimpleFileName(const std::string& fileName)
{
    std::string::size_type slash1 = fileName.find_last_of('/');
    std::string::size_type slash2 = fileName.find_last_of('\\');
    if (slash1==std::string::npos) 
    {
        if (slash2==std::string::npos) return fileName;
        return std::string(fileName.begin()+slash2+1,fileName.end());
    }
    if (slash2==std::string::npos) return std::string(fileName.begin()+slash1+1,fileName.end());
    return std::string(fileName.begin()+(slash1>slash2?slash1:slash2)+1,fileName.end());
}


std::string osgDB::getFileExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return std::string("");
    return std::string(fileName.begin()+dot+1,fileName.end());
}

std::string osgDB::convertFileNameToWindowsStyle(const std::string& fileName)
{
    std::string new_fileName(fileName);
    
    std::string::size_type slash = 0;
    while( (slash=new_fileName.find_first_of('/',slash)) != std::string::npos)
    {
        new_fileName[slash]='\\';
    }
    return new_fileName;
}

std::string osgDB::convertFileNameToUnixStyle(const std::string& fileName)
{
    std::string new_fileName(fileName);
    
    std::string::size_type slash = 0;
    while( (slash=new_fileName.find_first_of('\\',slash)) != std::string::npos)
    {
        new_fileName[slash]='/';
    }

    return new_fileName;
}

bool osgDB::isFileNameNativeStyle(const std::string& fileName)
{
#ifdef WIN32
    return fileName.find('/') == std::string::npos; // return true if no unix style slash exist
#else
    return fileName.find('\\') == std::string::npos; // return true if no windows style slash exist
#endif
}

std::string osgDB::convertFileNameToNativeStyle(const std::string& fileName)
{
#ifdef WIN32
    return convertFileNameToWindowsStyle(fileName);
#else
    return convertFileNameToUnixStyle(fileName);
#endif
}



std::string osgDB::getLowerCaseFileExtension(const std::string& filename)
{
    std::string ext = osgDB::getFileExtension(filename);
    for(std::string::iterator itr=ext.begin();
        itr!=ext.end();
        ++itr)
    {
        *itr = tolower(*itr);
    }
    return ext;
}


// strip one level of extension from the filename.
std::string osgDB::getNameLessExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return fileName;
    return std::string(fileName.begin(),fileName.begin()+dot);
}


std::string osgDB::getStrippedName(const std::string& fileName)
{
    std::string simpleName = getSimpleFileName(fileName);
    return getNameLessExtension( simpleName );
}


bool osgDB::equalCaseInsensitive(const std::string& lhs,const std::string& rhs)
{
    if (lhs.size()!=rhs.size()) return false;
    std::string::const_iterator litr = lhs.begin();
    std::string::const_iterator ritr = rhs.begin();
    while (litr!=lhs.end())
    {
        if (tolower(*litr)!=tolower(*ritr)) return false;
        ++litr;
        ++ritr;
    }
    return true;
}

bool osgDB::equalCaseInsensitive(const std::string& lhs,const char* rhs)
{
    if (rhs==NULL || lhs.size()!=strlen(rhs)) return false;
    std::string::const_iterator litr = lhs.begin();
    const char* cptr = rhs;
    while (litr!=lhs.end())
    {
        if (tolower(*litr)!=tolower(*cptr)) return false;
        ++litr;
        ++cptr;
    }
    return true;
}

bool osgDB::containsServerAddress(const std::string& filename)
{
    // need to check for http://
    if (filename.size()<7) return false;
    if (filename.compare(0,7,"http://")==0) return true;
    return false;
}

std::string osgDB::getServerAddress(const std::string& filename)
{
    if (filename.size()>=7 && filename.compare(0,7,"http://")==0)
    {
        std::string::size_type pos_slash = filename.find_first_of('/',7);
        if (pos_slash!=std::string::npos)
        {
            return filename.substr(7,pos_slash-7);
        }
        else
        {
            return filename.substr(7,std::string::npos);
        }
    }
    return "";
}

std::string osgDB::getServerFileName(const std::string& filename)
{
    if (filename.size()>=7 && filename.compare(0,7,"http://")==0)
    {
        std::string::size_type pos_slash = filename.find_first_of('/',7);
        if (pos_slash!=std::string::npos)
        {
            return filename.substr(pos_slash+1,std::string::npos);
        }
        else
        {
            return "";
        }
    
    }
    return filename;
}


// // here a little test I wrote to make sure a couple of the above methods are
// // working fine.
// void test()
// {
//     std::string test("/here/we/are.exe");
//     std::string test2("\\there\\you\\go.dll");
//     std::cout << "getFilePath("<<test<<") "<<osgDB::getFilePath(test)<<std::endl;
//     std::cout << "getFilePath("<<test2<<") "<<osgDB::getFilePath(test2)<<std::endl;
//     std::cout << "getSimpleFileName("<<test<<") "<<osgDB::getSimpleFileName(test)<<std::endl;
//     std::cout << "getSimpleFileName("<<test2<<") "<<osgDB::getSimpleFileName(test2)<<std::endl;
//     std::cout << "getStrippedName("<<test<<") "<<osgDB::getStrippedName(test)<<std::endl;
//     std::cout << "getStrippedName("<<test2<<") "<<osgDB::getStrippedName(test2)<<std::endl;
// }
