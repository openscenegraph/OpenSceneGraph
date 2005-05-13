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

#ifndef DXF_FILE
#define DXF_FILE 1

#include <osg/Group>

#include <iostream>
#include <string>

#include "dxfSectionBase.h"
#include "dxfReader.h"
#include "dxfSection.h"
#include "scene.h"
#include "codeValue.h"

class dxfFile {
public:
    dxfFile(std::string fileName) : 
                _fileName(fileName),
                _isNewSection(false)
    {}
    bool            parseFile();
    osg::Group*        dxf2osg();
    dxfBlock*        findBlock(std::string name);
    VariableList    getVariable(std::string var);
    
protected:

    short assign(codeValue& cv);
    std::string                 _fileName;
    bool                        _isNewSection;
    osg::ref_ptr<dxfReader>     _reader;
    osg::ref_ptr<dxfSection>    _current;
    osg::ref_ptr<dxfHeader>     _header;
    osg::ref_ptr<dxfTables>     _tables;
    osg::ref_ptr<dxfBlocks>     _blocks;
    osg::ref_ptr<dxfEntities>   _entities;
    osg::ref_ptr<dxfSection>    _unknown;
    osg::ref_ptr<scene>         _scene;
};


#endif
