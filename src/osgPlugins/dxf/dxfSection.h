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

/**
	Classes used to parse each section of a DXF file. Not all
	types of section has been defined here, just the ones
	I found of interest, ie HEADER, TABLES, BLOCKS, and ENTITIES.
	Yet to be implemented: CLASSES, OBJECTS, and THUMBNAILIMAGE.
*/
#ifndef DXF_SECTION
#define DXF_SECTION 1

#include "dxfDataTypes.h"
#include "dxfSectionBase.h"
#include "dxfTable.h"
#include "codeValue.h"
#include "scene.h"

#include <map>
#include <vector>
#include <iostream>
#include <string>

class dxfFile;
class dxfEntity;
class dxfBlock;


class dxfSection : public dxfSectionBase
{
public:
	dxfSection() {}
	virtual ~dxfSection() {}
};

class dxfHeader : public dxfSection
{
public:
	dxfHeader() : _inVariable(false) {}
	virtual ~dxfHeader() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);
	VariableList& getVariable(std::string inVar) { return _variables[inVar]; }
protected:
	std::map<std::string, VariableList> _variables;
	bool _inVariable;
	std::string _currentVariable;
};


class dxfTables : public dxfSection
{
public:
	dxfTables() : _inLayerTable(false) {}
	virtual ~dxfTables() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);
	dxfLayerTable*	getOrCreateLayerTable() 
	{ 
		if (!_layerTable.get())
			_layerTable = new dxfLayerTable;
		return _layerTable.get(); 
	}

protected:
	bool _inLayerTable;
	osg::ref_ptr<dxfLayerTable>				_layerTable;
	std::vector<osg::ref_ptr<dxfTable> >	_others;
	osg::ref_ptr<dxfTable>				_currentTable;
};

class dxfEntities : public dxfSection
{
public:
	dxfEntities() : _currentEntity(NULL) {}
	virtual ~dxfEntities() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);
	virtual void drawScene(scene* sc);

protected:
	dxfEntity*				_currentEntity;
	std::vector<dxfEntity*> _entityList;

};

class dxfBlocks : public dxfSection
{
public:
	dxfBlocks() : _currentBlock(NULL) {}
	virtual ~dxfBlocks() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);
	dxfBlock* findBlock(std::string s);

protected:
	dxfBlock*	_currentBlock;
	std::map<std::string, dxfBlock*> _blockNameList;
	std::vector<dxfBlock*> _blockList;
};

#endif
