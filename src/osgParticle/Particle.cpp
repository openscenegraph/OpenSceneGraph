#include <osgParticle/Particle>
#include <osgParticle/LinearInterpolator>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Matrix>
#include <osg/GL>
#include <osg/Notify>

namespace
{

    const float cosPI3 = cosf(osg::PI / 3.0f);
    const float sinPI3 = sinf(osg::PI / 3.0f);
    const float hex_texcoord_x1 = 0.5f + 0.5f * cosPI3;
    const float hex_texcoord_x2 = 0.5f - 0.5f * cosPI3;
    const float hex_texcoord_y1 = 0.5f + 0.5f * sinPI3;
    const float hex_texcoord_y2 = 0.5f - 0.5f * sinPI3;

}

osgParticle::Particle::Particle()
:   shape_(QUAD),
    sr_(0.2f, 0.2f),
    ar_(1, 0),
    cr_(osg::Vec4(1, 1, 1, 1), osg::Vec4(1, 1, 1, 1)),
    si_(new LinearInterpolator), 
    ai_(new LinearInterpolator), 
    ci_(new LinearInterpolator),
    alive_(true),
    mustdie_(false),
    lifetime_(2),
    radius_(0.2f),
    mass_(0.1f),
    massinv_(10.0f),
    prev_pos_(0, 0, 0),
    position_(0, 0, 0),
    velocity_(0, 0, 0),
    prev_angle_(0, 0, 0),
    angle_(0, 0, 0),
    angular_vel_(0, 0, 0),
    t0_(0),
    current_size_(0),
    current_alpha_(0),
	s_tile_(1.0f),
    t_tile_(1.0f),
	num_tile_(1),
	cur_tile_(-1),
	s_coord_(0.0f),
	t_coord_(0.0f)
{
}

bool osgParticle::Particle::update(double dt)
{
    // this method should return false when the particle dies;
    // so, if we were instructed to die, do it now and return.
    if (mustdie_) {
        alive_ = false;
        return false;
    }

    double x = 0;    

    // if we don't live forever, compute our normalized age.
    if (lifetime_ > 0) {
        x = t0_ / lifetime_;
    }

    t0_ += dt;

    // if our age is over the lifetime limit, then die and return.
    if (x > 1) {
        alive_ = false;
        return false;
    }

	//Compute the current texture tile based on our normalized age
	int currentTile = static_cast<int>(x * num_tile_);
	
	//If the current texture tile is different from previous, then compute new texture coords
	if(currentTile != cur_tile_) {
		cur_tile_ = currentTile;
		s_coord_ = s_tile_ * fmod(cur_tile_ , 1.0 / s_tile_);
		t_coord_ = 1.0 - t_tile_ * (static_cast<int>(cur_tile_ * t_tile_) + 1);
	}
	
    // compute the current values for size, alpha and color.
    current_size_ = si_.get()->interpolate(x, sr_);
    current_alpha_ = ai_.get()->interpolate(x, ar_);
    current_color_ = ci_.get()->interpolate(x, cr_);

    // update position
    prev_pos_ = position_;
    position_ += velocity_ * dt;

    // update angle
    prev_angle_ = angle_;
    angle_ += angular_vel_ * dt;

    if (angle_.x() > osg::PI*2) angle_.x() -= osg::PI*2;
    if (angle_.x() < -osg::PI*2) angle_.x() += osg::PI*2;
    if (angle_.y() > osg::PI*2) angle_.y() -= osg::PI*2;
    if (angle_.y() < -osg::PI*2) angle_.y() += osg::PI*2;
    if (angle_.z() > osg::PI*2) angle_.z() -= osg::PI*2;
    if (angle_.z() < -osg::PI*2) angle_.z() += osg::PI*2;

    return true;
}

void osgParticle::Particle::render(const osg::Vec3 &xpos, const osg::Vec3 &px, const osg::Vec3 &py, float scale) const
{
    glColor4f(  current_color_.x(), 
                current_color_.y(), 
                current_color_.z(), 
                current_color_.w() * current_alpha_);

    osg::Matrix R;
    R.makeRotate(
        angle_.x(), osg::Vec3(1, 0, 0), 
        angle_.y(), osg::Vec3(0, 1, 0), 
        angle_.z(), osg::Vec3(0, 0, 1));

    osg::Vec3 p1(px * current_size_ * scale);
    osg::Vec3 p2(py * current_size_ * scale);

    switch (shape_)
    {
    case POINT:        
        glVertex3f(xpos.x(), xpos.y(), xpos.z()); 
        break;

    case QUAD:
        glTexCoord2f(s_coord_, t_coord_);
        glVertex3fv((xpos-(p1+p2)*R).ptr());
        glTexCoord2f(s_coord_+s_tile_, t_coord_);
        glVertex3fv((xpos+(p1-p2)*R).ptr());
        glTexCoord2f(s_coord_+s_tile_, t_coord_+t_tile_);
        glVertex3fv((xpos+(p1+p2)*R).ptr());
        glTexCoord2f(s_coord_, t_coord_+t_tile_);
        glVertex3fv((xpos-(p1-p2)*R).ptr());
        break;

    case QUAD_TRIANGLESTRIP:
        glPushMatrix();
        glTranslatef(xpos.x(), xpos.y(), xpos.z());
        glMultMatrix(R.ptr());
        // we must glBegin() and glEnd() here, because each particle is a single strip
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(s_coord_+s_tile_, t_coord_+t_tile_);
        glVertex3fv((p1+p2).ptr());      
        glTexCoord2f(s_coord_, t_coord_+t_tile_);
        glVertex3fv((-p1+p2).ptr());        
        glTexCoord2f(s_coord_+s_tile_, t_coord_);
        glVertex3fv((p1-p2).ptr());
        glTexCoord2f(s_coord_, t_coord_);
        glVertex3fv((-p1-p2).ptr());
        glEnd();
        glPopMatrix();
        break;

    case HEXAGON:
        glPushMatrix();
        glTranslatef(xpos.x(), xpos.y(), xpos.z());
        glMultMatrix(R.ptr());        
        // we must glBegin() and glEnd() here, because each particle is a single fan
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(s_coord_ + s_tile_ * 0.5f, t_coord_ + t_tile_ * 0.5f);
        glVertex3f(0,0,0);
        glTexCoord2f(s_coord_ + s_tile_ * hex_texcoord_x1, t_coord_ + t_tile_ * hex_texcoord_y1);
        glVertex3fv((p1*cosPI3+p2*sinPI3).ptr());
        glTexCoord2f(s_coord_ + s_tile_ * hex_texcoord_x2, t_coord_ + t_tile_ * hex_texcoord_y1);
        glVertex3fv((-p1*cosPI3+p2*sinPI3).ptr());
        glTexCoord2f(s_coord_, t_coord_ + t_tile_ * 0.5f);
        glVertex3fv((-p1).ptr());
        glTexCoord2f(s_coord_ + s_tile_ * hex_texcoord_x2, t_coord_ + t_tile_ * hex_texcoord_y2);
        glVertex3fv((-p1*cosPI3-p2*sinPI3).ptr());
        glTexCoord2f(s_coord_ + s_tile_ * hex_texcoord_x1, t_coord_ + t_tile_ * hex_texcoord_y2);
        glVertex3fv((p1*cosPI3-p2*sinPI3).ptr());
        glTexCoord2f(s_coord_ + s_tile_, t_coord_ + t_tile_ * 0.5f);
        glVertex3fv((p1).ptr());
        glTexCoord2f(s_coord_ + s_tile_ * hex_texcoord_x1, t_coord_ + t_tile_ * hex_texcoord_y1);
        glVertex3fv((p1*cosPI3+p2*sinPI3).ptr());
        glEnd();
        glPopMatrix();
        break;

    case LINE:
        {
            // Get the normalized direction of the particle, to be used in the 
            // calculation of one of the linesegment endpoints.
            float vl = velocity_.length();
            if (vl != 0) {
                osg::Vec3 v = velocity_ * current_size_ * scale / vl;

                glTexCoord1f(0);
                glVertex3f(xpos.x(), xpos.y(), xpos.z());
                glTexCoord1f(1);
                glVertex3f(xpos.x() + v.x(), xpos.y() + v.y(), xpos.z() + v.z());
            }
        }
        break;

    default:
        osg::notify(osg::WARN) << "Invalid shape for particles\n";
    }
}
