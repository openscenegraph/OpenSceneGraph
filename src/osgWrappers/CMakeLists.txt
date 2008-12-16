PROJECT(OSG_WRAPPERS)

IF   (NOT DYNAMIC_OPENSCENEGRAPH)
    ADD_DEFINITIONS(-DOSG_LIBRARY_STATIC)
ENDIF(NOT DYNAMIC_OPENSCENEGRAPH)

SET(LIBRARY_OUTPUT_PATH "${LIBRARY_OUTPUT_PATH}/${OSG_PLUGINS}")

SET(CMAKE_SHARED_MODULE_PREFIX ${OSG_PLUGIN_PREFIX})

IF(MSVC80)
  IF(NOT OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
  ENDIF(NOT OSG_MSVC_GENERATE_PLUGINS_AND_WRAPPERS_MANIFESTS)
ENDIF(MSVC80)

MACRO(ADD_WRAPPER_LIB SUBDIR EXPORTDEF)
    SET(TARGET_NAME "osgwrapper_${SUBDIR}")
#    MESSAGE(STATUS "--inizio-->${TARGET_NAME}<->${LINK}<-")

    FILE(GLOB SRC_FILES ${OSGWRAPPER_DIR}/${SUBDIR}/*.cpp)
    
    #-- extract link files, defines, exclude files form additional arguments 

    SET(LISTNAME "TEMP")
    SET(DEFSTR "")

    FOREACH(ARG ${ARGN})                    # parse remaining args 
        #MESSAGE(STATUS "+ [${ARG}]")
        #MESSAGE("ARG-->${ARG}")
        # if we find our keywords set the active list to given keyname 
        STRING(COMPARE EQUAL "${ARG}" "LINK" IS_LINK)
        STRING(COMPARE EQUAL "${ARG}" "DEFINE" IS_DEFINE)
        STRING(COMPARE EQUAL "${ARG}" "EXCLUDE" IS_EXCLUDE)
        
        #MESSAGE(STATUS "STRSTUFF L ${IS_LINK} D ${IS_DEFINE} E ${IS_EXCLUDE}")
        
        SET(EXPRESSION ${IS_LINK} OR ${IS_DEFINE} OR ${IS_EXCLUDE})
        IF(${EXPRESSION})
            SET(${LISTNAME} ${CURRLIST})
            # MESSAGE(STATUS "STORED LIST [${LISTNAME}] = (${CURRLIST})")
            SET(LISTNAME ${ARG})
            REMOVE(CURRLIST ${CURRLIST} )
            
        ELSE(${EXPRESSION})
            SET(CURRLIST ${CURRLIST} ${ARG})    
            
        ENDIF(${EXPRESSION})
    ENDFOREACH(ARG)
    SET(${LISTNAME} ${CURRLIST})
    #MESSAGE(STATUS "STORED LIST [${LISTNAME}] = (${CURRLIST})")
    REMOVE(CURRLIST ${CURRLIST} )
    
    #MESSAGE(STATUS "AFTER: EXC (${EXCLUDE}) DEF (${DEFINE}) LINK (${LINK})")
    FOREACH(EXF ${EXCLUDE})
        REMOVE(SRC_FILES ${OPENSCENEGRAPH_DIR}/src/${SUBDIR}/${EXF})
    ENDFOREACH(EXF)
        
    FOREACH(DEF ${DEFINE})
        SET(DEFSTR "${DEFSTR} /D \"${DEF}\"")
        #MESSAGE(STATUS "add symbol : " ${DEF})
    ENDFOREACH(DEF)
    
    IF(NOT DEFSTR STREQUAL "")
        SET_SOURCE_FILES_PROPERTIES(${SRC_FILES} PROPERTIES COMPILE_FLAGS ${DEFSTR})
        #MESSAGE(STATUS "*********  ADD COMPILE FLAGS ${DEFSTR} **********")
    ENDIF(NOT DEFSTR STREQUAL "")
    
    #ADD_LIBRARY(${TARGET_NAME} SHARED ${SRC_FILES} )
        ADD_LIBRARY(${TARGET_NAME} MODULE ${SRC_FILES} )
        
    #not sure if needed, but for plugins only msvc need the d suffix
    IF(NOT MSVC)
        SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES DEBUG_POSTFIX "")
  ELSE(NOT MSVC)
        IF(OSG_MSVC_VERSIONED_DLL) 
                       
                #this is a hack... the build place is set to lib/<debug or release> by LIBARARY_OUTPUT_PATH equal to OUTPUT_LIBDIR
                #the .lib will be crated in ../ so going straight in lib by the IMPORT_PREFIX property
                #because we want dll placed in OUTPUT_BINDIR ie the bin folder sibling of lib, we can use ../../bin to go there,
                #it is hardcoded, we should compute OUTPUT_BINDIR position relative to OUTPUT_LIBDIR ... to be implemented
                #changing bin to something else breaks this hack
                #the dll are placed in bin/${OSG_PLUGINS} 
                
                IF(NOT MSVC_IDE)
                    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "../bin/${OSG_PLUGINS}/")
                ELSE(NOT MSVC_IDE)
                    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "../../bin/${OSG_PLUGINS}/" IMPORT_PREFIX "../")
                ENDIF(NOT MSVC_IDE)
                
                
        ELSE(OSG_MSVC_VERSIONED_DLL)
            
            #in standard mode (unversioned) the .lib and .dll are placed in lib/<debug or release>/${OSG_PLUGINS}.
            #here the PREFIX property has been used, the same result would be accomplidhe by prepending ${OSG_PLUGINS}/ to OUTPUT_NAME target property
            
            SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES PREFIX "${OSG_PLUGINS}/")
        ENDIF(OSG_MSVC_VERSIONED_DLL)
    ENDIF(NOT MSVC)

    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES DEFINE_SYMBOL "${EXPORTDEF}" PROJECT_LABEL "Wrapper ${SUBDIR}")
    IF(MSVC AND OSG_MSVC_VERSIONED_DLL)
        LINK_INTERNAL(${TARGET_NAME} ${LINK})
        #when using full path name to specify linkage, it seems that already linked libs must be specified
        LINK_EXTERNAL(${TARGET_NAME} ${OPENGL_LIBRARIES}) 

    ELSE(MSVC AND OSG_MSVC_VERSIONED_DLL)
        TARGET_LINK_LIBRARIES(${TARGET_NAME} ${LINK})
    ENDIF(MSVC AND OSG_MSVC_VERSIONED_DLL)
    #MESSAGE(STATUS "--TARGET_LINK_LIBRARIES-->${TARGET_NAME}<->${LINK}<-")
    
    REMOVE(DEFINE ${DEFINE})    
    REMOVE(LINK ${LINK})    
    REMOVE(EXCLUDE ${EXCLUDE})    
    
    IF(WIN32)
        INSTALL(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib/${OSG_PLUGINS} LIBRARY DESTINATION bin/${OSG_PLUGINS} COMPONENT libopenscenegraph-wrappers)
    ELSE(WIN32)
        INSTALL(TARGETS ${TARGET_NAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} LIBRARY DESTINATION lib${LIB_POSTFIX}/${OSG_PLUGINS} COMPONENT libopenscenegraph-wrappers)
    ENDIF(WIN32)
    
ENDMACRO(ADD_WRAPPER_LIB)



SET(OSGWRAPPER_LIB_LIST 
    OpenThreads
    osg
    osgDB
    osgFX
    osgGA
    osgManipulator
    osgParticle
    osgShadow
    osgSim
    osgTerrain
    osgText
    osgUtil
    osgViewer
    osgVolume
    osgWidget
)


SET(OSGWRAPPER_DIR ${CMAKE_CURRENT_SOURCE_DIR})
IF(MSVC)
    SET(LIBRARY_OUTPUT_PATH ${OUTPUT_LIBDIR})
ENDIF(MSVC)
FOREACH(LIB ${OSGWRAPPER_LIB_LIST} )
    ADD_WRAPPER_LIB(${LIB}  OSGWRAPPERS_LIBRARY LINK OpenThreads osg osgGA ${LIB} osgIntrospection)    
ENDFOREACH(LIB ${OSGWRAPPER_LIB_LIST} )
