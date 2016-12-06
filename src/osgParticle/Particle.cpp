#include <osgParticle/Particle>
#include <osgParticle/LinearInterpolator>
#include <osgParticle/ParticleSystem>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Matrix>
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
    _alive(1.0f),
    _current_size(0.0f),
    _current_alpha(0.0f),
    _s_tile(1.0f),
    _t_tile(1.0f),
    _start_tile(0),
    _end_tile(0),
    _cur_tile(-1),
    _s_coord(0.0f),
    _t_coord(0.0f),
    _previousParticle(INVALID_INDEX),
    _nextParticle(INVALID_INDEX),
    _depth(0.0)
{
}

bool osgParticle::Particle::update(double dt, bool onlyTimeStamp)
{
    // this method should return false when the particle dies;
    // so, if we were instructed to die, do it now and return.
    if (_mustdie) {
        _alive = -1.0;
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
        _alive = -1.0;
        return false;
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

    // return now if we indicate that only time stamp should be updated
    // the shader will handle remain properties in this case
    if (onlyTimeStamp) return true;

    //Compute the current texture tile based on our normalized age
    int currentTile = _start_tile + static_cast<int>(x * getNumTiles());

    //If the current texture tile is different from previous, then compute new texture coords
    if(currentTile != _cur_tile)
    {

        _cur_tile = currentTile;
        _s_coord = _s_tile * fmod(_cur_tile , 1.0 / _s_tile);
        _t_coord = 1.0 - _t_tile * (static_cast<int>(_cur_tile * _t_tile) + 1);

        // OSG_NOTICE<<this<<" setting tex coords "<<_s_coord<<" "<<_t_coord<<std::endl;
    }

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

void osgParticle::Particle::setUpTexCoordsAsPartOfConnectedParticleSystem(ParticleSystem* ps)
{
    if (getPreviousParticle()!=Particle::INVALID_INDEX)
    {
        update(0.0, false);

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
