#include <osgUtil/SmoothingVisitor>

#include <stdio.h>
#include <list>
#include <set>


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
struct SmoothTriangleFunctor
{

    osg::Vec3* _coordBase;
    osg::Vec3* _normalBase;

    typedef std::multiset<const osg::Vec3*,LessPtr> CoordinateSet;
    CoordinateSet _coordSet;

    SmoothTriangleFunctor():
         _coordBase(0),
         _normalBase(0) {}
    
    void set(osg::Vec3 *cb,int noVertices, osg::Vec3 *nb)
    {
        _coordBase=cb;
        _normalBase=nb;

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

void SmoothingVisitor::smooth(osg::Geometry& geom)
{
    Geometry::PrimitiveSetList& primitives = geom.getPrimitiveSetList();
    Geometry::PrimitiveSetList::iterator itr;
    unsigned int numSurfacePrimitives=0;
    for(itr=primitives.begin();
        itr!=primitives.end();
        ++itr)
    {
        switch((*itr)->getMode())
        {
            case(PrimitiveSet::TRIANGLES):
            case(PrimitiveSet::TRIANGLE_STRIP):
            case(PrimitiveSet::TRIANGLE_FAN):
            case(PrimitiveSet::QUADS):
            case(PrimitiveSet::QUAD_STRIP):
            case(PrimitiveSet::POLYGON):
                ++numSurfacePrimitives;
                break;
            default:
                break;
        }
    }
    
    if (!numSurfacePrimitives) return;

    osg::Vec3Array *coords = geom.getVertexArray();
    if (!coords || !coords->size()) return;
    
    osg::Vec3Array *normals = osgNew osg::Vec3Array(coords->size());

    osg::Vec3Array::iterator nitr;
    for(nitr = normals->begin();
        nitr!=normals->end();
        ++nitr)
    {
        nitr->set(0.0f,0.0f,0.0f);
    }

    TriangleFunctor<SmoothTriangleFunctor> stf;
    stf.set(&(coords->front()),coords->size(),&(normals->front()));
    
    geom.accept(stf);

    for(nitr= normals->begin();
        nitr!=normals->end();
        ++nitr)
    {
        nitr->normalize();
    }

    geom.setNormalArray( normals );
    geom.setNormalBinding(osg::Geometry::BIND_PER_VERTEX);


    geom.dirtyDisplayList();
}


void SmoothingVisitor::apply(osg::Geode& geode)
{
    for(unsigned int i = 0; i < geode.getNumDrawables(); i++ )
    {
        osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
        if (geom) smooth(*geom);
    }
}
