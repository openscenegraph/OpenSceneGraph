SET(TARGET_SRC MFC_OSG_MDI.cpp MFC_OSG_MDI.rc MFC_OSG_MDIDoc.cpp MFC_OSG_MDIView.cpp stdafx.cpp MFC_OSG.cpp MainFrm.cpp ChildFrm.cpp)
SET(TARGET_H   MFC_OSG_MDI.h   MFC_OSG_MDIDoc.h   MFC_OSG_MDIView.h   stdafx.h   MFC_OSG.h   MainFrm.h   ChildFrm.h Resource.h)

ADD_DEFINITIONS(-D_AFXDLL)
SET(REQUIRE_WINMAIN_FLAG 1)
          
#### end var setup  ###
SETUP_EXAMPLE(osgviewerMFC)
