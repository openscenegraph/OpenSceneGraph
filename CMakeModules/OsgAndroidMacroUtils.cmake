MACRO(SETUP_ANDROID_LIBRARY LIB_NAME)

    #foreach(arg ${TARGET_LIBRARIES})
    #    set(MODULE_LIBS "${MODULE_LIBS} -l${arg}")
    #endforeach(arg ${TARGET_LIBRARIES})
    
    foreach(arg ${TARGET_SRC})
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" n_f ${arg})
        IF ("${arg}" MATCHES ".*\\.c$" OR "${arg}" MATCHES ".*\\.cpp$")
            #We only include source files, not header files, this removes anoying warnings
            set(MODULE_SOURCES "${MODULE_SOURCES} ${n_f}")
    ENDIF()
    endforeach(arg ${TARGET_SRC})
    
       #SET(MODULE_INCLUDES "${CMAKE_SOURCE_DIR}/include include") 
    GET_DIRECTORY_PROPERTY(loc_includes INCLUDE_DIRECTORIES)
    foreach(arg ${loc_includes})
        IF(NOT "${arg}" MATCHES "/usr/include" AND NOT "${arg}" MATCHES "/usr/local/include")
            set(MODULE_INCLUDES "${MODULE_INCLUDES} ${arg}")
        ENDIF()
    endforeach(arg ${loc_includes})

    GET_DIRECTORY_PROPERTY(loc_definitions COMPILE_DEFINITIONS) 
    foreach(arg ${loc_definitions})
        set(DEFINITIONS "${DEFINITIONS} -D${arg}")
    endforeach(arg ${loc_definitions})

    message(STATUS "##############Creating Android Makefile#################")
    message(STATUS "name: ${LIB_NAME}")
        
    set(MODULE_NAME        ${LIB_NAME})
    set(MODULE_DIR         ${CMAKE_CURRENT_SOURCE_DIR})
    set(MODULE_FLAGS_C       ${DEFINITIONS})
    set(MODULE_FLAGS_CPP   ${DEFINITIONS})
    #TODO: determine if GLES2 or GLES
    IF(OSG_GLES1_AVAILABLE)
        SET(OPENGLES_LIBRARY -lGLESv1_CM)
    ELSEIF(OSG_GLES2_AVAILABLE)
        SET(OPENGLES_LIBRARY -lGLESv2)
    ENDIF()
    #${MODULE_LIBS}
    set(MODULE_LIBS_FLAGS "${OPENGLES_LIBRARY} -ldl")   
    if(NOT CPP_EXTENSION)
        set(CPP_EXTENSION "cpp")
    endif()
    IF(NOT MODULE_USER_STATIC_OR_DYNAMIC)
        MESSAGE(FATAL_ERROR "Not defined MODULE_USER_STATIC_OR_DYNAMIC")
    ENDIF()
    IF("MODULE_USER_STATIC_OR_DYNAMIC" MATCHES "STATIC")
        SET(MODULE_BUILD_TYPE "\$\(BUILD_STATIC_LIBRARY\)")
    SET(MODULE_LIBS_SHARED " ")
    SET(MODULE_LIBS_STATIC ${TARGET_LIBRARIES})
    ELSE()
        SET(MODULE_BUILD_TYPE "\$\(BUILD_SHARED_LIBRARY\)")
    SET(MODULE_LIBS_SHARED ${TARGET_LIBRARIES})
    SET(MODULE_LIBS_STATIC " ")
    ENDIF()
    set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} ${LIB_NAME}")
    set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${CMAKE_CURRENT_BINARY_DIR}/Android.mk \n")
    
    configure_file("${OSG_ANDROID_TEMPLATES}/Android.mk.modules.in" "${CMAKE_CURRENT_BINARY_DIR}/Android.mk")
    
ENDMACRO()

MACRO(ANDROID_3RD_PARTY)
    ################################################
    #JPEG
    ################################################
    FIND_PATH(JPEG_INCLUDE_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/libjpeg
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libjpeg")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${JPEG_INCLUDE_DIR}/Android.mk \n")
    if(JPEG_INCLUDE_DIR)
        message(STATUS "Jpeg found ${JPEG_INCLUDE_DIR}" )
        set(JPEG_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/libjpeg/ DESTINATION ./ )
    else(JPEG_INCLUDE_DIR)
        message(STATUS "Jpeg missing" )
    endif()
    ################################################
    #PNG
    ################################################
    FIND_PATH(PNG_INCLUDE_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/libpng
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libpng")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${PNG_INCLUDE_DIR}/Android.mk \n")
    if(PNG_INCLUDE_DIR)
        message(STATUS "PNG found ${PNG_INCLUDE_DIR}" )
        set(PNG_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/libpng/ DESTINATION ./ )
    else(PNG_INCLUDE_DIR)
        message(STATUS "PNG missing" )
    endif()
    ################################################
    #GIF
    ################################################
    FIND_PATH(GIFLIB_INCLUDE_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/giflib
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libgif")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${GIFLIB_INCLUDE_DIR}/Android.mk \n")
    if(GIFLIB_INCLUDE_DIR)
        message(STATUS "GIF found ${GIFLIB_INCLUDE_DIR}" )
        set(GIFLIB_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/giflib/ DESTINATION ./ )
    else(GIFLIB_INCLUDE_DIR)
        message(STATUS "GIF missing" )
    endif()
    ################################################
    #TIF
    ################################################
    FIND_PATH(TIFF_INCLUDE_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/libtiff
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libtiff")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${TIFF_INCLUDE_DIR}/Android.mk \n")
    if(TIFF_INCLUDE_DIR)
        message(STATUS "TIF found ${TIFF_INCLUDE_DIR}" )
        set(TIFF_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/libtiff/ DESTINATION ./ )
    else(TIFF_INCLUDE_DIR)
        message(STATUS "TIF missing" )
    endif()
    ################################################
    #ZLIB
    ################################################
    #FIND_PATH(ZLIB_INCLUDE_DIR Android.mk
    #    ${CMAKE_SOURCE_DIR}/3rdparty/zlib
    #)
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} zlib")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${ZLIB_INCLUDE_DIR}/Android.mk \n")
    #if(ZLIB_INCLUDE_DIR)
    #    message(STATUS "ZLIB found ${ZLIB_INCLUDE_DIR}" )
    #    set(ZLIB_FOUND "Yes")
    #    install(DIRECTORY 3rdparty/build/libjpeg/ DESTINATION ./ )
    #else(ZLIB_INCLUDE_DIR)
    #    message(STATUS "ZLIB missing" )
    #endif()
    ################################################
    #CURL
    ################################################
    FIND_PATH(CURL_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/curl
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libcurl")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${CURL_DIR}/Android.mk \n")
    set(CURL_INCLUDE_DIR ${CURL_DIR}/include) 
    set(CURL_INCLUDE_DIRS ${CURL_DIR}/include) #Both are defined in FindCurl
    if(CURL_DIR)
        message(STATUS "Curl found ${CURL_DIR}" )
        set(CURL_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/curl/ DESTINATION ./ )
    else(CURL_DIR)
        message(STATUS "Curl missing" )
    endif()
    ################################################
    #FREETYPE
    ################################################
    FIND_PATH(FREETYPE_DIR Android.mk
        ${CMAKE_SOURCE_DIR}/3rdparty/freetype
    )
    #set(ENV{AND_OSG_LIB_NAMES} "$ENV{AND_OSG_LIB_NAMES} libft2")
    #set(ENV{AND_OSG_LIB_PATHS} "$ENV{AND_OSG_LIB_PATHS}include ${FREETYPE_DIR}/Android.mk \n")
    set(FREETYPE_INCLUDE_DIRS "${FREETYPE_DIR}/include ${FREETYPE_DIR}/include/freetype/config")
    if(FREETYPE_DIR)
        message(STATUS "FREETYPE found ${FREETYPE_DIR}" )
        set(FREETYPE_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/freetype/ DESTINATION ./ )
    else(FREETYPE_DIR)
        message(STATUS "FREETYPE missing" )
    endif()
    ################################################
    #GDAL
    ################################################
    FIND_PATH(GDAL_DIR gdal.h
        ${CMAKE_SOURCE_DIR}/3rdparty/gdal/include
    )
    set(GDAL_INCLUDE_DIR "${GDAL_DIR}")
    if(GDAL_DIR)
        message(STATUS "GDAL found ${GDAL_DIR}" )
        set(GDAL_FOUND "Yes")
        install(DIRECTORY 3rdparty/build/gdal/ DESTINATION ./ )
    else(GDAL_DIR)
        message(STATUS "GDAL missing" )
    endif()
ENDMACRO()
