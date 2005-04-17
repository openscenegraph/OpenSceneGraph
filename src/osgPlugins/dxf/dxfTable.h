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

#ifndef DXF_TABLE
#define DXF_TABLE 1

#include <string>
#include <map>

#include <osg/Referenced>
#include <osg/ref_ptr>

class dxfFile;
class codeValue;

// special case: the layer table

class dxfTable : public osg::Referenced
{
public:
	dxfTable() {}
	virtual ~dxfTable() {}
	virtual void assign(dxfFile* , codeValue& ) { }
};


class dxfLayer : public osg::Referenced
{
public:
	dxfLayer(std::string name = "0") : _name(name), _color(7), _frozen(false) {}
	virtual ~dxfLayer() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);
	virtual const std::string& getName() const { return _name; }
	virtual const unsigned short& getColor() const { return _color; }
	virtual void setName(const std::string& name) { _name = name; }
	const bool& getFrozen() const { return _frozen; }
protected:
	std::string	_name;
	unsigned short _color;
	bool			_frozen;
};

class dxfLayerTable : public dxfTable
{
public:
	dxfLayerTable() {}
	virtual ~dxfLayerTable() {}
	virtual void assign(dxfFile* dxf, codeValue& cv);

	dxfLayer* findOrCreateLayer(std::string name) 
	{ 
		if (name == "") name = "0"; // nowhere it is said "" is invalid, but...
		dxfLayer* layer = _layers[name].get();
		if (!layer) {
			layer = new dxfLayer;
			_layers[name] = layer;
		}
		return layer;
	}

protected:
	std::map<std::string, osg::ref_ptr<dxfLayer> > _layers;
	osg::ref_ptr<dxfLayer> _currentLayer;
};

#endif
