#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/ReaderWriter>

#include <string>

// Built-in types

ABSTRACT_OBJECT_REFLECTOR(void)

ATOMIC_VALUE_REFLECTOR(char)
ATOMIC_VALUE_REFLECTOR(signed char)
ATOMIC_VALUE_REFLECTOR(unsigned char)

ATOMIC_VALUE_REFLECTOR(int)
ATOMIC_VALUE_REFLECTOR(unsigned int)
ATOMIC_VALUE_REFLECTOR(long int)
ATOMIC_VALUE_REFLECTOR(long long int)
ATOMIC_VALUE_REFLECTOR(unsigned long int)
ATOMIC_VALUE_REFLECTOR(unsigned long long int)
ATOMIC_VALUE_REFLECTOR(short int)
ATOMIC_VALUE_REFLECTOR(unsigned short int)

ATOMIC_VALUE_REFLECTOR(bool)

ATOMIC_VALUE_REFLECTOR(float)

ATOMIC_VALUE_REFLECTOR(double)
ATOMIC_VALUE_REFLECTOR(long double)


// STL types

ATOMIC_VALUE_REFLECTOR(std::string)

