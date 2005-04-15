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

#ifndef DXF_BLOCK
#define DXF_BLOCK 1

#include <map>
#include <vector>
#include <string>

#include <osg/Referenced>
#include <osg/Vec3d>

class dxfFile;
class codeValue;
class dxfEntity;

class dxfBlock : public osg::Referenced
{
public:
	dxfBlock() : _currentEntity(NULL) {}
	virtual ~dxfBlock() {}
	inline const std::string& getName() const { return _name; }
	virtual void assign(dxfFile* dxf, codeValue& cv);
	std::vector<dxfEntity*> getEntityList() { return _entityList; }
	const osg::Vec3d& getPosition() const;
protected:
	std::vector<dxfEntity*> _entityList;
	dxfEntity* _currentEntity;
	std::string _name;
	osg::Vec3d _position;
};

#endif
