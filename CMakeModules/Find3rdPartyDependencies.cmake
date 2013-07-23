################################################################################################
# this Macro find a generic dependency, handling debug suffix
# all the paramenter are required, in case of lists, use "" in calling
################################################################################################

MACRO(FIND_DEPENDENCY DEPNAME INCLUDEFILE LIBRARY_NAMES_BASE SEARCHPATHLIST DEBUGSUFFIX EXSUFFIX)

    MESSAGE(STATUS "searching ${DEPNAME} -->${INCLUDEFILE}<-->${LIBRARY_NAMES_BASE}<-->${SEARCHPATHLIST}<--")

    SET(MY_PATH_INCLUDE )
    SET(MY_PATH_LIB )
    
    FOREACH( MYPATH ${SEARCHPATHLIST} )
        SET(MY_PATH_INCLUDE ${MY_PATH_INCLUDE} ${MYPATH}/include)
        SET(MY_PATH_LIB ${MY_PATH_LIB} ${MYPATH}/lib)
    ENDFOREACH( MYPATH ${SEARCHPATHLIST} )
    
    FIND_PATH("${DEPNAME}_INCLUDE_DIR" ${INCLUDEFILE}
      ${MY_PATH_INCLUDE}
      NO_DEFAULT_PATH
    )
    MARK_AS_ADVANCED("${DEPNAME}_INCLUDE_DIR")
    #MESSAGE( " ${DEPNAME}_INCLUDE_DIR --> ${${DEPNAME}_INCLUDE_DIR}<--")
    
    SET(LIBRARY_NAMES "")
    FOREACH(LIBNAME ${LIBRARY_NAMES_BASE})
        LIST(APPEND LIBRARY_NAMES "${LIBNAME}${EXSUFFIX}")
    ENDFOREACH(LIBNAME)
    FIND_LIBRARY("${DEPNAME}_LIBRARY"
        NAMES ${LIBRARY_NAMES}
      PATHS ${MY_PATH_LIB}
      NO_DEFAULT_PATH
    )
    SET(LIBRARY_NAMES_DEBUG "")
    FOREACH(LIBNAME ${LIBRARY_NAMES_BASE})
        LIST(APPEND LIBRARY_NAMES_DEBUG "${LIBNAME}${DEBUGSUFFIX}${EXSUFFIX}")
    ENDFOREACH(LIBNAME)
    FIND_LIBRARY("${DEPNAME}_LIBRARY_DEBUG" 
        NAMES ${LIBRARY_NAMES_DEBUG}
      PATHS ${MY_PATH_LIB}
      NO_DEFAULT_PATH
    )
    MARK_AS_ADVANCED("${DEPNAME}_LIBRARY")
    #MESSAGE( " ${DEPNAME}_LIBRARY --> ${${DEPNAME}_LIBRARY}<--")
    SET( ${DEPNAME}_FOUND "NO" )
    IF(${DEPNAME}_INCLUDE_DIR AND ${DEPNAME}_LIBRARY)
      SET( ${DEPNAME}_FOUND "YES" )
      IF(NOT ${DEPNAME}_LIBRARY_DEBUG)
          MESSAGE("-- Warning Debug ${DEPNAME} not found, using: ${${DEPNAME}_LIBRARY}")
          SET(${DEPNAME}_LIBRARY_DEBUG "${${DEPNAME}_LIBRARY}")
      ENDIF(NOT ${DEPNAME}_LIBRARY_DEBUG)
    ENDIF(${DEPNAME}_INCLUDE_DIR AND ${DEPNAME}_LIBRARY)
ENDMACRO(FIND_DEPENDENCY DEPNAME INCLUDEFILE LIBRARY_NAMES_BASE SEARCHPATHLIST DEBUGSUFFIX)


################################################################################################
# this Macro is tailored to Mike and Torbens dependencies
################################################################################################

MACRO(SEARCH_3RDPARTY OSG_3RDPARTY_BIN)
        FIND_DEPENDENCY(TIFF tiff.h libtiff ${OSG_3RDPARTY_BIN} "D" "_i")
        FIND_DEPENDENCY(FREETYPE ft2build.h "freetype;freetype2311MT;freetype234;freetype234MT;freetype235;freetype237;freetype238;freetype244;freetype250;" ${OSG_3RDPARTY_BIN} "d" "")
        IF(FREETYPE_FOUND)
            #forcing subsequent FindFreeType stuff to not search for other variables.... kind of a hack 
            SET(FREETYPE_INCLUDE_DIR_ft2build ${FREETYPE_INCLUDE_DIR} CACHE PATH "" FORCE)
            SET(FREETYPE_INCLUDE_DIR_freetype2 ${FREETYPE_INCLUDE_DIR} CACHE PATH "" FORCE)
            MARK_AS_ADVANCED(FREETYPE_INCLUDE_DIR_ft2build FREETYPE_INCLUDE_DIR_freetype2)
            SET(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build};${FREETYPE_INCLUDE_DIR_freetype2}")
        ENDIF(FREETYPE_FOUND)
        FIND_DEPENDENCY(CURL curl/curl.h "libcurl;curllib" ${OSG_3RDPARTY_BIN} "D" "")
        FIND_DEPENDENCY(JPEG jpeglib.h "libjpeg;jpeg" ${OSG_3RDPARTY_BIN} "D" "")
        FIND_DEPENDENCY(GDAL gdal.h "gdal;gdal16" ${OSG_3RDPARTY_BIN} "d" "_i")
        FIND_DEPENDENCY(GLUT GL/glut.h glut32 ${OSG_3RDPARTY_BIN} "D" "")
        IF(GLUT_FOUND)
            #forcing subsequent FindGlut stuff to not search for other variables.... kind of a hack 
            SET(GLUT_glut_LIBRARY ${GLUT_LIBRARY} CACHE FILEPATH "")
            MARK_AS_ADVANCED(GLUT_glut_LIBRARY)
        ENDIF(GLUT_FOUND)
        FIND_DEPENDENCY(GIFLIB gif_lib.h "ungif;libungif;giflib" ${OSG_3RDPARTY_BIN} "D" "")
        FIND_DEPENDENCY(ZLIB zlib.h "z;zlib;zlib1" ${OSG_3RDPARTY_BIN} "D" "")
        IF(ZLIB_FOUND)
            FIND_DEPENDENCY(PNG png.h "libpng;libpng13;libpng15;libpng16" ${OSG_3RDPARTY_BIN} "D" "")
            IF(PNG_FOUND)
                #forcing subsequent FindPNG stuff to not search for other variables.... kind of a hack 
                SET(PNG_PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE FILEPATH "")
                MARK_AS_ADVANCED(PNG_PNG_INCLUDE_DIR)
            ENDIF(PNG_FOUND)
        ENDIF(ZLIB_FOUND)
        FIND_DEPENDENCY(LIBXML2 libxml2 "libxml2" ${OSG_3RDPARTY_BIN} "D" "")   
        IF(LIBXML2_FOUND)
            # The CMAKE find libxml module uses LIBXML2_LIBRARIES -> fill it.... kind of a hack 
            SET(LIBXML2_LIBRARIES ${LIBXML2_LIBRARY} CACHE FILEPATH "" FORCE)
            SET(LIBXML2_XMLLINT_EXECUTABLE ${OSG_3RDPARTY_BIN}/bin/xmllint.exe CACHE FILEPATH "Path to xmllint executable" FORCE)
        ENDIF(LIBXML2_FOUND)
        #FIND_DEPENDENCY(DEPNAME INCLUDEFILE LIBRARY_NAMES_BASE SEARCHPATHLIST DEBUGSUFFIX EXSUFFIX)
        FIND_Package(NVTT)
#luigi#INCLUDE(FindOSGDepends.cmake)
ENDMACRO(SEARCH_3RDPARTY OSG_3RDPARTY_BIN)




################################################################################################
# this is code for handling optional 3RDPARTY usage
################################################################################################

OPTION(USE_3RDPARTY_BIN "Set to ON to use Mike prebuilt dependencies situated side of OpenSceneGraph source.  Use OFF for avoiding." ON)
IF(USE_3RDPARTY_BIN)

    # Check Architecture
    IF( CMAKE_SIZEOF_VOID_P EQUAL 4 )
        MESSAGE( STATUS "32 bit architecture detected" )
        SET(DESTINATION_ARCH "x86")
    ENDIF()
    IF( CMAKE_SIZEOF_VOID_P EQUAL 8 )
        MESSAGE( STATUS "64 bit architecture detected" )
        SET(DESTINATION_ARCH "x64")
    ENDIF()

    GET_FILENAME_COMPONENT(PARENT_DIR ${PROJECT_SOURCE_DIR} PATH)
    SET(TEST_3RDPARTY_DIR "${PARENT_DIR}/3rdparty")
    IF(NOT EXISTS ${TEST_3RDPARTY_DIR})
        SET(3RDPARTY_DIR_BY_ENV $ENV{OSG_3RDPARTY_DIR})
        IF(3RDPARTY_DIR_BY_ENV)
            MESSAGE( STATUS "3rdParty-Package ENV variable found:${3RDPARTY_DIR_BY_ENV}/${DESTINATION_ARCH}" )
            SET(TEST_3RDPARTY_DIR "${3RDPARTY_DIR_BY_ENV}/${DESTINATION_ARCH}")
        ELSEIF(MSVC71)
            SET(TEST_3RDPARTY_DIR "${PARENT_DIR}/3rdParty_win32binaries_vs71")
        ELSEIF(MSVC80)
            SET(TEST_3RDPARTY_DIR "${PARENT_DIR}/3rdParty_win32binaries_vs80sp1")
        ELSEIF(MSVC90)
            SET(TEST_3RDPARTY_DIR "${PARENT_DIR}/3rdParty_win32binaries_vs90sp1")
        ENDIF()
    ENDIF(NOT EXISTS ${TEST_3RDPARTY_DIR})
    
    SET(ACTUAL_3RDPARTY_DIR "${TEST_3RDPARTY_DIR}" CACHE PATH "Location of 3rdparty dependencies")
    SET(ACTUAL_3DPARTY_DIR "${ACTUAL_3RDPARTY_DIR}")  # kept for backcompatibility
    IF(EXISTS ${ACTUAL_3RDPARTY_DIR})
        SET (3rdPartyRoot ${ACTUAL_3RDPARTY_DIR})
        SEARCH_3RDPARTY(${ACTUAL_3RDPARTY_DIR})
    ENDIF(EXISTS ${ACTUAL_3RDPARTY_DIR})
ENDIF(USE_3RDPARTY_BIN)
