#ifndef IVE_EXCEPTION
#define IVE_EXCEPTION 1

#include <string>

namespace ive{

class Exception{
public:
	Exception(std::string error);
	~Exception();
	std::string getError(){return _error;};
private:
	std::string _error;
};

}

#endif
