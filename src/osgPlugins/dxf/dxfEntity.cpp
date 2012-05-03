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

#include <osg/io_utils> // just for debugging

using namespace std;
using namespace osg;

// static
std::map<std::string, ref_ptr<dxfBasicEntity> > dxfEntity::_registry;
RegisterEntityProxy<dxf3DFace> g_dxf3DFace;
RegisterEntityProxy<dxfCircle> g_dxfCircle;
RegisterEntityProxy<dxfArc> g_dxfArc;
RegisterEntityProxy<dxfPoint> g_dxfPoint;
RegisterEntityProxy<dxfLine> g_dxfLine;
RegisterEntityProxy<dxfVertex> g_dxfVertex;
RegisterEntityProxy<dxfPolyline> g_dxfPolyline;
RegisterEntityProxy<dxfLWPolyline> g_dxfLWPolyline;
RegisterEntityProxy<dxfInsert> g_dxfInsert;
RegisterEntityProxy<dxfText> g_dxfText;

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

    double theta=5.0; // we generate polyline from "spokes" at theta degrees at arc's center

    if (_useAccuracy) {
        // we generate points on a polyline where each point lies on the arc, thus the maximum error occurs at the midpoint of each line segment where it lies furthest inside the arc
        // If we divide the segment in half and connect the bisection point to the arc's center, we have two rightangled triangles with
        // one side=r-maxError, hypotenuse=r, and internal angle at center is half the angle we will step with:
        double maxError=min(_maxError,_radius); // Avoid offending acos() in the edge case where allowable deviation is greater than radius.
        double newtheta=acos( (_radius-maxError) / _radius);
        newtheta=osg::RadiansToDegrees(newtheta)*2.0;

        // Option to only use the new accuracy code when it would improve on the accuracy of the old method
        if (_improveAccuracyOnly) {
            theta=min(newtheta,theta);
        } else {
            theta=newtheta;
        }
    }
    theta=osg::DegreesToRadians(theta);

    // We create an anglestep<=theta so that the line's points are evenly distributed around the circle
    unsigned int numsteps=static_cast<unsigned int>(floor(osg::PI*2/theta));
    if (numsteps<3) numsteps=3; // Sanity check: minimal representation of a circle is a tri
    double anglestep=osg::PI*2/numsteps;

    double angle1 = 0.0;
    Vec3d a = _center;
    Vec3d b;
    for(unsigned int r=0;r<=numsteps;r++) {
        b = a + Vec3d(_radius * (double) sin(angle1), _radius * (double) cos(angle1), 0);
        angle1 += anglestep;
        vlist.push_back(b);
    }

    sc->addLineStrip(getLayer(), _color, vlist); // Should really add LineLoop implementation and save a vertex
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

    double theta=5.0; // we generate polyline from "spokes" at theta degrees at arc's center

    if (_useAccuracy) {
        // we generate points on a polyline where each point lies on the arc, thus the maximum error occurs at the midpoint of each line segment where it lies furthest inside the arc
        // If we divide the segment in half and connect the bisection point to the arc's center, we have two rightangled triangles with
        // one side=r-maxError, hypotenuse=r, and internal angle at center is half the angle we will step with:
        double maxError=min(_maxError,_radius); // Avoid offending acos() in the edge case where allowable deviation is greater than radius.
        double newtheta=acos( (_radius-maxError) / _radius);
        newtheta=osg::RadiansToDegrees(newtheta)*2.0;
        //cout<<"r="<<_radius<<" _me="<<_maxError<<" (_radius-_maxError)="<<(_radius-_maxError)<<" newtheta="<<newtheta<<endl;
        // Option to only use the new accuracy code when it would improve on the accuracy of the old method
        if (_improveAccuracyOnly) {
            theta=min(newtheta,theta);
        } else {
            theta=newtheta;
        }
    }

    double angle_step = DegreesToRadians(end - start);
    int numsteps = (int)((end - start)/theta);
    //cout<<"arc theta="<<osg::RadiansToDegrees(theta)<<" end="<<end<<" start="<<start<<" numsteps="<<numsteps<<" e-s/theta="<<((end-start)/theta)<<" end-start="<<(end-start)<<endl;
    if (numsteps * theta < (end - start)) numsteps++;
    numsteps=max(numsteps,2); // Whatever else, minimum representation of an arc is a straightline
    angle_step /=  (double) numsteps;
    end = DegreesToRadians((-_startAngle)+90.0);
    start = DegreesToRadians((-_endAngle)+90.0);
    double angle1 = start;

    Vec3d a = _center;
    Vec3d b;

    for (int r = 0; r <= numsteps; r++)
    {
        b = a + Vec3d(_radius * (double) sin(angle1), _radius * (double) cos(angle1), 0);
        angle1 += angle_step;
        vlist.push_back(b);
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
dxfPoint::assign(dxfFile* dxf, codeValue& cv)
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
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}

void
dxfPoint::drawScene(scene* sc)
{
    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->addPoint(getLayer(), _color,_a);
}

void
dxfPolyline::assign(dxfFile* dxf, codeValue& cv)
{
    string s = cv._string;
    if (cv._groupCode == 0) {
        if (s == "VERTEX") {
            _currentVertex = new dxfVertex;
            _vertices.push_back(_currentVertex);
        }
    } else if (_currentVertex) {
        _currentVertex->assign(dxf, cv);

        if ((_flag & 64 /*i.e. polymesh*/) &&
            (cv._groupCode == 70 /*i.e. vertex flag*/) &&
            (cv._int & 128 /*i.e. vertex is actually a face*/))
            _indices.push_back(_currentVertex); // Add the index only if _currentvertex is actually an index
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
                // Meaningful only when _surfacetype == 6, don' trust it for polymeshes.
                // From the docs :
                // "The 71 group specifies the number of vertices in the mesh, and the 72 group
                // specifies the number of faces. Although these counts are correct for all meshes
                // created with the PFACE command, applications are not required to place correct
                // values in these fields.)"
                // Amusing isn't it ?
                _mcount = cv._int; // 2005.12.13 pdr: group codes [70,78] now signed int.
                break;
            case 72:
                // Meaningful only when _surfacetype == 6, don' trust it for polymeshes.
                // From the docs :
                // "The 71 group specifies the number of vertices in the mesh, and the 72 group
                // specifies the number of faces. Although these counts are correct for all meshes
                // created with the PFACE command, applications are not required to place correct
                // values in these fields.)"
                // Amusing isn't it ?
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
        unsigned int ncount;
        unsigned int mcount;
        if (_surfacetype == 6) {
            // I dont have examples of type 5 and 8, but they may be the same as 6
            mcount = _mdensity;
            ncount = _ndensity;
        } else {
            mcount = _mcount;
            ncount = _ncount;
        }
        for (unsigned int n = 0; n < ncount-1; n++) {
            for (unsigned int m = 1; m < mcount; m++) {
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
            for (unsigned int n = 0; n < ncount-1; n++) {
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
            for (unsigned int m = 1; m < mcount; m++) {
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

/*
        // a naive attempt to determine vertex ordering
        VList::iterator itr = nlist.begin();
        Vec3d lastn = (*itr++);
        double bad_c = 0;
        double good_c = 0;
        long bad=0,good=0;
        for (; itr != nlist.end(); ++itr) {
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
*/

        if (qlist.size())
            sc->addQuads(getLayer(), _color, qlist, invert_order);
        if (vlist.size())
            sc->addTriangles(getLayer(), _color, vlist, invert_order);

    } else if (_flag & 64) {
        unsigned short _facetype = 3;

        for (unsigned int i = 0; i < _indices.size(); i++) {
            dxfVertex* vindice = _indices[i].get();
            if (!vindice) continue;
            if (vindice->getIndice4()) {
                _facetype = 4;
                d = _vertices[vindice->getIndice4()-1].get()->getVertex();
            } else {
                _facetype = 3;
            }
            if (vindice->getIndice3()) {
                c = _vertices[vindice->getIndice3()-1].get()->getVertex();
            } else {
                c = vindice->getVertex(); // Vertex not indexed. Use as is
            }
            if (vindice->getIndice2()) {
                b = _vertices[vindice->getIndice2()-1].get()->getVertex();
            } else {
                b = vindice->getVertex(); // Vertex not indexed. Use as is
            }
            if (vindice->getIndice1()) {
                a = _vertices[vindice->getIndice1()-1].get()->getVertex();
            } else {
                a = vindice->getVertex(); // Vertex not indexed. Use as is
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
        for (int i = _vertices.size()-1; i >= 0; i--)
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

    // sanity check (useful when no block remains after all unsupported entities have been filtered out)
    if (!_block)
        return;

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

    EntityList& l = _block->getEntityList();
    for (EntityList::iterator itr = l.begin(); itr != l.end(); ++itr) {
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

void
dxfText::assign(dxfFile* dxf, codeValue& cv)
{
    switch (cv._groupCode) {
        case 1:
            _string = cv._string;
            break;
        case 10:
            _point1.x() = cv._double;
            break;
        case 20:
            _point1.y() = cv._double;
            break;
        case 30:
            _point1.z() = cv._double;
            break;
        case 11:
            _point2.x() = cv._double;
            break;
        case 21:
            _point2.y() = cv._double;
            break;
        case 31:
            _point2.z() = cv._double;
            break;
        case 40:
            _height = cv._double;
            break;
        case 41:
            _xscale = cv._double;
            break;
        case 50:
            _rotation = cv._double;
            break;
        case 71:
            _flags = cv._int;
            break;
        case 72:
            _hjustify = cv._int;
            break;
        case 73:
            _vjustify = cv._int;
            break;
        case 210:
            _ocs.x() = cv._double;
            break;
        case 220:
            _ocs.y() = cv._double;
            break;
        case 230:
            _ocs.z() = cv._double;
            break;
        default:
            dxfBasicEntity::assign(dxf, cv);
            break;
    }
}


void
dxfText::drawScene(scene* sc)
{
    osgText::Text::AlignmentType align;

    Matrixd m;
    getOCSMatrix(_ocs, m);
    sc->ocs(m);

    ref_ptr<osgText::Text> _text = new osgText::Text;
    _text->setText(_string);

    _text->setCharacterSize( _height, 1.0/_xscale );
    _text->setFont("arial.ttf");

    Quat qr( DegreesToRadians(_rotation), Z_AXIS );

    if ( _flags & 2 ) qr = Quat( PI, Y_AXIS ) * qr;
    if ( _flags & 4 ) qr = Quat( PI, X_AXIS ) * qr;

    _text->setAxisAlignment(osgText::Text::USER_DEFINED_ROTATION);
    _text->setRotation(qr);

    if ( _hjustify != 0 || _vjustify !=0 ) _point1 = _point2;

    switch (_vjustify) {
    case 3:
        switch (_hjustify) {
        case 2:
            align = osgText::Text::RIGHT_TOP;
            break;
        case 1:
            align = osgText::Text::CENTER_TOP;
            break;
        default:
            align = osgText::Text::LEFT_TOP;
        }
        break;
    case 2:
        switch (_hjustify) {
        case 2:
            align = osgText::Text::RIGHT_CENTER;
            break;
        case 1:
            align = osgText::Text::CENTER_CENTER;
            break;
        default:
            align = osgText::Text::LEFT_CENTER;
        }
        break;
    case 1:
        switch (_hjustify) {
        case 2:
            align = osgText::Text::RIGHT_BOTTOM;
            break;
        case 1:
            align = osgText::Text::CENTER_BOTTOM;
            break;
        default:
            align = osgText::Text::LEFT_BOTTOM;
        }
        break;
    default:
        switch (_hjustify) {
        case 2:
            align = osgText::Text::RIGHT_BOTTOM_BASE_LINE;
            break;
        case 1:
            align = osgText::Text::CENTER_BOTTOM_BASE_LINE;
            break;
        default:
            align = osgText::Text::LEFT_BOTTOM_BASE_LINE;
        }
        break;
    }

    _text->setAlignment(align);

    sc->addText(getLayer(), _color, _point1, _text.get());
    sc->ocs_clear();
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
