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
#include <osg/ClusterCullingCallback>
#include <osg/TriangleFunctor>
#include <osg/CullSettings>

using namespace osg;


///////////////////////////////////////////////////////////////////////////////////////////
//
//  Cluster culling callback
//

ClusterCullingCallback::ClusterCullingCallback():
    _radius(-1.0f),
    _deviation(-1.0f)
{
}

ClusterCullingCallback::ClusterCullingCallback(const ClusterCullingCallback& ccc,const CopyOp& copyop):
    Drawable::CullCallback(ccc,copyop),
    _controlPoint(ccc._controlPoint),_normal(ccc._normal),_deviation(ccc._deviation)
{
}

ClusterCullingCallback::ClusterCullingCallback(const osg::Vec3& controlPoint, const osg::Vec3& normal, float deviation):
    _controlPoint(controlPoint),_normal(normal), _deviation(deviation)
{
}

ClusterCullingCallback::ClusterCullingCallback(const osg::Drawable* drawable)
{
    computeFrom(drawable);
}

struct ComputeAveragesFunctor
{

    ComputeAveragesFunctor():
        _num(0) {}

    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool)
    {
        // calc orientation of triangle.
        osg::Vec3d normal = (v2-v1)^(v3-v1);
        if (normal.normalize()!=0.0f)
        {
            _normal += normal;
        }        
        _center += v1;
        _center += v2;
        _center += v3;
                
        ++_num;

    }
    
    osg::Vec3 center() { return _center /  (double)(3*_num); }
    osg::Vec3 normal() { _normal.normalize(); return _normal; }
    
    unsigned int _num;
    Vec3d _center;
    Vec3d _normal;
};

struct ComputeDeviationFunctor
{

    ComputeDeviationFunctor():
        _deviation(1.0),
        _radius2(0.0) {}
        
    void set(const osg::Vec3& center,const osg::Vec3& normal)
    {
        _center = center;
        _normal = normal;
    }

    inline void operator() ( const osg::Vec3 &v1, const osg::Vec3 &v2, const osg::Vec3 &v3, bool)
    {
        // calc orientation of triangle.
        osg::Vec3 normal = (v2-v1)^(v3-v1);
        if (normal.normalize()!=0.0f)
        {
            _deviation = osg::minimum(_normal*normal,_deviation);
        }
        _radius2 = osg::maximum((v1-_center).length2(),_radius2);
        _radius2 = osg::maximum((v2-_center).length2(),_radius2);
        _radius2 = osg::maximum((v3-_center).length2(),_radius2);

    }
    osg::Vec3 _center;
    osg::Vec3 _normal;
    float _deviation;
    float _radius2;
};


void ClusterCullingCallback::computeFrom(const osg::Drawable* drawable)
{
    TriangleFunctor<ComputeAveragesFunctor> caf;
    drawable->accept(caf);
    
    _controlPoint = caf.center();
    _normal = caf.normal();
    
    TriangleFunctor<ComputeDeviationFunctor> cdf;
    cdf.set(_controlPoint,_normal);
    drawable->accept(cdf);
    
//    osg::notify(osg::NOTICE)<<"ClusterCullingCallback::computeFrom() _controlPoint="<<_controlPoint<<std::endl;
//    osg::notify(osg::NOTICE)<<"                                      _normal="<<_normal<<std::endl;
//    osg::notify(osg::NOTICE)<<"                                      cdf._deviation="<<cdf._deviation<<std::endl;
    

    if (_normal.length2()==0.0) _deviation = -1.0f;
    else 
    {
        float angle = acosf(cdf._deviation)+osg::PI*0.5f;
        if (angle<osg::PI) _deviation = cosf(angle);
        else _deviation = -1.0f;
    }
    
    _radius = sqrtf(cdf._radius2);
}

void ClusterCullingCallback::set(const osg::Vec3& controlPoint, const osg::Vec3& normal, float deviation, float radius)
{
    _controlPoint = controlPoint;
    _normal = normal;
    _deviation = deviation;
    _radius = radius;
}

void ClusterCullingCallback::transform(const osg::Matrixd& matrix)
{
    _controlPoint = Vec3d(_controlPoint)*matrix;
    _normal = Matrixd::transform3x3(Matrixd::inverse(matrix),Vec3d(_normal));
    _normal.normalize();
}


bool ClusterCullingCallback::cull(osg::NodeVisitor* nv, osg::Drawable* , osg::State*) const
{
    CullSettings* cs = dynamic_cast<CullSettings*>(nv);
    if (cs && !(cs->getCullingMode() & CullSettings::CLUSTER_CULLING))
    {
        // cluster culling switched off cull settings.
        return false;
    }

    if (_deviation<=-1.0f)
    {
        // cluster culling switch off by deviation.
        return false;
    }
    
    osg::Vec3 eye_cp = nv->getEyePoint() - _controlPoint;
    float radius = eye_cp.length();
    
    if (radius<_radius)
    {
        return false;
    }
    
    
    float deviation = (eye_cp * _normal)/radius;

//    osg::notify(osg::NOTICE)<<"ClusterCullingCallback::cull() _normal="<<_normal<<" _controlPointtest="<<_controlPoint<<" eye_cp="<<eye_cp<<std::endl;
//    osg::notify(osg::NOTICE)<<"                             deviation="<<deviation<<" _deviation="<<_deviation<<" test="<<(deviation < _deviation)<<std::endl;

    return deviation < _deviation;
}


void ClusterCullingCallback::operator()(Node* node, NodeVisitor* nv)
{
    if (nv)
    {
        if (cull(nv,0,0)) return;
        
        traverse(node,nv); 
    }
}
