#include <osgParticle/Particle>
#include <osgParticle/LinearInterpolator>
#include <osgParticle/ParticleSystem>

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
:   _shape(QUAD),
    _sr(0.2f, 0.2f),
    _ar(1, 0),
    _cr(osg::Vec4(1, 1, 1, 1), osg::Vec4(1, 1, 1, 1)),
    _si(new LinearInterpolator), 
    _ai(new LinearInterpolator), 
    _ci(new LinearInterpolator),
    _alive(true),
    _mustdie(false),
    _lifeTime(2),
    _radius(0.2f),
    _mass(0.1f),
    _massinv(10.0f),
    _prev_pos(0, 0, 0),
    _position(0, 0, 0),
    _velocity(0, 0, 0),
    _prev_angle(0, 0, 0),
    _angle(0, 0, 0),
    _angul_arvel(0, 0, 0),
    _t0(0),
    _current_size(0),
    _current_alpha(0),
    _s_tile(1.0f),
    _t_tile(1.0f),
    _num_tile(1),
    _cur_tile(-1),
    _s_coord(0.0f),
    _t_coord(0.0f),
    _previousParticle(INVALID_INDEX),
    _nextParticle(INVALID_INDEX)
{
}

bool osgParticle::Particle::update(double dt)
{
    // this method should return false when the particle dies;
    // so, if we were instructed to die, do it now and return.
    if (_mustdie) {
        _alive = false;
        return false;
    }

    double x = 0;    

    // if we don't live forever, compute our normalized age.
    if (_lifeTime > 0) {
        x = _t0 / _lifeTime;
    }

    _t0 += dt;

    // if our age is over the lifetime limit, then die and return.
    if (x > 1) {
        _alive = false;
        return false;
    }

    //Compute the current texture tile based on our normalized age
    int currentTile = static_cast<int>(x * _num_tile);
    
    //If the current texture tile is different from previous, then compute new texture coords
    if(currentTile != _cur_tile)
    {
    
        _cur_tile = currentTile;
        _s_coord = _s_tile * fmod(_cur_tile , 1.0 / _s_tile);
        _t_coord = 1.0 - _t_tile * (static_cast<int>(_cur_tile * _t_tile) + 1);

        // osg::notify(osg::NOTICE)<<this<<" setting tex coords "<<_s_coord<<" "<<_t_coord<<std::endl;
    }
    
    // compute the current values for size, alpha and color.
    if (_lifeTime <= 0) {
       if (dt == _t0) {
          _current_size = _sr.get_random();
          _current_alpha = _ar.get_random();
          _current_color = _cr.get_random();
       }
    } else {
       _current_size = _si.get()->interpolate(x, _sr);
       _current_alpha = _ai.get()->interpolate(x, _ar);
       _current_color = _ci.get()->interpolate(x, _cr);
    }

    // update position
    _prev_pos = _position;
    _position += _velocity * dt;

    // update angle
    _prev_angle = _angle;
    _angle += _angul_arvel * dt;

    if (_angle.x() > osg::PI*2) _angle.x() -= osg::PI*2;
    if (_angle.x() < -osg::PI*2) _angle.x() += osg::PI*2;
    if (_angle.y() > osg::PI*2) _angle.y() -= osg::PI*2;
    if (_angle.y() < -osg::PI*2) _angle.y() += osg::PI*2;
    if (_angle.z() > osg::PI*2) _angle.z() -= osg::PI*2;
    if (_angle.z() < -osg::PI*2) _angle.z() += osg::PI*2;

    return true;
}

void osgParticle::Particle::render(const osg::Vec3& xpos, const osg::Vec3& px, const osg::Vec3& py, float scale) const
{
    glColor4f(  _current_color.x(), 
                _current_color.y(), 
                _current_color.z(), 
                _current_color.w() * _current_alpha);

    osg::Vec3 p1(px * _current_size * scale);
    osg::Vec3 p2(py * _current_size * scale);

    switch (_shape)
    {
    case POINT:        
        glVertex3f(xpos.x(), xpos.y(), xpos.z()); 
        break;

    case QUAD:
        glTexCoord2f(_s_coord, _t_coord);
        glVertex3fv((xpos-(p1+p2)).ptr());
        glTexCoord2f(_s_coord+_s_tile, _t_coord);
        glVertex3fv((xpos+(p1-p2)).ptr());
        glTexCoord2f(_s_coord+_s_tile, _t_coord+_t_tile);
        glVertex3fv((xpos+(p1+p2)).ptr());
        glTexCoord2f(_s_coord, _t_coord+_t_tile);
        glVertex3fv((xpos-(p1-p2)).ptr());
        break;

    case QUAD_TRIANGLESTRIP:
        glPushMatrix();
        glTranslatef(xpos.x(), xpos.y(), xpos.z());
        // we must glBegin() and glEnd() here, because each particle is a single strip
        glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(_s_coord+_s_tile, _t_coord+_t_tile);
        glVertex3fv((p1+p2).ptr());      
        glTexCoord2f(_s_coord, _t_coord+_t_tile);
        glVertex3fv((-p1+p2).ptr());        
        glTexCoord2f(_s_coord+_s_tile, _t_coord);
        glVertex3fv((p1-p2).ptr());
        glTexCoord2f(_s_coord, _t_coord);
        glVertex3fv((-p1-p2).ptr());
        glPopMatrix();
        break;

    case HEXAGON:
        glPushMatrix();
        glTranslatef(xpos.x(), xpos.y(), xpos.z());
        // we must glBegin() and glEnd() here, because each particle is a single fan
        glBegin(GL_TRIANGLE_FAN);
        glTexCoord2f(_s_coord + _s_tile * 0.5f, _t_coord + _t_tile * 0.5f);
        glVertex3f(0,0,0);
        glTexCoord2f(_s_coord + _s_tile * hex_texcoord_x1, _t_coord + _t_tile * hex_texcoord_y1);
        glVertex3fv((p1*cosPI3+p2*sinPI3).ptr());
        glTexCoord2f(_s_coord + _s_tile * hex_texcoord_x2, _t_coord + _t_tile * hex_texcoord_y1);
        glVertex3fv((-p1*cosPI3+p2*sinPI3).ptr());
        glTexCoord2f(_s_coord, _t_coord + _t_tile * 0.5f);
        glVertex3fv((-p1).ptr());
        glTexCoord2f(_s_coord + _s_tile * hex_texcoord_x2, _t_coord + _t_tile * hex_texcoord_y2);
        glVertex3fv((-p1*cosPI3-p2*sinPI3).ptr());
        glTexCoord2f(_s_coord + _s_tile * hex_texcoord_x1, _t_coord + _t_tile * hex_texcoord_y2);
        glVertex3fv((p1*cosPI3-p2*sinPI3).ptr());
        glTexCoord2f(_s_coord + _s_tile, _t_coord + _t_tile * 0.5f);
        glVertex3fv((p1).ptr());
        glTexCoord2f(_s_coord + _s_tile * hex_texcoord_x1, _t_coord + _t_tile * hex_texcoord_y1);
        glVertex3fv((p1*cosPI3+p2*sinPI3).ptr());
        glEnd();
        break;

    case LINE:
        {
            // Get the normalized direction of the particle, to be used in the 
            // calculation of one of the linesegment endpoints.
            float vl = _velocity.length();
            if (vl != 0) {
                osg::Vec3 v = _velocity * _current_size * scale / vl;

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

void osgParticle::Particle::setUpTexCoordsAsPartOfConnectedParticleSystem(ParticleSystem* ps)
{
    if (getPreviousParticle()!=Particle::INVALID_INDEX)
    {
        update(0.0);

        Particle* previousParticle = ps->getParticle(getPreviousParticle());
        const osg::Vec3& previousPosition = previousParticle->getPosition();
        const osg::Vec3& newPosition = getPosition();
        float distance = (newPosition-previousPosition).length();
        float s_coord_delta = 0.5f*distance/getCurrentSize();
        float s_coord = previousParticle->_s_coord + s_coord_delta;

        setTextureTile(1,1,0);
        _cur_tile = 0;
        _s_coord = s_coord;
        _t_coord = 0.0f;
    }
}
