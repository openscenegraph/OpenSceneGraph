#ifndef AC3D_EXCEPTION
#define AC3D_EXCEPTION 1

#include <string>

namespace ac3d{

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
