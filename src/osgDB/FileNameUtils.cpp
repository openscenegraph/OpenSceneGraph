#include <osgDB/FileNameUtils>

#if defined(__sgi)
    #include <ctype.h>
#elif !defined(WIN32)
    #include <cctype>
    using std::tolower;
    using std::strlen;    
#endif

// 
// // mac requires std::tolower, but IRIX MipsPro doesn't like it,
// // so use this preprocessor to allow mac and mipspro to work.
// #ifdef macintosh
// using std::tolower;
// using std::strlen;
// #endif

std::string osgDB::getFilePath(const std::string& fileName)
{
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos) return std::string("");
    return std::string(fileName.begin(),fileName.begin()+slash+1);
}


std::string osgDB::getSimpleFileName(const std::string& fileName)
{
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos) return fileName;
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
    std::string::size_type slash = fileName.find_last_of('/');
    std::string::size_type dot = fileName.find_last_of('.');

    // Ignore '.'s that aren't in the last component
    if (slash != std::string::npos && dot < slash)
      dot = std::string::npos;

    if (slash==std::string::npos)
    {
        if (dot==std::string::npos) return fileName;
        else return std::string(fileName.begin(),fileName.begin()+dot);
    }
    else
    {
        if (dot==std::string::npos) return std::string(fileName.begin()+slash+1,fileName.end());
        else return std::string(fileName.begin()+slash+1,fileName.begin()+dot);
    }
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
