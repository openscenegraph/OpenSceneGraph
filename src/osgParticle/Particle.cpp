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
:    shape_(QUAD),
    sr_(0.2f, 0.2f),
    ar_(1, 0),
    cr_(osg::Vec4(1, 1, 1, 1), osg::Vec4(1, 1, 1, 1)),
    si_(osgNew LinearInterpolator), 
    ai_(osgNew LinearInterpolator), 
    ci_(osgNew LinearInterpolator),
    alive_(true),
    mustdie_(false),
    lifetime_(2),
    radius_(0.2f),
    mass_(0.1f),
    massinv_(10.0f),
    prev_pos_(0, 0, 0),
    position_(0, 0, 0),
    velocity_(0, 0, 0),
    t0_(0),
    current_size_(0),
    current_alpha_(0)
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

    // compute the current values for size, alpha and color.
    current_size_ = si_.get()->interpolate(x, sr_);
    current_alpha_ = ai_.get()->interpolate(x, ar_);
    current_color_ = ci_.get()->interpolate(x, cr_);

    // update position
    prev_pos_ = position_;
    position_ += velocity_ * dt;

    return true;
}

void osgParticle::Particle::render(const osg::Matrix &modelview, float scale) const
{
    osg::Vec3 xpos = modelview.preMult(position_);

    glColor4f(    current_color_.x(), 
                current_color_.y(), 
                current_color_.z(), 
                current_color_.w() * current_alpha_);

    float cs = current_size_ * scale;

    switch (shape_)
    {
    case POINT:        
        glVertex3f(xpos.x(), xpos.y(), xpos.z());        
        break;

    case QUAD:
        glTexCoord2f(0, 0);
        glVertex3f(xpos.x() - cs, xpos.y() - cs, xpos.z());
        glTexCoord2f(1, 0);
        glVertex3f(xpos.x() + cs, xpos.y() - cs, xpos.z());
        glTexCoord2f(1, 1);
        glVertex3f(xpos.x() + cs, xpos.y() + cs, xpos.z());
        glTexCoord2f(0, 1);
        glVertex3f(xpos.x() - cs, xpos.y() + cs, xpos.z());
        break;

    case QUAD_TRIANGLESTRIP:
        // we must glBegin() and glEnd() here, because each particle is a single strip
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(1, 1);
        glVertex3f(xpos.x() + cs, xpos.y() + cs, xpos.z());
        glTexCoord2f(0, 1);
        glVertex3f(xpos.x() - cs, xpos.y() + cs, xpos.z());
        glTexCoord2f(1, 0);
        glVertex3f(xpos.x() + cs, xpos.y() - cs, xpos.z());
        glTexCoord2f(0, 0);
        glVertex3f(xpos.x() - cs, xpos.y() - cs, xpos.z());
        glEnd();
        break;

    case HEXAGON:
        // we must glBegin() and glEnd() here, because each particle is a single fan
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(0.5f, 0.5f);
        glVertex3f(xpos.x(), xpos.y(), xpos.z());
        glTexCoord2f(hex_texcoord_x1, hex_texcoord_y1);
        glVertex3f(xpos.x() + cs * cosPI3, xpos.y() + cs * sinPI3, xpos.z());
        glTexCoord2f(hex_texcoord_x2, hex_texcoord_y1);
        glVertex3f(xpos.x() - cs * cosPI3, xpos.y() + cs * sinPI3, xpos.z());
        glTexCoord2f(0, 0.5f);
        glVertex3f(xpos.x() - cs, xpos.y(), xpos.z());
        glTexCoord2f(hex_texcoord_x2, hex_texcoord_y2);
        glVertex3f(xpos.x() - cs * cosPI3, xpos.y() - cs * sinPI3, xpos.z());
        glTexCoord2f(hex_texcoord_x1, hex_texcoord_y2);
        glVertex3f(xpos.x() + cs * cosPI3, xpos.y() - cs * sinPI3, xpos.z());
        glTexCoord2f(1, 0.5f);
        glVertex3f(xpos.x() + cs, xpos.y(), xpos.z());
        glTexCoord2f(hex_texcoord_x1, hex_texcoord_y1);
        glVertex3f(xpos.x() + cs * cosPI3, xpos.y() + cs * sinPI3, xpos.z());
        glEnd();
        break;

    default:
        osg::notify(osg::WARN) << "Invalid shape for particles\n";
    }
}
