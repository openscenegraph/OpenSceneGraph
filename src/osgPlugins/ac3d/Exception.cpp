#include "Exception.h"

using namespace ac3d;

Exception::Exception(std::string error){
    _error = error;
}

Exception::~Exception(){}
