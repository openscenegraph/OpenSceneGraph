/*
 *  QTUtils.h
 *  NativeContext
 *
 *  Created by Stephan Huber on Fri Sep 06 2002.
 *  Copyright (c) 2002 digital mind. All rights reserved.
 *
 */

#include <Carbon/Carbon.h>
#include <QuickTime/QuickTime.h>

namespace osgQuicktime {

	void initQuicktime();

	OSStatus MakeFSSpecFromPath(const char* path, FSSpec* spec);
	OSStatus MakeMovieFromPath(const char* path, Movie* movie);

}
