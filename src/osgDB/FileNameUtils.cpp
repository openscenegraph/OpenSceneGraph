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
    // try unix directory slash first.
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos) 
    {
        // then try windows directory slash.
        slash = fileName.find_last_of('\\');
        if (slash==std::string::npos) return std::string();
    }
    return std::string(fileName,0,slash);
}


std::string osgDB::getSimpleFileName(const std::string& fileName)
{
    // try unix directory slash first.
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos)
    {
        // then try windows directory slash.
        slash = fileName.find_last_of('\\');
        if (slash==std::string::npos) return fileName;
    }
    return std::string(fileName.begin()+slash+1,fileName.end());
}


std::string osgDB::getFileExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return std::string("");
    return std::string(fileName.begin()+dot+1,fileName.end());
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


std::string osgDB::getStrippedName(const std::string& fileName)
{
    std::string simpleName = getSimpleFileName(fileName);
    std::string::size_type dot = simpleName.find_last_of('.');
    if (dot==std::string::npos) return simpleName;
    return std::string(simpleName.begin(),simpleName.begin()+dot);
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
