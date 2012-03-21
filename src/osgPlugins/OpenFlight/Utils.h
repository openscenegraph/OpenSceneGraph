/*
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#ifndef __FLTEXP_UTILS_H__
#define __FLTEXP_UTILS_H__ 1


// FLTEXP_DELETEFILE macro is used to delete temp files created during file export.
// (Too bad OSG doesn't use Boost.)

#if defined(_WIN32)

    #include <windows.h>
    #define FLTEXP_DELETEFILE(file) DeleteFile((file))

#else   // Unix

    #include <stdio.h>
    #define FLTEXP_DELETEFILE(file) remove((file))

#endif

#endif
