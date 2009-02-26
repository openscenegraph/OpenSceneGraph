################################################################################################
# this Macro find a generic dependency, handling debug suffix
# all the paramenter are required, in case of lists, use "" in calling
################################################################################################

MACRO(FIND_DEPENDENCY DEPNAME INCLUDEFILE LIBRARY_NAMES SEARCHPATHLIST DEBUGSUFFIX)

    MESSAGE(STATUS "searching ${DEPNAME} -->${INCLUDEFILE}<-->${LIBRARY_NAMES}<-->${SEARCHPATHLIST}<--")

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
    
    FIND_LIBRARY("${DEPNAME}_LIBRARY" 
        NAMES ${LIBRARY_NAMES}
      PATHS ${MY_PATH_LIB}
      NO_DEFAULT_PATH
    )
    SET(LIBRARY_NAMES_DEBUG "")
    FOREACH(LIBNAME ${LIBRARY_NAMES})
        LIST(APPEND LIBRARY_NAMES_DEBUG "${LIBNAME}${DEBUGSUFFIX}")
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
ENDMACRO(FIND_DEPENDENCY DEPNAME INCLUDEFILE LIBRARY_NAMES SEARCHPATHLIST DEBUGSUFFIX)


################################################################################################
# this Macro is tailored to Mike dependencies
################################################################################################

MACRO(SEARCH_3RDPARTY OSG_3RDPARTY_BIN)
        FIND_DEPENDENCY(TIFF tiff.h libtiff ${OSG_3RDPARTY_BIN} "D")
        FIND_DEPENDENCY(FREETYPE ft2build.h "freetype;freetype234;freetype234MT;freetype235;freetype237" ${OSG_3RDPARTY_BIN} "_D")
        IF(FREETYPE_FOUND)
            #forcing subsequent FindFreeType stuff to not search for other variables.... kind of a hack 
            SET(FREETYPE_INCLUDE_DIR_ft2build ${FREETYPE_INCLUDE_DIR} CACHE PATH "" FORCE)
            SET(FREETYPE_INCLUDE_DIR_freetype2 ${FREETYPE_INCLUDE_DIR} CACHE PATH "" FORCE)
            MARK_AS_ADVANCED(FREETYPE_INCLUDE_DIR_ft2build FREETYPE_INCLUDE_DIR_freetype2)
            SET(FREETYPE_INCLUDE_DIRS "${FREETYPE_INCLUDE_DIR_ft2build};${FREETYPE_INCLUDE_DIR_freetype2}")
        ENDIF(FREETYPE_FOUND)
        FIND_DEPENDENCY(CURL curl/curl.h "libcurl;curllib" ${OSG_3RDPARTY_BIN} "D")
        FIND_DEPENDENCY(JPEG jpeglib.h libjpeg ${OSG_3RDPARTY_BIN} "D")
        #FIND_DEPENDENCY(GDAL gdal.h "gdal;gdal_i" ${OSG_3RDPARTY_BIN})
        FIND_DEPENDENCY(GLUT GL/glut.h glut32 ${OSG_3RDPARTY_BIN} "D")
        IF(GLUT_FOUND)
            #forcing subsequent FindGlut stuff to not search for other variables.... kind of a hack 
            SET(GLUT_glut_LIBRARY ${GLUT_LIBRARY} CACHE FILEPATH "")
            MARK_AS_ADVANCED(GLUT_glut_LIBRARY)
        ENDIF(GLUT_FOUND)
        FIND_DEPENDENCY(GIFLIB gif_lib.h "ungif;libungif" ${OSG_3RDPARTY_BIN} "D")
        FIND_DEPENDENCY(ZLIB zlib.h "z;zlib;zlib1" ${OSG_3RDPARTY_BIN} "D")
        IF(ZLIB_FOUND)
            FIND_DEPENDENCY(PNG png.h "libpng;libpng13" ${OSG_3RDPARTY_BIN} "D")
            IF(PNG_FOUND)
                #forcing subsequent FindPNG stuff to not search for other variables.... kind of a hack 
                SET(PNG_PNG_INCLUDE_DIR ${PNG_INCLUDE_DIR} CACHE FILEPATH "")
                MARK_AS_ADVANCED(PNG_PNG_INCLUDE_DIR)
            ENDIF(PNG_FOUND)
        ENDIF(ZLIB_FOUND)        
#luigi#INCLUDE(FindOSGDepends.cmake)
ENDMACRO(SEARCH_3RDPARTY OSG_3RDPARTY_BIN)




################################################################################################
# this is code for handling optional 3DPARTY usage
################################################################################################

OPTION(USE_3DPARTY_BIN "Set to ON to use Mike prebuilt dependencies situated side of OpenSceneGraph source.  Use OFF for avoiding." ON)
IF(USE_3DPARTY_BIN)
    GET_FILENAME_COMPONENT(PARENT_DIR ${PROJECT_SOURCE_DIR} PATH)
    SET(ACTUAL_3DPARTY_DIR "${PARENT_DIR}/3rdparty" CACHE PATH "Location of 3rdparty dependencies")
    IF(EXISTS ${ACTUAL_3DPARTY_DIR})
        SEARCH_3RDPARTY(${ACTUAL_3DPARTY_DIR})
    ENDIF(EXISTS ${ACTUAL_3DPARTY_DIR})
ENDIF(USE_3DPARTY_BIN)
