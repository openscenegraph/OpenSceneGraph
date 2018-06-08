
#include <stdlib.h>
#include <osg/Geode>
#include <osgUtil/MeshOptimizers>

#include "VBSPGeometry.h"


using namespace osg;
using namespace bsp;


VBSPGeometry::VBSPGeometry(VBSPData * bspData)
{
    // Keep track of the bsp data, as it has all of the data lists that we
    // need
    bsp_data = bspData;

    // Create arrays for the vertex attributes
    vertex_array = new Vec3Array();
    normal_array = new Vec3Array();
    texcoord_array = new Vec2Array();

    // Create a primitive set for drawing variable length primitives (VBSP
    // primitives are only guaranteed to be convex polygons)
    primitive_set = new DrawArrayLengths(PrimitiveSet::POLYGON);

    // Create a second set of arrays for displacement surfaces
    disp_vertex_array = new Vec3Array();
    disp_normal_array = new Vec3Array();
    disp_texcoord_array = new Vec2Array();
    disp_vertex_attr_array = new Vec4Array();

    // Create a second primitive set for drawing indexed triangles, which is
    // the quickest method for drawing the displacement surfaces
    disp_primitive_set = new DrawElementsUInt(PrimitiveSet::TRIANGLES);
}


VBSPGeometry::~VBSPGeometry()
{
}


bool VBSPGeometry::doesEdgeExist(int row, int col, int direction,
                                 int vertsPerEdge)
{
    // See if there is an edge on the displacement surface from the given
    // vertex in the given direction (we only need to know the vertices
    // indices, because all displacement surfaces are tessellated in the
    // same way)
    switch (direction)
    {
        case 0:
            // False if we're on the left edge, otherwise true
            if ((row - 1) < 0)
                return false;
            else
                return true;

        case 1:
            // False if we're on the top edge, otherwise true
            if ((col + 1) >= vertsPerEdge)
                return false;
            else
                return true;

        case 2:
            // False if we're on the right edge, otherwise true
            if ((row + 1) >= vertsPerEdge)
                return false;
            else
                return true;

        case 3:
            // False if we're on the bottom edge, otherwise true
            if ((col - 1) < 0)
                return false;
            else
                return true;

        default:
            return false;
    }
}


osg::Vec3 VBSPGeometry::getNormalFromEdges(int row, int col,
                                           unsigned char edgeBits,
                                           int firstVertex, int vertsPerEdge)
{
    osg::Vec3 *   vertexData;
    osg::Vec3 *   surfaceVerts;
    osg::Vec3     finalNormal;
    osg::Vec3     v1, v2, v3;
    osg::Vec3     e1, e2;
    osg::Vec3     tempNormal;
    int           normalCount;

    // Constants for direction.  If the bit is set in the edgeBits, then
    // there is an edge connected to the current vertex in that direction
    const unsigned char   NEG_X = 1 << 0;
    const unsigned char   POS_Y = 1 << 1;
    const unsigned char   POS_X = 1 << 2;
    const unsigned char   NEG_Y = 1 << 3;

    // Constants for quadrants.  If both bits are set, then there are
    // exactly two triangles in that quadrant
    const unsigned char   QUAD_1 = POS_X | POS_Y;
    const unsigned char   QUAD_2 = NEG_X | POS_Y;
    const unsigned char   QUAD_3 = NEG_X | NEG_Y;
    const unsigned char   QUAD_4 = POS_X | NEG_Y;


    // Grab the vertex data from the displaced vertex array (if there's a
    // better way to randomly access the data in this array, I'm all ears)
    vertexData = (osg::Vec3 *)disp_vertex_array->getDataPointer();

    // Move to the surface we're interested in, and start counting vertices
    // from there
    surfaceVerts = &vertexData[firstVertex];

    // Start with no normals computed
    finalNormal.set(0.0, 0.0, 0.0);
    normalCount = 0;

    // The process is fairly simple.  For all four quadrants surrounding
    // the vertex, check each quadrant to see if there are triangles there.
    // If so, calculate the normals of the two triangles in that quadrant, and
    // add them to the final normal.  When fininshed, scale the final normal
    // based on the number of contributing triangle normals

    // Check quadrant 1 (+X,+Y)
    if ((edgeBits & QUAD_1) == QUAD_1)
    {
        // First triangle
        v1 = surfaceVerts[(col+1) * vertsPerEdge + row];
        v2 = surfaceVerts[col * vertsPerEdge + row];
        v3 = surfaceVerts[col * vertsPerEdge + (row+1)];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;

        // Second triangle
        v1 = surfaceVerts[(col+1) * vertsPerEdge + row];
        v2 = surfaceVerts[col * vertsPerEdge + (row+1)];
        v3 = surfaceVerts[(col+1) * vertsPerEdge + (row+1)];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;
    }

    // Check quadrant 2 (-X,+Y)
    if ((edgeBits & QUAD_2) == QUAD_2)
    {
        // First triangle
        v1 = surfaceVerts[(col+1) * vertsPerEdge + (row-1)];
        v2 = surfaceVerts[col * vertsPerEdge + (row-1)];
        v3 = surfaceVerts[col * vertsPerEdge + row];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;

        // Second triangle
        v1 = surfaceVerts[(col+1) * vertsPerEdge + (row-1)];
        v2 = surfaceVerts[col * vertsPerEdge + row];
        v3 = surfaceVerts[(col+1) * vertsPerEdge + row];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;
    }

    // Check quadrant 3 (-X,-Y)
    if ((edgeBits & QUAD_3) == QUAD_3)
    {
        // First triangle
        v1 = surfaceVerts[col * vertsPerEdge + (row-1)];
        v2 = surfaceVerts[(col-1) * vertsPerEdge + (row-1)];
        v3 = surfaceVerts[(col-1) * vertsPerEdge + row];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;

        // Second triangle
        v1 = surfaceVerts[col * vertsPerEdge + (row-1)];
        v2 = surfaceVerts[(col-1) * vertsPerEdge + row];
        v3 = surfaceVerts[col * vertsPerEdge + row];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;
    }

    // Check quadrant 4 (+X,-Y)
    if ((edgeBits & QUAD_4) == QUAD_4)
    {
        // First triangle
        v1 = surfaceVerts[col * vertsPerEdge + row];
        v2 = surfaceVerts[(col-1) * vertsPerEdge + row];
        v3 = surfaceVerts[(col-1) * vertsPerEdge + (row+1)];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;

        // Second triangle
        v1 = surfaceVerts[col * vertsPerEdge + row];
        v2 = surfaceVerts[(col-1) * vertsPerEdge + (row+1)];
        v3 = surfaceVerts[col * vertsPerEdge + (row+1)];
        e1 = v1 - v2;
        e2 = v3 - v2;
        tempNormal = e2 ^ e1;
        tempNormal.normalize();
        finalNormal += tempNormal;
        normalCount++;
    }

    // Scale the final normal according to how many triangle normals are
    // contributing
    if (normalCount>0) finalNormal *= (1.0f / (float)normalCount);

    return finalNormal;
}


void VBSPGeometry::createDispSurface(Face & face, DisplaceInfo & dispInfo)
{
    TexInfo           currentTexInfo;
    TexData           currentTexData;
    Vec3              texU;
    float             texUOffset;
    float             texUScale;
    Vec3              texV;
    float             texVOffset;
    float             texVScale;
    int               i, j;
    unsigned int      k;
    double            dist, minDist;
    int               minIndex = 0;
    osg::Vec3         temp;
    int               edgeIndex;
    int               currentSurfEdge;
    Edge              currentEdge;
    osg::Vec3         currentVertex;
    osg::Vec3         vertices[4];
    unsigned int      firstVertex;
    int               numEdgeVertices;
    double            subdivideScale;
    osg::Vec3         leftEdge, rightEdge;
    osg::Vec3         leftEdgeStep, rightEdgeStep;
    osg::Vec3         leftEnd, rightEnd;
    osg::Vec3         leftRightSeg, leftRightStep;
    unsigned int      dispVertIndex;
    DisplacedVertex   dispVertInfo;
    osg::Vec3         flatVertex, dispVertex;
    unsigned int      index;
    osg::Vec3         normal;
    float             u, v;
    osg::Vec2         texCoord;
    float             alphaBlend;
    unsigned char     edgeBits;


    // Get the texture info for this face
    currentTexInfo = bsp_data->getTexInfo(face.texinfo_index);
    currentTexData = bsp_data->getTexData(currentTexInfo.texdata_index);

    // Get the texture vectors and offsets.  These are used to calculate
    // texture coordinates
    texU.set(currentTexInfo.texture_vecs[0][0],
             currentTexInfo.texture_vecs[0][1],
             currentTexInfo.texture_vecs[0][2]);
    texUOffset = currentTexInfo.texture_vecs[0][3];
    texV.set(currentTexInfo.texture_vecs[1][0],
             currentTexInfo.texture_vecs[1][1],
             currentTexInfo.texture_vecs[1][2]);
    texVOffset = currentTexInfo.texture_vecs[1][3];

    // Scale the texture vectors from inches to meters
    texU *= 39.37f;
    texV *= 39.37f;

    // Get the size of the texture involved, as the planar texture projection
    // assumes non-normalized texture coordinates
    texUScale = 1.0f / (float)currentTexData.texture_width;
    texVScale = 1.0f / (float)currentTexData.texture_height;

    // Get the first edge index
    edgeIndex = face.first_edge;

    // Get the base vertices for this face
    for (i = 0; i < face.num_edges; i++)
    {
        // Look up the edge specified by the surface edge index, the
        // index might be negative (see below), so take the absolute
        // value
        currentSurfEdge = bsp_data->getSurfaceEdge(edgeIndex);
        currentEdge = bsp_data->getEdge(abs(currentSurfEdge));

        // The sign of the surface edge index specifies which vertex is
        // "first" for this face.  A negative index means the edge should
        // be flipped, and the second vertex treated as the first
        if (currentSurfEdge < 0)
            currentVertex = bsp_data->getVertex(currentEdge.vertex[1]);
        else
            currentVertex = bsp_data->getVertex(currentEdge.vertex[0]);

        // Add the vertex to the array
        vertices[i] = currentVertex;

        // Move on to the next vertex
        edgeIndex++;
    }

    // Rotate the base coordinates for the surface until the first vertex
    // matches the start position
    minDist = 1.0e9;
    for (i = 0; i < 4; i++)
    {
       // Calculate the distance of the start position from this vertex
       dist = (vertices[i] - dispInfo.start_position * 0.0254f).length();

       // If this is the smallest distance we've seen, remember it
       if (dist < minDist)
       {
          minDist = dist;
          minIndex = i;
       }
    }

    // Rotate the displacement surface quad until we get the starting vertex
    // in the 0th position
    for (i = 0; i < minIndex; i++)
    {
        temp = vertices[0];
        vertices[0] = vertices[1];
        vertices[1] = vertices[2];
        vertices[2] = vertices[3];
        vertices[3] = temp;
    }

    // Calculate the vectors for the left and right edges of the surface
    // (remembering that the surface is wound clockwise)
    leftEdge = vertices[1] - vertices[0];
    rightEdge = vertices[2] - vertices[3];

    // Calculate the number of vertices along each edge of the surface
    numEdgeVertices = (1 << dispInfo.power) + 1;

    // Calculate the subdivide scale, which will tell us how far apart to
    // put each vertex (relative to the length of the surface's edges)
    subdivideScale = 1.0 / (double)(numEdgeVertices - 1);

    // Calculate the step size between vertices on the left and right edges
    leftEdgeStep = leftEdge * subdivideScale;
    rightEdgeStep = rightEdge * subdivideScale;

    // Remember the first vertex index in the vertex array
    firstVertex = disp_vertex_array->size();

    // Generate the displaced vertices (this technique comes from the
    // Source SDK)
    for (i = 0; i < numEdgeVertices; i++)
    {
        // Calculate the two endpoints for this section of the surface
        leftEnd = leftEdgeStep * (double) i;
        leftEnd += vertices[0];
        rightEnd = rightEdgeStep * (double) i;
        rightEnd += vertices[3];

        // Now, get the vector from left to right, and subdivide it as well
        leftRightSeg = rightEnd - leftEnd;
        leftRightStep = leftRightSeg * subdivideScale;

        // Generate the vertices for this section
        for (j = 0; j < numEdgeVertices; j++)
        {
            // Get the displacement info for this vertex
            dispVertIndex = dispInfo.disp_vert_start;
            dispVertIndex += i * numEdgeVertices + j;
            dispVertInfo = bsp_data->getDispVertex(dispVertIndex);

            // Calculate the flat vertex
            flatVertex = leftEnd + (leftRightStep * (double) j);

            // Calculate the displaced vertex
            dispVertex =
                dispVertInfo.displace_vec *
                (dispVertInfo.displace_dist * 0.0254);
            dispVertex += flatVertex;

            // Add the vertex to the displaced vertex array
            disp_vertex_array->push_back(dispVertex);

            // Calculate the texture coordinates for this vertex.  Texture
            // coordinates are calculated using a planar projection, so we need
            // to use the non-displaced vertex position here
            u = texU * flatVertex + texUOffset;
            u *= texUScale;
            v = texV * flatVertex + texVOffset;
            v *= texVScale;
            texCoord.set(u, v);

            // Add the texture coordinate to the array
            disp_texcoord_array->push_back(texCoord);

            // Get the texture blend parameter for this vertex as well, and
            // assign it as the alpha channel for the primary vertex color.
            // We'll use a combiner operation to do the texture blending
            alphaBlend = dispVertInfo.alpha_blend / 255.0;
            disp_vertex_attr_array->push_back(
                osg::Vec4f(1.0, 1.0, 1.0, 1.0 - alphaBlend));
        }
    }

    // Calculate normals at each vertex (this is adapted from the Source SDK,
    // including the two helper functions)
    for (i = 0; i < numEdgeVertices; i++)
    {
        for (j = 0; j < numEdgeVertices; j++)
        {
            // See which of the 4 possible edges (left, up, right, or down) are
            // incident on this vertex
            edgeBits = 0;
            for (k = 0; k < 4; k++)
            {
                if (doesEdgeExist(j, i, k, numEdgeVertices))
                    edgeBits |= 1 << k;
            }

            // Calculate the normal based on the adjacent edges
            normal = getNormalFromEdges(j, i, edgeBits, firstVertex,
                                        numEdgeVertices);

            // Add the normal to the normal array
            disp_normal_array->push_back(normal);
        }
    }

    // Now, triangulate the surface (this technique comes from the Source SDK)
    for (i = 0; i < numEdgeVertices-1; i++)
    {
        for (j = 0; j < numEdgeVertices-1; j++)
        {
            // Get the current vertex index (local to this surface)
            index = i * numEdgeVertices + j;

            // See if this index is odd
            if ((index % 2) == 1)
            {
                // Add the vertex offset (so we reference this surface's
                // vertices in the array)
                index += firstVertex;

                // Create two triangles on this vertex from top-left to
                // bottom-right
                disp_primitive_set->push_back(index);
                disp_primitive_set->push_back(index + 1);
                disp_primitive_set->push_back(index + numEdgeVertices);
                disp_primitive_set->push_back(index + 1);
                disp_primitive_set->push_back(index + numEdgeVertices + 1);
                disp_primitive_set->push_back(index + numEdgeVertices);
            }
            else
            {
                // Add the vertex offset (so we reference this surface's
                // vertices in the array)
                index += firstVertex;

                // Create two triangles on this vertex from bottom-left to
                // top-right
                disp_primitive_set->push_back(index);
                disp_primitive_set->push_back(index + numEdgeVertices + 1);
                disp_primitive_set->push_back(index + numEdgeVertices);
                disp_primitive_set->push_back(index);
                disp_primitive_set->push_back(index + 1);
                disp_primitive_set->push_back(index + numEdgeVertices + 1);
            }
        }
    }
}


void VBSPGeometry::addFace(int faceIndex)
{
    Face                  currentFace;
    Edge                  currentEdge;
    DisplaceInfo          currentDispInfo;
    TexInfo               currentTexInfo;
    TexData               currentTexData;
    Vec3                  normal;
    int                   edgeIndex;
    int                   i;
    int                   currentSurfEdge;
    Vec3                  currentVertex;
    Vec3                  texU;
    float                 texUOffset;
    float                 texUScale;
    Vec3                  texV;
    float                 texVOffset;
    float                 texVScale;
    float                 u, v;
    Vec2f                 texCoord;

    // Make sure this face is not "on node" (an internal node of the BSP tree).
    // These faces are not used for visible geometry
    currentFace = bsp_data->getFace(faceIndex);

    // See if this is a displacement surface
    if (currentFace.dispinfo_index != -1)
    {
        // Get the displacement info
        currentDispInfo =
            bsp_data->getDispInfo(currentFace.dispinfo_index);

        // Generate the displacement surface
        createDispSurface(currentFace, currentDispInfo);
    }
    else
    {
        // Get the face normal, using the plane information
        normal = bsp_data->getPlane(currentFace.plane_index).plane_normal;
        if (currentFace.plane_side != 0)
            normal = -normal;

        // Get the texture info and data structures
        currentTexInfo = bsp_data->getTexInfo(currentFace.texinfo_index);
        currentTexData = bsp_data->getTexData(currentTexInfo.texdata_index);

        // Get the texture vectors and offsets.  These are used to calculate
        // texture coordinates
        texU.set(currentTexInfo.texture_vecs[0][0],
                 currentTexInfo.texture_vecs[0][1],
                 currentTexInfo.texture_vecs[0][2]);
        texUOffset = currentTexInfo.texture_vecs[0][3];
        texV.set(currentTexInfo.texture_vecs[1][0],
                 currentTexInfo.texture_vecs[1][1],
                 currentTexInfo.texture_vecs[1][2]);
        texVOffset = currentTexInfo.texture_vecs[1][3];

        // Scale the texture vectors from inches to meters
        texU *= 39.37f;
        texV *= 39.37f;

        // Get the texture size, as the planar texture projection results in
        // non-normalized texture coordinates
        texUScale = 1.0f / (float)currentTexData.texture_width;
        texVScale = 1.0f / (float)currentTexData.texture_height;

        // Start with the last edge index, because we need to switch from
        // clockwise winding (DirectX) to counter-clockwise winding (OpenGL)
        edgeIndex = currentFace.first_edge + currentFace.num_edges - 1;

        // Set the length of this primitive on the primitive set
        primitive_set->push_back(currentFace.num_edges);

        // Iterate over the edges in this face, and extract the vertex data
        for (i = 0; i < currentFace.num_edges; i++)
        {
            // Look up the edge specified by the surface edge index, the
            // index might be negative (see below), so take the absolute
            // value
            currentSurfEdge = bsp_data->getSurfaceEdge(edgeIndex);
            currentEdge = bsp_data->getEdge(abs(currentSurfEdge));

            // The sign of the surface edge index specifies which vertex is
            // "first" for this face.  A negative index means the edge should
            // be flipped, and the second vertex treated as the first
            if (currentSurfEdge < 0)
                currentVertex = bsp_data->getVertex(currentEdge.vertex[1]);
            else
                currentVertex = bsp_data->getVertex(currentEdge.vertex[0]);

            // Add the vertex to the array
            vertex_array->push_back(currentVertex);

            // Set the normal
            normal_array->push_back(normal);

            // Calculate the texture coordinates for this vertex
            u = texU * currentVertex + texUOffset;
            u *= texUScale;
            v = texV * currentVertex + texVOffset;
            v *= texVScale;
            texCoord.set(u, v);

            // Add the texture coordinate to the array
            texcoord_array->push_back(texCoord);

            // Move on to the next (previous?) vertex
            edgeIndex--;
        }
    }
}


ref_ptr<Group> VBSPGeometry::createGeometry()
{
    ref_ptr<Group>       rootGroup;
    ref_ptr<Geode>       geode;
    ref_ptr<Geometry>    geometry;
    Vec4f                color;
    ref_ptr<Vec4Array>   colorArray;

    // Create the root group (we'll attach everything to this group and
    // return it)
    rootGroup = new Group();

    // Create a geode for the geometries
    geode = new Geode();
    rootGroup->addChild(geode.get());

    // See if there are any regular (non-displaced) faces to render
    if (primitive_set->size() > 0)
    {
        // Create a geometry object for the regular surfaces
        geometry = new Geometry();

        // Add the vertex attributes
        geometry->setVertexArray(vertex_array.get());
        geometry->setNormalArray(normal_array.get(), Array::BIND_PER_VERTEX);
        geometry->setTexCoordArray(0, texcoord_array.get());

        // Add an overall color
        color.set(1.0, 1.0, 1.0, 1.0);
        colorArray = new Vec4Array(1, &color);
        geometry->setColorArray(colorArray.get(), Array::BIND_OVERALL);

        // Add our primitive set to the geometry
        geometry->addPrimitiveSet(primitive_set.get());

        // Add the geometry to the geode
        geode->addDrawable(geometry.get());

        // Now, stripify the geode to convert the POLYGON primitives to
        // triangle strips
        osgUtil::optimizeMesh(geode.get());
    }

    // Now do the same for the displacement surfaces (if any)
    if (disp_primitive_set->size() > 0)
    {
        // Create a geometry object for the regular surfaces
        geometry = new Geometry();

        // Add the vertex attributes
        geometry->setVertexArray(disp_vertex_array.get());
        geometry->setNormalArray(disp_normal_array.get(), Array::BIND_PER_VERTEX);
        geometry->setColorArray(disp_vertex_attr_array.get(), Array::BIND_PER_VERTEX);
        geometry->setTexCoordArray(0, disp_texcoord_array.get());
        geometry->setTexCoordArray(1, disp_texcoord_array.get());

        // Add our primitive set to the geometry
        geometry->addPrimitiveSet(disp_primitive_set.get());

        // Add the geometry to the geode
        geode->addDrawable(geometry.get());
    }

    // Return the root group
    return rootGroup;
}

