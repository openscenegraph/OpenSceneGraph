// GeoSetBuilder.h

#ifndef __FLT_GEOSETS_H
#define __FLT_GEOSETS_H

#include <map>
#include <vector>

#include <osg/GeoSet>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>


namespace osg {
class Node;
class LOD;
class GeoSet;
class Geode;
class GeoState;
class Material;
class Texture;
}


namespace flt {

class Record;
class TmpGeoSet;

typedef osg::GeoSet::PrimitiveType  PrimitiveType;
typedef std::vector<osg::Vec3>      CoordList;
typedef std::vector<osg::Vec3>      NormalList;
typedef std::vector<osg::Vec2>      TexUVList;
typedef std::vector<osg::Vec4>      ColorList;
typedef std::vector<Record*>        VertexList;
typedef std::vector<int>            PrimLenList;



class Appearance
{
public:

    typedef osg::GeoSet::BindingType BindingType;

    Appearance() { init(); }

    void init()
    {
        _nVertexOp = 0;
        _primtype = osg::GeoSet::NO_TYPE;
        _material = NULL;
        _texture = NULL;
        _color = osg::Vec4(1,1,1,1);
        _color_binding = osg::GeoSet::BIND_OFF;
        _cullface = false;
        _transparency = false;
        _lighting = false;
        _subface = 0;
    }

    void setVertexOp(int op)                    { _nVertexOp = op; }
    int getVertexOp()                           { return _nVertexOp; }

    void setPrimType(PrimitiveType pt)          { _primtype = pt; }
    PrimitiveType getPrimType()                 { return _primtype; }

    void setColor(osg::Vec4 color)              { _color = color; }
    osg::Vec4& getColor()                       { return _color; }

    void setColorBinding( BindingType binding ) { _color_binding = binding; }
    BindingType getColorBinding()               { return _color_binding; }
    
    void setMaterial(osg::Material *material)   { _material = material; }
    osg::Material* getMaterial()                { return _material; }

    void setTexture(osg::Texture *texture)      { _texture = texture; }
    osg::Texture* getTexture()                  { return _texture; }

    void setCullface(bool cullface)             { _cullface = cullface; }
    bool getCullface()                          { return _cullface; }

    void setTransparency(bool transp)           { _transparency = transp; }
    bool getTransparency()                      { return _transparency; }

    void setLighting(bool light)                { _lighting = light; }
    bool getLighting()                          { return _lighting; }

    void setSubface(int level)                  { _subface = level; }
    int getSubface()                            { return _subface; }

    bool mat_equal(const void *m) const
    {
        if (_material && m)
            return !memcmp(_material, m, sizeof(osg::Material));
        return (!_material && !m);
    }

    bool col_equal(const BindingType b, const osg::Vec4 c) const
    {
        if (_color_binding != b)
            return false;
        if (_color_binding == osg::GeoSet::BIND_OVERALL)
            return (_color == c);
        return true;
    }
    
    /*inline*/ bool operator == (const Appearance& a) const
    {
        return ((_nVertexOp == a._nVertexOp)
            &&  (_primtype == a._primtype)
            &&  mat_equal(a._material)
            &&  col_equal(a._color_binding, a._color)
            &&  (_texture == a._texture)
            &&  (_cullface == a._cullface)
            &&  (_transparency == a._transparency)
            &&  (_lighting == a._lighting)
            &&  (_subface == a._subface));
    }

private:

    int             _nVertexOp;
    PrimitiveType   _primtype;
    osg::Material*  _material;
    osg::Texture*   _texture;
    osg::Vec4       _color;         // BIND_OVERALL
    BindingType     _color_binding;
    bool            _cullface;
    bool            _transparency;
    bool            _lighting;
    int             _subface;
};



////////////////////////////////////////////////////////////////////
//
//                       GeoSetBuilder
//
////////////////////////////////////////////////////////////////////

class GeoSetBuilder
{
public:

    typedef osg::GeoSet::BindingType BindingType;

    GeoSetBuilder(FltFile* pFltFile);
    virtual ~GeoSetBuilder();

    void addVertex(Record* vertex);
    void setPrimType(PrimitiveType pt)              { _appearance.setPrimType(pt); }
    void setColor(osg::Vec4 color)                  { _appearance.setColor(color); }
    void setColorBinding(BindingType binding )      { _appearance.setColorBinding(binding); }
    void setMaterial(osg::Material *material)       { _appearance.setMaterial(material); }
    void setTexture(osg::Texture *texture)          { _appearance.setTexture(texture); }
    void setCullface(bool cullface)                 { _appearance.setCullface(cullface); }
    void setTransparency(bool transp)               { _appearance.setTransparency(transp); }
    void setLighting(bool light)                    { _appearance.setLighting(light); }
    void setSubface(int level)                      { _appearance.setSubface(level); }

    PrimitiveType  getPrimType()                    { return _appearance.getPrimType(); }
    osg::Vec4&     getColor()                       { return _appearance.getColor(); }
    BindingType    getColorBinding()                { return _appearance.getColorBinding(); }
    osg::Material* getMaterial()                    { return _appearance.getMaterial(); }
    osg::Texture*  getTexture()                     { return _appearance.getTexture(); }
    bool           getCullface()                    { return _appearance.getCullface(); }
    bool           getTransparency()                { return _appearance.getTransparency(); }
    bool           getLighting()                    { return _appearance.getLighting(); }
    int            getSubface()                     { return _appearance.getSubface(); }

    bool addPrimitive();
    osg::Geode* createOsgGeoSets(osg::Geode* geode=NULL);

protected:

    void initPrimData();
    TmpGeoSet* findMatchingGeoSet();
    void addTo(TmpGeoSet* gset);
    void addToNew();
    PrimitiveType findPrimType( int nVertices);

private:

    VertexList      _aVertex;
    Appearance      _appearance;

    typedef std::vector<TmpGeoSet*>    GeoSetList;
    GeoSetList      _aGeoSet;

    FltFile*        _pFltFile;
};


////////////////////////////////////////////////////////////////////
//
//                       TmpGeoSet
//
////////////////////////////////////////////////////////////////////


class TmpGeoSet
{
public:

    TmpGeoSet(FltFile* pFltFile);
    virtual ~TmpGeoSet();

    void addVertex(Record* vertex);
    void addPrimLen(int len);
    osg::GeoSet* createOsgGeoSet();

    Appearance      _appearance;

private:

    void setVertex(osg::GeoSet* gset, int index, Record* vertex);

    PrimLenList     _aPrimLen;
    VertexList      _aVertex;

    FltFile*        _pFltFile;
};



}	// end of namespace flt



#endif // __FLT_GEOSETS_H

