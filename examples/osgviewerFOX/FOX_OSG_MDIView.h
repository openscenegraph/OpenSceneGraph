#ifndef _FOXOSGMDIVIEW_H_
#define _FOXOSGMDIVIEW_H_

#include "FOX_OSG.h"

#include <fx.h>

#include <osgViewer/Viewer>

using namespace FX;

class FOX_OSG_MDIView : public FXMDIChild{

    FXDECLARE(FOX_OSG_MDIView)

public:
    FOX_OSG_MDIView(FXMDIClient *p, const FXString &name,
        FXIcon *ic=NULL, FXPopup *pup=NULL, FXuint opts=0,
        FXint x=0, FXint y=0, FXint w=0, FXint h=0);

    virtual ~FOX_OSG_MDIView();

    enum{
        ID_CHORE=FXMDIChild::ID_LAST,
        ID_LAST
    };

    // callback
    long OnIdle(FXObject* , FXSelector, void*);

    void SetViewer(osgViewer::Viewer *viewer);

protected:
    FOX_OSG_MDIView(){};

private:
    osg::ref_ptr<osgViewer::Viewer>        m_osgViewer;
    GraphicsWindowFOX*                    m_gwFox;
};

#endif // _FOXOSGMDIVIEW_H_
