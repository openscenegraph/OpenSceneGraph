
IF   (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSGINTROSPECTION_LIBRARY)
ELSE (DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(DYNAMIC_OPENSCENEGRAPH)

SET(LIB_NAME osgIntrospection)
SET(HEADER_PATH ${OpenSceneGraph_SOURCE_DIR}/include/${LIB_NAME})
SET(LIB_PUBLIC_HEADERS
    ${HEADER_PATH}/Attributes
    ${HEADER_PATH}/Comparator
    ${HEADER_PATH}/ConstructorInfo
    ${HEADER_PATH}/Converter
    ${HEADER_PATH}/ConverterProxy
    ${HEADER_PATH}/CustomAttribute
    ${HEADER_PATH}/CustomAttributeProvider
    ${HEADER_PATH}/Exceptions
    ${HEADER_PATH}/Export
    ${HEADER_PATH}/ExtendedTypeInfo
    ${HEADER_PATH}/InstanceCreator
    ${HEADER_PATH}/MethodInfo
    ${HEADER_PATH}/ParameterInfo
    ${HEADER_PATH}/PropertyInfo
    ${HEADER_PATH}/PublicMemberAccessor
    ${HEADER_PATH}/ReaderWriter
    ${HEADER_PATH}/Reflection
    ${HEADER_PATH}/ReflectionMacros
    ${HEADER_PATH}/Reflector
    ${HEADER_PATH}/StaticMethodInfo
    ${HEADER_PATH}/Type
    ${HEADER_PATH}/TypedConstructorInfo
    ${HEADER_PATH}/TypedMethodInfo
    ${HEADER_PATH}/TypeNameAliasProxy
    ${HEADER_PATH}/type_traits
    ${HEADER_PATH}/Utility
    ${HEADER_PATH}/Value
    ${HEADER_PATH}/Version
    ${HEADER_PATH}/variant_cast
)

# FIXME: For OS X, need flag for Framework or dylib
ADD_LIBRARY(${LIB_NAME}
    ${OPENSCENEGRAPH_USER_DEFINED_DYNAMIC_OR_STATIC}
    ${LIB_PUBLIC_HEADERS}
    ConstructorInfo.cpp
    CustomAttributeProvider.cpp
    DefaultReflectors.cpp
    MethodInfo.cpp
    PropertyInfo.cpp
    Reflection.cpp
    Reflector.cpp
    Type.cpp
    Utility.cpp
    Value.cpp
    Version.cpp
)

LINK_INTERNAL(${LIB_NAME}
       OpenThreads
       osg
)

LINK_CORELIB_DEFAULT(${LIB_NAME})


INCLUDE(ModuleInstall OPTIONAL)

