#ifndef IVE_EXCEPTION
#define IVE_EXCEPTION 1

#include "Export.h"
#include <string>

namespace ive{

class IVE_EXPORT Exception{
public:
	Exception(std::string error);
	~Exception();
	std::string getError(){return _error;};
private:
	std::string _error;
};

}

#endif
