/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */


#include <osg/Matrix>
#include <osg/Notify>

#include "normals.h"

#define CREASE_ANGLE 3.14159265356 * 45 / 180

float difAng(osg::Vec3 a, osg::Vec3 b) {
    float div = a.length() * b.length();
    return (div != 0.0f ? acosf( (a*b) / div) : 0.0f );
}

osg::Vec3 calcNormal(osg::Vec3 &a, osg::Vec3 &b, osg::Vec3 &c) {
    osg::Vec3 norm;
    osg::Vec3 v1;
    osg::Vec3 v2;
    v1=b-c;
    v2=a-b;
    norm=v1 ^ v2; // cross product
    norm.normalize();
    return norm;
}

void get3v(VertexList &vertices, VertexIndexList &vindex, int &a, int &b, int &c) {
    unsigned tam=vindex.size();
    unsigned i;
    b=0;c=0;
    a=vindex[0];
    for (i=1;i<tam;i++) {
	if (vertices[vindex[i]] != vertices[a]) { b=vindex[i];break; }
    }
    for (i=1;i<tam;i++) {
	if (vertices[vindex[i]] != vertices[a] && vertices[vindex[i]] != vertices[b]) { c=vindex[i];break; }
    }
}

osg::Vec3 *calcNormals(VertexList &vertices, PolygonList &polygons, unsigned nvert_total) {
    unsigned nPoly=polygons.size();
    osg::Vec3 *normales_polys = new osg::Vec3[nPoly];
    osg::Vec3 *normales = new osg::Vec3[nvert_total];
    unsigned pos=0;
    unsigned poly;
    /* Phase 1: Get flat normals */
    for (poly=0;poly<nPoly;poly++) {
	VertexIndexList vindex=*polygons[poly];
	if (vindex.size() > 2) {
            int v0,v1,v2;
            get3v(vertices,vindex,v0,v1,v2);
	    if (v0 == v1 || v0 == v2) {
		osg::notify(osg::WARN) << "ERROR: Vertices alineados: nv=" << vindex.size() <<  std::endl;
	    }
	    normales_polys[poly] = calcNormal(vertices[v0],
					      vertices[v1],
					      vertices[v2]);
	    if (normales_polys[poly] == osg::Vec3(0,0,0) && vindex.size() > 2) {
		osg::notify(osg::WARN) << "##***" << "Normal nula VERTICES=" << vindex.size() << " " << vertices[v0] << " " << vertices[v1] << " " << vertices[v2] << std::endl;
	    }
	}
    }
    /* Phase 2: Selective smooth depending on crease angle */
    for (poly=0;poly<nPoly;poly++) {
	VertexIndexList vindex=*polygons[poly];
        unsigned j;
	osg::Vec3 pnormal=normales_polys[poly];
	for (j=0;j<vindex.size();j++) {
	    int vertice=vindex[j];
	    normales[pos]=osg::Vec3(pnormal[0],pnormal[1],pnormal[2]);
	    unsigned poly2;
	    for (poly2=0;poly2<nPoly;poly2++) {
                if (poly2 == poly) continue;
		VertexIndexList vindex2=*polygons[poly2];
		unsigned k;
		osg::Vec3 pnormal2=normales_polys[poly2];
		for (k=0;k<vindex2.size();k++) {
		    if (vindex2[k] == vertice) {
			float ang=difAng(pnormal,pnormal2);
			if (ang < CREASE_ANGLE) {
			    normales[pos]+=osg::Vec3(pnormal2[0],pnormal2[1],pnormal2[2]);
			}
		    }
		}
	    }
	    normales[pos].normalize();
	    pos++;
	}
    }
    /* Phase 3: All done ;) */
    return normales;
}
