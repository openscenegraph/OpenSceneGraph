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

#ifndef LLEMBEDDEDBROWSER_H
#define LLEMBEDDEDBROWSER_H

#include <string>
#include <map>

class LLEmbeddedBrowserWindow;
class LLEmbeddedBrowserWindowObserver;

class LLEmbeddedBrowser
{
    public:
        LLEmbeddedBrowser();
        virtual ~LLEmbeddedBrowser();

        static LLEmbeddedBrowser* getInstance();

        bool init( std::string applicationDir, std::string componentDir, std::string profileDir, void* nativeWindowHandleIn );
        bool reset();
        bool clearCache();
        bool enableProxy( bool proxyEnabledIn, std::string proxyHostNameIn, int proxyPortIn );
        bool enableCookies( bool enabledIn );
        bool clearAllCookies();
        bool enablePlugins( bool enabledIn );
        std::string getGREVersion();
        void setBrowserAgentId( std::string idIn );
        LLEmbeddedBrowserWindow* createBrowserWindow( int browserWidthIn, int browserHeightIn );
        bool destroyBrowserWindow( LLEmbeddedBrowserWindow* browserWindowIn );
        void setLastError( int errorNumIn );
        void clearLastError();
        int getLastError();

    private:
        static LLEmbeddedBrowser* sInstance;
        void* mNativeWindowHandle;
        int mErrorNum;
};

#endif    // LLEMBEDDEDBROWSER_H

