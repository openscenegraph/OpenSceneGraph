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


#include "dxfBlock.h"
#include "dxfFile.h"
#include "codeValue.h"
#include "dxfEntity.h"

using namespace std;

void
dxfBlock::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;
    if (cv._groupCode == 0) {
        if (_currentEntity && _currentEntity->done()) {
            _currentEntity = new dxfEntity(s);
            _entityList.push_back(_currentEntity);
        } else if (_currentEntity) {
            _currentEntity->assign(dxf, cv);
        } else {
            _currentEntity = new dxfEntity(s);
            _entityList.push_back(_currentEntity);
        }
    } else if (_currentEntity) {
        _currentEntity->assign(dxf, cv);
    } else if (cv._groupCode != 0) {
        switch (cv._groupCode) {
            case 2:
                _name = s;
                break;
            case 10:
                _position.x() = cv._double;
                break;
            case 20:
                _position.y() = cv._double;
                break;
            case 30:
                _position.z() = cv._double;
                break;
            default:
                // dxf garble
                break;
        }
    }
}

const osg::Vec3d& dxfBlock::getPosition() const { return _position; }
