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

#include "dxfSection.h"
#include "dxfEntity.h"
#include "dxfBlock.h"

void dxfHeader::assign(dxfFile*, codeValue& cv)
{
    if (cv._groupCode == 9) {
        _inVariable = true;
        VariableList var;
        _variables[cv._string] = var;
        _currentVariable = cv._string;
    } else if (_inVariable) {
        VariableList& var = getVariable(_currentVariable);
        var.push_back(cv);
    }
}

void dxfTables::assign(dxfFile* dxf, codeValue& cv) 
{
    if (cv._groupCode == 0 && cv._string == "TABLE") {
        _currentTable = NULL;
    } else if (cv._groupCode == 2 && !_currentTable.get()) {
        // treat layer table as a special case
        if (cv._string == "LAYER") {
            _layerTable = new dxfLayerTable;
            _currentTable = _layerTable.get();
        } else {
            _currentTable = new dxfTable;
            _others.push_back(_currentTable.get());
        }
    } else if (_currentTable.get()) {
        _currentTable->assign(dxf, cv);
    }
}

void dxfBlocks::assign(dxfFile* dxf, codeValue& cv)
{
    if (cv._groupCode == 0 && cv._string == std::string("BLOCK")) {
        _currentBlock = new dxfBlock;
        _blockList.push_back(_currentBlock);
    } else if (cv._groupCode == 0 && cv._string == std::string("ENDBLK") && _currentBlock) {
        std::string bn = _currentBlock->getName();
        _blockNameList[bn] = _currentBlock;
    } else if (_currentBlock) {
        _currentBlock->assign(dxf, cv);
    }
}

void dxfEntities::assign(dxfFile* dxf, codeValue& cv) 
{
    if (cv._groupCode == 0) {
        if (_currentEntity && _currentEntity->done()) {
            _currentEntity = new dxfEntity(cv._string);
            _entityList.push_back(_currentEntity);
        } else if (_currentEntity) {
            _currentEntity->assign(dxf, cv);
        } else {
            _currentEntity = new dxfEntity(cv._string);
            _entityList.push_back(_currentEntity);
        }
    } else if (_currentEntity) {
        _currentEntity->assign(dxf, cv);
    }
}

void 
dxfEntities::drawScene(scene* sc)
{
    for (EntityList::iterator itr = _entityList.begin();
        itr != _entityList.end(); ++itr)
        (*itr)->drawScene(sc);
}
dxfBlock* 
dxfBlocks::findBlock(std::string s) { return _blockNameList[s]; }

