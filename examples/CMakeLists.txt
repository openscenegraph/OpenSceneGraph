PROJECT(OSG_EXAMPLES)

SET(OSGCORE_BUNDLED TRUE)

IF(NOT OSGCORE_BUNDLED)
	FIND_PACKAGE(OSGCORE)
	IF(OSGCORE_FOUND)
		INCLUDE(${OSGCORE_USE_FILE})
	ELSE(OSGCORE_FOUND)
		MESSAGE(ERROR "OSGCORE neeeded but NOT FOUND")
	ENDIF(OSGCORE_FOUND)
	SET(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/../../Macros)
ENDIF(NOT OSGCORE_BUNDLED)
SET(OPENSCENEGRAPH_EXAMPLE_DIR ${PROJECT_SOURCE_DIR})


SET(TARGET_DEFAULT_PREFIX "example_")
SET(TARGET_DEFAULT_LABEL_PREFIX "Examples")
SET(TARGET_COMMON_LIBRARIES 
	OpenThreads
	osg 
	osgDB 
	osgUtil
	osgGA
	osgViewer 
	osgText 
)

#this is for asking the production of a single CMakeLists.txt
#ADD_SUBDIRECTORY(osganimate)
ADD_SUBDIRECTORY(osganimate)
ADD_SUBDIRECTORY(osgautotransform)
ADD_SUBDIRECTORY(osgbillboard)
ADD_SUBDIRECTORY(osgblendequation)
ADD_SUBDIRECTORY(osgcallback)
ADD_SUBDIRECTORY(osgcamera)
ADD_SUBDIRECTORY(osgcatch)
#ADD_SUBDIRECTORY(osgcegui)
ADD_SUBDIRECTORY(osgclip)
#to add subject to find socket#ADD_SUBDIRECTORY(osgcluster)
ADD_SUBDIRECTORY(osgcopy)
ADD_SUBDIRECTORY(osgcubemap)
ADD_SUBDIRECTORY(osgdelaunay)
ADD_SUBDIRECTORY(osgdepthpartition)
ADD_SUBDIRECTORY(osgdepthshadow)
ADD_SUBDIRECTORY(osgdistortion)
ADD_SUBDIRECTORY(osgfadetext)
ADD_SUBDIRECTORY(osgforest)
ADD_SUBDIRECTORY(osgfxbrowser)
ADD_SUBDIRECTORY(osggeodemo)
ADD_SUBDIRECTORY(osggeometry)
#to add subject to find Glut
#ADD_SUBDIRECTORY(osgGLUTkeyboardmouse)
#ADD_SUBDIRECTORY(osgGLUTsimple)
ADD_SUBDIRECTORY(osghangglide)
ADD_SUBDIRECTORY(osghud)
ADD_SUBDIRECTORY(osgimpostor)
ADD_SUBDIRECTORY(osgintersection)
IF   (BUILD_OSG_WRAPPERS)#
    ADD_SUBDIRECTORY(osgintrospection)
ENDIF(BUILD_OSG_WRAPPERS)#
ADD_SUBDIRECTORY(osgkeyboard)
ADD_SUBDIRECTORY(osgkeyboardmouse)
ADD_SUBDIRECTORY(osglauncher)
ADD_SUBDIRECTORY(osglight)
ADD_SUBDIRECTORY(osglightpoint)
ADD_SUBDIRECTORY(osglogicop)
ADD_SUBDIRECTORY(osglogo)
ADD_SUBDIRECTORY(osgmanipulator)
ADD_SUBDIRECTORY(osgmotionblur)
ADD_SUBDIRECTORY(osgmovie)
ADD_SUBDIRECTORY(osgmultiplecameras)
ADD_SUBDIRECTORY(osgmultitexture)
ADD_SUBDIRECTORY(osgoccluder)
ADD_SUBDIRECTORY(osgpagedlod)
ADD_SUBDIRECTORY(osgparametric)
ADD_SUBDIRECTORY(osgparticle)
ADD_SUBDIRECTORY(osgparticleeffects)
ADD_SUBDIRECTORY(osgphotoalbum)
ADD_SUBDIRECTORY(osgpick)
ADD_SUBDIRECTORY(osgplanets)
ADD_SUBDIRECTORY(osgpoints)
ADD_SUBDIRECTORY(osgpointsprite)
ADD_SUBDIRECTORY(osgprecipitation)
ADD_SUBDIRECTORY(osgprerender)
ADD_SUBDIRECTORY(osgprerendercubemap)
ADD_SUBDIRECTORY(osgreflect)
ADD_SUBDIRECTORY(osgscalarbar)
ADD_SUBDIRECTORY(osgscribe)
ADD_SUBDIRECTORY(osgsequence)
ADD_SUBDIRECTORY(osgshaders)
ADD_SUBDIRECTORY(osgshaderterrain)
ADD_SUBDIRECTORY(osgshadowtexture)
ADD_SUBDIRECTORY(osgshadow)
ADD_SUBDIRECTORY(osgshape)
ADD_SUBDIRECTORY(osgsimple)
##################################################
#ADD_SUBDIRECTORY(osgsimpleviewerXXX)
##################################################
ADD_SUBDIRECTORY(osgsimplifier)
ADD_SUBDIRECTORY(osgsimulation)
ADD_SUBDIRECTORY(osgterrain)
ADD_SUBDIRECTORY(osgslice)
ADD_SUBDIRECTORY(osgspacewarp)
ADD_SUBDIRECTORY(osgspheresegment)
ADD_SUBDIRECTORY(osgspotlight)
ADD_SUBDIRECTORY(osgstereoimage)

###########################################################################################################
## this is to specify external libraries linking, these libraries do not differentiate in debug and release

ADD_SUBDIRECTORY(osgteapot)

###########################################################################################################

ADD_SUBDIRECTORY(osgtessellate)#)
ADD_SUBDIRECTORY(osgtext)
ADD_SUBDIRECTORY(osgtexture1D)
ADD_SUBDIRECTORY(osgtexture2D)
ADD_SUBDIRECTORY(osgtexture3D)
ADD_SUBDIRECTORY(osgtexturerectangle)
ADD_SUBDIRECTORY(osgunittests)
ADD_SUBDIRECTORY(osgvertexprogram)
ADD_SUBDIRECTORY(osgvolume)
ADD_SUBDIRECTORY(osgwindows)

#REWRITE_CMAKELIST(ADD_OSG_EXAMPLE)

#MESSAGE("---->${MY_EXAMPLE_LIST}<---------")
##########to get all the variables of Cmake
#GET_CMAKE_PROPERTY(MYVARS VARIABLES)
#FOREACH(myvar ${MYVARS})
#	FILE(APPEND ${CMAKE_CURRENT_BINARY_DIR}/AllVariables.txt
#		"${myvar} -->${${myvar}}<-\n"
#	)
#ENDFOREACH(myvar)

