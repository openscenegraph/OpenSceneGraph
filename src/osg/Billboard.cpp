#include <stdio.h>
#include <math.h>

#include <osg/Billboard>

using namespace osg;

#define square(x)   ((x)*(x))

Billboard::Billboard()
{
    _mode = AXIAL_ROT;
    _axis.set(0.0f,0.0f,1.0f);
    _normal.set(0.0f,-1.0f,0.0f);
    updateCache();
}

Billboard::Billboard(const Billboard& billboard,const CopyOp& copyop):
        Geode(billboard,copyop),
        _mode(billboard._mode),
        _axis(billboard._axis),
        _normal(billboard._normal),
        _positionList(billboard._positionList),
        _computeBillboardCallback(_computeBillboardCallback),
        _cachedMode(billboard._cachedMode),
        _side(billboard._side) {}

Billboard::~Billboard()
{
}

void Billboard::setMode(const Mode mode)
{
    _mode = mode;
    _cachedMode = CACHE_DIRTY;
    updateCache();
}

void Billboard::setAxis(const Vec3& axis)
{
    _axis = axis;
    _axis.normalize();
    updateCache();
}

void Billboard::setNormal(const Vec3& normal)
{
    _normal = normal;
    _normal.normalize();
    updateCache();
}

void Billboard::updateCache()
{
    if (_mode==AXIAL_ROT)
    {
        if      (_axis==Vec3(1.0f,0.0,0.0f) && _normal==Vec3(0.0f,-1.0,0.0f)) _cachedMode = AXIAL_ROT_X_AXIS;
        else if (_axis==Vec3(0.0f,1.0,0.0f) && _normal==Vec3(1.0f, 0.0,0.0f)) _cachedMode = AXIAL_ROT_Y_AXIS;
        else if (_axis==Vec3(0.0f,0.0,1.0f) && _normal==Vec3(0.0f,-1.0,0.0f)) _cachedMode = AXIAL_ROT_Z_AXIS;
        else                                                                  _cachedMode = AXIAL_ROT;
    }
    else _cachedMode = _mode;
    
    _side = _axis^_normal;
    _side.normalize();    
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

const bool Billboard::computeMatrix(Matrix& modelview, const Vec3& eye_local, const Vec3& pos_local) const
{
    //Vec3 up_local(matrix(0,1),matrix(1,1),matrix(2,1));

    Matrix matrix;

    Vec3 ev(eye_local-pos_local);
    switch(_cachedMode)
    {
        case(AXIAL_ROT_Z_AXIS): // need to implement 
        {

            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                matrix.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float s = ev.x()*inv;
                float c = -ev.y()*inv;
                matrix(0,0) = c;
                matrix(1,0) = -s;
                matrix(0,1) = s;
                matrix(1,1) = c;
            }
            break;
        }
        case(AXIAL_ROT_Y_AXIS): // need to implement 
        {
            ev.y() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                matrix.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float s = -ev.z()*inv;
                float c = ev.x()*inv;
                matrix(0,0) = c;
                matrix(2,0) = s;
                matrix(0,2) = -s;
                matrix(2,2) = c;
            }
            break;
        }
        case(AXIAL_ROT_X_AXIS): // implemented correctly..
        {
            ev.x() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

                matrix.makeIdentity();
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRotate(inRadians(rotation_z),0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float s = -ev.z()*inv;
                float c = -ev.y()*inv;
                matrix(1,1) = c;
                matrix(2,1) = -s;
                matrix(1,2) = s;
                matrix(2,2) = c;
            }
            break;
        }
        case(AXIAL_ROT): // need to implement 
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

                Vec3 cp(ev^_normal);
                float dot = ev*_normal;

                float cp_len = cp.length();
                if (cp_len != 0.0f)
                {
                    cp /= cp_len;

                    float rotation_cp = acosf(dot);
                    matrix.makeRotate(-inRadians(rotation_cp),cp[0],cp[1],cp[2]);
                }
            }
            break;
        }
    }

    matrix.setTrans(pos_local);

    modelview.preMult(matrix);

    return true;

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
