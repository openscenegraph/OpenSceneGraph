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

// seems to be required by LibXUL/Mozilla code to avoid crashes in their debug code, but only on Windows.
// undef'd at end of this
#ifdef _DEBUG
    #ifdef WIN32
        #define DEBUG 1
    #endif
#endif

// needed for the code in LLEmbeddedBrowserWindow::NotifyInvalidated() which will
// one day be moved to platform agnostic code when I find out how...
#ifdef WIN32
#include "windows.h"
#endif

#include "llembeddedbrowser.h"
#include "llembeddedbrowserwindow.h"

// Mozilla code has non-virtual destructors
#ifdef WIN32
#pragma warning( disable : 4291 ) // (no matching operator delete found; memory will not be freed if initialization throws an exception)
#pragma warning( disable : 4265 ) // "class has virtual functions, but destructor is not virtual"
#endif

#include "nsCWebBrowser.h"
#include "nsGUIEvent.h"
#include "nsICaret.h"
#include "nsIContent.h"
#include "nsIDOMDocument.h"
#include "nsIDOMElement.h"
#include "nsIDOMWindow.h"
#include "nsIDOMEvent.h"
#include "nsIDocShell.h"
#include "nsIDocShellTreeItem.h"
#include "nsIDocument.h"
#include "nsIFrame.h"
#include "nsIHttpChannel.h"
#include "nsIInterfaceRequestorUtils.h"
#include "nsIScrollableView.h"
#include "nsISelection.h"
#include "nsISelectionController.h"
#include "nsIWebBrowserChrome.h"
#include "nsIWebBrowserChromeFocus.h"
#include "nsIWebBrowserFocus.h"
#include "nsIWebProgress.h"
#include "nsIWebProgressListener.h"
#include "nsPresContext.h"
#include "nsProfileDirServiceProvider.h"
#include "nsXPCOMGlue.h"
#include "nsXULAppAPI.h"

#include "llembeddedbrowserwindow.h"

#include <iostream>

#ifdef WIN32
#pragma warning( 3 : 4265 ) // "class has virtual functions, but destructor is not virtual"
#endif

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowserWindow::LLEmbeddedBrowserWindow() :
    mParent( 0 ),
    mPercentComplete( 0 ),
    mStatusText( "" ),
    mCurrentUri( "" ),
    mClickHref( "" ),
    mClickTarget( "" ),
    mNoFollowScheme( "secondlife://" ),
    mWebBrowser( nsnull ),
    mBaseWindow( nsnull ),
    mWindowId( 0 ),
    mPageBuffer( 0 ),
    m404RedirectUrl( "" ),
    mEnabled( true ),
    mFlipBitmap( false ),
    mBrowserRowSpan( 0 ),
    mBrowserWidth( 0 ),
    mBrowserHeight( 0 ),
    mBrowserDepth( 4 ),
    mBkgRed( 0xff ),
    mBkgGreen( 0xff ),
    mBkgBlue( 0xff ),
    mCaretRed( 0x00 ),
    mCaretGreen( 0x00 ),
    mCaretBlue( 0x00 )
{
}

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowserWindow::~LLEmbeddedBrowserWindow()
{
    if ( mWebNav )
    {
        mWebNav->Stop ( nsIWebNavigation::STOP_ALL );
        mWebNav = nsnull;
    }

    if ( mBaseWindow )
    {
        mBaseWindow->Destroy();
        mBaseWindow = nsnull;
    }

    if ( mPageBuffer )
    {
        delete[] mPageBuffer;
        mPageBuffer = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
nsresult LLEmbeddedBrowserWindow::createBrowser( void* nativeWindowHandleIn, PRInt32 widthIn, PRInt32 heightIn, nsIWebBrowser **aBrowser )
{
    NS_ENSURE_ARG_POINTER(aBrowser);
    *aBrowser = nsnull;

    nsresult rv;
    mWebBrowser = do_CreateInstance( NS_WEBBROWSER_CONTRACTID, &rv );
    if ( ! mWebBrowser )
    {
        return NS_ERROR_FAILURE;
    }

    (void)mWebBrowser->SetContainerWindow( NS_STATIC_CAST( nsIWebBrowserChrome*, this ) );

    nsCOMPtr<nsIDocShellTreeItem> dsti = do_QueryInterface( mWebBrowser );
    dsti->SetItemType( nsIDocShellTreeItem::typeContentWrapper );

    mBaseWindow = do_QueryInterface( mWebBrowser );

    mBaseWindow->InitWindow( nativeWindowHandleIn, nsnull,  0, 0, mBrowserWidth, mBrowserHeight );
    mBaseWindow->Create();

    nsCOMPtr< nsIWebProgressListener > listener( NS_STATIC_CAST( nsIWebProgressListener*, this ) );
    nsCOMPtr< nsIWeakReference > thisListener( do_GetWeakReference( listener ) );
    mWebBrowser->AddWebBrowserListener( thisListener, NS_GET_IID( nsIWebProgressListener ) );

#if LL_DARWIN
    // Without this, the mac doesn't get upates for animating gifs, mouseovers, etc.
    mBaseWindow->SetVisibility( PR_TRUE );
#else
    mBaseWindow->SetVisibility( PR_FALSE );
#endif

    nsresult result;
    mWebNav = do_QueryInterface( mWebBrowser, &result );
    if ( NS_FAILED( result ) || ! mWebNav )
    {
        return NS_ERROR_FAILURE;
    }

    setSize( widthIn, heightIn );

    if ( mWebBrowser )
    {
        *aBrowser = mWebBrowser;
        NS_ADDREF( *aBrowser );

        return NS_OK;
    }

    return NS_ERROR_FAILURE;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMPL_ADDREF( LLEmbeddedBrowserWindow )
NS_IMPL_RELEASE( LLEmbeddedBrowserWindow )

////////////////////////////////////////////////////////////////////////////////
//
NS_INTERFACE_MAP_BEGIN( LLEmbeddedBrowserWindow )
    NS_INTERFACE_MAP_ENTRY_AMBIGUOUS( nsISupports, nsIWebBrowserChrome )
    NS_INTERFACE_MAP_ENTRY( nsIInterfaceRequestor )
    NS_INTERFACE_MAP_ENTRY( nsIWebBrowserChrome )
    NS_INTERFACE_MAP_ENTRY( nsIWebProgressListener )
    NS_INTERFACE_MAP_ENTRY( nsIURIContentListener )
#ifdef SUPPORTS_WEAK_REFENCE
    NS_INTERFACE_MAP_ENTRY( nsISupportsWeakReference )
#endif
#ifdef NS_DECL_NSITOOLKITOBSERVER     
    NS_INTERFACE_MAP_ENTRY( nsIToolkitObserver )
#endif
NS_INTERFACE_MAP_END

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::GetInterface( const nsIID &aIID, void** aInstancePtr )
{
    if ( aIID.Equals( NS_GET_IID( nsIDOMWindow ) ) )
    {
        if ( mWebBrowser )
        {
            return mWebBrowser->GetContentDOMWindow( ( nsIDOMWindow** )aInstancePtr );
        }

        return NS_ERROR_NOT_INITIALIZED;
    }

    return QueryInterface( aIID, aInstancePtr );
}

////////////////////////////////////////////////////////////////////////////////
// called when something changes the status text - emits event to consumer
NS_IMETHODIMP LLEmbeddedBrowserWindow::SetStatus( PRUint32 aType, const PRUnichar* aStatus )
{
    mStatusText = std::string( NS_ConvertUTF16toUTF8( aStatus ).get() );

    LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mStatusText );
    mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onStatusTextChange, event );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::GetWebBrowser( nsIWebBrowser** aWebBrowser )
{
    NS_ENSURE_ARG_POINTER( aWebBrowser );

    *aWebBrowser = mWebBrowser;

    NS_IF_ADDREF( *aWebBrowser );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::SetWebBrowser( nsIWebBrowser* aWebBrowser )
{
    NS_ENSURE_ARG_POINTER( aWebBrowser );

    mWebBrowser = aWebBrowser;

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::GetChromeFlags( PRUint32* aChromeMask )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::SetChromeFlags( PRUint32 aChromeMask )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::DestroyBrowserWindow()
{
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::SizeBrowserTo( PRInt32 aCX, PRInt32 aCY )
{
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::ShowAsModal()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::IsWindowModal( PRBool* retval )
{
    *retval = PR_FALSE;

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::ExitModalEventLoop( nsresult aStatus )
{
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// called when the page loading progress changes - emits event to consumer
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnProgressChange( nsIWebProgress* progress, nsIRequest* request,
                                                    PRInt32 curSelfProgress, PRInt32 maxSelfProgress,
                                                        PRInt32 curTotalProgress, PRInt32 maxTotalProgress )
{
    mPercentComplete = static_cast< PRInt16 >
        ( static_cast< float >( curTotalProgress * 100.0f ) / static_cast< float >( maxTotalProgress ) );

    if ( mPercentComplete < 0 )
        mPercentComplete = 0;

    if ( mPercentComplete > 100 )
        mPercentComplete = 100;

    LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mPercentComplete );
    mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onUpdateProgress, event );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// called when the browser state changes - as described below - emits event to consumer
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnStateChange( nsIWebProgress* progress, nsIRequest* request,
                                                    PRUint32 progressStateFlags, nsresult status )
{
     if ( ( progressStateFlags & STATE_START ) && ( progressStateFlags & STATE_IS_DOCUMENT ) && ( status == NS_OK ) )
    {
        // TODO: move this to a better place.
        enableToolkitObserver( false );
        enableToolkitObserver( true );

        // page load is starting so remove listener that catches "click" events
        nsCOMPtr< nsIDOMWindow > window;
        nsresult result = progress->GetDOMWindow( getter_AddRefs( window ) );
        if ( result == NS_OK )
        {
            nsCOMPtr< nsIDOMEventTarget > target = do_QueryInterface( window );
            if ( target )
                target->RemoveEventListener(NS_ConvertUTF8toUTF16( "click" ), this, PR_TRUE );
        }

        // set the listener to we can catch nsURIContentListener events
        if ( mWebBrowser )
        {
            mWebBrowser->SetParentURIContentListener( NS_STATIC_CAST( nsIURIContentListener*, this ) );
        }

        // emit event that navigation is beginning
        mStatusText = std::string( "Browser loaded" );
        LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mStatusText );
        mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onNavigateBegin, event );

        // about to move to a different page so have to stop grabbing a page
        // but done one final grab in case the app doesn't ever call grabWindow again
        grabWindow( 0, 0, mBrowserWidth, mBrowserHeight );
    }

    if ( ( progressStateFlags & STATE_STOP ) && ( progressStateFlags & STATE_IS_WINDOW ) && ( status == NS_OK ) )
    {
        // page load is complete so add listener that catches "click" events
        nsCOMPtr< nsIDOMWindow > window;
        nsresult result = progress->GetDOMWindow( getter_AddRefs( window ) );
        if ( result == NS_OK )
        {
            nsCOMPtr< nsIDOMEventTarget > target = do_QueryInterface( window );
            if ( target )
                target->AddEventListener(NS_ConvertUTF8toUTF16( "click" ), this, PR_TRUE );
        }

        // pick up raw HTML response status code
        PRUint32 responseStatus = 0;
        if ( request )
        {
            nsCOMPtr< nsIHttpChannel > httpChannel = do_QueryInterface( request );
            if ( httpChannel )
            {
                httpChannel->GetResponseStatus( &responseStatus );
            }
        }

        // emit event that navigation is finished
        mStatusText = std::string( "Done" );
        LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mStatusText, (int)responseStatus );
        mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onNavigateComplete, event );

        // also set the flag here since back/forward navigation doesn't call progress change
        grabWindow( 0, 0, mBrowserWidth, mBrowserHeight );
    }

    if ( progressStateFlags & STATE_REDIRECTING )
    {
        mStatusText = std::string( "Redirecting..." );
    }

    if ( progressStateFlags & STATE_TRANSFERRING )
    {
        mStatusText = std::string( "Transferring..." );
    }

    if ( progressStateFlags & STATE_NEGOTIATING )
    {
        mStatusText = std::string( "Negotiating..." );
    }

    LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mStatusText );
    mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onStatusTextChange, event );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// call when the location changes - e.g. when a site redirects - emits event to consumer
// TODO: ought to check that this change is on the top frame and
// indicate this to the consumer of this class
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnLocationChange( nsIWebProgress* webProgress,
                                                        nsIRequest* request,
                                                            nsIURI* location )
{
    if ( request )
    {
        nsCOMPtr< nsIHttpChannel > http_channel = do_QueryInterface( request );
        if ( http_channel )
        {
            PRUint32 response_status = 0;
            http_channel->GetResponseStatus( &response_status );

            if ( response_status == 404 )
            {
                if ( ! m404RedirectUrl.empty() )
                {
                    if ( mWebNav )
                    {
                        mWebNav->LoadURI( reinterpret_cast< const PRUnichar* >
                            ( NS_ConvertUTF8toUTF16( m404RedirectUrl.c_str() ).get() ),
                                nsIWebNavigation::LOAD_FLAGS_REPLACE_HISTORY,
                                    nsnull, nsnull, nsnull );
                    }
                }
            }
        }
    }

    nsCAutoString newURI;
    location->GetSpec( newURI );

    mCurrentUri = newURI.get();

    LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri() );
    mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onLocationChange, event );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// change the background color that gets used between pages (usually white)
void LLEmbeddedBrowserWindow::setBackgroundColor( const PRUint8 redIn, const PRUint8 greenIn, const PRUint8 blueIn )
{
    mBkgRed = redIn;
    mBkgGreen = greenIn;
    mBkgBlue = blueIn;
}

////////////////////////////////////////////////////////////////////////////////
// change the caret color (we have different backgrounds to edit fields - black caret on black background == bad)
void LLEmbeddedBrowserWindow::setCaretColor( const PRUint8 redIn, const PRUint8 greenIn, const PRUint8 blueIn )
{
    mCaretRed = redIn;
    mCaretGreen = greenIn;
    mCaretBlue = blueIn;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::setEnabled( PRBool enabledIn )
{
    mEnabled = enabledIn;
}

////////////////////////////////////////////////////////////////////////////////
// allow consumers of this class to observe events - add themselves as an observer
bool LLEmbeddedBrowserWindow::addObserver( LLEmbeddedBrowserWindowObserver* observerIn )
{
    return mEventEmitter.addObserver( observerIn );
}

////////////////////////////////////////////////////////////////////////////////
// allow consumers of this class to observe events - remove themselves as an observer
bool LLEmbeddedBrowserWindow::remObserver( LLEmbeddedBrowserWindowObserver* observerIn )
{
    return mEventEmitter.remObserver( observerIn );
}

////////////////////////////////////////////////////////////////////////////////
// used by observers of this class to get the current URI
const std::string& LLEmbeddedBrowserWindow::getCurrentUri()
{
    return mCurrentUri;
}

////////////////////////////////////////////////////////////////////////////////
// utility method that is used by observers to retrieve data after an event
const PRInt16 LLEmbeddedBrowserWindow::getPercentComplete()
{
    return mPercentComplete;
}

////////////////////////////////////////////////////////////////////////////////
// utility method that is used by observers to retrieve data after an event
const std::string& LLEmbeddedBrowserWindow::getStatusMsg()
{
    return mStatusText;
}

////////////////////////////////////////////////////////////////////////////////
// utility method that is used by observers to retrieve data after an event
const std::string& LLEmbeddedBrowserWindow::getClickLinkHref()
{
    return mClickHref;
}

////////////////////////////////////////////////////////////////////////////////
// utility method that is used by observers to retrieve data after an event
const std::string& LLEmbeddedBrowserWindow::getClickLinkTarget()
{
    return mClickTarget;
}

////////////////////////////////////////////////////////////////////////////////
// called when the status text is changed - emits event to consumer
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnStatusChange( nsIWebProgress* aWebProgress,
                                                        nsIRequest* aRequest,
                                                            nsresult aStatus,
                                                                const PRUnichar* aMessage )
{
    mStatusText = std::string( NS_ConvertUTF16toUTF8( aMessage ).get() );

    LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mStatusText );
    mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onStatusTextChange, event );

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// implement this if you want to do something when the security state changtes
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnSecurityChange( nsIWebProgress* aWebProgress,
                                                    nsIRequest* aRequest,
                                                        PRUint32 state )
{
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// render a page into memory and grab the window
// TODO: 0,0, browser width, browser height is always passed in right now
//       need to make this work with arbitrary rects (i.e. the dirty rect)
unsigned char* LLEmbeddedBrowserWindow::grabWindow( int xIn, int yIn, int widthIn, int heightIn )
{
    // sanity check
    if ( ! mWebBrowser )
        return 0;

    // only grab the window if it's enabled
    if ( ! mEnabled )
        return false;

    // get the docshell
    nsCOMPtr< nsIDocShell > docShell = do_GetInterface( mWebBrowser );
    if ( ! docShell )
        return PR_FALSE;


    // get pres context
    nsCOMPtr< nsPresContext > presContext;
    nsresult result = docShell->GetPresContext( getter_AddRefs( presContext ) );
    if ( NS_FAILED( result ) || ( ! presContext ) )
        return PR_FALSE;

    // get view manager
    nsIViewManager* viewManager = presContext->GetViewManager();
    if ( ! viewManager )
        return PR_FALSE;

    // get the view
    nsIScrollableView* scrollableView = NULL;
    viewManager->GetRootScrollableView( &scrollableView );
    nsIView* view = NULL;
    if ( scrollableView )
        scrollableView->GetScrolledView( view );
    else
        viewManager->GetRootView( view );

    // get the rectangle we want to render in twips (this looks odd but takees care of scrolling too)
    nsRect rect = view->GetBounds() - view->GetPosition() - view->GetPosition();
    if ( rect.IsEmpty() )
        return 0;

    float p2t = presContext->PixelsToTwips();
    rect.width = NSIntPixelsToTwips( widthIn, p2t );
    rect.height = NSIntPixelsToTwips( heightIn, p2t );

    // render the page
    nsCOMPtr< nsIRenderingContext > context;
    result = viewManager->RenderOffscreen( view, rect, PR_FALSE, PR_FALSE, NS_RGB( mBkgRed, mBkgGreen, mBkgBlue  ), getter_AddRefs( context ) );
    if ( NS_FAILED( result ) )
        return 0;

    // retrieve the surface we rendered to
    nsIDrawingSurface* surface = nsnull;
    context->GetDrawingSurface( &surface );
    if ( ! surface )
        return 0;

    // lock the surface and retrieve a pointer to the rendered data and current row span
    PRUint8* data;
    PRInt32 rowLen;
    // sometime rowspan ! width in pixels * bytes per pixel so save row span value and use in application
    result = surface->Lock( xIn, yIn, widthIn, heightIn, reinterpret_cast< void** >( &data ), &mBrowserRowSpan, &rowLen, NS_LOCK_SURFACE_READ_ONLY );
    if ( NS_FAILED ( result ) )
        return 0;

    // save row span - it *can* change during the life of the app
    mBrowserDepth = rowLen / mBrowserWidth;

    // create memory buffer here so it can be deleted and recreated elsewhere
    if ( ! mPageBuffer )
        mPageBuffer = new unsigned char[ mBrowserRowSpan * mBrowserHeight ];

    // save the pixels and optionally invert them 
    // (it's useful the SL client to get bitmaps that are inverted compared
    // to the way that Mozilla renders them - allow to optionally flip
    if ( mFlipBitmap )
    {
        for( int y = mBrowserHeight - 1; y > -1; --y )
        {
            memcpy( mPageBuffer + y * mBrowserRowSpan, 
                        data + ( mBrowserHeight - y - 1 ) * mBrowserRowSpan, 
                            mBrowserRowSpan );
        }
    }
    else
    {
        memcpy( mPageBuffer, data, mBrowserRowSpan * mBrowserHeight );
    }

    // release and destroy the surface we rendered to
    surface->Unlock();
    context->DestroyDrawingSurface( surface );

    renderCaret();

    return mPageBuffer;
}

////////////////////////////////////////////////////////////////////////////////
// all this just to render a caret!
PRBool LLEmbeddedBrowserWindow::renderCaret()
{
    nsCOMPtr< nsIWebBrowserFocus > focus = do_QueryInterface( mWebBrowser );
    if ( ! focus )
        return NS_ERROR_FAILURE;

    nsCOMPtr< nsIDOMElement > focusedElement;
    focus->GetFocusedElement( getter_AddRefs( focusedElement ) );
    if ( ! focusedElement )
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsIContent> focusedContent = do_QueryInterface( focusedElement );
    if ( ! focusedContent )
        return NS_ERROR_FAILURE;

    nsIPresShell* presShell = focusedContent->GetCurrentDoc()->GetShellAt( 0 );
    if ( ! presShell )
        return NS_ERROR_FAILURE;

    nsCOMPtr< nsICaret > caret;
    presShell->GetCaret( getter_AddRefs( caret ) );
    if ( ! caret )
        return NS_ERROR_FAILURE;

    nsIFrame* frame = nsnull;
    presShell->GetPrimaryFrameFor( focusedContent, &frame );
    if ( ! frame )
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsISelectionController> selCtrl;
    frame->GetSelectionController( presShell->GetPresContext(), getter_AddRefs( selCtrl ) );
    if ( ! selCtrl )
        return NS_ERROR_FAILURE;

    nsCOMPtr<nsISelection> selection;
    selCtrl->GetSelection( nsISelectionController::SELECTION_NORMAL, getter_AddRefs( selection ) );
    if ( ! selection )
        return NS_ERROR_FAILURE;

    PRBool collapsed;
    nsRect coords;
    nsIView* caretView;
    caret->GetCaretCoordinates( nsICaret::eTopLevelWindowCoordinates, selection, &coords, &collapsed, &caretView );
    if ( ! caretView )
        return NS_ERROR_FAILURE;

    float twips2Pixls = presShell->GetPresContext()->TwipsToPixels();

    PRInt32 caretX = NSTwipsToIntPixels( coords.x, twips2Pixls );
    PRInt32 caretY = NSTwipsToIntPixels( coords.y, twips2Pixls );
    PRInt32 caretHeight = NSTwipsToIntPixels( coords.height, twips2Pixls );

    if ( caretX > -1 && caretX < mBrowserWidth && caretY > -1 && caretY < mBrowserHeight )
    {
        if ( mPageBuffer )
        {
            for( int y = 1; y < caretHeight - 1; ++y )
            {
                PRInt32 base_pos = caretY + y;
                if ( mFlipBitmap )
                    base_pos = mBrowserHeight - ( caretY + y );

                // sometimes the caret seems valid when it really isn't - cap it to size of screen
                if ( caretY + y + caretHeight < mBrowserHeight )
                {
                    mPageBuffer[ base_pos * getBrowserRowSpan() + ( caretX + 1 ) * mBrowserDepth + 0 ] = mCaretBlue;
                    mPageBuffer[ base_pos * getBrowserRowSpan() + ( caretX + 1 ) * mBrowserDepth + 1 ] = mCaretGreen;
                    mPageBuffer[ base_pos * getBrowserRowSpan() + ( caretX + 1 ) * mBrowserDepth + 2 ] = mCaretRed;
                }
            }
        }
    }

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// return the buffer that contains the rendered page
unsigned char* LLEmbeddedBrowserWindow::getPageBuffer()
{
    return mPageBuffer;
}

////////////////////////////////////////////////////////////////////////////////
//
PRInt16 LLEmbeddedBrowserWindow::getBrowserWidth()
{
    return mBrowserWidth;
}

////////////////////////////////////////////////////////////////////////////////
//
PRInt16 LLEmbeddedBrowserWindow::getBrowserHeight()
{
    return mBrowserHeight;
}

////////////////////////////////////////////////////////////////////////////////
//
PRInt16 LLEmbeddedBrowserWindow::getBrowserDepth()
{
    return mBrowserDepth;
}

////////////////////////////////////////////////////////////////////////////////
//
PRInt32 LLEmbeddedBrowserWindow::getBrowserRowSpan()
{
    return mBrowserRowSpan;
}

////////////////////////////////////////////////////////////////////////////////
//
PRBool LLEmbeddedBrowserWindow::navigateTo( const std::string uriIn )
{
    if ( mWebNav )
    {
        mWebNav->LoadURI( reinterpret_cast< const PRUnichar* >( NS_ConvertUTF8toUTF16( uriIn.c_str() ).get() ),
            nsIWebNavigation::LOAD_FLAGS_NONE,
                nsnull, nsnull, nsnull );

        return PR_TRUE;
    }

    return PR_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
PRBool LLEmbeddedBrowserWindow::canNavigateBack()
{
    if ( ! mWebNav )
    {
        return PR_FALSE;
    }

    PRBool canGoBack = PR_FALSE;

    nsresult result = mWebNav->GetCanGoBack( &canGoBack );
    if ( NS_FAILED( result ) )
    {
        return PR_FALSE;
    }

    return canGoBack;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::navigateStop()
{
    if ( mWebNav )
        mWebNav->Stop( nsIWebNavigation::STOP_ALL );
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::navigateBack()
{
    if ( mWebNav )
        mWebNav->GoBack();
}

////////////////////////////////////////////////////////////////////////////////
//
PRBool LLEmbeddedBrowserWindow::canNavigateForward()
{
    if ( ! mWebNav )
        return PR_FALSE;

    PRBool canGoForward = PR_FALSE;

    nsresult result = mWebNav->GetCanGoForward( &canGoForward );
    if ( NS_FAILED( result ) )
    {
        return PR_FALSE;
    }

    return canGoForward;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::navigateForward()
{
    if ( mWebNav )
        mWebNav->GoForward();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::navigateReload()
{
    // maybe need a cache version of this too?
    if ( mWebNav )
        mWebNav->Reload( nsIWebNavigation::LOAD_FLAGS_BYPASS_CACHE );
}

////////////////////////////////////////////////////////////////////////////////
// set the size of the browser window
PRBool LLEmbeddedBrowserWindow::setSize( PRInt16 widthIn, PRInt16 heightIn )
{
    if ( mBaseWindow )
    {
        // if there is a buffer already, get rid of it (it will get created as required in grabWindow())
        if ( mPageBuffer )
        {
            delete[] mPageBuffer;
            mPageBuffer = 0;
        }

        // record new size (important: may change after grabWindow() is called);
        mBrowserWidth = widthIn;
        mBrowserHeight = heightIn;
        mBrowserRowSpan = mBrowserWidth * mBrowserDepth;

        // On the Mac, these calls do strange things to the main viewer window, and they don't seem necessary in any case.
        #ifdef WIN32
        // this is the actual OS (on Win32) Window so it needs to be hidden
        mBaseWindow->SetVisibility( PR_FALSE );

        // move WAY off screen (and in a place that makes the combobox hack work)
        mBaseWindow->SetPosition( 8000, -6000 );
        #endif

        // tell Mozilla about the new size
        mBaseWindow->SetSize( widthIn, heightIn, PR_FALSE );

        return PR_TRUE;
    }

    return PR_FALSE;
}

////////////////////////////////////////////////////////////////////////////////
//
PRBool LLEmbeddedBrowserWindow::flipWindow( PRBool flip )
{
    mFlipBitmap = flip;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// higher level mouse event
void LLEmbeddedBrowserWindow::mouseLeftDoubleClick( PRInt16 xPosIn, PRInt16 yPosIn )
{
    // Internally Mozilla represents double-click as a 2-count mouse down event.
    // TODO: support triple-click
    const PRUint32 clickCount = 2;
    sendMozillaMouseEvent( NS_MOUSE_LEFT_BUTTON_DOWN, xPosIn, yPosIn, clickCount );
}

////////////////////////////////////////////////////////////////////////////////
// higher level mouse event
void LLEmbeddedBrowserWindow::mouseDown( PRInt16 xPosIn, PRInt16 yPosIn )
{
    const PRUint32 clickCount = 1;
    sendMozillaMouseEvent( NS_MOUSE_LEFT_BUTTON_DOWN, xPosIn, yPosIn, clickCount );
}

////////////////////////////////////////////////////////////////////////////////
// higher level mouse event
void LLEmbeddedBrowserWindow::mouseUp( PRInt16 xPosIn, PRInt16 yPosIn )
{
    const PRUint32 clickCount = 1;
    sendMozillaMouseEvent( NS_MOUSE_LEFT_BUTTON_UP, xPosIn, yPosIn, clickCount );
}

////////////////////////////////////////////////////////////////////////////////
// higher level mouse event
void LLEmbeddedBrowserWindow::mouseMove( PRInt16 xPosIn, PRInt16 yPosIn )
{
    const PRUint32 clickCount = 1;    // ignored?
    sendMozillaMouseEvent( NS_MOUSE_MOVE, xPosIn, yPosIn, clickCount );
}

////////////////////////////////////////////////////////////////////////////////
// utility methods to set an error message so something else can look at it
void LLEmbeddedBrowserWindow::scrollByLines( PRInt16 linesIn )
{
    if ( mWebBrowser )
    {
        nsCOMPtr< nsIDOMWindow > window;
        nsresult result = mWebBrowser->GetContentDOMWindow( getter_AddRefs( window ) );

        if ( ! NS_FAILED( result ) && window )
        {
            result = window->ScrollByLines( linesIn );
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// synthesizes a mouse event and sends into the embedded instance
// eventIn - NS_MOUSE_LEFT_BUTTON_DOWN, NS_MOUSE_LEFT_BUTTON_UP, etc.
// xPosIn, yPosIn - coordinates (in browser window space)
// clickCountIn - use 1 for single click, 2 for double-click, etc.
PRBool LLEmbeddedBrowserWindow::sendMozillaMouseEvent( PRInt16 eventIn, PRInt16 xPosIn, PRInt16 yPosIn, PRUint32 clickCountIn )
{
    if ( ! mEnabled )
        return PR_FALSE;

    if ( ! mWebBrowser )
        return PR_FALSE;

    nsCOMPtr< nsIDocShell > docShell = do_GetInterface( mWebBrowser );
    if ( ! docShell )
        return PR_FALSE;

    nsCOMPtr< nsPresContext > presContext;
    nsresult result = docShell->GetPresContext( getter_AddRefs( presContext ) );
    if ( NS_FAILED( result ) || ( ! presContext ) )
        return PR_FALSE;

    nsIViewManager* viewManager = presContext->GetViewManager();
    if ( ! viewManager )
        return PR_FALSE;

    nsIView* rootView;
    result = viewManager->GetRootView( rootView );
    if ( NS_FAILED( result ) || ( ! rootView ) )
        return PR_FALSE;

    nsCOMPtr< nsIWidget > widget = rootView->GetWidget();
    if ( ! widget )
        return PR_FALSE;

    nsMouseEvent mouseEvent( PR_TRUE, eventIn, widget, nsMouseEvent::eReal );
    mouseEvent.clickCount = clickCountIn;
    mouseEvent.isShift = 0;
    mouseEvent.isControl = 0;
    mouseEvent.isAlt = 0;
    mouseEvent.isMeta = 0;
    mouseEvent.widget = widget;
    mouseEvent.nativeMsg = nsnull;
    mouseEvent.point.x = xPosIn;
    mouseEvent.point.y = yPosIn;
    mouseEvent.refPoint.x = xPosIn;
    mouseEvent.refPoint.y = yPosIn;
    mouseEvent.flags = 0;

    nsEventStatus status;
    result = viewManager->DispatchEvent( &mouseEvent, &status );
    if ( NS_FAILED( result ) )
        return PR_FALSE;

    return PR_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// higher level keyboard functions

// accept a (mozilla-style) keycode
void LLEmbeddedBrowserWindow::keyPress( PRInt16 keyCode )
{
    sendMozillaKeyboardEvent( 0, keyCode );
}

// accept keyboard input that's already been translated into a unicode char.
void LLEmbeddedBrowserWindow::unicodeInput( PRUint32 uni_char )
{
    sendMozillaKeyboardEvent( uni_char, 0 );
}

////////////////////////////////////////////////////////////////////////////////
// synthesizes a keyboard event and sends into the embedded instance
PRBool LLEmbeddedBrowserWindow::sendMozillaKeyboardEvent( PRUint32 uni_char, PRUint32 ns_vk_code )
{
    if ( ! mEnabled )
        return PR_FALSE;

    if ( ! mWebBrowser )
        return PR_FALSE;

    nsCOMPtr< nsIDocShell > docShell = do_GetInterface( mWebBrowser );
    if ( ! docShell )
        return PR_FALSE;

    nsCOMPtr< nsPresContext > presContext;
    docShell->GetPresContext( getter_AddRefs( presContext ) );
    if ( ! presContext )
        return PR_FALSE;

    nsIViewManager* viewManager = presContext->GetViewManager();
    if ( ! viewManager )
        return PR_FALSE;

    nsIView* rootView;
    viewManager->GetRootView( rootView );
    if ( ! rootView )
        return PR_FALSE;

    nsCOMPtr< nsIWidget > widget = rootView->GetWidget();
    if ( ! widget )
        return PR_FALSE;

    nsKeyEvent keyEvent( PR_TRUE, NS_KEY_PRESS, widget );
    keyEvent.keyCode = ns_vk_code;
    keyEvent.charCode = uni_char;
    keyEvent.isChar = uni_char ? PR_TRUE : PR_FALSE;
    keyEvent.isShift = 0;
    keyEvent.isControl = 0;
    keyEvent.isAlt = 0;
    keyEvent.isMeta = 0;
    keyEvent.widget = widget;
    keyEvent.nativeMsg = nsnull;
    keyEvent.point.x = 0;
    keyEvent.point.y = 0;
    keyEvent.refPoint.x = 0;
    keyEvent.refPoint.y = 0;
    keyEvent.flags = 0;
    
    nsEventStatus status;
    nsresult result = viewManager->DispatchEvent( &keyEvent, &status );
    if ( NS_FAILED( result ) )
        return PR_FALSE;

    return PR_TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// override nsIWebBrowserChrome::HandleEvent ()
NS_IMETHODIMP LLEmbeddedBrowserWindow::HandleEvent( nsIDOMEvent* anEvent )
{
    nsCOMPtr< nsIDOMEventTarget > eventTarget;
    anEvent->GetTarget( getter_AddRefs( eventTarget ) );

    nsCOMPtr<nsIDOMElement> linkElement ( do_QueryInterface ( eventTarget ) );
    if ( linkElement )
    {
        // look for an href link
        nsString name;
        linkElement->GetAttribute( NS_ConvertUTF8toUTF16( "href" ), name );
        mClickHref = std::string( NS_ConvertUTF16toUTF8( name ).get() );

        // look for a target element
        linkElement->GetAttribute( NS_ConvertUTF8toUTF16( "target" ), name );
        mClickTarget = std::string( NS_ConvertUTF16toUTF8( name ).get() );

        // if the href link contains something
        if ( mClickHref.length() )
        {
            LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), mClickHref, mClickTarget );
            mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onClickLinkHref, event );
        }
    }

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
// override nsIURIContentListener methods
NS_IMETHODIMP LLEmbeddedBrowserWindow::OnStartURIOpen( nsIURI *aURI, PRBool *_retval )
{
    nsCAutoString newURI;
    aURI->GetSpec( newURI );
    std::string rawUri = newURI.get();

    // are we navigating to a 'nofollow' link
    if ( mNoFollowScheme.length() && rawUri.substr( 0, mNoFollowScheme.length() ) == mNoFollowScheme )
    {
        LLEmbeddedBrowserWindowEvent event( getWindowId(), rawUri, rawUri );
        mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onClickLinkNoFollow, event );

        // tell browser we're handling things and don't follow link
        *_retval = PR_TRUE;
    }
    else
    {
        // tell browser to proceed as normal
        *_retval = PR_FALSE;
    }

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::setNoFollowScheme( std::string schemeIn )
{
    mNoFollowScheme = schemeIn;
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLEmbeddedBrowserWindow::getNoFollowScheme()
{
    return mNoFollowScheme;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::DoContent( const char *aContentType,
                                                    PRBool aIsContentPreferred,
                                                        nsIRequest *aRequest,
                                                            nsIStreamListener **aContentHandler,
                                                                PRBool *_retval )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::IsPreferred( const char *aContentType,
                                                        char **aDesiredContentType,
                                                            PRBool *_retval )
{
    // important (otherwise, links try to open in a new window and trigger the window watcher code)
    *_retval = PR_TRUE;
    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::CanHandleContent( const char *aContentType,
                                                            PRBool aIsContentPreferred,
                                                                char **aDesiredContentType,
                                                                    PRBool *_retval )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::GetLoadCookie( nsISupports * *aLoadCookie )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::SetLoadCookie( nsISupports * aLoadCookie )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::GetParentContentListener( nsIURIContentListener** aParentContentListener )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
//
NS_IMETHODIMP LLEmbeddedBrowserWindow::SetParentContentListener( nsIURIContentListener* aParentContentListener )
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

////////////////////////////////////////////////////////////////////////////////
// give focus to the browser so that input keyboard events work
void LLEmbeddedBrowserWindow::focusBrowser( PRBool focusBrowserIn )
{
    if ( mWebBrowser )
    {
        if ( focusBrowserIn )
        {
            nsCOMPtr< nsIWebBrowserFocus > focus( do_GetInterface( mWebBrowser ) );
            focus->Activate();
        }
        else
        {
            nsCOMPtr< nsIWebBrowserFocus > focus( do_GetInterface( mWebBrowser ) );
            focus->Deactivate();
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowserWindow::setWindowId( int windowIdIn )
{
    mWindowId = windowIdIn;
}

////////////////////////////////////////////////////////////////////////////////
//
int LLEmbeddedBrowserWindow::getWindowId()
{
    //printf("## Getting id for %p and it is %d\n", this, mWindowId );

    return mWindowId;
}

////////////////////////////////////////////////////////////////////////////////
// add / remove the toolkit (and therefore 'page changed') observer
PRBool LLEmbeddedBrowserWindow::enableToolkitObserver( PRBool enableIn )
{
    //TODO: AddObserver fails if I grab the toolkit this way.
    //static NS_DEFINE_CID(kNS_TOOLKIT_CID, NS_TOOLKIT_CID);
    //nsresult result1;
    //nsCOMPtr< nsIToolkit > toolkit = do_GetService( kNS_TOOLKIT_CID, &result1 );
    //if ( ( result1 == NS_OK ) && toolkit )
    //{
    //    if ( toolkit->AddObserver( this ) )
    //    {
    //        return true;
    //    }
    //}
    //return false;

    // TODO: this is horrible but seems to work - need a better way to get the toolkit
    nsCOMPtr< nsIDocShell > docShell = do_GetInterface( mWebBrowser );
    if ( ! docShell )
        return false;

    nsCOMPtr< nsPresContext > presContext;
    nsresult result = docShell->GetPresContext( getter_AddRefs( presContext ) );
    if ( NS_FAILED( result ) || ( ! presContext ) )
        return false;

    nsIViewManager* viewManager = presContext->GetViewManager();
    if ( ! viewManager )
        return false;

    nsIView* rootView;
    result = viewManager->GetRootView( rootView );
    if ( NS_FAILED( result ) || ( ! rootView ) )
        return false;

    nsCOMPtr< nsIWidget > widget = rootView->GetWidget();
    if ( ! widget )
        return false;

    nsCOMPtr< nsIToolkit > mToolkit = widget->GetToolkit();
    if ( ! mToolkit )
        return false;

#ifdef NS_DECL_NSITOOLKITOBSERVER     
    if ( enableIn )
        mToolkit->AddObserver( this );
    else
        mToolkit->RemoveObserver( this );
#endif
    return true;
}

////////////////////////////////////////////////////////////////////////////////
// something on the page changed - e.g. a new page loaded, scrolling, user
// input or as the result of some kind of animation.
// NOTE: we don't call grabWindow here as this can stall graphics apps -
//       we merely pass back an event to the app and let it decide when/if
//       to call grabWindow()
NS_METHOD LLEmbeddedBrowserWindow::NotifyInvalidated( nsIWidget *aWidget, PRInt32 x, PRInt32 y, PRInt32 width, PRInt32 height )
{
//    printf("LLEmbeddedBrowserWindow::NotifyInvalidated(%p, %d, %d, %d, %d)\n", (void*)aWidget, (int)x, (int)y, (int)width, (int)height);

    // try to match widget-window against ourselves to see if we need to update the texture
    // only works using native widgets (on Windows) at the moment - needs to be moved to platform agnostic code ASAP
    #ifdef WIN32

    // this is horrible beyond words but it seems to work...
    // nsToolkit tells us that a widget changed and we need to see if it's this instance
    // so we can emit an event that causes the parent app to update the browser texture
    nsIWidget* mainWidget;
    mBaseWindow->GetMainWidget( &mainWidget );

    HWND nativeWidget = (HWND)aWidget->GetNativeData( NS_NATIVE_WIDGET );
    HWND nativeWidgetChild = 0;
    while ( ::GetParent( nativeWidget ) )
    {
        nativeWidgetChild = nativeWidget;
        nativeWidget = ::GetParent( nativeWidget );
    }

    if ( ( (HWND)mainWidget->GetNativeData( NS_NATIVE_WIDGET ) ) == nativeWidgetChild )
    {
        LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), x, y, width, height );
        mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onPageChanged, event );
    }

    // other platforms will always update - desperately inefficient but you'll see something.
    #else
        LLEmbeddedBrowserWindowEvent event( getWindowId(), getCurrentUri(), x, y, width, height );
        mEventEmitter.update( &LLEmbeddedBrowserWindowObserver::onPageChanged, event );
    #endif

    return NS_OK;
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLEmbeddedBrowserWindow::evaluateJavascript( std::string scriptIn )
{
    nsCOMPtr< nsIScriptGlobalObjectOwner > theGlobalObjectOwner( do_GetInterface( mWebBrowser ) );

    if ( theGlobalObjectOwner )
    {
        nsIScriptGlobalObject* theGlobalObject;
        theGlobalObject = theGlobalObjectOwner->GetScriptGlobalObject();

        nsIScriptContext* theScriptContext = theGlobalObject->GetContext();

        PRBool IsUndefined;
        nsString output;
        nsresult result = theScriptContext->EvaluateString(NS_ConvertUTF8toUTF16(scriptIn.c_str()),
           nsnull, nsnull, "", 1, nsnull, &output, &IsUndefined);

        if( NS_FAILED( result ) )
            return "";

        return std::string( NS_ConvertUTF16toUTF8( output ).get() );
    }

    return "";
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowserWindow::set404RedirectUrl( std::string redirect_url )
{
    m404RedirectUrl = redirect_url;

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowserWindow::clr404RedirectUrl()
{
    m404RedirectUrl = std::string( "" );

    return true;
}

// #define required by this file for LibXUL/Mozilla code to avoid crashes in their debug code
#ifdef _DEBUG
    #ifdef WIN32
        #undef DEBUG
    #endif
#endif
