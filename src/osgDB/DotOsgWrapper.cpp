#include <osgDB/DotOsgWrapper>

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
    

    // copy the names in the space deliminated associates input into
    // a vector of seperated names.    
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
