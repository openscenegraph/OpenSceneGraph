# - Figure out what compiler (and version) cmake is generating for
# Once done this will define:
#
# Variable:
#   OSG_COMPILER
# 

IF(CMAKE_COMPILER_IS_GNUCXX)

    EXEC_PROGRAM(
        ${CMAKE_CXX_COMPILER}  
        ARGS     ${CMAKE_CXX_COMPILER_ARG1} -dumpversion 
        OUTPUT_VARIABLE gcc_compiler_version
    )
    #MESSAGE("GCC Version: ${gcc_compiler_version}")

ELSE(CMAKE_COMPILER_IS_GNUCXX)
    IF(MSVC)
        IF(MSVC60)
            SET(OSG_COMPILER "vc60")
        ENDIF(MSVC60)
        IF(MSVC70)
            SET(OSG_COMPILER "vc70")
        ENDIF(MSVC70)
        IF(MSVC71)
            SET(OSG_COMPILER "vc71")
        ENDIF(MSVC71)
        IF(MSVC80)
            SET(OSG_COMPILER "vc80")
        ENDIF(MSVC80)
        IF(MSVC90)
            SET(OSG_COMPILER "vc90")
        ENDIF(MSVC90)

        IF(MSVC80)
            MESSAGE(STATUS "Checking if compiler has service pack 1 installed...")
            FILE(WRITE "${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx" "int main() {return 0;}\n")

            TRY_COMPILE(_TRY_RESULT
              ${CMAKE_BINARY_DIR}
              ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/src.cxx
              CMAKE_FLAGS -D CMAKE_VERBOSE_MAKEFILE=ON
              OUTPUT_VARIABLE OUTPUT
            )

            IF(_TRY_RESULT)
                # parse for exact compiler version
                STRING(REGEX MATCH "Compiler Version [0-9]+.[0-9]+.[0-9]+.[0-9]+" vc_compiler_version "${OUTPUT}")
                IF(vc_compiler_version)
                    #MESSAGE("${vc_compiler_version}")
                    STRING(REGEX MATCHALL "[0-9]+" CL_VERSION_LIST "${vc_compiler_version}")
                    LIST(GET CL_VERSION_LIST 0 CL_MAJOR_VERSION)
                    LIST(GET CL_VERSION_LIST 1 CL_MINOR_VERSION)
                    LIST(GET CL_VERSION_LIST 2 CL_PATCH_VERSION)
                    LIST(GET CL_VERSION_LIST 3 CL_EXTRA_VERSION)
                ENDIF(vc_compiler_version)

                # Standard vc80 is 14.00.50727.42, sp1 14.00.50727.762, sp2?
                # Standard vc90 is 9.0.30729.1, sp1 ?
                IF(CL_EXTRA_VERSION EQUAL 762)
                    SET(OSG_COMPILER "vc80sp1")
                ELSE(CL_EXTRA_VERSION EQUAL 762)
                    SET(OSG_COMPILER "vc80")
                ENDIF(CL_EXTRA_VERSION EQUAL 762)

                # parse for exact visual studio version
                #IF(MSVC_IDE)
                # string(REGEX MATCH "Visual Studio Version [0-9]+.[0-9]+.[0-9]+.[0-9]+" vs_version "${OUTPUT}")
                # IF(vs_version)
                    # MESSAGE("${vs_version}")
                    # string(REGEX MATCHALL "[0-9]+" VS_VERSION_LIST "${vs_version}")
                    # list(GET VS_VERSION_LIST 0 VS_MAJOR_VERSION)
                    # list(GET VS_VERSION_LIST 1 VS_MINOR_VERSION)
                    # list(GET VS_VERSION_LIST 2 VS_PATCH_VERSION)
                    # list(GET VS_VERSION_LIST 3 VS_EXTRA_VERSION)
                # ENDIF(vs_version)
                #ENDIF(MSVC_IDE)
            ENDIF(_TRY_RESULT)
        ENDIF(MSVC80)
    ENDIF(MSVC)
ENDIF(CMAKE_COMPILER_IS_GNUCXX)
