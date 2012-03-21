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


#ifndef DXF_ENTITY
#define DXF_ENTITY 1

#include <vector>
#include <string>
#include <iostream>

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/Array>
#include <osg/Vec3d>
#include <osg/Node>
#include <osg/Matrixd>
#include <osgText/Text>

#include "dxfBlock.h"


class scene;
class codeValue;
class dxfFile;

static inline void
getOCSMatrix(const osg::Vec3d& ocs, osg::Matrixd& m)
{
    static const double one_64th = 1.0/64.0;
    m.makeIdentity();
    if (ocs == osg::Vec3d(0,0,1)) return;
    osg::Vec3d ax(1,0,0), ay(0,1,0), az(0,0,1);
    osg::Vec3d ocsaxis(ocs);
    ocsaxis.normalize();
    if (fabs(ocsaxis.x()) < one_64th && fabs(ocsaxis.y()) < one_64th) {
        ax = ay ^ ocsaxis;
    } else {
        ax = az ^ ocsaxis;
    }
    ax.normalize();
    ay = ocsaxis ^ ax;
    ay.normalize();
    m = osg::Matrixd(    ax.x(), ax.y(), ax.z(), 0,
                        ay.x(), ay.y(), ay.z(), 0,
                        ocsaxis.x(), ocsaxis.y(), ocsaxis.z(), 0,
                        0,0,0,1);
//    m = m.inverse(m);
}

class dxfBasicEntity : public osg::Referenced
{
public:
    dxfBasicEntity() : _color(0), _useAccuracy(false), _maxError(0.01), _improveAccuracyOnly(false) {}
    virtual ~dxfBasicEntity() {}
    virtual dxfBasicEntity* create() = 0;
    virtual const char* name() = 0;
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene*) {}
    const std::string getLayer() const { return _layer; }

    void setAccuracy(bool useAccuracy,double maxError,bool improveAccuracyOnly) {
        _useAccuracy=useAccuracy;
        _maxError=maxError;
        _improveAccuracyOnly=improveAccuracyOnly;
    }


protected:
    std::string    _layer;
    unsigned short    _color;

    bool _useAccuracy;          // true to specify a maximum deviation for curve rendering
    double _maxError;         // the error in model units, if _useAccuracy==true
    bool _improveAccuracyOnly;// if true only use _maxError where it would increase the quality of curves compared to the previous algorithm

};


class dxfCircle : public dxfBasicEntity
{
public:
    dxfCircle() : _radius(0), _ocs(0,0,1) {}
    virtual ~dxfCircle() {}
    virtual dxfBasicEntity* create() { // we create a copy which uses our accuracy settings
        dxfBasicEntity* circle=new dxfCircle;
        circle->setAccuracy(_useAccuracy,_maxError,_improveAccuracyOnly);
        return circle;
    }
    virtual const char* name() { return "CIRCLE"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);
protected:
    osg::Vec3d    _center;
    double    _radius;
    osg::Vec3d    _ocs;
};

class dxfArc : public dxfBasicEntity
{
public:
    dxfArc() : _radius(0), _startAngle(0), _endAngle(360), _ocs(0,0,1) {}
    virtual ~dxfArc() {}
    virtual dxfBasicEntity* create() { // we create a copy which uses our accuracy settings
        dxfBasicEntity* arc=new dxfArc;
        arc->setAccuracy(_useAccuracy,_maxError,_improveAccuracyOnly);
        //std::cout<<"dxfArc::create with _useAccuracy="<<_useAccuracy<<" maxError="<<_maxError<<" improveAccuracyOnly="<<_improveAccuracyOnly<<std::endl;
        return arc;
    }
    virtual const char* name() { return "ARC"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);
protected:
    osg::Vec3d    _center;
    double    _radius;
    double    _startAngle;
    double    _endAngle;
    osg::Vec3d    _ocs;
};

class dxfPoint : public dxfBasicEntity
{
public:
    dxfPoint() : _ocs(0,0,1) {}
    virtual ~dxfPoint() {}
    virtual dxfBasicEntity* create() { return new dxfPoint; }
    virtual const char* name() { return "POINT"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);
protected:
    osg::Vec3d    _a;
    //osg::Vec3d    _b;
    osg::Vec3d    _ocs;
};

class dxfLine : public dxfBasicEntity
{
public:
    dxfLine() : _ocs(0,0,1) {}
    virtual ~dxfLine() {}
    virtual dxfBasicEntity* create() { return new dxfLine; }
    virtual const char* name() { return "LINE"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);
protected:
    osg::Vec3d    _a;
    osg::Vec3d    _b;
    osg::Vec3d    _ocs;
};

class dxf3DFace : public dxfBasicEntity
{
public:
    dxf3DFace()
    {
        _vertices[0] = osg::Vec3d(0,0,0);
        _vertices[1] = osg::Vec3d(0,0,0);
        _vertices[2] = osg::Vec3d(0,0,0);
        _vertices[3] = osg::Vec3d(0,0,0);
    }
    virtual ~dxf3DFace() {}
    virtual dxfBasicEntity* create() { return new dxf3DFace; }
    virtual const char* name() { return "3DFACE"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);
protected:
    osg::Vec3d _vertices[4];
};

class dxfVertex : public dxfBasicEntity
{
public:
    dxfVertex() : _vertex(osg::Vec3d(0,0,0)), _indice1(0), _indice2(0), _indice3(0), _indice4(0) {}
    virtual ~dxfVertex() {}
    virtual dxfBasicEntity* create() { return new dxfVertex; }
    virtual const char* name() { return "VERTEX"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    void getVertex(double &x, double &y, double &z) { x=_vertex.x();y=_vertex.y();z=_vertex.z(); }
    const osg::Vec3d& getVertex() const { return _vertex; }
    const unsigned int getIndice1() const { return _indice1; }
    const unsigned int getIndice2() const { return _indice2; }
    const unsigned int getIndice3() const { return _indice3; }
    const unsigned int getIndice4() const { return _indice4; }

protected:
    osg::Vec3d    _vertex;
    unsigned int _indice1, _indice2, _indice3, _indice4;
};

class dxfPolyline : public dxfBasicEntity
{
public:
    dxfPolyline() : _currentVertex(NULL),
                    _elevation(0.0),
                    _flag(0),
                    _mcount(0),
                    _ncount(0),
                    _nstart(0),
                    _nend(0),
                    _ocs(osg::Vec3d(0,0,1)),
                    _mdensity(0),
                    _ndensity(0),
                    _surfacetype(0)
                    {}
    virtual ~dxfPolyline() {}
    virtual dxfBasicEntity*        create() { return new dxfPolyline; }
    virtual const char*            name() { return "POLYLINE"; }
    virtual void                assign(dxfFile* dxf, codeValue& cv);
    virtual int                    vertexCount() { return _vertices.size(); }
    virtual void                drawScene(scene* sc);

protected:
    dxfVertex*                    _currentVertex;
    std::vector<osg::ref_ptr<dxfVertex> >        _vertices;
    std::vector<osg::ref_ptr<dxfVertex> >        _indices;
    double                        _elevation;
    unsigned short                _flag;
    unsigned int                  _mcount;
    unsigned int                  _ncount;
    unsigned short                _nstart; // 71
    unsigned short                _nend; //72
    osg::Vec3d                    _ocs; //210 220 230
    unsigned short                _mdensity; // 73
    unsigned short                _ndensity; // 74
    unsigned short                _surfacetype; //75

};

class dxfLWPolyline : public dxfBasicEntity
{
public:
    dxfLWPolyline() :
        _elevation(0.0),
        _flag(0),
        _vcount(0),
        _ocs(osg::Vec3d(0,0,1)),
        _lastv(0,0,0)
        {}
    virtual ~dxfLWPolyline() {}
    virtual dxfBasicEntity*        create() { return new dxfLWPolyline; }
    virtual const char*            name() { return "LWPOLYLINE"; }
    virtual void                assign(dxfFile* dxf, codeValue& cv);
    virtual int                    vertexCount() { return _vertices.size(); }
    virtual void                drawScene(scene* sc);

protected:
    double                        _elevation;
    unsigned short                _flag;
    unsigned short                _vcount; // 90
    osg::Vec3d                    _ocs; //210 220 230
    osg::Vec3d                    _lastv;
    std::vector< osg::Vec3d >     _vertices;

};

class dxfInsert : public dxfBasicEntity
{
public:
    dxfInsert() : _block(NULL),
                    _done(false),
                    _rotation(0),
                    _scale(1,1,1),
                    _point(osg::Vec3d(0,0,0)),
                    _ocs(osg::Vec3d(0,0,1)) {}
    virtual ~dxfInsert() {}
    virtual dxfBasicEntity* create() { return new dxfInsert; }
    virtual const char* name() { return "INSERT"; }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual void drawScene(scene* sc);

protected:
    std::string _blockName;
    osg::ref_ptr<dxfBlock> _block;
    bool _done; // since we are on a SEQEND, we must
                // make sure not getting values addressed to other
                // entities (dxf garble things) in the sequence
    double            _rotation;
    osg::Vec3d        _scale;
    osg::Vec3d        _point;
    osg::Vec3d        _ocs;
};

class dxfText : public dxfBasicEntity
{
public:
    dxfText() :
        _string(""),
        _point1(0,0,0),
        _point2(0,0,0),
        _ocs(0,0,1),
        _height(1),
        _xscale(1),
        _rotation(0),
        _flags(0),
        _hjustify(0),
        _vjustify(0) {}

    virtual ~dxfText() {}
    virtual dxfBasicEntity*        create() { return new dxfText; }
    virtual const char*            name() { return "TEXT"; }
    virtual void                   assign(dxfFile* dxf, codeValue& cv);
    virtual void                   drawScene(scene* sc);

protected:
    std::string       _string;    // 1
    osg::Vec3d        _point1;    // 10,20,30
    osg::Vec3d        _point2;    // 11,21,31
    osg::Vec3d        _ocs;       // 210,220,230
    double            _height;    // 40
    double            _xscale;    // 41
    double            _rotation;  // 50
    int               _flags;     // 71
    int               _hjustify;  // 72
    int               _vjustify;  // 73
};

class dxfEntity : public osg::Referenced
{
public:
    dxfEntity(std::string s) : _entity(NULL), _seqend(false)
    {
        _entity = findByName(s);
        if (_entity) {
            _entityList.push_back(_entity);
        //    std::cout << "entity " << s << std::endl;
        }
    }
    virtual void assign(dxfFile* dxf, codeValue& cv);
    virtual bool done() { return !_seqend; }
    static void registerEntity(dxfBasicEntity*);
    static void unregisterEntity(dxfBasicEntity*);
    static dxfBasicEntity* findByName(std::string s)
    {
        dxfBasicEntity* be = _registry[s].get();
        if (be)
            return be->create();
        else {
            std::cout << " no " << s << std::endl;
            return NULL;
        }
    }
    virtual void drawScene(scene* sc);
    dxfBasicEntity* getEntity() { return _entity; }

    // Returns the exemplar from the registry - all other entities of this type are created by this one via entity->create
    static dxfBasicEntity* getRegistryEntity(std::string s) {
            return _registry[s].get();
    }

protected:
    std::vector<osg::ref_ptr<dxfBasicEntity> > _entityList;
    static std::map<std::string, osg::ref_ptr<dxfBasicEntity> > _registry;
    dxfBasicEntity* _entity;
    bool    _seqend; // bypass 0 codes. needs a 0 seqend to close.



};

/** Proxy class for automatic registration of dxf entities reader/writers.*/
template<class T>
class RegisterEntityProxy
{
    public:
        RegisterEntityProxy()
        {
            _rw = new T;
            dxfEntity::registerEntity(_rw.get());
        }

        ~RegisterEntityProxy()
        {
            dxfEntity::unregisterEntity(_rw.get());
        }

        T* get() { return _rw.get(); }

    protected:
        osg::ref_ptr<T> _rw;
};

#endif
