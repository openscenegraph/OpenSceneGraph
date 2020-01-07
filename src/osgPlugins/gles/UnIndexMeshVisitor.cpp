/* -*-c++-*- OpenSceneGraph - Copyright (C) Cedric Pinson 
*
* This application is open source and may be redistributed and/or modified   
* freely and without restriction, both in commercial and non commercial
* applications, as long as this copyright notice is maintained.
* 
* This application is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
*/

#include <osg/Geometry>
#include <osg/PrimitiveSet>

#include "UnIndexMeshVisitor"
#include "GeometryArray"
#include "PrimitiveIndexors"

typedef std::vector<unsigned int> IndexList;
// this help works only for indexed primitive to unindex it


void UnIndexMeshVisitor::process(osg::Geometry& geom)
{
    // no point optimizing if we don't have enough vertices.
    if (!geom.getVertexArray()) return;

    // check for the existence of surface primitives
    unsigned int numIndexedPrimitives = 0;
    osg::Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    osg::Geometry::PrimitiveSetList::iterator itr;
    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        osg::PrimitiveSet::Type type = (*itr)->getType();
        if ((type == osg::PrimitiveSet::DrawElementsUBytePrimitiveType
             || type == osg::PrimitiveSet::DrawElementsUShortPrimitiveType
             || type == osg::PrimitiveSet::DrawElementsUIntPrimitiveType))
            numIndexedPrimitives++;
    }
    
    // no polygons or no indexed primitive, nothing to do
    if (!numIndexedPrimitives) {
        return;
    }

    // we don't manage lines
    
    GeometryArrayList arraySrc(geom);
    GeometryArrayList arrayList = arraySrc.cloneType();

    osg::Geometry::PrimitiveSetList newPrimitives;

    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        osg::PrimitiveSet::Mode mode = (osg::PrimitiveSet::Mode)(*itr)->getMode();

        switch(mode) {

            // manage triangles
        case(osg::PrimitiveSet::TRIANGLES):
        case(osg::PrimitiveSet::TRIANGLE_STRIP):
        case(osg::PrimitiveSet::TRIANGLE_FAN):
        case(osg::PrimitiveSet::QUADS):
        case(osg::PrimitiveSet::QUAD_STRIP):
        case(osg::PrimitiveSet::POLYGON):
        {
            // for each geometry list indexes of vertices
            // to makes triangles
            TriangleIndexor triangleIndexList;
            (*itr)->accept(triangleIndexList);

            unsigned int index = arrayList.size();

            // now copy each vertex to new array, like a draw array
            arraySrc.append(triangleIndexList._indices, arrayList);

            newPrimitives.push_back(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,
                                                        index,
                                                        triangleIndexList._indices.size()));
        }
        break;

        // manage lines
        case(osg::PrimitiveSet::LINES):
        case(osg::PrimitiveSet::LINE_STRIP):
        case(osg::PrimitiveSet::LINE_LOOP):
        {
            EdgeIndexor edgesIndexList;
            (*itr)->accept(edgesIndexList);

            unsigned int index = arrayList.size();

            // now copy each vertex to new array, like a draw array
            arraySrc.append(edgesIndexList._indices, arrayList);

            newPrimitives.push_back(new osg::DrawArrays(osg::PrimitiveSet::LINES,
                                                        index,
                                                        edgesIndexList._indices.size()));
        }
        break;
        case(osg::PrimitiveSet::POINTS):
        {
            PointIndexor pointsIndexList;
            (*itr)->accept(pointsIndexList);

            unsigned int index = arrayList.size();

            // now copy each vertex to new array, like a draw array
            arraySrc.append(pointsIndexList._indices, arrayList);
            newPrimitives.push_back(new osg::DrawArrays(osg::PrimitiveSet::POINTS,
                                                        index,
                                                        pointsIndexList._indices.size()));
        }
        break;
        default:
            break;
        }
    }

    arrayList.setToGeometry(geom);
    geom.setPrimitiveSetList(newPrimitives);
}
