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

#include "dxfFile.h"
#include "dxfReader.h"
#include "dxfBlock.h"
#include "dxfEntity.h"
#include "dxfDataTypes.h"
#include "scene.h"
#include "codeValue.h"

#include <osg/Group>

using namespace std;

bool
dxfFile::parseFile()
{
    if (_fileName == "") return false;
    _reader = new dxfReader;

    if (_reader->openFile(_fileName)) {
        codeValue cv;
        while(_reader->nextGroupCode(cv)) {
            short result = assign(cv);
            if (result < 0)
                return false;
            else if (result == 0) {
                return true;
            }
        }
        // we did not reach 0 EOF
        return false;
    } else {
        return false;
    }
}

osg::Group*
dxfFile::dxf2osg()
{
    if (!_entities) return NULL;
    if (!_tables) { // a dxfTable is needed to create undefined layers
        _tables = new dxfTables;
    }
    osg::ref_ptr<dxfLayerTable> layerTable = _tables->getOrCreateLayerTable();
    // to do: should be more general and pass a pointer to ourselves
    // which in turn should be able to feed any information
    // the scene might need
    _scene = new scene(layerTable.get());
    _entities->drawScene(_scene.get());
    osg::Group* g = _scene->scene2osg();
    return g;
}

dxfBlock*
dxfFile::findBlock(std::string name)
{
    if (_blocks.get())
        return _blocks->findBlock(name);
    return NULL;
}

/// not used. if you want to know what a header variable
/// contains, call this. pass the complete variable name
/// including "$", for example: "$EXTMAX"
VariableList
dxfFile::getVariable(std::string var)
{
    return _header->getVariable(var);
}

/// parse the dxf sections
short
dxfFile::assign(codeValue& cv)
{
    std::string s = cv._string;
    if (cv._groupCode == 0 && s == std::string("ENDSEC")) {
        _isNewSection = false;
        _current = _unknown.get();
    } else if (cv._groupCode == 0 && s == std::string("SECTION")) {
        _isNewSection = true;
    } else if (cv._groupCode == 0 && s == std::string("EOF")) {
        return 0;
    } else if (cv._groupCode == 999) {    // skip comments
    } else if (cv._groupCode == 2 && _isNewSection) {
        _isNewSection = false;
//        std::cout << "Reading section " << s << std::endl;
        if (s =="HEADER") {
            _header = new dxfHeader;
            _current = _header.get();
        } else if (s =="TABLES") {
            _tables = new dxfTables;
            _current = _tables.get();
        } else if (s =="BLOCKS") {
            _blocks = new dxfBlocks;
            _current = _blocks.get();
        } else if (s =="ENTITIES") {
            _entities = new dxfEntities;
            _current = _entities.get();
        } else {
            _current = _unknown.get();
        }
    } else if (_isNewSection) {
        // problem. a 0/SECTION should be followed by a 2/SECTION_NAME
        std::cout << "No groupcode for changing section " << cv._groupCode << " value: " << s << std::endl;
        return -1;
    } else if (_current.get()) {
        _current->assign(this, cv);
    }
    return 1;
}
