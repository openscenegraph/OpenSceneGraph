#ifndef _FOXOSG_H_
#define _FOXOSG_H_

#include <osgViewer/Viewer>
#include <string>

#include <fx.h>
#include <fx3d.h>

using namespace FX;

class GraphicsWindowFOX: public FXGLCanvas, public osgViewer::GraphicsWindow{

        FXDECLARE(GraphicsWindowFOX)

public:
    GraphicsWindowFOX(FXComposite *parent, FXGLVisual *vis,
        FXObject *tgt=NULL, FXSelector sel=0,
        FXuint opts=0, FXint x=0, FXint y=0,
        FXint w=0, FXint h=0);

    virtual ~GraphicsWindowFOX();

    // callback
    long onConfigure(FXObject*, FXSelector, void*);
    long onKeyPress(FXObject*, FXSelector, void*);
    long onKeyRelease(FXObject*, FXSelector, void*);
    long onLeftBtnPress(FXObject*, FXSelector, void*);
    long onLeftBtnRelease(FXObject*, FXSelector, void*);
    long onMiddleBtnPress(FXObject*, FXSelector, void*);
    long onMiddleBtnRelease(FXObject*, FXSelector, void*);
    long onRightBtnPress(FXObject*, FXSelector, void*);
    long onRightBtnRelease(FXObject*, FXSelector, void*);
    long onMotion(FXObject*, FXSelector, void*);

    void init();

    //
    // GraphicsWindow interface
    //
    void grabFocus();
    void grabFocusIfPointerInWindow();
    void useCursor(bool cursorOn);

    bool makeCurrentImplementation();
    bool releaseContext();
    void swapBuffersImplementation();

    // note implemented yet...just use dummy implementation to get working.    
    virtual bool valid() const { return true; }
    virtual bool realizeImplementation() { return true; }
    virtual bool isRealizedImplementation() const  { return true; }
    virtual void closeImplementation() {}
    virtual bool releaseContextImplementation() { return true; }

protected:
    GraphicsWindowFOX(){};

private:
    FXCursor* _oldCursor;
};

#endif // _FOXOSG_H_
