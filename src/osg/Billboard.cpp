#include <stdio.h>
#include <math.h>
#include "osg/Billboard"
#include "osg/Input"
#include "osg/Output"
#include "osg/Registry"

using namespace osg;

RegisterObjectProxy<Billboard> g_BillboardProxy;

#define square(x)   ((x)*(x))

Billboard::Billboard()
{
    _mode = AXIAL_ROT;
//    _mode = POINT_ROT_WORLD;
    _axis.set(0.0f,0.0f,1.0f);
}


Billboard::~Billboard()
{
}

bool Billboard::addGeoSet(GeoSet *gset)
{
    if (Geode::addGeoSet(gset))
    {
        Vec3 pos(0.0f,0.0f,0.0f);
        while (_positionList.size()<_geosets.size())
        {
            _positionList.push_back(pos);
        }
        return true;
    }
    return false;
}

bool Billboard::addGeoSet(GeoSet *gset,const Vec3& pos)
{
    if (Geode::addGeoSet(gset))
    {
        while (_positionList.size()<_geosets.size())
        {
            _positionList.push_back(pos);
        }
        return true;
    }
    return false;
}

bool Billboard::removeGeoSet( GeoSet *gset )
{
    PositionList::iterator pitr = _positionList.begin();
    for (GeoSetList::iterator itr=_geosets.begin();
         itr!=_geosets.end();
         ++itr,++pitr)
    {
        if (itr->get()==gset)
        {
            // note ref_ptr<> automatically handles decrementing gset's reference count.
            _geosets.erase(itr);
            _positionList.erase(pitr);
            _bsphere_computed = false;
            return true;
        }
    }
    return false;    
}


void Billboard::calcRotation(const Vec3& eye_local, const Vec3& pos_local,Matrix& mat)
{
    switch(_mode)
    {
        case(AXIAL_ROT):
        {
            Vec3 ev = pos_local-eye_local;
            ev.z() = 0.0f;
            float ev_length = ev.length();
            if (ev_length>0.0f) {
                //float rotation_zrotation_z = atan2f(ev.x(),ev.y());
                //mat.makeRot(rotation_z*180.0f/M_PI,0.0f,0.0f,1.0f);
                float inv = 1.0f/ev_length;
                float c = ev.y()*inv;
                float s = ev.x()*inv;
                mat._mat[0][0] = c;
                mat._mat[0][1] = -s;
                mat._mat[1][0] = s;
                mat._mat[1][1] = c;
            }
            break;
        }
        case(POINT_ROT_WORLD): 
        case(POINT_ROT_EYE):
        {
            Vec3 ev = pos_local-eye_local;
            ev.normalize();

            float ev_len = ev.length();
            if (ev_len != 0.0f)
            {
                ev /= ev_len;

                Vec3 cp = ev^Vec3(0.0f,1.0f,0.0f);
                float dot = ev*Vec3(0.0f,1.0f,0.0f);

                float cp_len = cp.length();
                if (cp_len != 0.0f)
                {
                    cp /= cp_len;

                    float rotation_cp = acosf(dot);
                    mat.makeRot(rotation_cp*180.0f/M_PI,cp[0],cp[1],cp[2]);
                }
            }
            break;
        }
    }
}

void Billboard::calcTransform(const Vec3& eye_local, const Vec3& pos_local,Matrix& mat)
{
//    mat.makeTrans(pos_local[0],pos_local[1],pos_local[2]);
//    mat.makeIdent();
    calcRotation(eye_local,pos_local,mat);

//    mat.postTrans(pos_local[0],pos_local[1],pos_local[2]);
    mat._mat[3][0] += pos_local[0];
    mat._mat[3][1] += pos_local[1];
    mat._mat[3][2] += pos_local[2];

}

bool Billboard::readLocalData(Input& fr)
{
    // note, free done by Node::read(Input& fr)
    
    bool iteratorAdvanced = false;

    if (fr[0].matchWord("Mode"))
    {
        if (fr[1].matchWord("AXIAL_ROT"))
        {
            _mode = AXIAL_ROT;
            fr+=2;
            iteratorAdvanced = true;
        }
        else if  (fr[1].matchWord("POINT_ROT_EYE"))
        {
            _mode = POINT_ROT_EYE;
            fr+=2;
            iteratorAdvanced = true;
        }
        else if  (fr[1].matchWord("POINT_ROT_WORLD"))
        {
            _mode = POINT_ROT_WORLD;
            fr+=2;
            iteratorAdvanced = true;
        }
    }

    // read the position data.
    bool matchFirst = false;
    if ((matchFirst=fr.matchSequence("Positions {")) || fr.matchSequence("Positions %i {"))
    {

        // set up coordinates.
        int entry = fr[0].getNoNestedBrackets();

        if (matchFirst)
        {
            fr += 2;
        }
        else
        {
            //_positionList.(capacity);
            fr += 3;
        }

        Vec3 pos;
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            if (fr[0].getFloat(pos[0]) && fr[1].getFloat(pos[1]) && fr[2].getFloat(pos[2]))
            {
                fr += 3;
                _positionList.push_back(pos);
            }
            else
            {
                ++fr;
            }
        }

        iteratorAdvanced = true;
        ++fr;

    }
    
    if (Geode::readLocalData(fr)) iteratorAdvanced = true;

    return iteratorAdvanced;
}

bool Billboard::writeLocalData(Output& fw)
{
    
    switch(_mode)
    {
    case(AXIAL_ROT): fw.indent() << "Mode AXIAL_ROT"<<endl; break;
    case(POINT_ROT_EYE): fw.indent() << "Mode POINT_ROT_EYE"<<endl; break;
    case(POINT_ROT_WORLD): fw.indent() << "Mode POINT_ROT_WORLD"<<endl; break;
    }

    fw.indent() << "Axis " << _axis[0] << " "<<_axis[1]<<" "<<_axis[2]<<endl;
    
    fw.indent() << "Positions {"<<endl;
    fw.moveIn();
    for(PositionList::iterator piter = _positionList.begin();
                               piter != _positionList.end();
                               ++piter)
    {
        fw.indent() << (*piter)[0] << " "<<(*piter)[1]<<" "<<(*piter)[2]<<endl;
    }
    fw.moveOut();
    fw.indent() << "}"<<endl;
    
    Geode::writeLocalData(fw);
    
    return true;
}


bool Billboard::computeBound( void )
{
    int i;
    int ngsets = _geosets.size();

    if( ngsets == 0 ) return false;

    _bsphere._center.set(0.0f,0.0f,0.0f);

    for( i = 0; i < ngsets; i++ )
    {
        GeoSet *gset = _geosets[i].get();
        const BoundingBox& bbox = gset->getBound();

        _bsphere._center += bbox.center();
        _bsphere._center += _positionList[i];
    }

    _bsphere._center /= (float)(ngsets);

    float maxd = 0.0;
    for( i = 0; i < ngsets; ++i )
    {
        GeoSet *gset = _geosets[i].get();
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
