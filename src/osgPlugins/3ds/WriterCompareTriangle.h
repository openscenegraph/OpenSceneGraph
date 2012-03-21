#ifndef _3DS_WRITER_COMPARE_TRIANGLE_HEADER__
#define _3DS_WRITER_COMPARE_TRIANGLE_HEADER__

#include <osg/Geode>
#include <osg/Geometry>
#include <iostream>

struct Triangle
{
    unsigned int t1;
    unsigned int t2;
    unsigned int t3;
    unsigned int material;
};

class WriterCompareTriangle {
public:
    WriterCompareTriangle(const osg::Geode & geode, unsigned int nbVertices);

    bool operator()(const std::pair<Triangle, int>    &    t1,
                    const std::pair<Triangle, int>    &    t2) const;
private:
    void // This function prevent from cut scene in too many blocs
        setMaxMin(unsigned int & nbVerticesX,
                  unsigned int & nbVerticesY,
                  unsigned int & nbVerticesZ) const;

    /**
    *  Cut the scene in different bloc to sort.
    *  \param nbVertices is the number of vertice in mesh.
    *  \param sceneBox contain the size of the scene.
    */
    void
    cutscene(int                        nbVertices,
             const osg::BoundingBox   &    sceneBox);

    /**
    *  Find in which box those points are.
    *  \return the place of the box in the vector.
    *  \sa See cutScene() about the definition of the boxes for faces sorting.
    */
    int inWhichBox(const osg::BoundingBox::value_type x,
                   const osg::BoundingBox::value_type y,
                   const osg::BoundingBox::value_type z) const;
    int inWhichBox(const osg::BoundingBox::vec_type & point) const;

    const osg::Geode                  &    geode;
    std::vector<osg::BoundingBox>        boxList;
};

#endif // _3DS_WRITER_COMPARE_TRIANGLE_HEADER__
