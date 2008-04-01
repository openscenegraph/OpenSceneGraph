/* -*-c++-*- OpenThreads library, Copyright (C) 2002 - 2007  The Open Thread Group
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

//
//
// Win32BarrierPrivateData.h - private data structure for barrier
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
//
#ifndef _HandleHolder_H_
#define _HandleHolder_H_

#ifndef _WINDOWS_
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

/************************************************************************/
/* Class that holds HANDLES ensuring proper destruction                 */
/* It is design decision to make this class noncopyable.                */    
/* It makes design much cleaner. If one wants to copy handle than one   */
/* can do hv.set(DuplicateHandle(....))                                 */
/************************************************************************/
namespace OpenThreads {

    class HandleHolder{
    private:
        HANDLE _handle;

        inline void close(){
            if( _handle != INVALID_HANDLE_VALUE) CloseHandle(_handle);
            _handle = INVALID_HANDLE_VALUE;
        };

        // copy constructor - disallow
        HandleHolder(const HandleHolder& rhs);

        // assignment operator - disallow
        HandleHolder& operator=(const HandleHolder& rhs );

    public:
        // constructor
        HandleHolder()
            :_handle(INVALID_HANDLE_VALUE)
        {};
        
        // constructor from HANDLE  
        explicit HandleHolder(HANDLE h)
            :_handle(h)
        {};

        // destructor - CloseHandle()
        ~HandleHolder()
        {
            close();
        };

        // accessor    
        const HANDLE& get() const { 
            return _handle;
        }
        
        // mutator
        void set (HANDLE h) { 
            if( _handle != INVALID_HANDLE_VALUE ) close();
            _handle = h;
        }

        operator bool (){
            return _handle != INVALID_HANDLE_VALUE && _handle != NULL;
        };

    };

} // namespace




#endif
