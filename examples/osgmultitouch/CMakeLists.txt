# On OSX, this example only compiles if using Cocoa
IF(APPLE AND NOT (OSG_WINDOWING_SYSTEM STREQUAL "Cocoa")) 
  MESSAGE(WARNING "Disabling osgmultitouch example because it requires OSG_WINDOWING_SYSTEM to be Cocoa")
  RETURN() 
ENDIF() 

SET(TARGET_SRC osgmultitouch.cpp )

#### end var setup  ###
SETUP_EXAMPLE(osgmultitouch)
