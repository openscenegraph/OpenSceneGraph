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


/** Simulate the scene with double precision before passing it back to osg.
    this permits us to scale down offsets from 0,0,0 with a few matrixtransforms, 
    in case the objects are too far from that center.
    */

#ifndef DXF_SCENE
#define DXF_SCENE 1

#include <osg/Matrixd>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Vec3d>
#include <osgText/Text>
#include <osgUtil/SmoothingVisitor>

class dxfLayerTable;

class bounds {
public:
    bounds() : _min(DBL_MAX, DBL_MAX, DBL_MAX), _max(-DBL_MAX, -DBL_MAX, -DBL_MAX) {}
    inline void expandBy(const osg::Vec3d & v) {
        if(v.x()<_min.x()) _min.x() = v.x();
        if(v.x()>_max.x()) _max.x() = v.x();

        if(v.y()<_min.y()) _min.y() = v.y();
        if(v.y()>_max.y()) _max.y() = v.y();

        if(v.z()<_min.z()) _min.z() = v.z();
        if(v.z()>_max.z()) _max.z() = v.z();
    }
    inline void makeMinValid() {
        // we count on _min to offset the whole scene
        // so, we make sure its at 0,0,0 if 
        // bounds are not set (anyway, the scene should be empty,
        // if we need to set any value of _min to 0).
        if (_min.x() == DBL_MAX) _min.x() = 0;
        if (_min.y() == DBL_MAX) _min.y() = 0;
        if (_min.z() == DBL_MAX) _min.z() = 0;
    }
    osg::Vec3d _min;
    osg::Vec3d _max;
};


static inline 
osg::Geometry* createLnGeometry( osg::PrimitiveSet::Mode lineType, osg::Vec3Array* vertices, const osg::Vec4 & color)
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(lineType, 0, vertices->size())); 
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    osg::Vec3Array *norms = new osg::Vec3Array;
    norms->push_back(osg::Vec3(0,0,1));
    geom->setNormalArray(norms);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    return geom;
}


static inline 
osg::Geometry* createTriGeometry( osg::Vec3Array* vertices, osg::Vec3Array* normals, const osg::Vec4 & color)
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, vertices->size())); 
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    return geom;
}

static inline 
osg::Geometry* createQuadGeometry( osg::Vec3Array* vertices, osg::Vec3Array* normals, const osg::Vec4 & color)
{
    osg::Geometry* geom = new osg::Geometry;
    geom->setVertexArray(vertices);
    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, vertices->size())); 
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(color);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
    return geom;
}

static inline 
osg::Geode* createModel(const std::string & name, osg::Drawable* drawable)
{
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(drawable);
    geode->setName(name);
    return geode;
}


static inline osg::Vec3d preMultd(const osg::Matrixd& m, const osg::Vec3d& v)
{
    double d = 1.0f/(m(3,0)*v.x()+m(3,1)*v.y()+m(3,2)*v.z()+m(3,3)) ;
    return osg::Vec3d( (m(0,0)*v.x() + m(1,0)*v.y() + m(2,0)*v.z() + m(3,0))*d,
        (m(0,1)*v.x() + m(1,1)*v.y() + m(2,1)*v.z() + m(3,1))*d,
        (m(0,2)*v.x() + m(1,2)*v.y() + m(2,2)*v.z() + m(3,2))*d) ;
}

static inline osg::Vec3d postMultd(const osg::Matrixd& m, const osg::Vec3d& v)
{
    double d = 1.0f/(m(3,0)*v.x()+m(3,1)*v.y()+m(3,2)*v.z()+m(3,3)) ;
    return osg::Vec3d( (m(0,0)*v.x() + m(0,1)*v.y() + m(0,2)*v.z() + m(0,3))*d,
        (m(1,0)*v.x() + m(1,1)*v.y() + m(1,2)*v.z() + m(1,3))*d,
        (m(2,0)*v.x() + m(2,1)*v.y() + m(2,2)*v.z() + m(2,3))*d) ;
}

typedef std::vector<osg::Vec3d> VList;
typedef std::map<unsigned short, VList> MapVList;
typedef std::vector<VList> VListList;
typedef std::map<unsigned short, VListList> MapVListList;


class sceneLayer : public osg::Referenced {
public:
    sceneLayer(std::string name) : _name(name) {}
    virtual ~sceneLayer() {}
    void layer2osg(osg::Group* root, bounds &b)
    {
        osgLines(root, b);
        osgTriangles(root, b);
        osgQuads(root, b);
        osgText(root, b);
    }
    MapVListList    _linestrips;
    MapVList        _lines;
    MapVList        _triangles;
    MapVList        _trinorms;
    MapVList        _quads;
    MapVList        _quadnorms;
    
    struct textInfo
    {
        textInfo(short int color, osg::Vec3 point, osgText::Text *text) :
            _color(color), _point(point), _text(text) {};
        short int _color;
        osg::Vec3d _point;
        osg::ref_ptr<osgText::Text> _text;
    };

    typedef std::vector<textInfo> TextList;    
    TextList _textList;
    
protected:
    std::string        _name;

    osg::Vec4        getColor(unsigned short color);
    void osgLines(osg::Group* root, bounds &b)
    {
        for(MapVListList::iterator mlitr = _linestrips.begin();
            mlitr != _linestrips.end();
            ++mlitr)
        {
            for(VListList::iterator itr = mlitr->second.begin();
                itr != mlitr->second.end(); 
                ++itr)
            {
                if (itr->size()) {
                    osg::Vec3Array *coords = new osg::Vec3Array;
                    for (VList::iterator vitr = itr->begin();
                        vitr != itr->end(); ++vitr) {
                        osg::Vec3 v(vitr->x() - b._min.x(), vitr->y() - b._min.y(), vitr->z() - b._min.z());
                        coords->push_back(v);
                    }
                    root->addChild(createModel(_name, createLnGeometry(osg::PrimitiveSet::LINE_STRIP, coords, getColor(mlitr->first))));
                }
            }
        }
        for (MapVList::iterator mitr = _lines.begin();
            mitr != _lines.end(); ++mitr) {
            osg::Vec3Array *coords = new osg::Vec3Array;
            for (VList::iterator itr = mitr->second.begin();
                itr != mitr->second.end(); ++itr) {
                osg::Vec3 v(itr->x() - b._min.x(), itr->y() - b._min.y(), itr->z() - b._min.z());
                coords->push_back(v);
            }
            root->addChild(createModel(_name, createLnGeometry(osg::PrimitiveSet::LINES, coords, getColor(mitr->first))));
        }
    }

    void osgTriangles(osg::Group* root, bounds &b)
    {
        if (_triangles.size()) {
            for (MapVList::iterator mitr = _triangles.begin();
                mitr != _triangles.end(); ++mitr) {
                osg::Vec3Array *coords = new osg::Vec3Array;
                VList::iterator itr;
                for (itr = mitr->second.begin();
                    itr != mitr->second.end(); ++itr)
                {
                    osg::Vec3 v(itr->x() - b._min.x(), itr->y() - b._min.y(), itr->z() - b._min.z());
                    coords->push_back(v);
                }
                osg::Vec3Array *norms = new osg::Vec3Array;
                VList normlist = _trinorms[mitr->first];
                for (itr = normlist.begin();
                    itr != normlist.end(); ++itr)
                {
                    norms->push_back(osg::Vec3(itr->x(), itr->y(), itr->z()));
                }
                root->addChild(createModel(_name, createTriGeometry(coords, norms, getColor(mitr->first))));
            }
        }
    }
    void osgQuads(osg::Group* root, bounds &b)
    {
        if (_quads.size()) {
            for (MapVList::iterator mitr = _quads.begin();
                mitr != _quads.end(); ++mitr) {
                osg::Vec3Array *coords = new osg::Vec3Array;
                VList::iterator itr;
                for (itr = mitr->second.begin();
                    itr != mitr->second.end(); ++itr) {
                    osg::Vec3 v(itr->x() - b._min.x(), itr->y() - b._min.y(), itr->z() - b._min.z());
                    coords->push_back(v);
                }
                osg::Vec3Array *norms = new osg::Vec3Array;
                VList normlist = _quadnorms[mitr->first];
                for (itr = normlist.begin();
                    itr != normlist.end(); ++itr) {
                    norms->push_back(osg::Vec3(itr->x(), itr->y(), itr->z()));
                }
                root->addChild(createModel(_name, createQuadGeometry(coords, norms, getColor(mitr->first))));
            }
        }
    }
    void osgText(osg::Group* root, bounds &b)
    {
        if (_textList.size()) {
            for (TextList::iterator titr = _textList.begin();
                    titr != _textList.end(); ++titr) {
                titr->_text->setColor(getColor(titr->_color));
                osg::Vec3d v1=titr->_point;
                osg::Vec3 v2(v1.x() - b._min.x(), v1.y() - b._min.y(), v1.z() - b._min.z());
                titr->_text->setPosition(v2);
                root->addChild(createModel(_name, titr->_text.get()));
            }
        }
    }
};


class scene : public osg::Referenced {
public:
    scene(dxfLayerTable* lt = NULL);
    virtual ~scene() {}
    void setLayerTable(dxfLayerTable* lt);
    void pushMatrix(const osg::Matrixd& m, bool protect = false)
    {
        _mStack.push_back(_m);
        if (protect) // equivalent to setMatrix
            _m = m;
        else
            _m = _m * m;
    }
    void popMatrix()
    {
        _mStack.pop_back();
        if (_mStack.size())
            _m = _mStack.back();
        else
            _m.makeIdentity();
    }
    void ocs(const osg::Matrixd& r)
    {
        _r = r;
    }
    void blockOffset(const osg::Vec3d& t)
    {
        _t = t;
    }
    void ocs_clear()
    {
        _r.makeIdentity();
    }
    osg::Matrixd& backMatrix() { if (_mStack.size()) return _mStack.back(); else return _m; }

    osg::Vec3d addVertex(osg::Vec3d v);
    osg::Vec3d addNormal(osg::Vec3d v);
    sceneLayer* findOrCreateSceneLayer(const std::string & l)
    {
        sceneLayer* ly = _layers[l].get();
        if (!ly) {
            ly = new sceneLayer(l);
            _layers[l] = ly;
        }
        return ly;
    }
    unsigned short correctedColorIndex(const std::string & l, unsigned short color);

    void addLine(const std::string & l, unsigned short color, osg::Vec3d & s, osg::Vec3d & e);
    void addLineStrip(const std::string & l, unsigned short color, std::vector<osg::Vec3d> & vertices);
    void addLineLoop(const std::string & l, unsigned short color, std::vector<osg::Vec3d> & vertices);
    void addTriangles(const std::string & l, unsigned short color, std::vector<osg::Vec3d> & vertices, bool inverted=false);
    void addQuads(const std::string & l, unsigned short color, std::vector<osg::Vec3d> & vertices, bool inverted=false);
    void addText(const std::string & l, unsigned short color, osg::Vec3d & point, osgText::Text *text);

    osg::Group* scene2osg()
    {
        osg::Group* root = NULL;
        osg::Group* child = NULL;
        _b.makeMinValid();
        osg::Vec3 v = osg::Vec3(_b._min.x(), _b._min.y(), _b._min.z());
        double x = _b._min.x() - (double)v.x();
        double y = _b._min.y() - (double)v.y();
        double z = _b._min.z() - (double)v.z();
        osg::Matrixd m = osg::Matrixd::translate(v);
        root = new osg::MatrixTransform(m);
        if (x || y || z) {
            m = osg::Matrixd::translate(x,y,z);
            child = new osg::MatrixTransform(m);
            root->addChild(child);
        } else {
            child = root;
        }
//            root = mt;
        for (std::map<std::string, osg::ref_ptr<sceneLayer> >::iterator litr = _layers.begin();
            litr != _layers.end(); ++litr) {
            sceneLayer* ly = (*litr).second.get();
            if (!ly) continue;
            osg::Group* lg = new osg::Group;
            lg->setName((*litr).first);
            child->addChild(lg);
            ly->layer2osg(lg, _b);
        }
        return root;
    }
protected:
    osg::Matrixd                _m;
    osg::Matrixd                _r;
    osg::Vec3d                  _t;
    bounds                      _b;
    std::map<std::string, osg::ref_ptr<sceneLayer> >        _layers;
    std::vector<osg::Matrixd>   _mStack;
    dxfLayerTable*              _layerTable;
};

#endif
