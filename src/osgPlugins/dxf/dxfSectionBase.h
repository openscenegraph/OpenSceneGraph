/* dxfReader for OpenSceneGraph  Copyright (C) 2005 by GraphArchitecture ( grapharchitecture.com )
 * Programmed by Paul de Repentigny <pdr@grapharchitecture.com>
 *
 * OpenSceneGraph is (C) 2004 Robert Osfield
 *
 * This library is provided as-is, without support of any kind.
 *
 * Read DXF docs or OSG docs for any related questions.
 *
 * You may contact the author if you have suggestions/corrections/enhancements.
 */

#ifndef DXF_SECTIONBASE
#define DXF_SECTIONBASE 1

#include <osg/Referenced>

class dxfFile;
class codeValue;

/// abstract base class for sections. see dxfSection.h
class dxfSectionBase : public osg::Referenced
{
public:
	dxfSectionBase() {}
	virtual ~dxfSectionBase() {}
	virtual void assign(dxfFile* dxf, codeValue& cv) = 0;
};

#endif
