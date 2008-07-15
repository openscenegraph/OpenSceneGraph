// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Version.cpp 64 2008-06-30 21:32:00Z cubicool $

#include <sstream>
#include <osgWidget/Version>

extern "C" {

const unsigned int OSGWIDGET_MAJOR_VERSION = 0;
const unsigned int OSGWIDGET_MINOR_VERSION = 1;
const unsigned int OSGWIDGET_PATCH_VERSION = 8;
const char*        OSGWIDGET_EXTRA_TEXT    = "(pre-merge)";

unsigned int osgWidgetGetMajorVersion() {
	return OSGWIDGET_MAJOR_VERSION;
}

unsigned int osgWidgetGetMinorVersion() {
	return OSGWIDGET_MINOR_VERSION;
}

unsigned int osgWidgetGetPatchVersion() {
	return OSGWIDGET_PATCH_VERSION;
}

const char* osgWidgetGetExtraText() {
	return OSGWIDGET_EXTRA_TEXT;
}

const char* osgWidgetGetVersion() {
	static std::string version;
	static bool versionInit = true;

	if(versionInit) {
		std::stringstream stream;
		
		stream.str(std::string());

		stream
			<< OSGWIDGET_MAJOR_VERSION << "."
			<< OSGWIDGET_MINOR_VERSION << "."
			<< OSGWIDGET_PATCH_VERSION << " "
			<< OSGWIDGET_EXTRA_TEXT
		;

		version     = stream.str();
		versionInit = false;
	}

	return version.c_str();
}

const char* osgWidgetGetLibraryName() {
	static std::string name("OpenSceneGraph Widget Library");

	return name.c_str();
}

bool osgWidgetVersionMinimum(unsigned int major, unsigned int minor, unsigned int patch) {
	return
		OSGWIDGET_MAJOR_VERSION >= major &&
		OSGWIDGET_MINOR_VERSION >= minor &&
		OSGWIDGET_PATCH_VERSION >= patch
	;
}

bool osgWidgetVersionMaximum(unsigned int major, unsigned int minor, unsigned int patch) {
	return 
		// OSGWIDGET_MAJOR_VERSION <= major &&
		OSGWIDGET_MINOR_VERSION <= minor &&
		OSGWIDGET_PATCH_VERSION <= patch
	;
}

bool osgWidgetVersionRequired(unsigned int major, unsigned int minor, unsigned int patch) {
	return 
		OSGWIDGET_MAJOR_VERSION == major &&
		OSGWIDGET_MINOR_VERSION == minor &&
		OSGWIDGET_PATCH_VERSION == patch
	;
}

}
