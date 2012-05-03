/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <stdio.h>
#include <math.h>

#include <osg/Billboard>

using namespace osg;

#define square(x)   ((x)*(x))

Billboard::Billboard()
{
    _mode = AXIAL_ROT;
    _axis.set(0.0f,0.0f,1.0f);
    //_normal.set(0.0f,-1.0f,0.0f);
    setNormal(Vec3(0.0f,-1.0f,0.0f));
    updateCache();
}

Billboard::Billboard(const Billboard& billboard,const CopyOp& copyop):
        Geode(billboard,copyop),
        _mode(billboard._mode),
        _axis(billboard._axis),
        _normal(billboard._normal),
        _positionList(billboard._positionList),
        _cachedMode(billboard._cachedMode),
        _side(billboard._side)
{
    setNormal(_normal);
}

Billboard::~Billboard()
{
}

void Billboard::setMode(Mode mode)
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

    // Build rotation from normal to z-axis,
    // for use with POINT_ROT_EYE
    Vec3 z(0.0, 0.0, 1.0);
    Vec3 cp(z^_normal);
    float dot = z*_normal;
    float cp_len = cp.length();
    if (cp_len != 0.0f)
    {
        cp /= cp_len;
        float rotation_cp = acosf(dot);
        Matrix rotateNormalToZ;
        _rotateNormalToZAxis.makeRotate(-rotation_cp, cp);
    }
    else
        _rotateNormalToZAxis.makeIdentity();
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
    else if( _mode == POINT_ROT_WORLD )
    {
        if(_axis==Vec3(0.0f, 0.0, 1.0f) && _normal==Vec3(0.0f, -1.0f, 0.0f))  _cachedMode = POINT_ROT_WORLD_Z_AXIS;
        else _cachedMode = _mode;

    }
    else _cachedMode = _mode;

    _side = _axis^_normal;
    _side.normalize();
}

bool Billboard::addDrawable(Drawable *gset)
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


bool Billboard::addDrawable(Drawable *gset,const Vec3& pos)
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


bool Billboard::removeDrawable( Drawable *gset )
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
            dirtyBound();
            return true;
        }
    }
    return false;
}

bool Billboard::computeMatrix(Matrix& modelview, const Vec3& eye_local, const Vec3& pos_local) const
{
    //Vec3 up_local(matrix(0,1),matrix(1,1),matrix(2,1));

    Matrix matrix;

    Vec3 ev(eye_local-pos_local);
    switch(_cachedMode)
    {
        case(AXIAL_ROT_Z_AXIS):
        {

            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {
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
        case(AXIAL_ROT_Y_AXIS):
        {
            ev.y() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {
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
        case(AXIAL_ROT_X_AXIS):
        {
            ev.x() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f)
            {

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
        {
            float ev_side = ev*_side;
            float ev_normal = ev*_normal;
            float rotation = atan2f(ev_side,ev_normal);
            matrix.makeRotate(rotation,_axis);
            break;
        }
        case(POINT_ROT_WORLD):
        {
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
        case(POINT_ROT_EYE):
        {
            float ev_len = ev.length();
            if (ev_len != 0.0f)
            {
                ev /= ev_len;

                Vec3 up(modelview(0,1), modelview(1,1), modelview(2,1));
                Vec3 right(up^ev);
                right.normalize();
                up = ev^right;
                up.normalize();

                matrix(0,0) = right.x();
                matrix(0,1) = right.y();
                matrix(0,2) = right.z();
                matrix(1,0) = up.x();
                matrix(1,1) = up.y();
                matrix(1,2) = up.z();
                matrix(2,0) = ev.x();
                matrix(2,1) = ev.y();
                matrix(2,2) = ev.z();

                matrix.preMult(_rotateNormalToZAxis);
            }
            break;
        }
        case(POINT_ROT_WORLD_Z_AXIS):
        {
           // float rotation_about_z = atan2( -ev.y(), ev.x() );
           // float xy_distance = sqrt( ev.x()*ev.x() + ev.y()*ev.y() );
           // float rotation_from_xy = atan2( xy_distance, -ev.z() );

           Vec2   about_z( -ev.y(), ev.x() );
           if( about_z.normalize() == 0.0f ) about_z.x() = 1.0f;
           float  xy_distance = sqrt( ev.x()*ev.x() + ev.y()*ev.y() );
           Vec2   from_xy( xy_distance, -ev.z() );
           if( from_xy.normalize() == 0.0f ) from_xy.x() = 1.0f;

           matrix(0,0) =  about_z.x();
           matrix(0,1) =  about_z.y();
           matrix(1,0) = -about_z.y()*from_xy.x();
           matrix(1,1) =  about_z.x()*from_xy.x();
           matrix(1,2) =  from_xy.y();
           matrix(2,0) =  about_z.y()*from_xy.y();
           matrix(2,1) = -about_z.x()*from_xy.y();
           matrix(2,2) =  from_xy.x();

           break;
        }
    }

    matrix.setTrans(pos_local);

    modelview.preMult(matrix);

    return true;

}

BoundingSphere Billboard::computeBound() const
{
    int i;
    int ngsets = _drawables.size();

    if( ngsets == 0 ) return BoundingSphere();

    BoundingSphere bsphere;
    bsphere._center.set(0.0f,0.0f,0.0f);

    for( i = 0; i < ngsets; i++ )
    {
        const Drawable *gset = _drawables[i].get();
        const BoundingBox& bbox = gset->getBound();

        bsphere._center += bbox.center();
        bsphere._center += _positionList[i];
    }

    bsphere._center /= (float)(ngsets);

    float maxd = 0.0;
    for( i = 0; i < ngsets; ++i )
    {
        const Drawable *gset = _drawables[i].get();
        const BoundingBox& bbox = gset->getBound();
        Vec3 local_center = bsphere._center-_positionList[i];
        for(unsigned int c=0;c<8;++c)
        {
            float d = (bbox.corner(c)-local_center).length2();
            if( d > maxd ) maxd = d;
        }
    }
    bsphere._radius = sqrtf(maxd);

    return bsphere;
}
