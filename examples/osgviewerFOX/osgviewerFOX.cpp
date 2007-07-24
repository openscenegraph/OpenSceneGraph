#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "osgviewerFOX.h"

#include "FOX_OSG_MDIView.h"

/* My frame constructor */
MainFrame::MainFrame(FXApp *app, const FXString &name, FXIcon *ic, FXIcon *mi, FXuint opts, FXint x, FXint y, FXint w, FXint h, FXint pl, FXint pr, FXint pt, FXint pb, FXint hs, FXint vs) : FXMainWindow(app, name, ic, mi, opts, x, y, w, h, pl, pr, pt, pb, hs, vs)
{


	// Site where to dock
	FXDockSite* topdock=new FXDockSite(this,DOCKSITE_NO_WRAP|LAYOUT_SIDE_TOP|LAYOUT_FILL_X);

	// Menubar 1
	m_fxToolbarShell1=new FXToolBarShell(this,FRAME_RAISED);
	FXMenuBar* menubar=new FXMenuBar(topdock,m_fxToolbarShell1,LAYOUT_DOCK_SAME|LAYOUT_SIDE_TOP|LAYOUT_FILL_X|FRAME_RAISED);
	new FXToolBarGrip(menubar,menubar,FXMenuBar::ID_TOOLBARGRIP,TOOLBARGRIP_DOUBLE);

	// Contents
	FXHorizontalFrame *frame=new FXHorizontalFrame(this,LAYOUT_SIDE_TOP|LAYOUT_FILL_X|LAYOUT_FILL_Y, 0,0,0,0, 0,0,0,0, 4,4);

	// Nice sunken box around GL viewer
	FXVerticalFrame *box=new FXVerticalFrame(frame,FRAME_SUNKEN|FRAME_THICK|LAYOUT_FILL_X|LAYOUT_FILL_Y,0,0,0,0, 0,0,0,0);

	// MDI Client
	FXMDIClient* mdiclient=new FXMDIClient(box,LAYOUT_FILL_X|LAYOUT_FILL_Y);

	// Make MDI Window Menu
	FXMDIMenu* mdimenu=new FXMDIMenu(this,mdiclient);

	// MDI buttons in menu:- note the message ID's!!!!!
	// Normally, MDI commands are simply sensitized or desensitized;
	// Under the menubar, however, they're hidden if the MDI Client is
	// not maximized.  To do this, they must have different ID's.
	new FXMDIWindowButton(menubar,mdimenu,mdiclient,FXMDIClient::ID_MDI_MENUWINDOW,LAYOUT_LEFT|LAYOUT_CENTER_Y);
	new FXMDIDeleteButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENUCLOSE,FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
	new FXMDIRestoreButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENURESTORE,FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_CENTER_Y);
	new FXMDIMinimizeButton(menubar,mdiclient,FXMDIClient::ID_MDI_MENUMINIMIZE,FRAME_RAISED|LAYOUT_RIGHT|LAYOUT_CENTER_Y);

	// Make an MDI Child
	FOX_OSG_MDIView* mdichild=new FOX_OSG_MDIView(mdiclient,"FOX osgViewer", NULL, mdimenu,MDI_TRACKING|MDI_MAXIMIZED,30,30,300,200);
	mdichild->setFocus();

	// Make it active
	mdiclient->setActiveChild(mdichild);

}

// Create and initialize
void MainFrame::create(){
  FXMainWindow::create();
  m_fxToolbarShell1->create();
  show(PLACEMENT_SCREEN);
}

int main(int argc, char** argv){

	// Make application
	FXApp application("OSGViewer","FoxTest");

	// Open the display
	application.init(argc,argv);

	// Make window
	new MainFrame(&application, "Fox Toolkit OSG Sample", NULL, NULL, DECOR_ALL, 100, 100, 800, 600);

	// Create the application's windows
	application.create();

	// Run the application
	return application.run();
}
