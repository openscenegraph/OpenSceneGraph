#include <stdio.h>
#include <math.h>

#include <osg/Billboard>

using namespace osg;

#define square(x)   ((x)*(x))

Billboard::Billboard()
{
    _mode = AXIAL_ROT;
    _axis.set(0.0f,0.0f,1.0f);
    setCachedMode();
}

Billboard::Billboard(const Billboard& billboard,const Cloner& cloner):
        Geode(billboard,cloner),
        _mode(billboard._mode),
        _axis(billboard._axis),
        _positionList(billboard._positionList),
        _cachedMode(billboard._cachedMode) {}

Billboard::~Billboard()
{
}

void Billboard::setMode(const Mode mode)
{
    _mode = mode;
    setCachedMode();
}

void Billboard::setAxis(const Vec3& axis)
{
    _axis = axis;
    setCachedMode();
}

void Billboard::setCachedMode()
{
    if (_mode==AXIAL_ROT)
    {
        if (_axis==osg::Vec3(1.0f,0.0,0.0f))      _cachedMode = AXIAL_ROT_X_AXIS;
        else if (_axis==osg::Vec3(0.0f,1.0,0.0f)) _cachedMode = AXIAL_ROT_Y_AXIS;
        else if (_axis==osg::Vec3(0.0f,0.0,1.0f)) _cachedMode = AXIAL_ROT_Z_AXIS;
        else                                      _cachedMode = AXIAL_ROT;
    }
    else _cachedMode = _mode;
}

const bool Billboard::addDrawable(Drawable *gset)
{
    if (Geode::addDrawable(gset))
    {
        Vec3 pos(0.0f,0.0f,0.0f);
        while (_positionList.size()<_drawables.size())
        {
            _positionList.push_back(pos);
        }
        return true;
    }
    return false;
}


const bool Billboard::addDrawable(Drawable *gset,const Vec3& pos)
{
    if (Geode::addDrawable(gset))
    {
        while (_positionList.size()<_drawables.size())
        {
            _positionList.push_back(pos);
        }
        return true;
    }
    return false;
}


const bool Billboard::removeDrawable( Drawable *gset )
{
    PositionList::iterator pitr = _positionList.begin();
    for (DrawableList::iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr,++pitr)
    {
        if (itr->get()==gset)
        {
            // note ref_ptr<> automatically handles decrementing gset's reference count.
            _drawables.erase(itr);
            _positionList.erase(pitr);
            _bsphere_computed = false;
            return true;
        }
    }
    return false;
}

void Billboard::calcTransform(const Vec3& eye_local, const Vec3& /*up_local*/, const Vec3& pos_local, Matrix& mat) const 
{
    Vec3 ev(pos_local-eye_local);
    switch(_cachedMode)
    {
        case(AXIAL_ROT_Z_AXIS):
        {
            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                mat.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float c = ev.y()*inv;
                float s = ev.x()*inv;
                mat(0,0) = c;
                mat(0,1) = -s;
                mat(1,0) = s;
                mat(1,1) = c;
            }
            break;
        }

        case(AXIAL_ROT_Y_AXIS):
        {
            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                mat.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float c = ev.y()*inv;
                float s = ev.x()*inv;
                mat(0,0) = c;
                mat(0,1) = -s;
                mat(1,0) = s;
                mat(1,1) = c;
            }
            break;
        }
        case(AXIAL_ROT_X_AXIS):
        {
            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                mat.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float c = ev.y()*inv;
                float s = ev.x()*inv;
                mat(0,0) = c;
                mat(0,1) = -s;
                mat(1,0) = s;
                mat(1,1) = c;
            }
            break;
        }
        
        case(AXIAL_ROT):
        {
            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                mat.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float c = ev.y()*inv;
                float s = ev.x()*inv;
                mat(0,0) = c;
                mat(0,1) = -s;
                mat(1,0) = s;
                mat(1,1) = c;
            }
            break;
        }

        case(POINT_ROT_WORLD):
        case(POINT_ROT_EYE):
        {
            // currently treat both POINT_ROT_WORLD and POINT_ROT_EYE the same,
            // this is only a temporary and 'incorrect' implementation, will
            // implement fully on second pass of Billboard.
            // Will also need up vector to calc POINT_ROT_EYE, so this will
            // have to be added to the above method paramters.
            // Robert Osfield, Jan 2001.

            float ev_len = ev.length();
            if (ev_len != 0.0f)
            {
                ev /= ev_len;

                Vec3 cp(ev^Vec3(0.0f,1.0f,0.0f));
                float dot = ev*Vec3(0.0f,1.0f,0.0f);

                float cp_len = cp.length();
                if (cp_len != 0.0f)
                {
                    cp /= cp_len;

                    float rotation_cp = acosf(dot);
                    mat.makeRotate(inRadians(rotation_cp),cp[0],cp[1],cp[2]);
                }
            }
            break;
        }
    }

    mat.setTrans(pos_local);

}

const bool Billboard::computeBound() const
{
    int i;
    int ngsets = _drawables.size();

    if( ngsets == 0 ) return false;

    _bsphere._center.set(0.0f,0.0f,0.0f);

    for( i = 0; i < ngsets; i++ )
    {
        const Drawable *gset = _drawables[i].get();
        const BoundingBox& bbox = gset->getBound();

        _bsphere._center += bbox.center();
        _bsphere._center += _positionList[i];
    }

    _bsphere._center /= (float)(ngsets);

    float maxd = 0.0;
    for( i = 0; i < ngsets; ++i )
    {
        const Drawable *gset = _drawables[i].get();
        const BoundingBox& bbox = gset->getBound();
        Vec3 local_center = _bsphere._center-_positionList[i];
        for(unsigned int c=0;c<8;++c)
        {
            float d = (bbox.corner(c)-local_center).length2();
            if( d > maxd ) maxd = d;
        }
    }
    _bsphere._radius = sqrtf(maxd);

    _bsphere_computed=true;

    return true;
}
