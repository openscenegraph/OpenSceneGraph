/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
 *
 * ViewDependentShadow codes Copyright (C) 2008 Wojciech Lewandowski
 * Thanks to to my company http://www.ai.com.pl for allowing me free this work.
*/

#include<osgShadow/ConvexPolyhedron>
#include<osg/Group>
#include<osg/Geode>
#include<osgDB/WriteFile>

#include<cassert>
#include<deque>
#include<algorithm>
#include<iterator>

#include <stdio.h>
#include <string.h>

using namespace osgShadow;


#if defined( DEBUG ) || defined( _DEBUG ) || defined( _DEBUG_ )
// ConvexPolyhedron may produce tons of warnings when it becomes non convex.
// Unfortuantely this condition often happens in daily routine of shadow usage
// due precision errors mixed with repeating frustum cuts performed by MinimalShadowClasses.
// However, in most of above cases this condition is not fatal
// because polyhedron becomes concave by very small margin (mesuring deep the hole).
// Unfortunately warnings are produced even for such small margin cases and can
// easily flood the console.
// So I leave MAKE_CHECKS commented out. Its really useful only for a guy who debugs
// larger concaveness issues which means most developers will want to keep it commented.
//    #define MAKE_CHECKS 1
#endif

#if MAKE_CHECKS

#define WARN  OSG_WARN

#define CONVEX_POLYHEDRON_CHECK_COHERENCY             1
#define CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA     2
#define CONVEX_POLYHEDRON_DUMP_ON_INCOHERENT_DATA     2
#define CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON         1
#define CONVEX_POLYHEDRON_DUMP_ON_BAD_POLYGON         1
#define CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON     1
#define CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON     1
#define CONVEX_POLYHEDRON_WARN_ON_INCORRECT_FACE_CUT  1
#define CONVEX_POLYHEDRON_DUMP_ON_INCORRECT_FACE_CUT  1


#else

#define WARN OSG_WARN

#define CONVEX_POLYHEDRON_CHECK_COHERENCY             0
#define CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA     0
#define CONVEX_POLYHEDRON_DUMP_ON_INCOHERENT_DATA     0
#define CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON         0
#define CONVEX_POLYHEDRON_DUMP_ON_BAD_POLYGON         0
#define CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON     0
#define CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON     0
#define CONVEX_POLYHEDRON_WARN_ON_INCORRECT_FACE_CUT  0
#define CONVEX_POLYHEDRON_DUMP_ON_INCORRECT_FACE_CUT  0

#endif

const osg::Matrix & ConvexPolyhedron::defaultMatrix = *(osg::Matrix*)NULL;
static const double epsi = pow( 2.0, -20.0 ); //circa 8 times float prec(~2^-23)
static const double plane_hull_tolerance    = 1.0e-5;
static const double point_plane_tolerance   = 0.0;
static const double point_point_equivalence = 0.;

// Tim Moore modifications for GCC 4.3 August 15, 2008
// they correspond to Adrian Egli tweaks for VS 7.3 on August 19, 2008
namespace
{
typedef std::vector< double > Distances;
typedef std::vector< osg::Vec4d > Points;

// Auxiliary params continued per face
struct FaceDistances
{
    ConvexPolyhedron::Faces::iterator itr;
    Points points;
    Distances distances;
    int below, above, on;
};
} // namespace

ConvexPolyhedron::ConvexPolyhedron( const osg::Matrix& matrix,
                        const osg::Matrix& inverse,
                        const osg::BoundingBox& bb )
{
    setToBoundingBox( bb );

    if( &matrix != &defaultMatrix && &inverse != &defaultMatrix )
        transform( matrix, inverse );
    else if( &matrix != &defaultMatrix && &inverse == &defaultMatrix )
        transform( matrix, osg::Matrix::inverse( matrix ) );
    else if( &matrix == &defaultMatrix && &inverse != &defaultMatrix )
        transform( osg::Matrix::inverse( inverse ), matrix );
}

void ConvexPolyhedron::setToUnitFrustum(bool withNear, bool withFar)
{
    const osg::Vec3d v000(-1.0,-1.0,-1.0);
    const osg::Vec3d v010(-1.0,1.0,-1.0);
    const osg::Vec3d v001(-1.0,-1.0,1.0);
    const osg::Vec3d v011(-1.0,1.0,1.0);
    const osg::Vec3d v100(1.0,-1.0,-1.0);
    const osg::Vec3d v110(1.0,1.0,-1.0);
    const osg::Vec3d v101(1.0,-1.0,1.0);
    const osg::Vec3d v111(1.0,1.0,1.0);

    _faces.clear();

    {   // left plane.
        Face& face = createFace();
        face.name = "left";
        face.plane.set(1.0,0.0,0.0,1.0);
        face.vertices.push_back(v000);
        face.vertices.push_back(v001);
        face.vertices.push_back(v011);
        face.vertices.push_back(v010);
    }

    {   // right plane.
        Face& face = createFace();
        face.name = "right";
        face.plane.set(-1.0,0.0,0.0,1.0);
        face.vertices.push_back(v100);
        face.vertices.push_back(v110);
        face.vertices.push_back(v111);
        face.vertices.push_back(v101);
    }

    {   // bottom plane.
        Face& face = createFace();
        face.name = "bottom";
        face.plane.set(0.0,1.0,0.0,1.0);
        face.vertices.push_back(v000);
        face.vertices.push_back(v100);
        face.vertices.push_back(v101);
        face.vertices.push_back(v001);
    }

    {   // top plane.
        Face& face = createFace();
        face.name = "top";
        face.plane.set(0.0,-1.0,0.0,1.0);
        face.vertices.push_back(v010);
        face.vertices.push_back(v011);
        face.vertices.push_back(v111);
        face.vertices.push_back(v110);
    }

    if (withNear)
    {   // near plane
        Face& face = createFace();
        face.name = "near";
        face.plane.set(0.0,0.0,1.0,1.0);
        face.vertices.push_back(v000);
        face.vertices.push_back(v010);
        face.vertices.push_back(v110);
        face.vertices.push_back(v100);
    }

    if (withFar)
    {   // far plane
        Face& face = createFace();
        face.name = "far";
        face.plane.set(0.0,0.0,-1.0,1.0);
        face.vertices.push_back(v001);
        face.vertices.push_back(v101);
        face.vertices.push_back(v111);
        face.vertices.push_back(v011);
    }
}

void ConvexPolyhedron::setToBoundingBox(const osg::BoundingBox& bb)
{
    _faces.clear();

    // Ignore invalid and one dimensional boxes
    if( bb._min[0] >= bb._max[0] ||
        bb._min[1] >= bb._max[1] ||
        bb._min[2] >= bb._max[2] ) return;

    const osg::Vec3d v000(bb.xMin(),bb.yMin(), bb.zMin());
    const osg::Vec3d v010(bb.xMin(),bb.yMax(), bb.zMin());
    const osg::Vec3d v001(bb.xMin(),bb.yMin(), bb.zMax());
    const osg::Vec3d v011(bb.xMin(),bb.yMax(), bb.zMax());
    const osg::Vec3d v100(bb.xMax(),bb.yMin(), bb.zMin());
    const osg::Vec3d v110(bb.xMax(),bb.yMax(), bb.zMin());
    const osg::Vec3d v101(bb.xMax(),bb.yMin(), bb.zMax());
    const osg::Vec3d v111(bb.xMax(),bb.yMax(), bb.zMax());



    {   // x min plane
        Face& face = createFace();
        face.name = "xMin";
        face.plane.set(1.0,0.0,0.0,-bb.xMin());
        face.vertices.push_back(v000);
        face.vertices.push_back(v001);
        face.vertices.push_back(v011);
        face.vertices.push_back(v010);
    }

    {   // x max plane.
        Face& face = createFace();
        face.name = "xMax";
        face.plane.set(-1.0,0.0,0.0,bb.xMax());
        face.vertices.push_back(v100);
        face.vertices.push_back(v110);
        face.vertices.push_back(v111);
        face.vertices.push_back(v101);
    }

    {   // y min plane.
        Face& face = createFace();
        face.name = "yMin";
        face.plane.set(0.0,1.0,0.0,-bb.yMin());
        face.vertices.push_back(v000);
        face.vertices.push_back(v100);
        face.vertices.push_back(v101);
        face.vertices.push_back(v001);
    }

    {   // y max plane.
        Face& face = createFace();
        face.name = "yMax";
        face.plane.set(0.0,-1.0,0.0,bb.yMax());
        face.vertices.push_back(v010);
        face.vertices.push_back(v011);
        face.vertices.push_back(v111);
        face.vertices.push_back(v110);
    }
    {   // z min plane
        Face& face = createFace();
        face.name = "zMin";
        face.plane.set(0.0,0.0,1.0,-bb.zMin());
        face.vertices.push_back(v000);
        face.vertices.push_back(v010);
        face.vertices.push_back(v110);
        face.vertices.push_back(v100);
    }

    {   // z max plane
        Face& face = createFace();
        face.name = "zMax";
        face.plane.set(0.0,0.0,-1.0,bb.zMax());
        face.vertices.push_back(v001);
        face.vertices.push_back(v101);
        face.vertices.push_back(v111);
        face.vertices.push_back(v011);
    }
}

void ConvexPolyhedron::transform(const osg::Matrix& matrix, const osg::Matrix& inverse)
{
    bool requires_infinite_plane_clip = false;

    ConvexPolyhedron cp = *this;
    for(Faces::iterator itr = _faces.begin();
        itr != _faces.end() && !requires_infinite_plane_clip;
        ++itr)
    {
        Face& face = *itr;
        face.plane.transformProvidingInverse(inverse);
        for(Vertices::iterator vitr = face.vertices.begin();
            vitr != face.vertices.end();
            ++vitr)
        {
            osg::Vec4d v( *vitr, 1.0 );
            v = v * matrix;

            if( v[3] <= 0 ) {
                requires_infinite_plane_clip = true;
                break;
            }

            vitr->set( v[0]/v[3], v[1]/v[3], v[2]/v[3] );
        }
    }

    if( requires_infinite_plane_clip ) {
        *this = cp;
        transformClip( matrix, inverse );
//        cp.dumpGeometry(&cp._faces.back(),&cp._faces.back().plane,this);
    }

    // Perpective transforms and lack of precision
    // occasionaly cause removal of some points

    removeDuplicateVertices( );

    checkCoherency( true, "ConvexPolyhedron::transform" );
}

void ConvexPolyhedron::transformClip(const osg::Matrix& matrix, const osg::Matrix& inverse)
{
    double tolerance = 0.00001;

    if( _faces.empty() ) return;


    ConvexPolyhedron cp( *this );

    typedef std::vector< FaceDistances > FaceDistancesList;
    FaceDistancesList faceDistances;
    faceDistances.resize( _faces.size() );

    double min = FLT_MAX, max = -FLT_MAX; //Hull max & min point distances

    FaceDistancesList::iterator fd = faceDistances.begin();
    // First step compute each face points distances to cutting plane
    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr, ++fd )
    {
        fd->itr = itr;
        fd->distances.reserve( itr->vertices.size() );
        fd->on = 0;
        fd->above = 0;
        fd->below = 0;

        itr->plane.transformProvidingInverse(inverse);

        for( Vertices::iterator vitr = itr->vertices.begin();
            vitr != itr->vertices.end();
            ++vitr)
        {
            osg::Vec4d p( *vitr, 1.0 );
            p = p * matrix;
            vitr->set( p[0]/p[3], p[1]/p[3], p[2]/p[3] );

            double d = p[3]-tolerance;

            fd->points.push_back( p );
            fd->distances.push_back( d );
            if ( d>point_plane_tolerance )       ++fd->above;
            else if ( d<-point_plane_tolerance ) ++fd->below;
            else                                 ++fd->on;
            min = osg::minimum( min, d );
            max = osg::maximum( max, d );
        }
    }

    if( max <= plane_hull_tolerance ) { // All points on or below cutting plane
        _faces.clear();
        return;
    }

    if( min >= -plane_hull_tolerance ) { // All points on or above cutting plane
        return;
    }

    typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
    typedef std::set< Edge > Edges;
    Edges edges;

    for( FaceDistancesList::iterator fd = faceDistances.begin();
        fd != faceDistances.end();
        ++fd )
    {
        if ( fd->below == 0 )
        {  // skip face if all points on or above cutting plane ( below == 0 )
            continue;
        }

        if ( /* fd->below > 0 && */ fd->above == 0 && fd->on == 0 )
        {
            _faces.erase( fd->itr ); // remove face if points below or on
            continue;
        }

        // cut the face if some points above and below plane
        // assert( fd->below > 0 && fd->above > 0 );

        Face& face = *(fd->itr);
        Points& vertices = fd->points;
        Distances& distances = fd->distances;
        Vertices newFaceVertices;
        Vertices newVertices;

        for(unsigned int i=0; i < vertices.size(); ++i)
        {
            osg::Vec4d &va = vertices[i];
            osg::Vec4d &vb = vertices[(i+1)%vertices.size()];
            double &distance_a = distances[i];
            double &distance_b = distances[(i+1)%vertices.size()];

            // Is first edge point above or on the plane?
            if ( -point_plane_tolerance <= distance_a ) {

                osg::Vec3d v( vertices[i][0], vertices[i][1], vertices[i][2] );
                v /= vertices[i][3];

                if( newVertices.empty() || v != newVertices.back() )
                    newVertices.push_back( v );

                if ( distance_a <= point_plane_tolerance ) {
                    if( newFaceVertices.empty() || v != newFaceVertices.back() )
                        newFaceVertices.push_back( v );
                }
            }

            // Does edge intersect plane ?
            if ( ( distance_a < -point_plane_tolerance && distance_b > point_plane_tolerance ) ||
                 ( distance_b < -point_plane_tolerance && distance_a > point_plane_tolerance ) )
            {
                osg::Vec4d intersection; // Inserting vertex
                double da = fabs( distance_a ), db = fabs( distance_b );

                // tweaks to improve coherency of polytope after cut
                if( da <= point_point_equivalence && da <= db ) {
                    intersection = va;
                } else if( db <= point_point_equivalence && db <= da ) {
                    intersection = vb;
                } else {
                    double dab4 = 0.25 * ( da + db );
                    if( dab4 < da && dab4 < db ) {
                        intersection = (vb*distance_a - va*distance_b)/(distance_a-distance_b);
                    } else {
                        osg::Vec4d v = (vb - va)/(distance_a-distance_b);
                        if( da < db )
                            intersection = va + v * distance_a;
                        else
                            intersection = vb + v * distance_b;
                    }
                }


                osg::Vec3d v( intersection[0], intersection[1], intersection[2] );
                v /= intersection[3];

                if( newVertices.empty() || v != newVertices.back() )
                    newVertices.push_back( v );

                if( newFaceVertices.empty() || v != newFaceVertices.back() )
                    newFaceVertices.push_back( v );
            }
        }

        if( newVertices.size() && newVertices.front() == newVertices.back() )
            newVertices.pop_back();

        if( newFaceVertices.size() && newFaceVertices.front() == newFaceVertices.back() )
            newFaceVertices.pop_back();

        if( newFaceVertices.size() == 1 ) {  // This is very rare but correct
            WARN
                << "ConvexPolyhedron::transformClip - Slicing face polygon returns "
                << newFaceVertices.size()
                << " points. Should be 2 (usually) or 1 (rarely)."
                << std::endl;

        } else if( newFaceVertices.size() == 2 ) {
            if( newFaceVertices[0] < newFaceVertices[1] ) {
                edges.insert( Edge( newFaceVertices[0], newFaceVertices[1] ) );
            } else {
                edges.insert( Edge( newFaceVertices[1], newFaceVertices[0] ) );
            }
        } else if( newFaceVertices.size() > 2 ) {

#if CONVEX_POLYHEDRON_WARN_ON_INCORRECT_FACE_CUT

            // This may happen if face polygon is not planar or convex.
            // It may happen when face was distorted by projection transform
            // or when some polygon vertices land in incorrect positions after
            // some transformation by badly conditioned matrix (weak precison)

            WARN
                << "ConvexPolyhedron::transformClip - Slicing face polygon returns "
                << newFaceVertices.size()
                << " points. Should be 2 (usually) or 1 (rarely)."
                << std::endl;
#endif

#if CONVEX_POLYHEDRON_DUMP_ON_INCORRECT_FACE_CUT
            dumpGeometry( &face, NULL, &cp );
#endif

            // Lets try to recover from this uneasy situation
            // by comparing current face polygon edges cut by new plane
            // with edges selected for new plane

            unsigned i0 = 0, i1 = newFaceVertices.size() - 1;
            unsigned j0 = 0, j1 = newVertices.size() - 1;

            for( ; i0 < newFaceVertices.size(); i1 = i0++, j1 = j0++ ) {
                while( newFaceVertices[i0] != newVertices[j0] ) j1 = j0++;
                if( newFaceVertices[i1] == newVertices[j1] )
                {
                    if( newFaceVertices[i0] < newFaceVertices[i1] ) {
                        edges.insert( Edge( newFaceVertices[i0], newFaceVertices[i1] ) );
                    } else {
                        edges.insert( Edge( newFaceVertices[i1], newFaceVertices[i0] ) );
                    }
                }
            }
        }

        if( newVertices.size() >= 3 ) { //Add faces with at least 3 points

#if ( CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON || CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON )
            int convex = isFacePolygonConvex( face );
            face.vertices.swap( newVertices );
            if( convex && !isFacePolygonConvex( face ) ) {
#if CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON
                WARN << "ConvexPolyhedron::transformClip - polygon output non convex."
                << " This may lead to other issues in ConvexPolyhedron math" << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON
                dumpGeometry( &face, NULL, &cp );
#endif
            }
#else
            face.vertices.swap( newVertices );
#endif
        } else //Remove face reduced to 0, 1, 2 points
            _faces.erase( fd->itr );
    }

    osg::Vec3d center( 0, 0, 0 );
    unsigned count = 0;
    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr )
    {
        for( Vertices::iterator vitr = itr->vertices.begin();
            vitr != itr->vertices.end();
            ++vitr )
        {
            center += *vitr;
            count ++;
        }
    }

    center /= count;


    if ( edges.size() > 1 ) //Ignore faces reduced to 0, 1, 2 points
    {
        Face face;
        face.name = "Almost infinite plane";

        std::deque< osg::Vec3d > vertices;

        Edges::iterator itr = edges.begin();
        vertices.push_back( itr->first );
        vertices.push_back( itr->second );
        edges.erase( itr++ );

        for( unsigned int vertices_size = 0;
            vertices_size < vertices.size(); )
        {
            vertices_size = vertices.size();
            for( itr = edges.begin(); itr != edges.end(); )
            {
                bool not_added = false;

                if( itr->first == vertices.back() )
                    vertices.push_back( itr->second );
                else if ( itr->first == vertices.front() )
                    vertices.push_front( itr->second );
                else if ( itr->second == vertices.back() )
                    vertices.push_back( itr->first );
                else if ( itr->second == vertices.front() )
                    vertices.push_front( itr->first );
                else
                    not_added = true;

                if( not_added )
                    ++itr;
                else
                    edges.erase( itr++ );
            }
        }

#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
        if( !edges.empty() ) {
            WARN
                << "ConvexPolyhedron::transformClip - Building new face polygon - "
                << "Found edges not matching former polygon ends"
                << std::endl;
        }
#endif

        std::copy(vertices.begin(), vertices.end(), std::back_inserter(face.vertices));

        face.plane.set( vertices[0], vertices[1], vertices[2] );

        if( face.plane.distance( center ) < 0.0 ) face.plane.flip();

        _faces.push_back(face);

        // Last vertex is duplicated - remove one instance
        if( face.vertices.front() == face.vertices.back() )
            face.vertices.pop_back();
        else {// If not duplicated then it may mean we have open polygon ;-(
#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
            WARN
            << "ConvexPolyhedron::transformClip - Building new face polygon - "
            << " Polygon not properly closed."
            << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_BAD_POLYGON
            dumpGeometry( &_faces.back(), NULL, &cp );
#endif
        }

#if ( CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON || CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON )
        if( !isFacePolygonConvex( face ) ) {
#if CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON
            WARN << "ConvexPolyhedron::transformClip - new face polygon non convex."
                << " This may lead to other issues in ConvexPolyhedron math" << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON
            ConvexPolyhedron cp;
            cp.createFace() = face;
            cp.dumpGeometry( );
#endif
        }
#endif
    }

    // Perpective transforms and lack of precision
    // occasionaly cause removal of some points

    removeDuplicateVertices( );

    checkCoherency( true, "ConvexPolyhedron::transformClip" );
}

bool ConvexPolyhedron::mergeFaces
    ( const Face & face0, const Face & face1, Face & face )
{
    typedef std::pair< osg::Vec3d, osg::Vec3d >  Edge;
    typedef std::set<Edge> Edges;
    Edges edges;

    for(unsigned int i=0; i<face0.vertices.size(); ++i)
    {
        const osg::Vec3d* va = &face0.vertices[i];
        const osg::Vec3d* vb = &face0.vertices[(i+1)%face0.vertices.size()];
        if( *vb == *va ) continue; // ignore duplicated point edges
        if( *vb < *va ) std::swap( va, vb );
        edges.insert( Edge( *va, *vb ) );
    }

    bool intersection = false;
    for(unsigned int i=0; i<face1.vertices.size(); ++i)
    {
        const osg::Vec3d* va = &face1.vertices[i];
        const osg::Vec3d* vb = &face1.vertices[(i+1)%face1.vertices.size()];
        if( *vb == *va ) continue; // ignore duplicated point edges
        if( *vb < *va ) std::swap( va, vb );
        Edge edge( *va, *vb );
        Edges::iterator itr = edges.find( edge );
        if( itr == edges.end() )
            edges.insert( edge );
        else {
            edges.erase( itr );
            intersection = true;
        }
    }

    if( intersection )
        face.vertices.clear();

    if( intersection && edges.size() > 1 ) //Ignore faces reduced to 0, 1, 2 points
    {
        std::deque< osg::Vec3d > vertices;

        Edges::iterator itr = edges.begin();
        vertices.push_back( itr->first );
        vertices.push_back( itr->second );
        edges.erase( itr++ );

        for( unsigned int vertices_size = 0;
            vertices_size < vertices.size(); )
        {
            vertices_size = vertices.size();
            for( itr = edges.begin(); itr != edges.end(); )
            {
                bool not_added = false;

                if( itr->first == vertices.back() )
                    vertices.push_back( itr->second );
                else if ( itr->first == vertices.front() )
                    vertices.push_front( itr->second );
                else if ( itr->second == vertices.back() )
                    vertices.push_back( itr->first );
                else if ( itr->second == vertices.front() )
                    vertices.push_front( itr->first );
                else
                    not_added = true;

                if( not_added )
                    ++itr;
                else
                    edges.erase( itr++ );
            }
        }

#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
        if( !edges.empty() ) {
            WARN
                << "ConvexPolyhedron::mergeFaces - Building new face polygon - "
                << "Found edges not matching former polygon ends."
                << std::endl;
        }
#endif

        // Last vertex is duplicated - remove one instance
        if( vertices.front() == vertices.back() )
            vertices.pop_back();
        else {// If not duplicated then it may mean we have open polygon ;-(
#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
            WARN
            << "ConvexPolyhedron::mergeFaces - Building new face polygon - "
            << " Polygon not properly closed."
            << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_BAD_POLYGON
#endif
        }

#if 1 // Resulting plane will be the same as face0
        face.plane = face0.plane;

        std::copy(vertices.begin(), vertices.end(), std::back_inserter(face.vertices));

#else // Compute resulting plane as average of faces (not a good idea)
        osg::Vec3d normal = face0.plane.getNormal() + face1.plane.getNormal();
        normal.normalize();

        osg::Vec3d center;
        for( unsigned int i = 0; i < vertices.size(); ++i )
        {
            center += vertices[i];
            face.vertices.push_back( vertices[i] );
        }
        center /= vertices.size();

        face.plane.set( normal, center );
#endif

        face.name = face0.name + " + "  + face1.name;

        // No testing for concave polys
        // Its possible to build concave polygon from two merged faces
        // But after all coplanar faces are merged final result should be convex.
    }
    return intersection;
}

void ConvexPolyhedron::mergeCoplanarFaces
    ( const double & dot_tolerance, const double & delta_tolerance )
{
    for(Faces::iterator itr0 = _faces.begin();
        itr0 != _faces.end();
        ++itr0 )
    {
        double tolerance = delta_tolerance;
        for( unsigned i = 0; i < itr0->vertices.size(); ++i ) {
            tolerance = osg::maximum( tolerance,
                fabs( itr0->plane.distance( itr0->vertices[i] ) ) );
        }

        for(Faces::iterator itr1 = _faces.begin();
            itr1 != _faces.end();
            )
        {
            if( itr1 == itr0 ) {
                ++itr1;
                continue;
            }

            bool attempt_merge = true;
            for( unsigned i = 0; i < itr1->vertices.size(); ++i ) {
                if( fabs( itr0->plane.distance( itr1->vertices[i] ) ) > tolerance )
                {
                    attempt_merge = false;
                    break;
                }
            }

            if( !attempt_merge &&
                1.0 - itr0->plane.getNormal() * itr1->plane.getNormal() < dot_tolerance &&
                fabs( itr0->plane.ptr()[3] - itr1->plane.ptr()[3] ) < delta_tolerance )
                    attempt_merge = true;

            if( attempt_merge && mergeFaces( *itr0, *itr1, *itr0 ) )
                itr1 = _faces.erase( itr1 );
            else
                ++itr1;
        }
    }
}

void ConvexPolyhedron::removeDuplicateVertices( void )
{
#if 1
    // Aggressive removal, find very close points and replace them
    // with their average. Second step wil do the rest.

    typedef std::map< osg::Vec3f, osg::Vec4d > Points;
    typedef std::set< osg::Vec3d > VertexSet;

    VertexSet  vertexSet;
    Points     points;

    for( Faces::iterator itr = _faces.begin();
         itr != _faces.end();
         ++itr )
    {
        for( Vertices::iterator vitr = itr->vertices.begin();
             vitr != itr->vertices.end();
             ++vitr )
        {
            vertexSet.insert( *vitr );
        }
    }

    for( VertexSet::iterator vitr = vertexSet.begin();
         vitr != vertexSet.end();
         ++vitr )
    {
        points[ *vitr ] += osg::Vec4d( *vitr, 1.0 );
    }

    for( Points::iterator itr = points.begin();
         itr != points.end();
         ++itr )
    {
        if( itr->second[3] > 1.0 )
            itr->second /= itr->second[3];
    }

    for( Faces::iterator itr = _faces.begin();
         itr != _faces.end();
         ++itr )
    {
        for( Vertices::iterator vitr = itr->vertices.begin();
             vitr != itr->vertices.end();
             ++vitr )
        {
            osg::Vec4d &v = points[ *vitr ];
            *vitr = osg::Vec3d( v[0], v[1], v[2] );
        }
    }
#endif

    for( Faces::iterator itr = _faces.begin();
         itr != _faces.end();
        )
    {
        assert( itr->vertices.size() > 0 );

        osg::Vec3d prev = itr->vertices.back();

        for( Vertices::iterator vitr = itr->vertices.begin();
            vitr != itr->vertices.end();
            )
        {
            if( *vitr == prev ) {
                vitr = itr->vertices.erase( vitr );
            } else {
                prev = *vitr;
                ++vitr;
            }
        }

        if( itr->vertices.size() < 3 )
            itr = _faces.erase( itr );
        else
            ++itr;
    }

    mergeCoplanarFaces();

#if 1
    // Experimentally remove vertices on colinear edge chains
    // effectivelyy replacing them with one edge
    typedef std::map<osg::Vec3d,int> VertexCounter;
    VertexCounter vertexCounter;

    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        for( Vertices::iterator vitr = itr->vertices.begin();
             vitr != itr->vertices.end();
             ++vitr )
        {
            ++vertexCounter[ *vitr ];
        }
    }

    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        )
    {
        for( Vertices::iterator vitr = itr->vertices.begin();
             vitr != itr->vertices.end();
             )
        {
            if( vertexCounter[ *vitr ] == 2 ) {
#if 0 // Sanity check if we could remove this point without changing poly shape

                Vertices::iterator next = ( vitr + 1 == itr->vertices.end() ?
                                            itr->vertices.begin() :
                                            vitr + 1 );

                Vertices::iterator prev = ( vitr == itr->vertices.begin() ?
                                            itr->vertices.end() - 1 :
                                            vitr - 1 );


                osg::Vec3d va = *vitr - *prev;
                osg::Vec3d vb = *next - *vitr;

                double da = va.normalize();
                double db = vb.normalize();

                if( 0.001 < da && 0.001 < db )
                {
                    double dot = va * vb, limit = cos( 0.01 );
                    if( dot < limit ) {
                        WARN << "ConvexPolyhedron::removeDuplicateVertices"
                            << " - removed mid point connecting two non colinear edges."
                            << " Angle(deg): " << osg::RadiansToDegrees( acos( dot ) )
                            << " Length1: " << da
                            << " Length2: " << db
                            << std::endl;
                    }
                } else {
                    if( da == 0.0 || db == 0.0 )
                        WARN << "ConvexPolyhedron::removeDuplicateVertices"
                             << " - removed degenerated mid point connecting two edges"
                            << std::endl;

                }
#endif
                vitr = itr->vertices.erase( vitr );
            } else {
                ++vitr;
            }
        }

        if( itr->vertices.size() < 3 )
            itr = _faces.erase( itr );
        else
            ++itr;
    }
#endif

    checkCoherency( false, "Leave ConvexPolyhedron::removeDuplicateVertices" );
}

int ConvexPolyhedron::pointsColinear
    ( const osg::Vec3d & a, const osg::Vec3d & b, const osg::Vec3d & c,
      const double & dot_tolerance, const double & delta_tolerance )
{
    osg::Vec3d va = b - a;
    osg::Vec3d vb = c - b;

    double da = va.normalize();
    double db = vb.normalize();

    if( delta_tolerance >= da || delta_tolerance >= db )
        return -1; // assume collinearity if one of the edges is zero length

    if( 1.0 - fabs( va * vb ) <= dot_tolerance )
        return 1; // edge normals match collinearity condition

    return 0; // nope. not collinear
}

int ConvexPolyhedron::isFacePolygonConvex( Face & face, bool ignoreColinearVertices )
{
    int positive = 0, negative = 0, colinear = 0;
    for( unsigned int i = 0; i < face.vertices.size(); ++i )
    {
        osg::Vec3d va = face.vertices[i];
        osg::Vec3d vb = face.vertices[(i+1)%face.vertices.size()];
        osg::Vec3d vc = face.vertices[(i+2)%face.vertices.size()];


#if ( CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA  > 1 || CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON )
        double dist = fabs( face.plane.distance( va ) );
        if( dist > 0.0001 )
        {
            WARN << "ConvexPolyhedron::isFacePolygonConvex - plane point too far from plane (" << dist <<")" << std::endl;
        }
#endif

        // cast points on a plane
        va -= face.plane.getNormal() * face.plane.distance( va );
        vb -= face.plane.getNormal() * face.plane.distance( vb );
        vc -= face.plane.getNormal() * face.plane.distance( vc );

        if( pointsColinear( va, vb, vc ) ) {
            colinear++;
        } else {
            double side =( ( vc  - vb ) ^ ( vb - va ) ) * face.plane.getNormal();

            if( side < 0 ) negative++;
            if( side > 0 ) positive++;
        }
    }

    if( !ignoreColinearVertices && colinear > 0 )
        return 0;

    if( !negative && !positive )
        return 0;

    if( (negative + colinear) == static_cast<int>(face.vertices.size()) )
        return -( negative + colinear );

    if( (positive + colinear) == static_cast<int>(face.vertices.size()) )
        return +( positive + colinear );

    return 0;
}

bool ConvexPolyhedron::checkCoherency
    ( bool checkForNonConvexPolys, const char * errorPrefix )
{
    bool result = true;
    bool convex = true;

#if CONVEX_POLYHEDRON_CHECK_COHERENCY

    if( !errorPrefix ) errorPrefix = "ConvexPolyhedron";

    typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
    typedef std::map<Edge,int> EdgeCounter;
    typedef std::map<osg::Vec3d,int> VertexCounter;

    EdgeCounter edgeCounter;
    VertexCounter vertexCounter;

    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        Face& face = *itr;

        if( checkForNonConvexPolys && !isFacePolygonConvex( face ) ) {
            convex = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA || CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON )
            WARN << errorPrefix <<
                " - coherency fail - face polygon concave" << std::endl;
#endif
#if ( 1 < CONVEX_POLYHEDRON_DUMP_ON_INCOHERENT_DATA || CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON )
            dumpGeometry( &face );
#endif
        }

        Vertices& vertices = face.vertices;
        for(unsigned int i=0; i<vertices.size(); ++i)
        {
            osg::Vec3d& a = vertices[ i ];
            osg::Vec3d& b = vertices[ (i+1) % vertices.size()];
            ++vertexCounter[ a ];
            if (a<b)
                ++edgeCounter[Edge(a,b)];
            else
                ++edgeCounter[Edge(b,a)];
        }
    }


    for( EdgeCounter::iterator itr = edgeCounter.begin();
        itr != edgeCounter.end();
        ++itr)
    {
        const Edge &e = itr->first;

        if( e.first.isNaN() ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Vertex is NaN." << std::endl;
#endif
        }

        if( e.second.isNaN() ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Vertex is NaN." << std::endl;
#endif
        }

        if( e.first == e.second ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Edge with identical vertices." << std::endl;
#endif
        }

        if( vertexCounter[ e.first ] < 3 ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Vertex present " << vertexCounter[ e.first ] << " times" << std::endl;
#endif
        }

        if( vertexCounter[ e.second ] < 3 ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Vertex present " << vertexCounter[ e.second ] << " times" << std::endl;
#endif
        }

        if( itr->second != 2 ) {
            result = false;
#if ( 1 < CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
            WARN << errorPrefix <<
                " - coherency fail - Edge present " << itr->second << " times" << std::endl;
#endif
        }
    }

#if ( 1 == CONVEX_POLYHEDRON_WARN_ON_INCOHERENT_DATA )
    if( !convex )
        WARN << errorPrefix
            << " - coherency fail - non convex output" << std::endl;

    if ( !result )
        WARN << errorPrefix
            << " - coherency fail - incoherent output" << std::endl;
#endif

#if ( 1 == CONVEX_POLYHEDRON_DUMP_ON_INCOHERENT_DATA )
    if( !result || !convex )
        dumpGeometry( );
#endif

#if ( 1 < CONVEX_POLYHEDRON_DUMP_ON_INCOHERENT_DATA )
    if( !result )
        dumpGeometry( );
#endif


#endif // CONVEX_POLYHEDRON_CHECK_COHERENCY
    return result && convex;
}

osg::BoundingBox ConvexPolyhedron::computeBoundingBox( const osg::Matrix & m ) const
{
    osg::BoundingBox bb;

    if( &m != &defaultMatrix ) {
        for( Faces::const_iterator itr = _faces.begin(); itr != _faces.end(); ++itr )
            for( Vertices::const_iterator vitr = itr->vertices.begin();
                 vitr != itr->vertices.end();
                 ++vitr )
                    bb.expandBy( *vitr * m );
    } else {
        for( Faces::const_iterator itr = _faces.begin(); itr != _faces.end(); ++itr )
            for( Vertices::const_iterator vitr = itr->vertices.begin();
                 vitr != itr->vertices.end();
                 ++vitr )
                    bb.expandBy( *vitr );
    }

    return bb;
}

void ConvexPolyhedron::cut(const osg::Polytope & polytope)
{
    const char * apc[6] = { "left", "right", "bottom", "top", "near", "far" };
    char ac[16];
    int i = 0;

    for(osg::Polytope::PlaneList::const_iterator itr = polytope.getPlaneList().begin();
        itr != polytope.getPlaneList().end();
        ++itr)
    {
        const char* arg;
        if (i < 6) {
            arg = apc[i];
        } else {
            sprintf(ac, "%d", i);
            arg = ac;
        }
        cut(*itr, std::string( arg ) );

        i++;
    }

    removeDuplicateVertices();
}

void ConvexPolyhedron::cut(const ConvexPolyhedron& polytope)
{
    for(Faces::const_iterator itr = polytope._faces.begin();
        itr != polytope._faces.end();
        ++itr)
    {
        cut(itr->plane, itr->name);
    }

    removeDuplicateVertices();
}

void ConvexPolyhedron::cut(const osg::Plane& plane, const std::string& name)
{
    if( _faces.empty() ) return;

    ConvexPolyhedron cp( *this );

    typedef std::vector< FaceDistances > FaceDistancesList;
    FaceDistancesList faceDistances;
    faceDistances.resize( _faces.size() );

    double min = FLT_MAX, max = -FLT_MAX; //Hull max & min point distances

    FaceDistancesList::iterator fd = faceDistances.begin();
    // First step compute each face points distances to cutting plane
    for( Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr, ++fd )
    {
        fd->itr = itr;
        fd->distances.reserve( itr->vertices.size() );
        fd->on = 0;
        fd->above = 0;
        fd->below = 0;

#if 0 //  Skip if cutting plane the same as one of faces
        if( plane.ptr()[0] ) == itr->plane.ptr()[0] &&
            plane.ptr()[1] ) == itr->plane.ptr()[1] &&
            plane.ptr()[2] ) == itr->plane.ptr()[2] &&
#else    // check plane using less precise float values
        if( float( plane.ptr()[0] ) == float( itr->plane.ptr()[0] ) &&
            float( plane.ptr()[1] ) == float( itr->plane.ptr()[1] ) &&
            float( plane.ptr()[2] ) == float( itr->plane.ptr()[2] ) &&
#endif
            plane_hull_tolerance >= fabs( float( plane.ptr()[3] )-  float( itr->plane.ptr()[3] ) ) )
            return;

        for( Vertices::iterator vitr = itr->vertices.begin();
            vitr != itr->vertices.end();
            ++vitr)
        {
            double d = plane.distance( *vitr );

            fd->distances.push_back( d );
            if ( d>point_plane_tolerance )       ++fd->above;
            else if ( d<-point_plane_tolerance ) ++fd->below;
            else                                 ++fd->on;
            min = osg::minimum( min, d );
            max = osg::maximum( max, d );
        }
    }

    if( max <= plane_hull_tolerance ) { // All points on or below cutting plane
        _faces.clear();
        return;
    }

    if( min >= -plane_hull_tolerance ) { // All points on or above cutting plane
        return;
    }

    typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
    typedef std::set< Edge > Edges;
    Edges edges;

    for( FaceDistancesList::iterator fd = faceDistances.begin();
        fd != faceDistances.end();
        ++fd )
    {
        if ( fd->below == 0 )
        {  // skip face if all points on or above cutting plane ( below == 0 )
            continue;
        }

        if ( /* fd->below > 0 && */ fd->above == 0 && fd->on == 0 )
        {
            _faces.erase( fd->itr ); // remove face if points below or on
            continue;
        }

        // cut the face if some points above and below plane
        // assert( fd->below > 0 && fd->above > 0 );

        Face& face = *(fd->itr);
        Vertices& vertices = face.vertices;
        Distances& distances = fd->distances;
        Vertices newFaceVertices;
        Vertices newVertices;


        for(unsigned int i=0; i < vertices.size(); ++i)
        {
            osg::Vec3d &va = vertices[i];
            osg::Vec3d &vb = vertices[(i+1)%vertices.size()];
            double &distance_a = distances[i];
            double &distance_b = distances[(i+1)%vertices.size()];

            // Is first edge point above or on the plane?
            if ( -point_plane_tolerance <= distance_a ) {

                if( newVertices.empty() || vertices[i] != newVertices.back() )
                    newVertices.push_back( vertices[i] );

                if ( distance_a <= point_plane_tolerance ) {
                    if( newFaceVertices.empty() || vertices[i] != newFaceVertices.back() )
                        newFaceVertices.push_back( vertices[i] );
                }
            }

            // Does edge intersect plane ?
            if ( ( distance_a < -point_plane_tolerance && distance_b > point_plane_tolerance ) ||
                 ( distance_b < -point_plane_tolerance && distance_a > point_plane_tolerance ) )
            {
                osg::Vec3d intersection; // Inserting vertex
                double da = fabs( distance_a ), db = fabs( distance_b );

                // tweaks to improve coherency of polytope after cut
                if( da <= point_point_equivalence && da <= db ) {
                    intersection = va;
                } else if( db <= point_point_equivalence && db <= da ) {
                    intersection = vb;
                } else {
                    double dab4 = 0.25 * ( da + db );
                    if( dab4 < da && dab4 < db ) {
                        intersection = (vb*distance_a - va*distance_b)/(distance_a-distance_b);
                    } else {
                        osg::Vec3d v = (vb - va)/(distance_a-distance_b);
                        if( da < db )
                            intersection = va + v * distance_a;
                        else
                            intersection = vb + v * distance_b;
                    }
                }

                if( newVertices.empty() || intersection != newVertices.back() )
                    newVertices.push_back( intersection );

                if( newFaceVertices.empty() || intersection != newFaceVertices.back() )
                    newFaceVertices.push_back( intersection );
            }
        }

        if( newVertices.size() && newVertices.front() == newVertices.back() )
            newVertices.pop_back();

        if( newFaceVertices.size() && newFaceVertices.front() == newFaceVertices.back() )
            newFaceVertices.pop_back();

        if( newFaceVertices.size() == 1 ) {  // This is very rare but correct
            WARN
                << "ConvexPolyhedron::cut - Slicing face polygon returns "
                << newFaceVertices.size()
                << " points. Should be 2 (usually) or 1 (rarely)."
                << std::endl;

        } else if( newFaceVertices.size() == 2 ) {
            if( newFaceVertices[0] < newFaceVertices[1] ) {
                edges.insert( Edge( newFaceVertices[0], newFaceVertices[1] ) );
            } else {
                edges.insert( Edge( newFaceVertices[1], newFaceVertices[0] ) );
            }
        } else if( newFaceVertices.size() > 2 ) {

#if CONVEX_POLYHEDRON_WARN_ON_INCORRECT_FACE_CUT

            // This may happen if face polygon is not planar or convex.
            // It may happen when face was distorted by projection transform
            // or when some polygon vertices land in incorrect positions after
            // some transformation by badly conditioned matrix (weak precison)

            WARN
                << "ConvexPolyhedron::cut - Slicing face polygon returns "
                << newFaceVertices.size()
                << " points. Should be 2 (usually) or 1 (rarely)."
                << std::endl;
#endif

#if CONVEX_POLYHEDRON_DUMP_ON_INCORRECT_FACE_CUT
            dumpGeometry( &face, &plane );
#endif

            // Let try to recover from this uneasy situation
            // by comparing current face polygon edges cut by new plane
            // with edges selected for new plane

            unsigned i0 = 0, i1 = newFaceVertices.size() - 1;
            unsigned j0 = 0, j1 = newVertices.size() - 1;

            for( ; i0 < newFaceVertices.size(); i1 = i0++, j1 = j0++ ) {
                while( newFaceVertices[i0] != newVertices[j0] ) j1 = j0++;
                if( newFaceVertices[i1] == newVertices[j1] )
                {
                    if( newFaceVertices[i0] < newFaceVertices[i1] ) {
                        edges.insert( Edge( newFaceVertices[i0], newFaceVertices[i1] ) );
                    } else {
                        edges.insert( Edge( newFaceVertices[i1], newFaceVertices[i0] ) );
                    }
                }
            }
        }

        if( newVertices.size() >= 3 ) { //Add faces with at least 3 points
#if ( CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON || CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON )
            int convex = isFacePolygonConvex( face );
            vertices.swap( newVertices );
            if( convex && !isFacePolygonConvex( face ) ) {
#if CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON
                WARN << "ConvexPolyhedron::cut - polygon output non convex."
                << " This may lead to other issues in ConvexPolyhedron math" << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON
                dumpGeometry( &face, &plane, &cp );
#endif
            }
#else
            vertices.swap( newVertices );
#endif
        } else //Remove face reduced to 0, 1, 2 points
            _faces.erase( fd->itr );
    }

    if ( edges.size() > 1 ) //Ignore faces reduced to 0, 1, 2 points
    {
        Face face;
        face.name = name;
        face.plane = plane;

        std::deque< osg::Vec3d > vertices;

        Edges::iterator itr = edges.begin();
        vertices.push_back( itr->first );
        vertices.push_back( itr->second );
        edges.erase( itr++ );

        for( unsigned int vertices_size = 0;
            vertices_size < vertices.size(); )
        {
            vertices_size = vertices.size();
            for( itr = edges.begin(); itr != edges.end(); )
            {
                bool not_added = false;

                if( itr->first == vertices.back() )
                    vertices.push_back( itr->second );
                else if ( itr->first == vertices.front() )
                    vertices.push_front( itr->second );
                else if ( itr->second == vertices.back() )
                    vertices.push_back( itr->first );
                else if ( itr->second == vertices.front() )
                    vertices.push_front( itr->first );
                else
                    not_added = true;

                if( not_added )
                    ++itr;
                else
                    edges.erase( itr++ );
            }
        }

#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
        if( !edges.empty() ) {
            WARN
                << "ConvexPolyhedron::cut - Building new face polygon - "
                << "Found edges not matching former polygon ends"
                << std::endl;
        }
#endif

        std::copy(vertices.begin(), vertices.end(), std::back_inserter(face.vertices));

        _faces.push_back(face);

        // Last vertex is duplicated - remove one instance
        if( face.vertices.front() == face.vertices.back() )
            face.vertices.pop_back();
        else {// If not duplicated then it may mean we have open polygon ;-(
#if CONVEX_POLYHEDRON_WARN_ON_BAD_POLYGON
            WARN
            << "ConvexPolyhedron::cut - Building new face polygon - "
            << " Polygon not properly closed."
            << std::endl;
#endif
#if CONVEX_POLYHEDRON_DUMP_ON_BAD_POLYGON
            dumpGeometry( &_faces.back(), &plane, &cp );
#endif
        }

#if ( CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON || CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON )
        if( !isFacePolygonConvex( face ) ) {
#if CONVEX_POLYHEDRON_DUMP_ON_CONCAVE_POLYGON
            ConvexPolyhedron cp;
            cp.createFace() = face;
            cp.dumpGeometry( );
#endif
#if CONVEX_POLYHEDRON_WARN_ON_CONCAVE_POLYGON
            WARN << "ConvexPolyhedron::cut - new face polygon non convex."
                << " This may lead to other issues in ConvexPolyhedron math" << std::endl;
#endif
        }
#endif
    }

//    removeDuplicateVertices( );
}
////////////////////////////////////////////////////////////////////////////
void ConvexPolyhedron::extrude( const osg::Vec3d & offset )
{
    if( offset.length2() == 0 ) return;

    typedef std::pair<osg::Vec3d, osg::Vec3d> Edge;
    typedef std::vector<Face*> EdgeFaces;
    typedef std::map<Edge, EdgeFaces> EdgeMap;

    EdgeMap edgeMap;

    // Build edge maps
    for(Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        Face& face = *itr;
        for(unsigned int i=0; i<face.vertices.size(); ++i)
        {
            osg::Vec3d& va = face.vertices[i];
            osg::Vec3d& vb = face.vertices[(i+1)%face.vertices.size()];
            if (va < vb) edgeMap[Edge(va,vb)].push_back(&face);
            else edgeMap[Edge(vb,va)].push_back(&face);
        }
    }

    // Offset faces
    for(Faces::iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        Face& face = *itr;

        double dotOffset = face.plane.dotProductNormal( offset );

        if( dotOffset >= 0 ) continue;

        face.plane.ptr()[3] -= dotOffset;
        for(unsigned int i=0; i<face.vertices.size(); ++i)
        {
            face.vertices[i] += offset;
        }
    }

    typedef std::set< Edge > SilhouetteEdges;
    typedef std::map< Face*, SilhouetteEdges > SilhouetteFaces;
    SilhouetteFaces silhouetteFaces;

    int new_face_counter = 0;

    // Now add new faces from slhouette edges extrusion
    for(EdgeMap::iterator eitr = edgeMap.begin();
        eitr != edgeMap.end();
        ++eitr)
    {
        const Edge& edge = eitr->first;
        const EdgeFaces& edgeFaces = eitr->second;

        if ( edgeFaces.size()==1 )
        {
            // WL: Execution should not reach this line.
            // If you got here let me know: lewandowski@ai.com.pl
            assert( 0 );
        }
        else if ( edgeFaces.size()==2 )
        {
#if 0 // Use float normal computations
            osg::Vec3f vf( offset );
            double dotOffset0 = osg::Vec3f( edgeFaces[0]->plane.getNormal() ) * vf;
            double dotOffset1 = osg::Vec3f( edgeFaces[1]->plane.getNormal() ) * vf;
#else
            double dotOffset0 = edgeFaces[0]->plane.getNormal() * offset;
            double dotOffset1 = edgeFaces[1]->plane.getNormal() * offset;
#endif
            //Select orthogonal faces and vertices appropriate for offsetting
            if( (dotOffset0 == 0.0 && dotOffset1 < 0.0) ||
                (dotOffset1 == 0.0 && dotOffset0 < 0.0) )
            {
                Face * face = ( dotOffset0 == 0 ? edgeFaces[0] : edgeFaces[1] );
                silhouetteFaces[ face ].insert( edge );
            }

            if( (dotOffset0 < 0.0 && dotOffset1 > 0.0) ||
                (dotOffset1 < 0.0 && dotOffset0 > 0.0) )
            {
                Face & face = createFace();
                char ac[40] = "Side plane from edge extrude ";
                sprintf(ac + strlen(ac), "%d", new_face_counter++);
                face.name = ac;

                // Compute face plane
                face.vertices.push_back( edge.first );
                face.vertices.push_back( edge.second );

                osg::Vec3d n = face.vertices[0] - face.vertices[1];
                n.normalize();
                n = ( n ^ offset );
                n.normalize();

                if( n * ( edgeFaces[1]->plane.getNormal() + edgeFaces[0]->plane.getNormal() ) < 0 )
                {
                    n = -n;
                    std::swap( face.vertices[1], face.vertices[0] );
                }

                face.vertices.push_back( face.vertices[1] + offset );
                face.vertices.push_back( face.vertices[0] + offset );

                face.plane.set( n,(face.vertices[0] + face.vertices[1] +
                    face.vertices[2] + face.vertices[3]) * .25 );
            }
        }
        else if( edgeFaces.size() > 2 )
        {
            assert( 0 );
            // WL: Execution should not reach this line.
            // If you got here let me know: lewandowski@ai.com.pl
        }
    }

    // Finally update faces which are orthogonal to our normal
    for(SilhouetteFaces::iterator itr = silhouetteFaces.begin();
        itr != silhouetteFaces.end();
        ++itr)
    {
        SilhouetteEdges & edges = itr->second;
        Vertices & vertices =  itr->first->vertices;
        Vertices newVertices;

        for( unsigned int i = 0; i < vertices.size(); i++ )
        {
            osg::Vec3d
                &va = vertices[ ( i + vertices.size() - 1 ) % vertices.size() ],
                &vb = vertices[  i  ],
                &vc = vertices[ ( i +  1 ) % vertices.size() ];

            Edge eab =  va < vb ? Edge( va, vb ) : Edge( vb, va );
            Edge ebc =  vb < vc ? Edge( vb, vc ) : Edge( vc, vb );

            bool abFound = edges.find( eab ) != edges.end();
            bool bcFound = edges.find( ebc ) != edges.end();

            if( abFound && bcFound ) {
                newVertices.push_back( vb + offset );
            } else if( !abFound && !bcFound ) {
                newVertices.push_back( vb );
            } else if( !abFound && bcFound ) {
                newVertices.push_back( vb );
                newVertices.push_back( vb + offset );
            } else if( abFound && !bcFound ) {
                newVertices.push_back( vb + offset );
                newVertices.push_back( vb );
            }
        }

        vertices.swap( newVertices );
    }

    removeDuplicateVertices( );
    checkCoherency( true, "ConvexPolyhedron::extrude" );
}
////////////////////////////////////////////////////////////////////////////
void ConvexPolyhedron::translate( const osg::Vec3d & offset )
{
    for( Faces::iterator itr = _faces.begin(); itr != _faces.end(); ++itr )
    {
        itr->plane.ptr()[3] -= itr->plane.dotProductNormal( offset );

        for( Vertices::iterator vitr = itr->vertices.begin();
             vitr != itr->vertices.end();
             ++vitr )
        {
            *vitr += offset;
        }
    }
}
////////////////////////////////////////////////////////////////////////////
void ConvexPolyhedron::getPolytope(osg::Polytope& polytope) const
{
    for(Faces::const_iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        polytope.add(itr->plane);
    }
}
////////////////////////////////////////////////////////////////////////////
void ConvexPolyhedron::getPoints(Vertices& vertices) const
{
    typedef std::set<osg::Vec3d> VerticesSet;
    VerticesSet verticesSet;
    for(Faces::const_iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        const Face& face = *itr;
        for(Vertices::const_iterator vitr = face.vertices.begin();
            vitr != face.vertices.end();
            ++vitr)
        {
            verticesSet.insert(*vitr);
        }
    }

    for(VerticesSet::iterator sitr = verticesSet.begin();
        sitr != verticesSet.end();
        ++sitr)
    {
        vertices.push_back(*sitr);
    }
}
////////////////////////////////////////////////////////////////////////////
osg::Geometry* ConvexPolyhedron::buildGeometry( const osg::Vec4d& colorOutline,
                                         const osg::Vec4d& colorInside,
                                         osg::Geometry* geometry ) const
{
    if( !geometry ) {
        geometry = new osg::Geometry;
    } else {
        geometry->getPrimitiveSetList( ).clear();
    }

    osg::Vec3dArray* vertices = new osg::Vec3dArray;
    geometry->setVertexArray(vertices);

    osg::Vec4Array* colors = new osg::Vec4Array;
    geometry->setColorArray(colors, osg::Array::BIND_PER_PRIMITIVE_SET);

    for(Faces::const_iterator itr = _faces.begin();
        itr != _faces.end();
        ++itr)
    {
        if( colorInside[3] > 0 ) {
            geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_FAN,
                vertices->size(), itr->vertices.size() ) );

            colors->push_back( colorInside );
        }

        if( colorOutline[3] > 0 ) {
            geometry->addPrimitiveSet( new osg::DrawArrays( GL_LINE_LOOP,
                vertices->size(), itr->vertices.size() ) );

            colors->push_back( colorOutline );
        }

        vertices->insert
            ( vertices->end(), itr->vertices.begin(), itr->vertices.end() );
    }

    osg::StateSet* stateset = geometry->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OFF);
    stateset->setTextureMode(1, GL_TEXTURE_2D, osg::StateAttribute::OFF);

    return geometry;
}
////////////////////////////////////////////////////////////////////////////
bool ConvexPolyhedron::dumpGeometry
( const Face * face,
  const osg::Plane * plane,
  ConvexPolyhedron * base,
  const char * filename,
  const osg::Vec4d& colorOutline,
  const osg::Vec4d& colorInside,
  const osg::Vec4d& faceColorOutline,
  const osg::Vec4d& faceColorInside,
  const osg::Vec4d& planeColorOutline,
  const osg::Vec4d& planeColorInside,
  const osg::Vec4d& baseColorOutline,
  const osg::Vec4d& baseColorInside ) const
{
    osg::Group * group = new osg::Group();
    osg::Geode * geode = new osg::Geode();
    geode->getOrCreateStateSet()->setMode( GL_BLEND, osg::StateAttribute::ON );
    geode->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );
    geode->getOrCreateStateSet()->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );

    group->addChild( geode );

    Vertices vertices;
    getPoints( vertices );

    osg::BoundingBox bb;
    for( unsigned int i = 0; i < vertices.size(); i++ )
        bb.expandBy( vertices[i] );

    ConvexPolyhedron cp( *this ), cpFace;

    for( Faces::iterator itr = cp._faces.begin(); itr != cp._faces.end(); )
    {
        bool found = ( face &&
                       itr->name == face->name &&
                       itr->plane == face->plane &&
                       itr->vertices == face->vertices );
#if 1
        if( cp.isFacePolygonConvex( *itr ) < 0 )
            std::reverse( itr->vertices.begin(), itr->vertices.end() );
#endif

        if( found ) {
            cpFace.createFace() = *face;
            itr = cp._faces.erase( itr );
        } else {
            ++itr;
        }
    }

    osg::Geometry * geometry = cp.buildGeometry( colorOutline, colorInside );
    geometry->getOrCreateStateSet()->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

    geode->addDrawable( geometry );

    if( face )
        geode->addDrawable( cpFace.buildGeometry( faceColorOutline, faceColorInside ) );

    if( plane )
    {
        ConvexPolyhedron cp;
        Face & face = cp.createFace();
        face.plane = *plane;

        osg::Vec3d normal = face.plane.getNormal();
        osg::Vec3d side = fabs(normal.x()) < fabs(normal.y()) ?
                            osg::Vec3d(1.0, 0.0, 0.0) :
                            osg::Vec3d(0.0, 1.0, 0.0);

        osg::Vec3d v = normal ^ side;
        v.normalize();
        v *= bb.radius();

        osg::Vec3d u = v ^ normal;
        u.normalize();
        u *= bb.radius();

        osg::Vec3d c = bb.center();
        c -= face.plane.getNormal() * face.plane.distance( c );

        face.vertices.push_back( c - u - v );
        face.vertices.push_back( c - u + v );
        face.vertices.push_back( c + u + v );
        face.vertices.push_back( c + u - v );

        geode->addDrawable( cp.buildGeometry( planeColorOutline, planeColorInside ) );
    }

    if( base )
        geode->addDrawable( base->buildGeometry( baseColorOutline, baseColorInside ) );

    return osgDB::writeNodeFile( *group, std::string( filename ) );
}
////////////////////////////////////////////////////////////////////////////
