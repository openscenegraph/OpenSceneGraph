#include "osg/FileNameUtils"

std::string osg::getFilePath(const std::string& fileName)
{
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos) return std::string("");
    return std::string(fileName.begin(),fileName.begin()+slash+1);    
}

std::string osg::getSimpleFileName(const std::string& fileName)
{
    std::string::size_type slash = fileName.find_last_of('/');
    if (slash==std::string::npos) return fileName;
    return std::string(fileName.begin()+slash+1,fileName.end());    
}

std::string osg::getFileExtension(const std::string& fileName)
{
    std::string::size_type dot = fileName.find_last_of('.');
    if (dot==std::string::npos) return std::string("");
    return std::string(fileName.begin()+dot+1,fileName.end());    
}

std::string osg::getLowerCaseFileExtension(const std::string& filename)
{
    std::string ext = osg::getFileExtension(filename);
    for(std::string::iterator itr=ext.begin();
                              itr!=ext.end();
                              ++itr)
    {
        *itr = (char)tolower(*itr);
    }
    return ext;
}

std::string osg::getStrippedName(const std::string& fileName)
{
    std::string::size_type slash = fileName.find_last_of('/');
    std::string::size_type dot = fileName.find_last_of('.');
    if (slash==std::string::npos) {
        if (dot==std::string::npos) return fileName;
        else return std::string(fileName.begin(),fileName.begin()+dot);
    }
    else
    {
        if (dot==std::string::npos) return std::string(fileName.begin()+slash+1,fileName.end());
        else return std::string(fileName.begin()+slash+1,fileName.begin()+dot);
    }
}

