FILE(GLOB ui_files_1 "English.lproj/*.strings")
FILE(GLOB ui_files_2 "English.lproj/MainMenu.nib/*.nib")
SET(TARGET_SRC ViewerCocoa.mm main.mm Info.plist ${ui_files_1} ${ui_files_2})

SET(TARGET_EXTERNAL_LIBRARIES osg ${COCOA_LIBRARY})
SET_SOURCE_FILES_PROPERTIES(Info.plist PROPERTIES MACOSX_PACKAGE_LOCATION .)
SET_SOURCE_FILES_PROPERTIES(${ui_files_1} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj)
SET_SOURCE_FILES_PROPERTIES(${ui_files_2} PROPERTIES MACOSX_PACKAGE_LOCATION Resources/English.lproj/MainMenu.nib)

SETUP_EXAMPLE(osgviewerCocoa)
