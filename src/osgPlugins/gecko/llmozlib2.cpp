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

#include <sstream>
#include <iostream>
#include <iomanip>
#include <time.h>

#include "llmozlib2.h"

#include "llembeddedbrowser.h"
#include "llembeddedbrowserwindow.h"

LLMozLib* LLMozLib::sInstance = 0;

////////////////////////////////////////////////////////////////////////////////
//
LLMozLib::LLMozLib() :
    mMaxBrowserWindows( 16 )
{
}

////////////////////////////////////////////////////////////////////////////////
//
LLMozLib* LLMozLib::getInstance()
{
    if ( ! sInstance )
    {
        sInstance = new LLMozLib;
    };

    return sInstance;
}

////////////////////////////////////////////////////////////////////////////////
//
LLMozLib::~LLMozLib()
{
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::init( std::string applicationDir, std::string componentDir, std::string profileDir, void* nativeWindowHandleIn )
{
    return LLEmbeddedBrowser::getInstance()->init( applicationDir,
                                                       componentDir,
                                                       profileDir,
                                                       nativeWindowHandleIn );
}

////////////////////////////////////////////////////////////////////////////////
//
int LLMozLib::getLastError()
{
    return LLEmbeddedBrowser::getInstance()->getLastError();
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::reset()
{
    return LLEmbeddedBrowser::getInstance()->reset();
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::clearCache()
{
    return LLEmbeddedBrowser::getInstance()->clearCache();
}

////////////////////////////////////////////////////////////////////////////////
//
const std::string LLMozLib::getVersion()
{
    const int majorVersion = 2;
    const int minorVersion = 1;

    // number of hours since "time began" for this library - used to identify builds of same version
    const int magicNumber = static_cast< int >( ( time( NULL ) / 3600L  ) - ( 321190L ) );

    // return as a string for now - don't think we need to expose actual version numbers
    std::ostringstream codec;
    codec << std::setw( 1 ) << std::setfill( '0' );
    codec << majorVersion << ".";
    codec << std::setw( 2 ) << std::setfill( '0' );
    codec << minorVersion << ".";
    codec << std::setw( 5 ) << std::setfill( '0' );
    codec << magicNumber;
    codec << " (Mozilla GRE version ";
    codec << LLEmbeddedBrowser::getInstance()->getGREVersion();
    codec << ")";

    return codec.str();
}

////////////////////////////////////////////////////////////////////////////////
//
void LLMozLib::setBrowserAgentId( std::string idIn )
{
    LLEmbeddedBrowser::getInstance()->setBrowserAgentId( idIn );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::enableProxy( bool proxyEnabledIn, std::string proxyHostNameIn, int proxyPortIn )
{
    return LLEmbeddedBrowser::getInstance()->enableProxy( proxyEnabledIn, proxyHostNameIn, proxyPortIn );
}

////////////////////////////////////////////////////////////////////////////////
//
int LLMozLib::createBrowserWindow( int browserWindowWidthIn, int browserWindowHeightIn )
{
    LLEmbeddedBrowserWindow* browserWindow = LLEmbeddedBrowser::getInstance()->createBrowserWindow( browserWindowWidthIn, browserWindowHeightIn );

    if ( browserWindow )
    {
        // arbitrary limit so we don't exhaust system resources
        int id( 0 );
        while ( ++id < mMaxBrowserWindows )
        {
            std::pair< BrowserWindowMapIter, bool > result = mBrowserWindowMap.insert( std::make_pair( id, browserWindow ) );

            // find first place the insert succeeds and use that index as the id
            if ( result.second )
            {
                browserWindow->setWindowId( id );

                return id;
            };
        };
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::destroyBrowserWindow( int browserWindowIdIn )
{
    // don't use the utility method here since we need the iterator to remove the entry from the map
    BrowserWindowMapIter iter = mBrowserWindowMap.find( browserWindowIdIn );
    LLEmbeddedBrowserWindow* browserWindow = (*iter).second;

    if ( browserWindow )
    {
        LLEmbeddedBrowser::getInstance()->destroyBrowserWindow( browserWindow );
    };

    mBrowserWindowMap.erase( iter );

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::setBackgroundColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->setBackgroundColor( redIn, greenIn, blueIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::setCaretColor( int browserWindowIdIn, const int redIn, const int greenIn, const int blueIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->setCaretColor( redIn, greenIn, blueIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::setEnabled( int browserWindowIdIn, bool enabledIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->setEnabled( enabledIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::setSize( int browserWindowIdIn, int widthIn, int heightIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->setSize( widthIn, heightIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::scrollByLines( int browserWindowIdIn, int linesIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->scrollByLines( linesIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::addObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->addObserver( subjectIn );
    };

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::remObserver( int browserWindowIdIn, LLEmbeddedBrowserWindowObserver* subjectIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->remObserver( subjectIn );
    };

    return true;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::navigateTo( int browserWindowIdIn, const std::string uriIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->navigateTo( uriIn ) ? true : false;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::navigateStop( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->navigateStop();

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::canNavigateBack( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->canNavigateBack() ? true : false;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::navigateBack( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->navigateBack();

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::canNavigateForward( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->canNavigateForward() ? true : false;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::navigateForward( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->navigateForward();

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::navigateReload( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->navigateReload();

        return true;
    };

    return false;
}

///////////////////////////////////////////////////////////////////////////////
//
const unsigned char* LLMozLib::grabBrowserWindow( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->grabWindow( 0, 0, getBrowserWidth( browserWindowIdIn ), getBrowserHeight( browserWindowIdIn ) );
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
const unsigned char* LLMozLib::getBrowserWindowPixels( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getPageBuffer();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
const bool LLMozLib::flipWindow( int browserWindowIdIn, bool flipIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->flipWindow( flipIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
const int LLMozLib::getBrowserWidth( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getBrowserWidth();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
const int LLMozLib::getBrowserHeight( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getBrowserHeight();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
const int LLMozLib::getBrowserDepth( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getBrowserDepth();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
const int LLMozLib::getBrowserRowSpan( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getBrowserRowSpan();
    };

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::mouseDown( int browserWindowIdIn, int xPosIn, int yPosIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->mouseDown( xPosIn, yPosIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::mouseUp( int browserWindowIdIn, int xPosIn, int yPosIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->mouseUp( xPosIn, yPosIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::mouseMove( int browserWindowIdIn, int xPosIn, int yPosIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->mouseMove( xPosIn, yPosIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::mouseLeftDoubleClick( int browserWindowIdIn, int xPosIn, int yPosIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->mouseLeftDoubleClick( xPosIn, yPosIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::keyPress( int browserWindowIdIn, int keyCodeIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->keyPress( keyCodeIn );

        return true;
    };

    return false;
}

bool LLMozLib::unicodeInput( int browserWindowIdIn, unsigned long uni_char )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->unicodeInput( uni_char );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::focusBrowser( int browserWindowIdIn, bool focusBrowserIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->focusBrowser( focusBrowserIn );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
void LLMozLib::setNoFollowScheme( int browserWindowIdIn, std::string schemeIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        browserWindow->setNoFollowScheme( schemeIn );
    };
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLMozLib::getNoFollowScheme( int browserWindowIdIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->getNoFollowScheme();
    };

    return ( "" );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::enableCookies( bool enabledIn )
{
    return LLEmbeddedBrowser::getInstance()->enableCookies( enabledIn );
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::clearAllCookies()
{
    return LLEmbeddedBrowser::getInstance()->clearAllCookies();
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::enablePlugins( bool enabledIn )
{
    return LLEmbeddedBrowser::getInstance()->enablePlugins( enabledIn );
}

////////////////////////////////////////////////////////////////////////////////
//
std::string LLMozLib::evaluateJavascript( int browserWindowIdIn, const std::string scriptIn )
{
    LLEmbeddedBrowserWindow* browserWindow = getBrowserWindowFromWindowId( browserWindowIdIn );
    if ( browserWindow )
    {
        return browserWindow->evaluateJavascript( scriptIn );
    };

    return "";
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::set404RedirectUrl(  int browser_window_in, std::string redirect_url )
{
    LLEmbeddedBrowserWindow* browser_window = getBrowserWindowFromWindowId( browser_window_in );
    if ( browser_window )
    {
        browser_window->set404RedirectUrl( redirect_url );

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
//
bool LLMozLib::clr404RedirectUrl( int browser_window_in )
{
    LLEmbeddedBrowserWindow* browser_window = getBrowserWindowFromWindowId( browser_window_in );
    if ( browser_window )
    {
        browser_window->clr404RedirectUrl();

        return true;
    };

    return false;
}

////////////////////////////////////////////////////////////////////////////////
// utility method to get an LLEmbeddedBrowserWindow* from a window id (int)
LLEmbeddedBrowserWindow* LLMozLib::getBrowserWindowFromWindowId( int browserWindowIdIn )
{
    BrowserWindowMapIter iter = mBrowserWindowMap.find( browserWindowIdIn );

    if ( iter != mBrowserWindowMap.end() )
        return ( *iter ).second;
    else
        return 0;
}

