/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2011 Robert Osfield
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

#include <osgViewer/GraphicsWindow>
#include <osgViewer/View>
#include <osgViewer/ViewerBase>

using namespace osgViewer;


void GraphicsWindow::getViews(Views& views)
{
    views.clear();
    osgViewer::View *prev = NULL;

    for(Cameras::iterator it = _cameras.begin();
        it != _cameras.end();
        it++)
    {
        osgViewer::View *v = dynamic_cast<osgViewer::View*>((*it)->getView());
        if (v)
            // perform a simple test to reduce the number of duplicates
            if (v != prev)
                // append view
                views.push_back(v);
    }

    // remove duplicates
    views.sort();
    views.unique();
}

void GraphicsWindow::requestRedraw()
{
    Views views;
    getViews(views);

    if (views.empty())
    {
        OSG_INFO << "GraphicsWindow::requestRedraw(): No views assigned yet." << std::endl;
        return;
    }

    for(Views::iterator it = views.begin();
        it != views.end();
        it++)
    {
        (*it)->requestRedraw();
    }
}
