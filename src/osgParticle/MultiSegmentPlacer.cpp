#include <osgParticle/MultiSegmentPlacer>
#include <osgParticle/Placer>

#include <osg/CopyOp>
#include <osg/Vec3>

osgParticle::MultiSegmentPlacer::MultiSegmentPlacer()
: Placer(), total_length_(0)
{
}
    
osgParticle::MultiSegmentPlacer::MultiSegmentPlacer(const MultiSegmentPlacer &copy, const osg::CopyOp &copyop)
: Placer(copy, copyop), vx_(copy.vx_), total_length_(copy.total_length_)
{
}

void osgParticle::MultiSegmentPlacer::recompute_length()
{
    Vertex_vector::iterator i;
    Vertex_vector::iterator i0 = vx_.begin();
        
    total_length_ = 0;
    for (i=vx_.begin(); i!=vx_.end(); ++i) {
        total_length_ += (i->first - i0->first).length();
        i->second = total_length_;
        i0 = i;            
    }
}

void osgParticle::MultiSegmentPlacer::place(Particle *P) const
{
    if (vx_.size() >= 2) {
        float x = rangef(0, total_length_).get_random();
        
        Vertex_vector::const_iterator i;
        Vertex_vector::const_iterator i0 = vx_.begin();
        const Vertex_vector::const_iterator vend = vx_.end();

        for (i=vx_.begin(); i!=vend; ++i) {
            if (x <= i->second) {
                float t = (x - i0->second) / (i->second - i0->second);
                P->setPosition(i0->first + (i->first - i0->first) * t);
                return;
            }
            i0 = i;
        }            
    } else {
        osg::notify(osg::WARN) << "this MultiSegmentPlacer has less than 2 vertices\n";
    }
}

