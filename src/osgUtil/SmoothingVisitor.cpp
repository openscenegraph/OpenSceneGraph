#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <list>
#include <set>

#include <osg/GeoSet>

#include <osgUtil/SmoothingVisitor>

using namespace osg;
using namespace osgUtil;

struct LessPtr
{
    inline bool operator() (const osg::Vec3* lhs,const osg::Vec3* rhs) const
    {
        return *lhs<*rhs;
    }
};

// triangle functor.
struct TriangleFunctor
{

    osg::Vec3 *_coordBase;
    osg::Vec3 *_normalBase;

    typedef std::multiset<const osg::Vec3*,LessPtr> CoordinateSet;
    CoordinateSet _coordSet;

    TriangleFunctor(osg::Vec3 *cb,int noVertices, osg::Vec3 *nb) : _coordBase(cb),_normalBase(nb)
    {
        osg::Vec3* vptr = cb;
        for(int i=0;i<noVertices;++i)
        {
            _coordSet.insert(vptr++);
        }
    }

    inline void updateNormal(const osg::Vec3& normal,const osg::Vec3* vptr)
    {
        std::pair<CoordinateSet::iterator,CoordinateSet::iterator> p =
            _coordSet.equal_range(vptr);

        for(CoordinateSet::iterator itr=p.first;
            itr!=p.second;
            ++itr)
        {
            osg::Vec3* nptr = _normalBase + (*itr-_coordBase);
            (*nptr) += normal;
        }
    }

    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3 )
    {
        // calc orientation of triangle.
        osg::Vec3 normal = (v2-v1)^(v3-v1);
        // normal.normalize();

        updateNormal(normal,&v1);
        updateNormal(normal,&v2);
        updateNormal(normal,&v3);

    }
};

void SmoothingVisitor::smooth(osg::GeoSet& gset)
{
    GeoSet::PrimitiveType primTypeIn = gset.getPrimType();
    GeoSet::PrimitiveType primTypeOut = gset.getPrimType();

    // determine whether to do smoothing or not, and if
    // the primitive type needs to be modified enable smoothed normals.
    bool doSmoothing;
    switch(primTypeIn)
    {
        case(GeoSet::TRIANGLES):
        case(GeoSet::TRIANGLE_STRIP):
        case(GeoSet::TRIANGLE_FAN):
            doSmoothing = true;
            break;
/*
        case(GeoSet::FLAT_TRIANGLE_STRIP):
            primTypeOut = GeoSet::TRIANGLE_STRIP;
            doSmoothing = true;
            break;
        case(GeoSet::FLAT_TRIANGLE_FAN):
            primTypeOut = GeoSet::TRIANGLE_FAN;
            doSmoothing = true;
            break;
*/
        case(GeoSet::QUADS):
        case(GeoSet::QUAD_STRIP):
        case(GeoSet::POLYGON):
            doSmoothing = true;
            break;
        default:                 // points and lines etc.
            doSmoothing = false;
            break;
    }

    if (doSmoothing)
    {
        gset.computeNumVerts();
        int ncoords = gset.getNumCoords();
        osg::Vec3 *coords = gset.getCoords();
        osg::GeoSet::IndexPointer cindex = gset.getCoordIndices();
        osg::Vec3 *norms = new osg::Vec3[ncoords];

        int j;
        for(j = 0; j < ncoords; j++ )
        {
            norms[j].set(0.0f,0.0f,0.0f);
        }

        TriangleFunctor tf(coords,ncoords,norms);
        for_each_triangle( gset, tf );

        for(j = 0; j < ncoords; j++ )
        {
            float len = norms[j].length();
            if (len) norms[j]/=len;
        }

        gset.setNormalBinding(osg::GeoSet::BIND_PERVERTEX);
        
        if (cindex.valid())
        {
            if (cindex._is_ushort)
                gset.setNormals( norms, cindex._ptr._ushort );
            else
                gset.setNormals( norms, cindex._ptr._uint );
        }
        else
        {
            gset.setNormals( norms );
        }

        if (primTypeIn!=primTypeOut)
        {
            
            if (primTypeIn==GeoSet::FLAT_TRIANGLE_STRIP)
            {
                gset.setColorBinding( osg::GeoSet::BIND_OFF );
                gset.setColors(NULL);
            }
            else
            if (primTypeIn==GeoSet::FLAT_TRIANGLE_STRIP)
            {
                gset.setColorBinding( osg::GeoSet::BIND_OFF );
                gset.setColors(NULL);
            }
            
            gset.setPrimType( primTypeOut );
            
        }

        gset.dirtyDisplayList();

    }
}


void SmoothingVisitor::apply(osg::Geode& geode)
{
    for(int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::GeoSet* gset = dynamic_cast<osg::GeoSet*>(geode.getDrawable(i));
        if (gset) smooth(*gset);
    }
}
