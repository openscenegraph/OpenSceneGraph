/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Linden Lab Inc. (http://lindenlab.com) code.
 *
 * The Initial Developer of the Original Code is:
 *   Callum Prentice (callum@ubrowser.com)
 *
 * Portions created by the Initial Developer are Copyright (C) 2006
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *  Callum Prentice (callum@ubrowser.com)
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */

#ifndef LLEMBEDDEDBROWSERWINDOW_H
#define LLEMBEDDEDBROWSERWINDOW_H

// Mozilla code has non-virtual destructors
#ifdef WIN32
#pragma warning( disable : 4265 ) // "class has virtual functions, but destructor is not virtual"
#endif

#include "nsIBaseWindow.h"
#include "nsIDOMEventListener.h"
#include "nsIDOMEventTarget.h"
#include "nsIInterfaceRequestor.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebNavigation.h"
#include "nsIWebProgressListener.h"
#include "nsIURIContentListener.h"
#include "nsWeakReference.h"
#include "nsIWebBrowser.h"
#include "nsIToolkit.h"
#include "nsIScriptGlobalObject.h"
#include "nsIScriptGlobalObjectOwner.h"
#include "nsIScriptContext.h"

#ifdef WIN32
#pragma warning( 3 : 4265 ) // "class has virtual functions, but destructor is not virtual"
#endif

#include <string>
#include <list>
#include <algorithm>

#include "llmozlib2.h"

///////////////////////////////////////////////////////////////////////////////
// manages the process of storing and emitting events that the consumer
// of the embedding class can observe
template< class T >
class LLEmbeddedBrowserWindowEmitter
{
    public:
        LLEmbeddedBrowserWindowEmitter() { };
        ~LLEmbeddedBrowserWindowEmitter() { };

        typedef typename T::EventType EventType;
        typedef std::list< T* > ObserverContainer;
        typedef void( T::*observerMethod )( const EventType& );

        ///////////////////////////////////////////////////////////////////////////////
        //
        bool addObserver( T* observerIn )
        {
            if ( ! observerIn )
                return false;

            if ( std::find( observers.begin(), observers.end(), observerIn ) != observers.end() )
                return false;

            observers.push_back( observerIn );

            return true;
        };

        ///////////////////////////////////////////////////////////////////////////////
        //
        bool remObserver( T* observerIn )
        {
            if ( ! observerIn )
                return false;

            observers.remove( observerIn );

            return true;
        };

        ///////////////////////////////////////////////////////////////////////////////
        //
        void update( observerMethod method, const EventType& msgIn )
        {
            typename std::list< T* >::iterator iter = observers.begin();

            while( iter != observers.end() )
            {
                ( ( *iter )->*method )( msgIn );

                ++iter;
            };
        };

    protected:
        ObserverContainer observers;
};

class LLEmbeddedBrowser;

////////////////////////////////////////////////////////////////////////////////
// class for a "window" that holds a browser - there can be lots of these
class LLEmbeddedBrowserWindow :
    public nsIInterfaceRequestor,
    public nsIWebBrowserChrome,
    public nsIWebProgressListener,
    public nsIURIContentListener,
#ifdef SUPPORTS_WEAK_REFENCE
    public nsSupportsWeakReference,
#endif
#ifdef NS_DECL_NSITOOLKITOBSERVER     
        public nsIToolkitObserver,
#endif
    public nsIDOMEventListener
{
    public:
        LLEmbeddedBrowserWindow();
        virtual ~LLEmbeddedBrowserWindow();

        NS_DECL_ISUPPORTS
        NS_DECL_NSIINTERFACEREQUESTOR
        NS_DECL_NSIWEBBROWSERCHROME
        NS_DECL_NSIWEBPROGRESSLISTENER
        NS_DECL_NSIURICONTENTLISTENER
        NS_DECL_NSIDOMEVENTLISTENER
#ifdef NS_DECL_NSITOOLKITOBSERVER     
        NS_DECL_NSITOOLKITOBSERVER
#endif
        // housekeeping
        nsresult createBrowser( void* nativeWindowHandleIn, PRInt32 widthIn, PRInt32 heightIn, nsIWebBrowser** aBrowser );
        void setParent( LLEmbeddedBrowser* parentIn ) { mParent = parentIn; };
        PRBool setSize( PRInt16 widthIn, PRInt16 heightIn );
        void focusBrowser( PRBool focusBrowserIn );
        void scrollByLines( PRInt16 linesIn );
        void setWindowId( int windowIdIn );
        int getWindowId();

        NS_METHOD NotifyInvalidated(nsIWidget*, PRInt32, PRInt32, PRInt32, PRInt32);

        // random accessors
        const PRInt16 getPercentComplete();
        const std::string& getStatusMsg();
        const std::string& getCurrentUri();
        const std::string& getClickLinkHref();
        const std::string& getClickLinkTarget();

        // memory buffer management
        unsigned char* grabWindow( int xIn, int yIn, int widthIn, int heightIn );
        PRBool flipWindow( PRBool flip );
        unsigned char* getPageBuffer();
        PRInt16 getBrowserWidth();
        PRInt16 getBrowserHeight();
        PRInt16 getBrowserDepth();
        PRInt32 getBrowserRowSpan();

        // set background color that you see in between pages - default is white but sometimes useful to change
        void setBackgroundColor( const PRUint8 redIn, const PRUint8 greenIn, const PRUint8 blueIn );

        // change the caret color (we have different backgrounds to edit fields - black caret on black background == bad)
        void setCaretColor( const PRUint8 redIn, const PRUint8 greenIn, const PRUint8 blueIn );

        // can turn off updates to a page - e.g. when it's hidden by your windowing system
        void setEnabled( PRBool enabledIn );

        // navigation
        void navigateStop();
        PRBool navigateTo( const std::string uriIn );
        PRBool canNavigateBack();
        void navigateBack();
        PRBool canNavigateForward();
        void navigateForward();
        void navigateReload();

        // javascript access/control
        std::string evaluateJavascript( std::string scriptIn );

        // redirection when you hit a missing page
        bool set404RedirectUrl( std::string redirect_url );
        bool clr404RedirectUrl();

        // mouse & keyboard events
        void mouseDown( PRInt16 xPosIn, PRInt16 yPosIn );
        void mouseUp( PRInt16 xPosIn, PRInt16 yPosIn );
        void mouseMove( PRInt16 xPosIn, PRInt16 yPosIn );
        void mouseLeftDoubleClick( PRInt16 xPosIn, PRInt16 yPosIn );
        void keyPress( PRInt16 keyCode );
        void unicodeInput( PRUint32 uni_char );

        // allow consumers of this class and to observe browser events
        bool addObserver( LLEmbeddedBrowserWindowObserver* observerIn );
        bool remObserver( LLEmbeddedBrowserWindowObserver* observerIn );

        // accessor/mutator for scheme that browser doesn't follow - e.g. secondlife.com://
        void setNoFollowScheme( std::string schemeIn );
        std::string getNoFollowScheme();

    private:
        PRBool sendMozillaMouseEvent( PRInt16 eventIn, PRInt16 xPosIn, PRInt16 yPosIn, PRUint32 clickCountIn );
        PRBool sendMozillaKeyboardEvent( PRUint32 keyIn, PRUint32 ns_vk_code );
        PRBool renderCaret();
        PRBool enableToolkitObserver( PRBool enableIn );

        LLEmbeddedBrowserWindowEmitter< LLEmbeddedBrowserWindowObserver > mEventEmitter;

        LLEmbeddedBrowser* mParent;
        PRInt16 mPercentComplete;
        std::string mStatusText;
        std::string mCurrentUri;
        std::string mClickHref;
        std::string mClickTarget;
        std::string mNoFollowScheme;
        nsCOMPtr< nsIWebBrowser > mWebBrowser;
        nsCOMPtr< nsIBaseWindow > mBaseWindow;
        nsCOMPtr< nsIWebNavigation > mWebNav;
        int mWindowId;
        unsigned char* mPageBuffer;
        std::string m404RedirectUrl;
        PRBool mEnabled;
        PRBool mFlipBitmap;
        PRInt32 mBrowserRowSpan;
        PRInt16 mBrowserWidth;
        PRInt16 mBrowserHeight;
        PRInt16 mBrowserDepth;
        PRUint8 mBkgRed;
        PRUint8 mBkgGreen;
        PRUint8 mBkgBlue;
        PRUint8 mCaretRed;
        PRUint8 mCaretGreen;
        PRUint8 mCaretBlue;
};

#endif // LLEMBEDEDDBROWSERWINDOW_H
