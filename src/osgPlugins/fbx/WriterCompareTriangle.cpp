#include "WriterCompareTriangle.h"

WriterCompareTriangle::WriterCompareTriangle(const osg::Geode& in_geode, unsigned int in_nbVertices):
        geode(geode)
{
    cutscene(in_nbVertices, geode.getDrawable(0)->asGeometry()->getBoundingBox());
}

bool
WriterCompareTriangle::operator()(const std::pair<Triangle, int>& t1,
                                  const std::pair<Triangle, int>& t2) const
{
    const osg::Geometry* g = geode.getDrawable( t1.second )->asGeometry();

    const osg::Vec3Array* vecs= static_cast<const osg::Vec3Array*>(g->getVertexArray());
    const osg::Vec3::value_type x1 = (*vecs)[t1.first.t1].x();
    const osg::Vec3::value_type y1 = (*vecs)[t1.first.t1].y();
    const osg::Vec3::value_type z1 = (*vecs)[t1.first.t1].z();

    if (t1.second != t2.second)
    {
        const osg::Geometry* g2 = geode.getDrawable( t2.second )->asGeometry();
        vecs = static_cast<const osg::Vec3Array*>(g2->getVertexArray());
    }
    const osg::Vec3::value_type x2 = (*vecs)[t2.first.t1].x();
    const osg::Vec3::value_type y2 = (*vecs)[t2.first.t1].y();
    const osg::Vec3::value_type z2 = (*vecs)[t2.first.t1].z();
    int val1 = inWhichBox(x1,y1,z1);
    int val2 = inWhichBox(x2,y2,z2);

    return (val1 < val2);
}

void
WriterCompareTriangle::setMaxMin(unsigned int& nbVerticesX,
                                 unsigned int& nbVerticesY,
                                 unsigned int& nbVerticesZ) const
{
    static const unsigned int min = 1;
    if (nbVerticesX < min)
        nbVerticesX = min;
    if (nbVerticesY < min)
        nbVerticesY = min;
    if (nbVerticesZ < min)
        nbVerticesZ = min;

    static const unsigned int max = 20;

    if (nbVerticesX > max)
        nbVerticesX = max;
    if (nbVerticesY > max)
        nbVerticesY = max;
    if (nbVerticesZ > max)
        nbVerticesZ = max;
}

void
WriterCompareTriangle::cutscene(int                     nbVertices,
                                const osg::BoundingBox& sceneBox)
{
    osg::BoundingBox::vec_type length = sceneBox._max - sceneBox._min;

    static const unsigned int k = 4;

    unsigned int nbVerticesX = (nbVertices * k) / (length.z() * length.y());
    unsigned int nbVerticesY = (nbVertices * k) / (length.z() * length.x());
    unsigned int nbVerticesZ = (nbVertices * k) / (length.x() * length.y());

    setMaxMin (nbVerticesX, nbVerticesY, nbVerticesZ);

    OSG_DEBUG << "Cutting x by " << nbVerticesX << std::endl
        << "Cutting y by " << nbVerticesY << std::endl
        << "Cutting z by " << nbVerticesZ << std::endl;

    osg::BoundingBox::value_type blocX = length.x() / nbVerticesX; //These 3 lines set the size of a box in x, y and z
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

                boxList.push_back(osg::BoundingBox(xMin, // Add a box to the list
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
WriterCompareTriangle::inWhichBox(const osg::Vec3::value_type x,
                                  const osg::Vec3::value_type y,
                                  const osg::Vec3::value_type z) const
{
    for (unsigned int i = 0; i < boxList.size(); ++i)
    {
        if (x >= boxList[i].xMin() &&
            x < boxList[i].xMax() &&
            y >= boxList[i].yMin() &&
            y < boxList[i].yMax() &&
            z >= boxList[i].zMin() &&
            z < boxList[i].zMax())
        {
            return i;
        }
    }
    throw "Point is not in any box";
}
