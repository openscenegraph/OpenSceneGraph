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
#include "dxfTable.h"
#include "dxfFile.h"
#include "codeValue.h"


void dxfLayer::assign(dxfFile*, codeValue& cv)
{
    switch (cv._groupCode) {
        case 2:
            _name = cv._string;
            break;
        case 62:
            _color = cv._short;
            if ((short)_color < 0) _frozen = true;
            break;
        case 70:
            _frozen = (bool)(cv._short & 1);
            break;
    }
}

void dxfLayerTable::assign(dxfFile* dxf, codeValue& cv)
{
    std::string s = cv._string;
    if (cv._groupCode == 0 ) {
        if (_currentLayer.get()) {
            _layers[_currentLayer->getName()] = _currentLayer.get();
        }
        if (s == "LAYER") {
            _currentLayer = new dxfLayer;
        } // otherwise it's the close call from ENDTAB
    } else if (_currentLayer.get()) {
        _currentLayer->assign(dxf, cv);
    }
}
