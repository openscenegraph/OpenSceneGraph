#ifndef _FOXSIMPLEVIEWERFOX_H_
#define _FOXSIMPLEVIEWERFOX_H_


#include <fx.h>

using namespace FX;

class MainFrame : public FXMainWindow{

public:
	MainFrame(FXApp *a, const FXString &name,
		FXIcon *ic=NULL, FXIcon *mi=NULL,
		FXuint opts=DECOR_ALL,
		FXint x=0, FXint y=0,
		FXint w=0, FXint h=0,
		FXint pl=0, FXint pr=0, FXint pt=0, FXint pb=0,
		FXint hs=0, FXint vs=0);


	// Initialize
	virtual void create();

protected:
	MainFrame(){};

private:
	// GUI elements
	FXToolBarShell* m_fxToolbarShell1;
};

#endif // _FOXSIMPLEVIEWERFOX_H_
