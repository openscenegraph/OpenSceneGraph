#use pkg-config to find various modues
find_package(PkgConfig QUIET)

IF(PKG_CONFIG_FOUND)
    PKG_CHECK_MODULES(CAIRO cairo)
    PKG_CHECK_MODULES(POPPLER poppler-glib)

    IF (POPPLER_FOUND)

        INCLUDE(CheckCXXSourceRuns)

        SET(CMAKE_REQUIRED_INCLUDES ${POPPLER_INCLUDE_DIRS})

        # Do step by step checking,
        CHECK_CXX_SOURCE_RUNS("
        #include <cstdlib>
        #include <poppler.h>
        int main()
        {
        #ifdef POPPLER_HAS_CAIRO
           return EXIT_SUCCESS;
        #else
           return EXIT_FAILURE
        #endif
        }
        " POPPLER_HAS_CAIRO)

        IF (NOT POPPLER_HAS_CAIRO)
            SET(POPPLER_FOUND FALSE)
        ENDIF()

    ENDIF()

#    IF (POPPLER_FOUND AND (NOT POPPLER_LIBRARIES OR NOT POPPLER_INCLUDE_DIRS) )
#        SET(POPPLER_FOUND FALSE)
#    ENDIF()

ENDIF()
