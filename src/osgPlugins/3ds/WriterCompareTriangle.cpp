#include "WriterCompareTriangle.h"
#include <assert.h>

WriterCompareTriangle::WriterCompareTriangle(const osg::Geode & geode, unsigned int nbVertices) : geode(geode) 
{
    cutscene(nbVertices, geode.getBoundingBox());
}

bool 
WriterCompareTriangle::operator()(const std::pair<Triangle, int> & t1, 
                                  const std::pair<Triangle, int> & t2) const
{
    const osg::Geometry *g = geode.getDrawable( t1.second )->asGeometry();

    const osg::Vec3Array * vecs= static_cast<const osg::Vec3Array *>(g->getVertexArray());
    const osg::BoundingBox::vec_type v1( (*vecs)[t1.first.t1] );

    if (t1.second != t2.second)
    {
        const osg::Geometry *g = geode.getDrawable( t2.second )->asGeometry();
        vecs = static_cast<const osg::Vec3Array *>(g->getVertexArray());
    };
    const osg::BoundingBox::vec_type v2( (*vecs)[t2.first.t1] );
    int val1 = inWhichBox(v1);
    int val2 = inWhichBox(v2);

    return (val1 < val2);
}

void
WriterCompareTriangle::setMaxMin(unsigned int & nbVerticesX,
                                 unsigned int & nbVerticesY,
                                 unsigned int & nbVerticesZ) const    
{
    static const unsigned int min = 1;
    static const unsigned int max = 5;        // Number of blocks used to divide the scene (arbitrary but seems ok)
    nbVerticesX = osg::clampBetween<unsigned int>(nbVerticesX, min, max);
    nbVerticesY = osg::clampBetween<unsigned int>(nbVerticesY, min, max);
    nbVerticesZ = osg::clampBetween<unsigned int>(nbVerticesZ, min, max);
}

void WriterCompareTriangle::cutscene(int nbVertices, const osg::BoundingBox & sceneBox)
{
    osg::BoundingBox::vec_type length = sceneBox._max - sceneBox._min;

    static const float k = 1.3f;        // Arbitrary constant multiplier for density computation ("simulates" non-uniform point distributions)
    // Computes "density" of points, and thus the number of blocks to divide the mesh into
    unsigned int nbVerticesX = static_cast<unsigned int>( (nbVertices * k) / (length.z() * length.y()) );
    unsigned int nbVerticesY = static_cast<unsigned int>( (nbVertices * k) / (length.z() * length.x()) );
    unsigned int nbVerticesZ = static_cast<unsigned int>( (nbVertices * k) / (length.x() * length.y()) );

    setMaxMin (nbVerticesX, nbVerticesY, nbVerticesZ); // This function prevent from cutting the scene in too many blocs

    OSG_INFO
        << "Cutting x by " << nbVerticesX << std::endl
        << "Cutting y by " << nbVerticesY << std::endl
        << "Cutting z by " << nbVerticesZ << std::endl;

    osg::BoundingBox::value_type blocX = length.x() / nbVerticesX;    // These 3 lines set the size of a bloc in x, y and z
    osg::BoundingBox::value_type blocY = length.y() / nbVerticesY;
    osg::BoundingBox::value_type blocZ = length.z() / nbVerticesZ;

    boxList.reserve(nbVerticesX * nbVerticesY * nbVerticesZ);
    short yinc = 1;
    short xinc = 1;
    unsigned int y = 0;
    unsigned int x = 0;
    for (unsigned int z = 0; z < nbVerticesZ; ++z)
    {
        while (x < nbVerticesX && x >= 0)
        {
            while (y < nbVerticesY && y >= 0)
            {
                osg::BoundingBox::value_type xMin = sceneBox.xMin() + x * blocX;
                if (x == 0) //to prevent from mesh with no case
                    xMin -= 10;

                osg::BoundingBox::value_type yMin = sceneBox.yMin() + y * blocY;
                if (y == 0) //to prevent from mesh with no case
                    yMin -= 10;

                osg::BoundingBox::value_type zMin = sceneBox.zMin() + z * blocZ;
                if (z == 0) //to prevent from mesh with no case
                    zMin -= 10;

                osg::BoundingBox::value_type xMax = sceneBox.xMin() + (x + 1) * blocX;
                if (x == nbVerticesX - 1) //to prevent from mesh with no case
                    xMax += 10;

                osg::BoundingBox::value_type yMax = sceneBox.yMin() + (y + 1) * blocY;
                if (y == nbVerticesY - 1) //to prevent from mesh with no case
                    yMax += 10;

                osg::BoundingBox::value_type zMax = sceneBox.zMin() + (z + 1) * blocZ;
                if (z == nbVerticesZ - 1) //to prevent from mesh with no case
                    zMax += 10;

                boxList.push_back(osg::BoundingBox(xMin, // Add a bloc to the list
                    yMin,
                    zMin,
                    xMax, 
                    yMax,
                    zMax));
                y += yinc;
            }
            yinc = -yinc;
            y += yinc;
            x += xinc;
        }
        xinc = -xinc;
        x += xinc;
    }
}

int 
WriterCompareTriangle::inWhichBox(const osg::BoundingBox::value_type x, 
                                  const osg::BoundingBox::value_type y,
                                  const osg::BoundingBox::value_type z) const
{
    for (unsigned int i = 0; i < boxList.size(); ++i)
    {
        if (x >= boxList[i].xMin() && 
            x <  boxList[i].xMax() &&
            y >= boxList[i].yMin() &&
            y <  boxList[i].yMax() &&
            z >= boxList[i].zMin() &&
            z <  boxList[i].zMax())
        {
            return i;
        }
    }
    assert(false && "Point is not in any blocs");
    return 0;
}

int WriterCompareTriangle::inWhichBox(const osg::BoundingBox::vec_type & point) const {
    return inWhichBox(point.x(), point.y(), point.z());
}
