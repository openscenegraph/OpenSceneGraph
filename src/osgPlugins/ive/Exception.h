#ifndef IVE_EXCEPTION
#define IVE_EXCEPTION 1

#include <string>
#include <osg/Referenced>

namespace ive{

#define THROW_EXCEPTION(str) { throwException(new Exception(str)); return; }
#define in_THROW_EXCEPTION(str) { in->throwException(new Exception(str)); return; }
#define out_THROW_EXCEPTION(str) { out->throwException(new Exception(str)); return; }

class Exception : public osg::Referenced
{
public:
	Exception(std::string error);
	~Exception();
	const std::string& getError() const { return _error; };
private:
	std::string _error;
};

}

#endif
