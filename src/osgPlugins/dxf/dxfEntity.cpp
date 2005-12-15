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

#include "dxfEntity.h"
#include "dxfFile.h"
#include "scene.h"
#include "dxfBlock.h"
#include "codeValue.h"

using namespace std;
using namespace osg;

// static
std::map<std::string, ref_ptr<dxfBasicEntity> > dxfEntity::_registry;
RegisterEntityProxy<dxf3DFace> g_dxf3DFace;
RegisterEntityProxy<dxfCircle> g_dxfCircle;
RegisterEntityProxy<dxfArc> g_dxfArc;
RegisterEntityProxy<dxfLine> g_dxfLine;
RegisterEntityProxy<dxfVertex> g_dxfVertex;
RegisterEntityProxy<dxfPolyline> g_dxfPolyline;
RegisterEntityProxy<dxfLWPolyline> g_dxfLWPolyline;
RegisterEntityProxy<dxfInsert> g_dxfInsert;

void 
dxfBasicEntity::assign(dxfFile* , codeValue& cv)
{
    switch (cv._groupCode) {
        case 8:
            _layer = cv._string;
            break;
        case 62:
            _color = cv._short;
            break;
    }
}

void
dxf3DFace::assign(dxfFile* dxf, codeValue& cv)
{
    double d = cv._double;
    switch (cv._groupCode) {
        case 10:
        case 11:
        case 12:
        case 13:
            _vertices[cv._groupCode - 10].x() = d;
            break;
        case 20:
        case 21:
        case 22:
        case 23:
            _vertices[cv._groupCode - 20].y() = d;
            break;
        case 30:
        case 31:
        case 32:
        case 33:
            _vertices[cv._groupCode - 30].z() = d;
            break;
            
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxf3DFace::drawScene(scene* sc)
{
    std::vector<Vec3d> vlist;
    short nfaces = 3;

    // Hate to do that, but hey, that's written in the DXF specs:
    if (_vertices[2] != _vertices[3]) nfaces = 4;

    for (short i = nfaces-1; i >= 0; i--)
        vlist.push_back(_vertices[i]);

    if (nfaces == 3) {
        // to do make sure we're % 3
        sc->addTriangles(getLayer(), _color, vlist);
    } else if (nfaces == 4) {
        // to do make sure we're % 4
        sc->addQuads(getLayer(), _color, vlist);
    }
}

void
dxfVertex::assign(dxfFile* dxf, codeValue& cv)
{
    double d = cv._double;
    // 2005.12.13 pdr: learned today that negative indices mean something and were possible
    
    int s = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
    if ( s < 0 ) s = -s;
    switch (cv._groupCode) {
        case 10:
            _vertex.x() = d;
            break;
        case 20:
            _vertex.y() = d;
            break;
        case 30:
            _vertex.z() = d;
            break;
        case 71:
            _indice1 = s;
            break;
        case 72:
            _indice2 = s;
            break;
        case 73:
            _indice3 = s;
            break;
        case 74:
            _indice4 = s;
            break;
            
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxfCircle::assign(dxfFile* dxf, codeValue& cv)
{
    double d = cv._double;
    //unsigned short s = cv._short;
    switch (cv._groupCode) {
        case 10:
            _center.x() = d;
            break;
        case 20:
            _center.y() = d;
            break;
        case 30:
            _center.z() = d;
            break;
        case 40:
            _radius = d;
            break;
        case 210:
            _ocs.x() = d;
            break;
        case 220:
            _ocs.y() = d;
            break;
        case 230:
            _ocs.z() = d;
            break;
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxfCircle::drawScene(scene* sc)
{
    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->ocs(m);
    std::vector<Vec3d> vlist;
    int numsteps = 360/5; // baaarghf.
    double angle_step = osg::DegreesToRadians((double)360.0 / (double) numsteps);
    double angle1 = 0.0f;
    double angle2 = 0.0f;
    Vec3d a = _center;
    Vec3d b,c;
    for (int r = 0; r < numsteps; r++) 
    {
        angle1 = angle2;
        if (r == numsteps - 1)
            angle2 = 0.0f;
        else
            angle2 += angle_step;
        b = a + Vec3d(_radius * (double) sin(angle1), _radius * (double) cos(angle1), 0);
        c = a + Vec3d(_radius * (double) sin(angle2), _radius * (double) cos(angle2), 0);
//        vlist.push_back(a);
        vlist.push_back(b);
        vlist.push_back(c);
    }
    sc->addLineStrip(getLayer(), _color, vlist);
//    sc->addTriangles(getLayer(), _color, vlist);
    sc->ocs_clear();
}


void
dxfArc::assign(dxfFile* dxf, codeValue& cv)
{
    double d = cv._double;
    //unsigned short s = cv._short;
    switch (cv._groupCode) {
        case 10:
            _center.x() = d;
            break;
        case 20:
            _center.y() = d;
            break;
        case 30:
            _center.z() = d;
            break;
        case 40:
            _radius = d;
            break;
        case 50:
            _startAngle = d;
            break;
        case 51:
            _endAngle = d;
            break;
        case 210:
            _ocs.x() = d;
            break;
        case 220:
            _ocs.y() = d;
            break;
        case 230:
            _ocs.z() = d;
            break;
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxfArc::drawScene(scene* sc)
{
    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->ocs(m);
    std::vector<Vec3d> vlist;
    double end;
    double start;
    if (_startAngle > _endAngle) {
        start = _startAngle;
        end = _endAngle + 360;
    } else {
        start = _startAngle;
        end = _endAngle;
    }
    double angle_step = DegreesToRadians(end - start);
    int numsteps = (int)((end - start)/5.0); // hurmghf. say 5 degrees?
    if (numsteps * 5 < (end - start)) numsteps++;
    angle_step /=  (double) numsteps;
    end = DegreesToRadians((-_startAngle)+90.0);
    start = DegreesToRadians((-_endAngle)+90.0);
    double angle1 = 0.0f;
    double angle2 = (start);
    Vec3d a = _center;
    Vec3d b,c;
    for (int r = 0; r < numsteps; r++) 
    {
        angle1 = angle2;
        angle2 = angle1 + angle_step;
        b = a + Vec3d(_radius * (double) sin(angle1), _radius * (double) cos(angle1), 0);
        c = a + Vec3d(_radius * (double) sin(angle2), _radius * (double) cos(angle2), 0);
        vlist.push_back(b);
        vlist.push_back(c);
    }
    sc->addLineStrip(getLayer(), _color, vlist);
    sc->ocs_clear();
}

void
dxfLine::assign(dxfFile* dxf, codeValue& cv)
{
    double d = cv._double;
    //unsigned short s = cv._short;
    switch (cv._groupCode) {
        case 10:
            _a.x() = d;
            break;
        case 20:
            _a.y() = d;
            break;
        case 30:
            _a.z() = d;
            break;
        case 11:
            _b.x() = d;
            break;
        case 21:
            _b.y() = d;
            break;
        case 31:
            _b.z() = d;
            break;
        case 210:
            _ocs.x() = d;
            break;
        case 220:
            _ocs.y() = d;
            break;
        case 230:
            _ocs.z() = d;
            break;
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxfLine::drawScene(scene* sc)
{
    Matrixd m;
    getOCSMatrix(_ocs, m);
    // don't know why this doesn't work
//    sc->ocs(m);
    sc->addLine(getLayer(), _color, _b, _a);
//    static long lcount = 0;
//    std::cout << ++lcount << " ";
//    sc->ocs_clear();
}

void 
dxfPolyline::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;
    if (cv._groupCode == 0) {
        if (s == "VERTEX") {
            _currentVertex = new dxfVertex;
            if (_mcount && (_flag & 64) && _vertices.size() == _mcount) {
                _indices.push_back(_currentVertex);
            } else {
                // don't know if and what to do if no _mcount
                _vertices.push_back(_currentVertex);
            }
        } else {
            // hum not sure if 
            //        1) that is possible 
            //        2) what to do...
        }
    } else if (_currentVertex) {
        _currentVertex->assign(dxf, cv);
    } else {
        double d = cv._double;
        switch (cv._groupCode) {
            case 10:
                // dummy
                break;
            case 20:
                // dummy
                break;
            case 30:
                _elevation = d; // what is elevation?
                break;
            case 70:
                _flag = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 71:
                _mcount = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 72:
                _ncount = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 73:
                _mdensity = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 74:
                _ndensity = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 75:
                _surfacetype = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 210:
                _ocs.x() = d;
                break;
            case 220:
                _ocs.y() = d;
                break;
            case 230:
                _ocs.z() = d;
                break;
            default:
                dxfBasicEntity::assign(dxf, cv);
                break;
        }
    }
}


void
dxfPolyline::drawScene(scene* sc)
{
//    if (getLayer() != "UDF2" && getLayer() != "ENGINES") return;
//    if (!(_flag & 16)) return;
    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->ocs(m);
    std::vector<Vec3d> vlist;
    std::vector<Vec3d> qlist;
    Vec3d a, b, c, d;
    bool invert_order = false;
    if (_flag & 16) {
        std::vector<Vec3d> nlist;
        Vec3d nr;
        bool nset = false;
        //dxfVertex* v = NULL;
        unsigned short ncount;
        unsigned short mcount;
        if (_surfacetype ==6) { 
            // I dont have examples of type 5 and 8, but they may be the same as 6
            mcount = _mdensity;
            ncount = _ndensity;
        } else { 
            mcount = _mcount;
            ncount = _ncount;
        }
        for (unsigned short n = 0; n < ncount-1; n++) {
            for (unsigned short m = 1; m < mcount; m++) {
                // 0
                a = _vertices[(m-1)*ncount+n].get()->getVertex();
                // 1
                b = _vertices[m*ncount+n].get()->getVertex();
                // 3
                c = _vertices[(m)*ncount+n+1].get()->getVertex();
                // 2
                d = _vertices[(m-1)*ncount+n+1].get()->getVertex();
                if (a == b ) {
                    vlist.push_back(a);
                    vlist.push_back(c);
                    vlist.push_back(d);
                    b = c;
                    c = d;
                } else if (c == d) {
                    vlist.push_back(a);
                    vlist.push_back(b);
                    vlist.push_back(c);
                } else {
                    qlist.push_back(a);
                    qlist.push_back(b);
                    qlist.push_back(c);
                    qlist.push_back(d);
                }
                if (!nset) {
                    nset = true;
                    nr = (b - a) ^ (c - a);
                    nr.normalize();
                }
                nlist.push_back(a);
            }
        }
        if (_flag & 1) {
            for (unsigned short n = 0; n < ncount-1; n++) {
                // 0
                a = _vertices[(mcount-1)*ncount+n].get()->getVertex();
                // 1
                b = _vertices[0*ncount+n].get()->getVertex();
                // 3
                c = _vertices[(0)*ncount+n+1].get()->getVertex();
                // 2
                d = _vertices[(mcount-1)*ncount+n+1].get()->getVertex();
                if (a == b ) {
                    vlist.push_back(a);
                    vlist.push_back(c);
                    vlist.push_back(d);
                    b = c;
                    c = d;
                } else if (c == d) {
                    vlist.push_back(a);
                    vlist.push_back(b);
                    vlist.push_back(c);
                } else {
                    qlist.push_back(a);
                    qlist.push_back(b);
                    qlist.push_back(c);
                    qlist.push_back(d);
                }
                nlist.push_back(a);
            }
        }
        if (_flag & 32) {
            for (unsigned short m = 1; m < mcount; m++) {
                // 0
                a = _vertices[(m-1)*ncount+(ncount-1)].get()->getVertex();
                // 1
                b = _vertices[m*ncount+(ncount-1)].get()->getVertex();
                // 3
                c = _vertices[(m)*ncount].get()->getVertex();
                // 2
                d = _vertices[(m-1)*ncount].get()->getVertex();
                if (a == b ) {
                    vlist.push_back(a);
                    vlist.push_back(c);
                    vlist.push_back(d);
                    b = c;
                    c = d;
                } else if (c == d) {
                    vlist.push_back(a);
                    vlist.push_back(b);
                    vlist.push_back(c);
                } else {
                    qlist.push_back(a);
                    qlist.push_back(b);
                    qlist.push_back(c);
                    qlist.push_back(d);
                }
                nlist.push_back(a);
            }
        }

        // a naive attempt to determine vertex ordering
        VList::iterator itr = nlist.begin();
        Vec3d lastn = (*itr++);
        double bad_c = 0;
        double good_c = 0;
        long bad=0,good=0;
        for (; itr != nlist.end(); ) {
            ++itr;
            if ((*itr)== lastn) continue;
            Vec3d diff = ((*itr)-lastn);
            diff.normalize();
            float dot = diff * nr;
            if (dot > 0.0) {
                bad_c += dot;
                ++bad;
            } else {
                ++good;
                good_c += dot;
            }
        }
        if (bad > good) {
            invert_order = true;
        }

        if (qlist.size())
            sc->addQuads(getLayer(), _color, qlist, invert_order);
        if (vlist.size())
            sc->addTriangles(getLayer(), _color, vlist, invert_order);

    } else if (_flag & 64) { 
        if (_ncount > _indices.size()) 
            _ncount = _indices.size();
        unsigned short _facetype = 3;
        unsigned short count = 0;
        for (unsigned short i = 0; i < _ncount; i++) {
            dxfVertex* vindice = _indices[i].get();
            if (!vindice) continue;
            //dxfVertex* v = NULL;
            if (vindice->getIndice4()) {
                _facetype = 4;
                d = _vertices[vindice->getIndice4()-1].get()->getVertex();
            } else {
                _facetype = 3;
            }
            if (vindice->getIndice3()) {
                c = _vertices[vindice->getIndice3()-1].get()->getVertex();
            } else {
                c = _vertices[count++].get()->getVertex();
            }
            if (vindice->getIndice2()) {
                b = _vertices[vindice->getIndice2()-1].get()->getVertex();
            } else {
                b = _vertices[count++].get()->getVertex();
            }
            if (vindice->getIndice1()) {
                a = _vertices[vindice->getIndice1()-1].get()->getVertex();
            } else {
                a = _vertices[count++].get()->getVertex();
            }
            if (_facetype == 4) {
                qlist.push_back(d);
                qlist.push_back(c);
                qlist.push_back(b);
                qlist.push_back(a);
            } else {
                // 2005.12.13 pdr: vlist! not qlist!
                vlist.push_back(c);
                vlist.push_back(b);
                vlist.push_back(a);
            }
        }
        if (vlist.size())
            sc->addTriangles(getLayer(), _color, vlist);
        if (qlist.size())
            sc->addQuads(getLayer(), _color, qlist);
        // is there a flag 1 or 32 for 64?
    } else {
        // simple polyline?
        for (short i = _vertices.size()-1; i >= 0; i--)
            vlist.push_back(_vertices[i]->getVertex());
        if (_flag & 1) {
//            std::cout << "line loop " << _vertices.size() << std::endl;
            sc->addLineLoop(getLayer(), _color, vlist);
        } else {
//            std::cout << "line strip " << _vertices.size() << std::endl;
            sc->addLineStrip(getLayer(), _color, vlist);
        }

    }
    sc->ocs_clear();
}

void 
dxfLWPolyline::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;

    double d = cv._double;
    switch (cv._groupCode) {
        case 10:
            _lastv.x() = d;
            // x
            break;
        case 20:
            _lastv.y() = d;
            _lastv.z() = _elevation;
            _vertices.push_back ( _lastv );
            // y -> on shoot
            break;
        case 38:
            _elevation = d; // what is elevation?
            break;
        case 70:
            _flag = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
            break;
        case 90:
            _vcount = cv._short;
            break;
        case 210:
            _ocs.x() = d;
            break;
        case 220:
            _ocs.y() = d;
            break;
        case 230:
            _ocs.z() = d;
            break;
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}


void
dxfLWPolyline::drawScene(scene* sc)
{
//    if (getLayer() != "UDF2" && getLayer() != "ENGINES") return;
//    if (!(_flag & 16)) return;
    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->ocs(m);
    if (_flag & 1) {
//        std::cout << "lwpolyline line loop " << _vertices.size() << std::endl;
        sc->addLineLoop(getLayer(), _color, _vertices);
    } else {
//        std::cout << "lwpolyline line strip " << _vertices.size() << std::endl;
        sc->addLineStrip(getLayer(), _color, _vertices);
    }
    sc->ocs_clear();
}

void
dxfInsert::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;
    if (_done || (cv._groupCode == 0 && s != "INSERT")) {
        _done = true;
        return;
    }
    if (cv._groupCode == 2 && !_block) {
        _blockName = s;
        _block = dxf->findBlock(s);
    } else {
        double d = cv._double;
        switch (cv._groupCode) {
            case 10:
                _point.x() = d;
                break;
            case 20:
                _point.y() = d;
                break;
            case 30:
                _point.z() = d;
                break;
            case 41:
                _scale.x() = d;
                break;
            case 42:
                _scale.y() = d;
                break;
            case 43:
                _scale.z() = d;
                break;
            case 50:
                _rotation = d;
                break;
            case 210:
                _ocs.x() = d;
                break;
            case 220:
                _ocs.y() = d;
                break;
            case 230:
                _ocs.z() = d;
                break;
            default:
                dxfBasicEntity::assign(dxf, cv);
                break;
        }
    }
}

/// hum. read the doc, then come back here. then try to figure.
void
dxfInsert::drawScene(scene* sc)
{
    // INSERTs can be nested. So pull the current matrix
    // and push it back after we fill our context
    // This is a snapshot in time. I will rewrite all this to be cleaner,
    // but for now, it seems working fine 
    // (with the files I have, the results are equal to Voloview,
    // and better than Deep Exploration and Lightwave).
    Matrixd back = sc->backMatrix();
    Matrixd m;
    m.makeIdentity();
    sc->pushMatrix(m, true);
    Vec3d trans = _block->getPosition();
    sc->blockOffset(-trans);
    if (_rotation) {
        sc->pushMatrix(Matrixd::rotate(osg::DegreesToRadians(_rotation), 0,0,1));
    }
    sc->pushMatrix(Matrixd::scale(_scale.x(), _scale.y(), _scale.z()));
    sc->pushMatrix(Matrixd::translate(_point.x(), _point.y(), _point.z()));
    getOCSMatrix(_ocs, m);
    sc->pushMatrix(m);
    sc->pushMatrix(back);

    std::vector<dxfEntity*> l = _block->getEntityList();
    for (std::vector<dxfEntity*>::iterator itr = l.begin(); itr != l.end(); ++itr) {
        dxfBasicEntity* e = (*itr)->getEntity();
        if (e) {
            e->drawScene(sc);
        }
    }

    sc->popMatrix(); // ocs
    sc->popMatrix(); // translate
    sc->popMatrix(); // scale
    if (_rotation) {
        sc->popMatrix(); // rotate
    }
    sc->popMatrix(); // identity
    sc->popMatrix(); // back
    sc->blockOffset(Vec3d(0,0,0));

}


// static
void 
dxfEntity::registerEntity(dxfBasicEntity* entity)
{
    _registry[entity->name()] = entity;
}

// static
void 
dxfEntity::unregisterEntity(dxfBasicEntity* entity)
{
    map<string, ref_ptr<dxfBasicEntity > >::iterator itr = _registry.find(entity->name());
    if (itr != _registry.end()) {
        _registry.erase(itr);
    }
}

void dxfEntity::drawScene(scene* sc)
{
    for (std::vector<ref_ptr<dxfBasicEntity > >::iterator itr = _entityList.begin();
        itr != _entityList.end(); ++itr) {
            (*itr)->drawScene(sc);
    }
}

void 
dxfEntity::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;
    if (cv._groupCode == 66 && !(_entity && string("TABLE") == _entity->name())) {
        // The funny thing here. Group code 66 has been called 'obsoleted'
        // for a POLYLINE. But not for an INSERT. Moreover, a TABLE
        // can have a 66 for... an obscure bottom cell color value. 
        // I decided to rely on the presence of the 66 code for
        // the POLYLINE. If you find a better alternative,
        // contact me, or correct this code
        // and post the correction to osg mailing list
        _seqend = true;
    } else if (_seqend && cv._groupCode == 0 && s == "SEQEND") {
        _seqend = false;
//        cout << "... off" << endl;
    } else if (_entity) {
        _entity->assign(dxf, cv);
    }
}
