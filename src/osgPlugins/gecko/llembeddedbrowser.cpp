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

// Windows specific switches
#ifdef WIN32
    // appears to be required by LibXUL/Mozilla code to avoid crashes in debug versions of their code (undef'd at end of this file)
    #ifdef _DEBUG
        #define DEBUG 1
    #endif
#endif    // WIN32

#include <iostream>

#include "llembeddedbrowser.h"
#include "llembeddedbrowserwindow.h"

#ifdef WIN32
    #pragma warning( disable : 4265 )    // "class has virtual functions, but destructor is not virtual"
    #pragma warning( disable : 4291 )    // (no matching operator delete found; memory will not be freed if initialization throws an exception)
#endif    // WIN32

#include "nsBuildID.h"
#include "nsICacheService.h"
#include "nsICookieManager.h"
#include "nsIPref.h"
#include "nsNetCID.h"
#include "nsProfileDirServiceProvider.h"
#include "nsXULAppAPI.h"
#include "nsIAppShell.h"
#include "nsIPromptService.h"
#include "time.h"
#include "nsWidgetsCID.h"
#include "nsNetCID.h"

static nsIAppShell *sAppShell = nsnull;

// singleton pattern - initialization
LLEmbeddedBrowser* LLEmbeddedBrowser::sInstance = 0;

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowser::LLEmbeddedBrowser() :
    mNativeWindowHandle( 0 ),
    mErrorNum( 0 )
{
}

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowser::~LLEmbeddedBrowser()
{
}

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowser* LLEmbeddedBrowser::getInstance()
{
    if ( ! sInstance )
    {
        sInstance = new LLEmbeddedBrowser;
    }

    return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowser::setLastError( int errorNumIn )
{
    mErrorNum = errorNumIn;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowser::clearLastError()
{
    mErrorNum = 0x0000;
}

////////////////////////////////////////////////////////////////////////////////
//
int LLEmbeddedBrowser::getLastError()
{
    return mErrorNum;
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLEmbeddedBrowser::getGREVersion()
{
    // take the string directly from Mozilla
    return std::string( GRE_BUILD_ID );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::init( std::string applicationDir,
                              std::string componentDir,
                              std::string profileDir,
                              void* nativeWindowHandleIn )
{
    mNativeWindowHandle = nativeWindowHandleIn;

    NS_ConvertUTF8toUTF16 applicationDirUTF16(applicationDir.c_str());
    NS_ConvertUTF8toUTF16 componentDirUTF16(componentDir.c_str());
    NS_ConvertUTF8toUTF16 profileDirUTF16(profileDir.c_str());

    nsCOMPtr< nsILocalFile > applicationDirNative;
    nsresult result = NS_NewLocalFile( applicationDirUTF16, PR_FALSE, getter_AddRefs( applicationDirNative ) );
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1000 );
        return false;
    };

    nsCOMPtr< nsILocalFile > componentDirNative;
    result = NS_NewLocalFile( componentDirUTF16 , PR_FALSE, getter_AddRefs( componentDirNative ) );
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1001 );
        return false;
    };

    result = XRE_InitEmbedding( componentDirNative, applicationDirNative, nsnull, nsnull, 0 );
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1002 );
        return false;
    };

    nsCOMPtr< nsILocalFile > profileDirNative;
    result = NS_NewLocalFile( profileDirUTF16 , PR_TRUE, getter_AddRefs( profileDirNative ) );
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1007 );
        return false;
    };
    nsCOMPtr< nsProfileDirServiceProvider > locProvider;
    NS_NewProfileDirServiceProvider( PR_TRUE, getter_AddRefs( locProvider ) );
    if ( ! locProvider )
    {
        setLastError( 0x1003 );
        XRE_TermEmbedding();
        return PR_FALSE;
    };

    result = locProvider->Register();
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1004 );
        XRE_TermEmbedding();
        return PR_FALSE;
    };

    result = locProvider->SetProfileDir( profileDirNative );
    if ( NS_FAILED( result ) )
    {
        setLastError( 0x1005 );
        XRE_TermEmbedding();
        return PR_FALSE;
    };

    nsCOMPtr<nsIPref> pref = do_CreateInstance( NS_PREF_CONTRACTID );
    if ( pref )
    {
        pref->SetBoolPref( "security.warn_entering_secure", PR_FALSE );
        pref->SetBoolPref( "security.warn_entering_weak", PR_FALSE );
        pref->SetBoolPref( "security.warn_leaving_secure", PR_FALSE );
        pref->SetBoolPref( "security.warn_submit_insecure", PR_FALSE );
        pref->SetBoolPref( "network.protocol-handler.warn-external-default", PR_FALSE );
    }
    else
    {
        setLastError( 0x1006 );
    };

    // disable proxy by default
    enableProxy( false, "", 0 );

    // Originally from Linux version but seems to help other platforms too
    nsresult rv;
    nsCOMPtr<nsIAppShell> appShell;
    NS_DEFINE_CID(kAppShellCID, NS_APPSHELL_CID);
    appShell = do_CreateInstance(kAppShellCID, &rv);
    if (!appShell)
    {
        setLastError( 0x1008 );
        return PR_FALSE;
    }
    sAppShell = appShell.get();
    NS_ADDREF(sAppShell);
    sAppShell->Create(0, nsnull);
    sAppShell->Spinup();

    clearLastError();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::reset()
{
    XRE_TermEmbedding();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::clearCache()
{
    nsCOMPtr< nsICacheService > cacheService = do_GetService( NS_CACHESERVICE_CONTRACTID );
    if (! cacheService)
        return false;

    cacheService->EvictEntries( nsICache::STORE_ANYWHERE );

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::enableProxy( bool proxyEnabledIn, std::string proxyHostNameIn, int proxyPortIn )
{
    nsCOMPtr<nsIPref> pref = do_CreateInstance( NS_PREF_CONTRACTID );
    if ( pref )
    {
        if ( proxyEnabledIn )
            pref->SetIntPref( "network.proxy.type", 1 );
        else
            pref->SetIntPref( "network.proxy.type", 0 );

        pref->SetCharPref( "network.proxy.ssl", proxyHostNameIn.c_str() );
        pref->SetIntPref( "network.proxy.ssl_port", proxyPortIn );

        pref->SetCharPref( "network.proxy.ftp", proxyHostNameIn.c_str() );
        pref->SetIntPref( "network.proxy.ftp_port", proxyPortIn );

        pref->SetCharPref( "network.proxy.gopher", proxyHostNameIn.c_str() );
        pref->SetIntPref( "network.proxy.gopher_port", proxyPortIn );

        pref->SetCharPref( "network.proxy.http", proxyHostNameIn.c_str() );
        pref->SetIntPref( "network.proxy.http_port", proxyPortIn );

        pref->SetBoolPref( "network.proxy.share_proxy_settings", true );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::enableCookies( bool enabledIn )
{
    nsCOMPtr<nsIPref> pref = do_CreateInstance( NS_PREF_CONTRACTID );
    if ( pref )
    {
        if ( enabledIn )
            pref->SetIntPref( "network.cookie.cookieBehavior", 0 );
        else
            pref->SetIntPref( "network.cookie.cookieBehavior", 2 );

        return true;
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::clearAllCookies()
{
    nsCOMPtr< nsICookieManager > cookieManager = do_GetService( NS_COOKIEMANAGER_CONTRACTID );
    if ( ! cookieManager )
        return false;

    cookieManager->RemoveAll();

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::enablePlugins( bool enabledIn )
{
    nsCOMPtr<nsIPref> pref = do_CreateInstance( NS_PREF_CONTRACTID );
    if ( pref )
    {
        if ( enabledIn )
        {
            pref->SetBoolPref( "plugin.scan.plid.all", PR_TRUE );
            pref->SetBoolPref( "xpinstall-enabled", PR_TRUE );
        }
        else
        {
            pref->SetBoolPref( "plugin.scan.plid.all", PR_FALSE );
            pref->SetBoolPref( "xpinstall-enabled", PR_FALSE );
            pref->SetBoolPref( "plugin.scan.4xPluginFolder", PR_FALSE );
            pref->SetCharPref( "plugin.scan.Quicktime", "20.0" );
            pref->SetCharPref( "plugin.scan.Acrobat", "99.0" );
            pref->SetCharPref( "plugin.scan.SunJRE", "99.0" );
            pref->SetCharPref( "plugin.scan.WindowsMediaPlayer", "99.0" );
        };

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLEmbeddedBrowser::setBrowserAgentId( std::string idIn )
{
    nsCOMPtr<nsIPref> pref = do_CreateInstance( NS_PREF_CONTRACTID );
    if ( pref )
    {
        pref->SetCharPref( "general.useragent.extra.* ", idIn.c_str() );
    };
}

////////////////////////////////////////////////////////////////////////////////
//
LLEmbeddedBrowserWindow* LLEmbeddedBrowser::createBrowserWindow( int browserWidthIn, int browserHeightIn )
{
    nsCOMPtr< nsIWebBrowserChrome > chrome;

    LLEmbeddedBrowserWindow* newWin = new LLEmbeddedBrowserWindow();
    if ( ! newWin )
    {
        return 0;
    };

    nsIWebBrowserChrome** aNewWindow = getter_AddRefs( chrome );

    CallQueryInterface( NS_STATIC_CAST( nsIWebBrowserChrome*, newWin ), aNewWindow );

    NS_ADDREF( *aNewWindow );

    newWin->SetChromeFlags( nsIWebBrowserChrome::CHROME_ALL );

    nsCOMPtr< nsIWebBrowser > newBrowser;

    newWin->createBrowser( mNativeWindowHandle, browserWidthIn, browserHeightIn, getter_AddRefs( newBrowser ) );
    if ( ! newBrowser )
    {
        return 0;
    };

    if ( newWin && chrome )
    {
        newWin->setParent( this );
        nsCOMPtr< nsIWebBrowser > newBrowser;
        chrome->GetWebBrowser( getter_AddRefs( newBrowser ) );
        nsCOMPtr< nsIWebNavigation > webNav( do_QueryInterface ( newBrowser ) );
        webNav->LoadURI( NS_ConvertUTF8toUTF16( "about:blank" ).get(), nsIWebNavigation::LOAD_FLAGS_NONE, nsnull, nsnull, nsnull );

        clearLastError();

        return newWin;
    };

    setLastError( 0x2001 );
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
bool LLEmbeddedBrowser::destroyBrowserWindow( LLEmbeddedBrowserWindow* browserWindowIn )
{
    nsCOMPtr< nsIWebBrowser > webBrowser;
    nsCOMPtr< nsIWebNavigation > webNavigation;

    browserWindowIn->GetWebBrowser( getter_AddRefs( webBrowser ) );
    webNavigation = do_QueryInterface( webBrowser );
    if ( webNavigation )
    {
        webNavigation->Stop( nsIWebNavigation::STOP_ALL );
    };

    nsCOMPtr< nsIWebBrowser > browser = nsnull;
    browserWindowIn->GetWebBrowser( getter_AddRefs( browser ) );
    nsCOMPtr< nsIBaseWindow > browserAsWin = do_QueryInterface( browser );
    if ( browserAsWin )
    {
        browserAsWin->Destroy();
    };


    browserWindowIn->SetWebBrowser( nsnull );

    NS_RELEASE( browserWindowIn );

    delete browserWindowIn;

    clearLastError();

    return true;
}

// Windows specific switches
#ifdef WIN32
    #pragma warning( 3 : 4291 ) // (no matching operator delete found; memory will not be freed if initialization throws an exception)
    #pragma warning( 3 : 4265 )    // "class has virtual functions, but destructor is not virtual"

    // #define required by this file for LibXUL/Mozilla code to avoid crashes in their debug code
    #ifdef _DEBUG
        #undef DEBUG
    #endif

#endif    // WIN32
