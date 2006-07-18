/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

#ifndef OSG_GEOCLIPREGION
#define OSG_GEOCLIPREGION 1

#include <osg/Group>
#include <osg/Stencil>



/** A ClipRegion is a group node for which all children are clipped
  * by the projection into screen coordinates of the ClipGeode.
  * Used for cutouts in instrumentation.
  *
  * 
*/
class GeoClipRegion : public osg::Group
{
    public :

        GeoClipRegion(int bin=osg::StateSet::TRANSPARENT_BIN+3);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        GeoClipRegion(const GeoClipRegion&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

	// clip nodes define a screen region that is protected  
		void addClipNode(osg::Node *gd);
   /* clipped children are only drawn inside the clip Node(s) protected screen area
    * Obscured Nodes are (partly) hidden where the clipNodes overlap      
    */
        virtual bool addClippedChild( osg::Node *child );
        virtual bool addObscuredChild( osg::Node *child );
        virtual bool addChild( osg::Node *child );
	// drawClipNodes are special draw of geometry at the clip area which undoes the stencil value
		void addDrawClipNode(osg::Node *ndclip);
		void setBin(const int bin) { stencilbin=bin;}
    protected :
    
        virtual ~GeoClipRegion();
		int stencilbin;
};

#endif // match OSG_GEOCLIPREGION
