#include "osg/BoundingBox"
#include "osg/BoundingSphere"

using namespace osg;

void BoundingBox::expandBy(const Vec3& v)
{
    if(v.x()<_min.x()) _min.x() = v.x();
    if(v.x()>_max.x()) _max.x() = v.x();

    if(v.y()<_min.y()) _min.y() = v.y();
    if(v.y()>_max.y()) _max.y() = v.y();

    if(v.z()<_min.z()) _min.z() = v.z();
    if(v.z()>_max.z()) _max.z() = v.z();
}


void BoundingBox::expandBy(const BoundingBox& bb)
{
    if (!bb.isValid()) return;

    if(bb._min.x()<_min.x()) _min.x() = bb._min.x();
    if(bb._max.x()>_max.x()) _max.x() = bb._max.x();

    if(bb._min.y()<_min.y()) _min.y() = bb._min.y();
    if(bb._max.y()>_max.y()) _max.y() = bb._max.y();

    if(bb._min.z()<_min.z()) _min.z() = bb._min.z();
    if(bb._max.z()>_max.z()) _max.z() = bb._max.z();
}


void BoundingBox::expandBy(const BoundingSphere& sh)
{
    if (!sh.isValid()) return;

    if(sh._center.x()-sh._radius<_min.x()) _min.x() = sh._center.x()-sh._radius;
    if(sh._center.x()+sh._radius>_max.x()) _max.x() = sh._center.x()+sh._radius;

    if(sh._center.y()-sh._radius<_min.y()) _min.y() = sh._center.y()-sh._radius;
    if(sh._center.y()+sh._radius>_max.y()) _max.y() = sh._center.y()+sh._radius;

    if(sh._center.z()-sh._radius<_min.z()) _min.z() = sh._center.z()-sh._radius;
    if(sh._center.z()+sh._radius>_max.z()) _max.z() = sh._center.z()+sh._radius;
}
