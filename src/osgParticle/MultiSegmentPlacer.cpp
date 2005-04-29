#include <osgParticle/MultiSegmentPlacer>
#include <osgParticle/Placer>

#include <osg/CopyOp>
#include <osg/Vec3>

osgParticle::MultiSegmentPlacer::MultiSegmentPlacer()
: Placer(), _total_length(0)
{
}
    
osgParticle::MultiSegmentPlacer::MultiSegmentPlacer(const MultiSegmentPlacer& copy, const osg::CopyOp& copyop)
: Placer(copy, copyop), _vx(copy._vx), _total_length(copy._total_length)
{
}

void osgParticle::MultiSegmentPlacer::recompute_length()
{
    Vertex_vector::iterator i;
    Vertex_vector::iterator i0 = _vx.begin();
        
    _total_length = 0;
    for (i=_vx.begin(); i!=_vx.end(); ++i)
    {
        _total_length += (i->first - i0->first).length();
        i->second = _total_length;
        i0 = i;            
    }
}

void osgParticle::MultiSegmentPlacer::place(Particle* P) const
{
    if (_vx.size() >= 2) {
        float x = rangef(0, _total_length).get_random();
        
        Vertex_vector::const_iterator i;
        Vertex_vector::const_iterator i0 = _vx.begin();
        const Vertex_vector::const_iterator vend = _vx.end();

        for (i=_vx.begin(); i!=vend; ++i)
        {
            if (x <= i->second)
            {
                float t = (x - i0->second) / (i->second - i0->second);
                P->setPosition(i0->first + (i->first - i0->first) * t);
                return;
            }
            i0 = i;
        }            
    }
    else
    {
        osg::notify(osg::WARN) << "this MultiSegmentPlacer has less than 2 vertices\n";
    }
}

