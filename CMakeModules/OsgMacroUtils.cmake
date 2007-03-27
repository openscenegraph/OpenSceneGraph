
#######################################################################################################
#  macro for linking libraries that come from Findxxxx commands, so there is a variable that contains the
#  full path of the library name. in order to differentiate release and debug, this macro get the
#  NAME of the variables, so the macro gets as arguments the target name and the following list of parameters
#  is intended as a list of variable names each one containing  the path of the libraries to link to
#  The existance of a varibale name with _DEBUG appended is tested and, in case it' s value is used
#  for linking to when in debug mode 
#  the content of this library for linking when in debugging
#######################################################################################################


MACRO(LINK_WITH_VARIABLES TRGTNAME)
	FOREACH(varname ${ARGN})
		IF(${varname}_DEBUG)
			TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${${varname}}" debug "${${varname}_DEBUG}")
		ELSE(${varname}_DEBUG)
			TARGET_LINK_LIBRARIES(${TRGTNAME} "${${varname}}" )
		ENDIF(${varname}_DEBUG)
	ENDFOREACH(varname)
ENDMACRO(LINK_WITH_VARIABLES TRGTNAME)

MACRO(LINK_INTERNAL TRGTNAME)
	FOREACH(LINKLIB ${ARGN})
		TARGET_LINK_LIBRARIES(${TRGTNAME} optimized "${LINKLIB}" debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
	ENDFOREACH(LINKLIB)
ENDMACRO(LINK_INTERNAL TRGTNAME)

MACRO(LINK_EXTERNAL TRGTNAME)
	FOREACH(LINKLIB ${ARGN})
		TARGET_LINK_LIBRARIES(${TRGTNAME} "${LINKLIB}" )
	ENDFOREACH(LINKLIB)
ENDMACRO(LINK_EXTERNAL TRGTNAME)


#######################################################################################################
#  macro for common setup of core libraries: it links OPENGL_LIBRARIES in undifferentiated mode and
#  OPENTHREADS_LIBRARY as Differentiated, so if existe the variable OPENTHREADS_LIBRARY_DEBUG, it uses 
#  the content of this library for linking when in debugging
#######################################################################################################

MACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)
	LINK_EXTERNAL(${CORELIB_NAME} ${OPENGL_LIBRARIES}) 
	LINK_WITH_VARIABLES(${CORELIB_NAME} OPENTHREADS_LIBRARY)
ENDMACRO(LINK_CORELIB_DEFAULT CORELIB_NAME)


#######################################################################################################
#  macro for common setup of plugins, examples and applications it expect some variables to be set:
#  either within the local CMakeLists or higher in hierarchy
#  TARGET_NAME is the name of the folder and of the actually .exe or .so or .dll
#  TARGET_TARGETNAME  is the name of the target , this get buit out of a prefix, if present and TARGET_TARGETNAME
#  TARGET_SRC  are the sources of the target
#  TARGET_H are the eventual headers of the target
#  TARGET_LIBRARIES are the libraries to link to that are internal to the project and have d suffix for debug
#  TARGET_EXTERNAL_LIBRARIES are external libraries and are not differentiated with d suffix
#  TARGET_LABEL is the label IDE should show up for targets
##########################################################################################################

MACRO(SETUP_LINK_LIBRARIES)
    ######################################################################
    #
    # This set up the libraries to link to, it assumes there are two variable: one common for a group of examples or plagins
    # kept in the variable TARGET_COMMON_LIBRARIES and an example or plugin specific kept in TARGET_ADDED_LIBRARIES 
    # they are combined in a single list checked for unicity 
    # the suffix ${CMAKE_DEBUG_POSTFIX} is used for differentiating optimized and debug
    #
    # a second variable TARGET_EXTERNAL_LIBRARIES hold the list of  libraries not differentiated between debug and optimized 
    ##################################################################################
    SET(TARGET_LIBRARIES ${TARGET_COMMON_LIBRARIES})

    FOREACH(LINKLIB ${TARGET_ADDED_LIBRARIES})
  	SET(TO_INSERT TRUE)
  	FOREACH (value ${TARGET_COMMON_LIBRARIES})
    	    IF (${value} STREQUAL ${LINKLIB})
      	        SET(TO_INSERT FALSE)
    	    ENDIF (${value} STREQUAL ${LINKLIB})
        ENDFOREACH (value ${TARGET_COMMON_LIBRARIES})
  	IF(TO_INSERT)
  	    LIST(APPEND TARGET_LIBRARIES ${LINKLIB})
  	ENDIF(TO_INSERT)
    ENDFOREACH(LINKLIB)

    FOREACH(LINKLIB ${TARGET_LIBRARIES})
			TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} optimized ${LINKLIB} debug "${LINKLIB}${CMAKE_DEBUG_POSTFIX}")
    ENDFOREACH(LINKLIB)

    FOREACH(LINKLIB ${TARGET_EXTERNAL_LIBRARIES})
			TARGET_LINK_LIBRARIES(${TARGET_TARGETNAME} ${LINKLIB})
    ENDFOREACH(LINKLIB)
		IF(TARGET_LIBRARIES_VARS)
			LINK_WITH_VARIABLES(${TARGET_TARGETNAME} ${TARGET_LIBRARIES_VARS})
		ENDIF(TARGET_LIBRARIES_VARS)
ENDMACRO(SETUP_LINK_LIBRARIES)

############################################################################################
# this is the common set of command for all the plugins
#

MACRO(SETUP_PLUGIN PLUGIN_NAME)

        SET(TARGET_NAME ${PLUGIN_NAME} )

	#MESSAGE("in -->SETUP_PLUGIN<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")

	## we have set up the target label and targetname by taking into account global prfix (osgdb_)

	IF(NOT TARGET_TARGETNAME)
			SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
	ENDIF(NOT TARGET_TARGETNAME)
	IF(NOT TARGET_LABEL)
			SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
	ENDIF(NOT TARGET_LABEL)
	
# here we use the command to generate the library	
	
	ADD_LIBRARY(${TARGET_TARGETNAME} MODULE ${TARGET_SRC} ${TARGET_H})
	
	#not sure if needed, but for plugins only msvc need the d suffix
	IF(NOT MSVC)
		SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_POSTFIX "")
	ENDIF(NOT MSVC)
	SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")

	SETUP_LINK_LIBRARIES()

#the installation path are differentiated for win32 that install in bib versus other architecture that install in lib${LIB_POSTFIX}/osgPlugins
	IF(WIN32)
		INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib LIBRARY DESTINATION bin )
	ELSE(WIN32)
		INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin ARCHIVE DESTINATION lib${LIB_POSTFIX}/osgPlugins LIBRARY DESTINATION lib${LIB_POSTFIX}/osgPlugins )
	ENDIF(WIN32)
ENDMACRO(SETUP_PLUGIN)


#################################################################################################################
# this is the macro for example and application setup
###########################################################

MACRO(SETUP_EXE)
	#MESSAGE("in -->SETUP_EXE<-- ${TARGET_NAME}-->${TARGET_SRC} <--> ${TARGET_H}<--")
	IF(NOT TARGET_TARGETNAME)
			SET(TARGET_TARGETNAME "${TARGET_DEFAULT_PREFIX}${TARGET_NAME}")
	ENDIF(NOT TARGET_TARGETNAME)
	IF(NOT TARGET_LABEL)
			SET(TARGET_LABEL "${TARGET_DEFAULT_LABEL_PREFIX} ${TARGET_NAME}")
	ENDIF(NOT TARGET_LABEL)

	ADD_EXECUTABLE(${TARGET_TARGETNAME} ${TARGET_SRC} ${TARGET_H})
	SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES PROJECT_LABEL "${TARGET_LABEL}")
	SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
	SET_TARGET_PROPERTIES(${TARGET_TARGETNAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
	
	SETUP_LINK_LIBRARIES()	

ENDMACRO(SETUP_EXE)

MACRO(SETUP_APPLICATION APPLICATION_NAME)

        SET(TARGET_NAME ${APPLICATION_NAME} )

        SETUP_EXE()
        
	INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION bin  )

ENDMACRO(SETUP_APPLICATION)


MACRO(SETUP_EXAMPLE EXAMPLE_NAME)

        SET(TARGET_NAME ${EXAMPLE_NAME} )

        SETUP_EXE()
        
	INSTALL(TARGETS ${TARGET_TARGETNAME} RUNTIME DESTINATION share/OpenSceneGraph/bin  )			

ENDMACRO(SETUP_EXAMPLE)


