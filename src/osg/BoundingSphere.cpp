#include "osg/BoundingSphere"

using namespace osg;

void BoundingSphere::expandBy(const Vec3& v)
{
    if (isValid())
    {
        Vec3 dv = v-_center;
        float r = dv.length();
        if (r>_radius)
        { 
            float dr = (r-_radius)*0.5f;
            _center += dv*(dr/r);
            _radius += dr;
        } // else do nothing as vertex is within sphere.
    }
    else
    {
        _center = v;
        _radius = 0.0f;
    }
}

void BoundingSphere::expandRadiusBy(const Vec3& v)
{
    if (isValid())
    {
        float r = (v-_center).length();
        if (r>_radius) _radius = r;
        // else do nothing as vertex is within sphere.
    }
    else
    {
        _center = v;
        _radius = 0.0f;
    }
}

void BoundingSphere::expandBy(const BoundingSphere& sh)
{
    if (sh.isValid())
    {
        if (isValid())
        {
            Vec3 dv = sh._center-_center;
            float dv_len = dv.length();
            if (dv_len+sh._radius>_radius)
            {
            
                Vec3 e1 = _center-(dv*(_radius/dv_len));
                Vec3 e2 = sh._center+(dv*(sh._radius/dv_len));
                _center = (e1+e2)*0.5f;
                _radius = (e2-_center).length();
                
            } // else do nothing as vertex is within sphere.
        }
        else
        {
            _center = sh._center;
            _radius = sh._radius;
        }
    }
}
void BoundingSphere::expandRadiusBy(const BoundingSphere& sh)
{
    if (sh.isValid())
    {
        if (isValid())
        {
            float r = (sh._center-_center).length()+sh._radius;
            if (r>_radius) _radius = r;
            // else do nothing as vertex is within sphere.
        }
        else
        {
            _center = sh._center;
            _radius = sh._radius;
        }
    }
}
