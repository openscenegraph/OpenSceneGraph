#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/ReaderWriter>

#include <string>

// Built-in types

ABSTRACT_OBJECT_REFLECTOR(void)

STD_VALUE_REFLECTOR(char)
STD_VALUE_REFLECTOR(signed char)
STD_VALUE_REFLECTOR(unsigned char)

STD_VALUE_REFLECTOR(int)
STD_VALUE_REFLECTOR(unsigned int)
STD_VALUE_REFLECTOR(long int)
STD_VALUE_REFLECTOR(long long int)
STD_VALUE_REFLECTOR(unsigned long int)
STD_VALUE_REFLECTOR(short int)
STD_VALUE_REFLECTOR(unsigned short int)

STD_VALUE_REFLECTOR(bool)

STD_VALUE_REFLECTOR(float)

STD_VALUE_REFLECTOR(double)
STD_VALUE_REFLECTOR(long double)


// STL types

STD_VALUE_REFLECTOR(std::string)

