#include	"FTLibrary.h"


FTLibrary&	FTLibrary::Instance()
{
	static FTLibrary ftlib;
	return ftlib;
}


FTLibrary::~FTLibrary()
{
	if( lib != 0)
	{
		FT_Done_FreeType( *lib);

		delete lib;
		lib= 0;
	}

//	if( manager != 0)
//	{
//		FTC_Manager_Done( manager );
//
//		delete manager;
//		manager= 0;
//	}
}


FTLibrary::FTLibrary()
:	lib(0),
	err(0)
{
	Init();
}


bool FTLibrary::Init()
{
	if( lib != 0 )
		return true;

	lib = new FT_Library;
	
	err = FT_Init_FreeType( lib);
	if( err)
	{
		delete lib;
		lib = 0;
		return false;
	}
	
// 	FTC_Manager* manager;
// 	
// 	if( FTC_Manager_New( lib, 0, 0, 0, my_face_requester, 0, manager )
// 	{
// 		delete manager;
// 		manager= 0;
// 		return false;
// 	}

	return true;
}
