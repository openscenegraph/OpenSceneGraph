/* **************************************************************************
 * OpenSceneGraph loader for Terrapage format database
 *
 * That loader is redistributed under the terms listed on Terrain Experts
 * website (www.terrex.com/www/pages/technology/technologypage.htm)
 *
 * "TerraPage is provided as an Open Source format for use by anyone...
 * We supply the TerraPage C++ source code free of charge.  Anyone
 * can use it and redistribute it as needed (including our competitors).
 * We do, however, ask that you keep the TERREX copyrights intact."
 *
 * Copyright Terrain Experts Inc. 1999.
 * All Rights Reserved.
 *
 *****************************************************************************/

#ifndef _TRPAGEVIEWER_H_
#define _TRPAGEVIEWER_H_

#include <osg/Light>
#include <osg/NodeVisitor>
#include <osg/Geode>
#include <osg/Timer>
#include <osg/DisplaySettings>

#include <osgGLUT/Viewer>

#include <osgUtil/SceneView>

#include <osgGLUT/Window>

#include <string>

#include <osgTXP/trPagePageManager.h>

namespace txp
{
	/* Paging Viewer
		Variant of the regular viewer which knows to call
		the Page Manager at the beginning of each app() phase.
	 */
	class PagingViewer : public osgGLUT::Viewer {
	public:
		PagingViewer();
		bool Init(OSGPageManager *,txp::OSGPageManager::ThreadMode = txp::OSGPageManager::ThreadNone);

        // called on each frame redraw..return the time in ms for each operation.
        virtual float app(unsigned int viewport);
		// The default Viewer resets the cameras at the beginning of the run()
		//  This is annoying.
		bool run();
	protected:
		OSGPageManager *pageManage;
	};
};

#endif
