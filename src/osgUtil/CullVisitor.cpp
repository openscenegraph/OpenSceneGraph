#include <osg/Transform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/LightSource>
#include <osg/Notify>
#include <osg/TexEnv>
#include <osg/AlphaFunc>

#include <osg/LineSegment>

#include <osg/GeoSet>

#include <osgUtil/CullVisitor>
#include <osgUtil/RenderToTextureStage>

#include <osgDB/ReadFile>

#include <float.h>
#include <algorithm>

using namespace osg;
using namespace osgUtil;

inline float MAX_F(float a, float b)
    { return a>b?a:b; }
inline int EQUAL_F(float a, float b)
    { return a == b || fabsf(a-b) <= MAX_F(fabsf(a),fabsf(b))*1e-3f; }


class PrintVisitor : public NodeVisitor
{

   public:
   
        PrintVisitor():NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN)
        {
            _indent = 0;
            _step = 4;
        }
        
        inline void moveIn() { _indent += _step; }
        inline void moveOut() { _indent -= _step; }
        inline void writeIndent() 
        {
            for(int i=0;i<_indent;++i) std::cout << " ";
        }
                
        virtual void apply(Node& node)
        {
            moveIn();
            writeIndent(); std::cout << node.className() <<std::endl;
            traverse(node);
            moveOut();
        }

        virtual void apply(Geode& node)         { apply((Node&)node); }
        virtual void apply(Billboard& node)     { apply((Geode&)node); }
        virtual void apply(LightSource& node)   { apply((Node&)node); }
        
        virtual void apply(Group& node)         { apply((Node&)node); }
        virtual void apply(Transform& node)     { apply((Group&)node); }
        virtual void apply(Switch& node)        { apply((Group&)node); }
        virtual void apply(LOD& node)           { apply((Group&)node); }
        virtual void apply(Impostor& node)      { apply((LOD&)node); }

   protected:
    
        int _indent;
        int _step;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//    SandB change to this

struct TriangleViewFrustumIntersect
{
        //members .................

        //the clipping volume, so that triangle vertices can be shecked if inside
        osg::ClippingVolume _cv;

        //map serves not to have mulitple entries of same vertices
        std::map<osg::Vec3, bool> _listVectors;

        //transformation matrix
        const osg::Matrix* _t_mat;

        //value needed to set up triangles properly
        double _current_near;

        //eye point of camera
        osg::Vec3 _eye;

        osg::Vec3 _LeftUp;
        osg::Vec3 _LeftDown;
        osg::Vec3 _RightUp;
        osg::Vec3 _RightDown;

        //constructor
        TriangleViewFrustumIntersect(
            const osg::ClippingVolume& clip_vol, 
            const osg::Matrix* matr, 
            double current_near,
            const osg::Vec3& eyePoint,
            const osg::Vec3& LeftUp,
            const osg::Vec3& LeftDown,
            const osg::Vec3& RightUp,
            const osg::Vec3& RightDown)
        {
            _cv = clip_vol;
            _t_mat = matr;
            _current_near = current_near;
            _eye = eyePoint;
            _LeftUp = LeftUp;
            _LeftDown = LeftDown;
            _RightUp = RightUp;
            _RightDown = RightDown;
        }

        //pretty much the copy of IntersectVisitor intersect() function
        int intersect_linesegment_and_triangle(
            osg::Vec3& to_return,
            const osg::LineSegment& ls,
            const osg::Vec3& vertex1,
            const osg::Vec3& vertex2,
            const osg::Vec3& vertex3);
        
        void intersect_triangle(const osg::Vec3& vert1, const osg::Vec3& vert2, const osg::Vec3& vert3);

    //and crucial:
    void operator() (const osg::Vec3& vert1, const osg::Vec3& vert2, const osg::Vec3& vert3)
        {
            intersect_triangle(vert1, vert2, vert3);
        }
};


//SandB added: pretty much copy of the IntersectVisitor intersection of traingle function
int TriangleViewFrustumIntersect::intersect_linesegment_and_triangle(osg::Vec3& to_return,
        const osg::LineSegment& ls,
        const osg::Vec3& v1,
        const osg::Vec3& v2,
        const osg::Vec3& v3)
{

    if(v1 == v2 || v1 == v3 || v2 == v3) return -1;

    osg::Vec3 _s = ls.start();
    osg::Vec3 _d = ls.end() - ls.start();
    float _length = _d.length();
    _d /= _length;

    osg::Vec3 v12 = v2 - v1;
    osg::Vec3 n12 = v12^_d;

    float ds12 = (_s-v1)*n12;
    float d312 = (v3-v1)*n12;
    if (d312>=0.0f)
    {
        if (ds12<0.0f) return 3;
        if (ds12>d312) return 3;
    }
    else                     // d312 < 0
    {
        if (ds12>0.0f) return 3;
        if (ds12<d312) return 3;
    }

    osg::Vec3 v23 = v3-v2;
    osg::Vec3 n23 = v23^_d;
    float ds23 = (_s-v2)*n23;
    float d123 = (v1-v2)*n23;
    if (d123>=0.0f)
    {
        if (ds23<0.0f) return 3;
        if (ds23>d123) return 3;
    }
    else                     // d123 < 0
    {
        if (ds23>0.0f) return 3;
        if (ds23<d123) return 3;
    }

    osg::Vec3 v31 = v1-v3;
    osg::Vec3 n31 = v31^_d;
    float ds31 = (_s-v3)*n31;
    float d231 = (v2-v3)*n31;
    if (d231>=0.0f)
    {
        if (ds31<0.0f) return 3;
        if (ds31>d231) return 3;
    }
    else                     // d231 < 0
    {
        if (ds31>0.0f) return 3;
        if (ds31<d231) return 3;
    }

    float r3 = ds12/d312;
    float r1 = ds23/d123;
    float r2 = ds31/d231;

    to_return = v1*r1+v2*r2+v3*r3;

    float d = (to_return-_s)*_d;

    if (d<0.0f) return 1;
    if (d>_length) return 2;

    return 0;
}

void TriangleViewFrustumIntersect::intersect_triangle(const osg::Vec3& vert1, const osg::Vec3& vert2, const osg::Vec3& vert3)
{
    //if we have vertices in "transformed" coordinates, transform them to "global" coordinates
    osg::Vec3 v1, v2, v3;
    if(_t_mat)
    {
        v1 = vert1*(*_t_mat);
        v2 = vert2*(*_t_mat);
        v3 = vert3*(*_t_mat);
    }
    else
    {
        v1 = vert1;
        v2 = vert2;
        v3 = vert3;
    }

    //construct positions of truncated clipping volume corners
    osg::Vec3 UpLeft(_eye + _LeftUp * _current_near);
    osg::Vec3 DownLeft(_eye + _LeftDown*_current_near);
    osg::Vec3 UpRight(_eye + _RightUp*_current_near);
    osg::Vec3 DownRight(_eye + _RightDown*_current_near);

    //construct truncation "back plane"
    osg::Plane back_plane(DownLeft, DownRight, UpRight);//CCW, to have normal where it should be

    //add this plane to clipping volume
    _cv.add(back_plane);

    //check if all three triangle vertices are contained in truncated clipping volume
    unsigned int check = 0;

    //check if all three triangle vertices are behind truncation ("back") plane
    unsigned int check2 = 0;

    if(back_plane.distance(v1) <= 0.0)
            check2 |= 1;
    else if(_cv.contains(v1)) //can not be contained if behind
    {
        _listVectors[v1] = true;
        check |= 1;
    }

    if(back_plane.distance(v2)<=0.0)
            check2 |= 2;
    else if(_cv.contains(v2)) 
    {
        _listVectors[v2] = true;
        check |= 2;
    }

    if(back_plane.distance(v3) <= 0.0) 
        check2 |= 4;
    else if(_cv.contains(v3))
    {
        _listVectors[v3] = true;
        check |= 4;
    }

    if(check2 == 7) 
    {
        //all three traingle vertices are behind truncation plane so no need to check them
        //for heavily tesselated situation, htis is where most of tries will end
        return;
    }



    if(check != 7)
    {
        //just if it happens that all three are contained in truncated clipping volume, no need to do extra calculation
        //(and they already are added to candidate vertices))

        //at least one of the trianngle vertices is not contained in clipping volume, so extra checks are necessary

        //"working" variable
        osg::Vec3 returned;

        //construct line segment of two triangle vertices and check if they intersect any clipping plane
        //but within correct clipping plane triangle
        osg::ref_ptr<osg::LineSegment> s12 = new LineSegment(v1, v2);


        //left triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, DownLeft) == 0)
            _listVectors[returned] = true;

        //up triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, UpRight) == 0)
            _listVectors[returned] = true;

        //right triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpRight, DownRight) == 0)
            _listVectors[returned] = true;

        //bottom triangled
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, DownLeft, DownRight) == 0)
            _listVectors[returned] = true;

        //now for second edge of triangle
        s12->set(v2, v3);

            //left triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, DownLeft) == 0)
            _listVectors[returned] = true;

        //up triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, UpRight) == 0)
            _listVectors[returned] = true;

        //right triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpRight, DownRight) == 0)
            _listVectors[returned] = true;

        //bottom triangled
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, DownLeft, DownRight) == 0)
            _listVectors[returned] = true;

        s12->set(v3, v1);

        //left triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, DownLeft) == 0)
            _listVectors[returned] = true;

        //up triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpLeft, UpRight) == 0)
            _listVectors[returned] = true;

        //right triangle
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, UpRight, DownRight) == 0)
            _listVectors[returned] = true;

        //bottom triangled
        if(intersect_linesegment_and_triangle(returned, *s12, _eye, DownLeft, DownRight) == 0)
            _listVectors[returned] = true;


        //we still have possibility of camera being above huge triangle, so it is possible that clipping volume
        //intersects this triangle thus giving coordinates relevant for determination of near plane

        s12->set(_eye, UpLeft);

        if(intersect_linesegment_and_triangle(returned, *s12, v1, v2, v3) == 0)
            _listVectors[returned] = true;

        s12->set(_eye, DownLeft);

        if(intersect_linesegment_and_triangle(returned, *s12, v1, v2, v3) == 0)
            _listVectors[returned] = true;

        s12->set(_eye, UpRight);

        if(intersect_linesegment_and_triangle(returned, *s12, v1, v2, v3) == 0)
            _listVectors[returned] = true;

        s12->set(_eye, DownRight);

        if(intersect_linesegment_and_triangle(returned, *s12, v1, v2, v3) == 0)
            _listVectors[returned] = true;
    }
}

CullVisitor::CullVisitor()
{
    // overide the default node visitor mode.
    setTraversalMode(NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);

    _LODBias = 1.0f;


    // note all subsequent _cullingModeStack code assumes that there
    // is a least this one value on the stack, therefore they never
    // check whether the stack is empty. This shouldn't be a problem
    // unless there is bug somewhere...
    _cullingModeStack.push_back(CullViewState::ENABLE_ALL_CULLING);

    _tvs = new CullViewState;
    _tvs->_eyePoint.set(0.0f,0.0f,1.0f);
    _tvs->_centerPoint.set(0.0f,0.0f,0.0f);
    _tvs->_lookVector.set(0.0f,0.0f,-1.0f);
    _tvs->_upVector.set(0.0f,1.0f,0.0f);

    _cvs = _tvs;

    _tsm = LOOK_VECTOR_DISTANCE;
    _tsm = OBJECT_EYE_POINT_DISTANCE;


    _calculated_znear = FLT_MAX;
    _calculated_zfar = -FLT_MAX;
    
    _viewport = NULL;
    
    _impostorActive = true;
    _depthSortImpostorSprites = false;
    _impostorPixelErrorThreshold = 4.0f;
    _numFramesToKeepImpostorSprites = 10;
    _impostorSpriteManager = new ImpostorSpriteManager;

    //SandB change
    _detailedCulling = false;

}


CullVisitor::~CullVisitor()
{
    reset();
}


void CullVisitor::reset()
{

    //
    // first unref all referenced objects and then empty the containers.
    //
    _viewStateStack.clear();

    if (_cvs!=_tvs)
    {
        _cvs = _tvs;
    }

    // reset the calculated near far planes.
    _calculated_znear = FLT_MAX;
    _calculated_zfar = -FLT_MAX;
    
    
    // remove all accept the first element of the stack.
    _cullingModeStack.erase(_cullingModeStack.begin()+1,_cullingModeStack.end());
    
    // reset the resuse lists.
    _currentReuseMatrixIndex = 0;
    _currentReuseRenderLeafIndex = 0;
    
    for(RenderLeafList::iterator itr=_reuseRenderLeafList.begin();
        itr!=_reuseRenderLeafList.end();
        ++itr)
    {
        (*itr)->reset();
    }
    
}

void CullVisitor::setCamera(const Camera& camera)
{
    _camera = &camera;

    _tvs->_clippingVolume = camera.getClippingVolume();

    _tvs->_eyePoint = camera.getEyePoint_Model();

    _tvs->_centerPoint = camera.getCenterPoint_Model();

    _tvs->_lookVector = _tvs->_centerPoint-_tvs->_eyePoint;
    _tvs->_lookVector.normalize();

    _tvs->_upVector = camera.getUpVector_Model();

    _tvs->_bbCornerFar = (_tvs->_lookVector.x()>=0?1:0) |
                        (_tvs->_lookVector.y()>=0?2:0) |
                        (_tvs->_lookVector.z()>=0?4:0);

    _tvs->_bbCornerNear = (~_tvs->_bbCornerFar)&7;

}

void CullVisitor::pushCullViewState(Matrix* matrix)
{
    if (matrix)
    {
        osg::Matrix* inverse = new osg::Matrix;
        inverse->invert(*matrix);
        pushCullViewState(matrix,inverse);
    }
    else
        pushCullViewState(NULL,NULL);
}

void CullVisitor::pushCullViewState(Matrix* matrix,osg::Matrix* inverse)
{

    osg::ref_ptr<CullViewState> nvs = new CullViewState;

    Matrix* inverse_world = NULL;

    if (matrix)
    {    
        if (_cvs.valid() && _cvs->_matrix.valid())
        {
            matrix->postMult(*(_cvs->_matrix));
        }

        nvs->_matrix = matrix;
        
    }
    else
    {
        if (_cvs.valid())
        {
            nvs->_matrix = _cvs->_matrix;

        }
        else
        {
            nvs->_matrix = NULL;
        }
    }

    if (inverse)
    {    
        if (_cvs.valid() && _cvs->_inverse.valid())
        {
            inverse->preMult(*(_cvs->_inverse));
        }

        nvs->_inverse = inverse;
        
    }
    else
    {
        if (_cvs.valid())
        {
            nvs->_inverse = _cvs->_inverse;
        }
        else
        {
            nvs->_inverse = NULL;
        }
    }
    inverse_world = nvs->_inverse.get();
    
    if (inverse_world)
    {
        nvs->_eyePoint = _tvs->_eyePoint*(*inverse_world);
        //nvs->_eyePoint = inverse_world->getTrans();

        nvs->_centerPoint = _tvs->_centerPoint*(*inverse_world);

        nvs->_lookVector = nvs->_centerPoint - nvs->_eyePoint;
        nvs->_lookVector.normalize();

        Vec3 zero_transformed = Vec3(0.0f,0.0f,0.0f)*(*inverse_world);
        nvs->_upVector = (_tvs->_upVector)*(*inverse_world) - zero_transformed;
        nvs->_upVector.normalize();

        nvs->_clippingVolume = _tvs->_clippingVolume;
        nvs->_clippingVolume.transformProvidingInverse(*(nvs->_matrix));

    }
    else
    {
        nvs->_eyePoint = _tvs->_eyePoint;

        nvs->_lookVector = _tvs->_lookVector;

        nvs->_centerPoint = _tvs->_centerPoint;

        nvs->_upVector = _tvs->_upVector;

        nvs->_clippingVolume = _tvs->_clippingVolume;
    }


    nvs->_bbCornerFar = (nvs->_lookVector.x()>=0?1:0) |
                        (nvs->_lookVector.y()>=0?2:0) |
                        (nvs->_lookVector.z()>=0?4:0);

    nvs->_bbCornerNear = (~nvs->_bbCornerFar)&7;

    _cvs = nvs;

    _viewStateStack.push_back(nvs);
}

void CullVisitor::popCullViewState()
{
    // pop the top of the view stack and unref it.
    _viewStateStack.pop_back();

    // to new cvs and ref it.
    if (_viewStateStack.empty())
    {
        _cvs = _tvs;
    }
    else
    {
        _cvs = _viewStateStack.back().get();
    }

}

double CullVisitor::calculateZNear(const osg::Vec3& position, const osg::Vec3& eye, const osg::Vec3& look)
{
    //note: the candidate points are always in "global" coordinates
    return (position - eye)*look;
}

void CullVisitor::calcClippingDirections() const
{
    //need to calculate intersections of clipping planes
    osg::Vec3 t_up = _camera->getUpVector();
    osg::Vec3 t_side = _camera->getSideVector();

    double t_VFOV_2 = osg::DegreesToRadians(_camera->calc_fovy() * 0.5);//half of vertical FOV in radians

    //we need to pitch up the cameras up vector for angle that is half fovy, 
        osg::Vec3 pitched_up_up = t_up * osg::Matrix::rotate(t_VFOV_2, t_side.x(), t_side.y(), t_side.z());

    //we need also pitched down cameras up vector
        osg::Vec3 pitched_down_up = t_up * osg::Matrix::rotate(-t_VFOV_2, t_side.x(), t_side.y(), t_side.z());

    //we need either left and right or up and down planes of clipping volume (their normals better said)

    osg::Vec4 temp_plane = _cvs.get()->_clippingVolume.getPlaneList()[0].asVec4();//take left
    osg::Vec3 left(temp_plane.x(), temp_plane.y(), temp_plane.z());

    temp_plane = _cvs.get()->_clippingVolume.getPlaneList()[1].asVec4();//take right
    osg::Vec3 right(temp_plane.x(), temp_plane.y(), temp_plane.z());

        //now, the line from eye along intersecion of left and up clipping planes is cross product of properly pitched up "up" vector and left
        //clipping plane normal
            _LeftUp = pitched_up_up^left; _LeftUp.normalize();//upper left line of clipping volume
            _LeftDown = pitched_down_up^left; _LeftDown.normalize();//lower left line of clipping volume
            _RightUp = right^pitched_up_up; _RightUp.normalize();//upper right line of clipping volume
            _RightDown = right^pitched_down_up; _RightDown.normalize();//lower right line of clipping volume
}

void CullVisitor::updateCalculatedNearFar(osg::Drawable* pDrawable)
{
    //new philosophy, to have detailed checking

    //do all the same as non-detailed update near and far
    const BoundingBox& bb = pDrawable->getBound();

    const osg::Vec3& eyePoint = _tvs->_eyePoint; // note world eye point.
    const osg::Vec3& lookVector = _tvs->_lookVector; // world look vector.

    float d_near,d_far;

    if (_cvs->_matrix.valid())
    {

        const osg::Matrix& matrix = *(_cvs->_matrix);
        // calculate the offset from the eye in local coords then transform
        // the offset into world and then compare against the world look vector.
        d_near = ((bb.corner(_cvs->_bbCornerNear)*matrix) - eyePoint)*lookVector;
        d_far = ((bb.corner(_cvs->_bbCornerFar)*matrix) - eyePoint)*lookVector;

    }
    else
    {
        d_near = (bb.corner(_cvs->_bbCornerNear)-eyePoint)*lookVector;
        d_far = (bb.corner(_cvs->_bbCornerFar)-eyePoint)*lookVector;
    }

    //this is where difference arises: check if near is less than zero:

    if(d_near >= 0.0)
    {
        //this is the same as before (non detailed:
        if(d_near <= d_far)
        {
            if(d_near < _calculated_znear) _calculated_znear = d_near;
            if(d_far > _calculated_zfar) _calculated_zfar = d_far;
        }
        else
        {
            if ( !EQUAL_F(d_near, d_far) ) 
            {
                osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<<std::endl;
                osg::notify(osg::WARN)<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<<std::endl;
            }
            // note, need to reverse the d_near/d_far association because they are
            // the wrong way around...
            if (d_far<_calculated_znear) _calculated_znear = d_far;
            if (d_near>_calculated_zfar) _calculated_zfar = d_near;
        }
    }
    else if(d_far > 0.0)
    {
        //SandB change
        

        //we need to determine what has to be checked: everything that is actually behind current near clipping
        //plane needs not be rechecked

        double current_near = _camera->right()/_camera->zNear();//this is tan (HFOV/2)
        current_near = sqrt(1.0 + current_near*current_near);//his is 1 / cos(HFOV/2)
        
        if(_calculated_znear != FLT_MAX)//just in case this is the very first entry (i.e. the first bounding box contained eyePoint of camera
            current_near = _calculated_znear * current_near;//this is side of triangle ...
        else if(_calculated_zfar != -FLT_MAX) 
            current_near = _calculated_zfar * current_near;
        else current_near = 10000.0;//something must be put

        //construct functor: needs clipping volume, matrix, and current near, while some members for speed are kept in CullVisitor since
        //they need be calculated only once per frame
        TriangleViewFrustumIntersect ti(_cvs->_clippingVolume, 
            _cvs->_matrix.get(), current_near, eyePoint, 
            _LeftUp,_LeftDown,_RightUp,_RightDown);

        //this is ok, since the GeoSets are the ones we are really interested in here
        osg::GeoSet* p_gset = (osg::GeoSet*) pDrawable;
        for_each_triangle(*p_gset, ti);//that's it, all triangles of this geoset have been checked out

        //now, take the smallest positive near from candidate coordinates
        std::map<osg::Vec3, bool>::iterator it = ti._listVectors.begin();
        double calc_znear = 0.0;
        for(; it != ti._listVectors.end(); ++it)
        {
            calc_znear = calculateZNear(it->first, eyePoint, lookVector);//determine near produced by this coordinate
            if(calc_znear > 0.0 && calc_znear < _calculated_znear) _calculated_znear = calc_znear;//just to make sure , but should not be negative here
            //since we intersect triangles nad line segments
        }
    }    
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

inline float distance(const osg::Vec3& coord,const osg::Matrix& matrix)
{
    return -(coord[0]*matrix(0,2)+coord[1]*matrix(1,2)+coord[2]*matrix(2,2)+matrix(3,2));
}


void CullVisitor::updateCalculatedNearFar(const osg::BoundingBox& bb)
{

    if (!bb.isValid())
    {
        osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(..) passed a null bounding box."<< std::endl;
        return;
    }
    
    float d_near,d_far;
    if (_cvs->_matrix.valid())
    {
    
        const osg::Matrix& matrix = *(_cvs->_matrix);
        d_near = distance(bb.corner(_cvs->_bbCornerNear),matrix);
        d_far = distance(bb.corner(_cvs->_bbCornerFar),matrix);

    }
    else
    {
        d_near = -(bb.corner(_cvs->_bbCornerNear)).z();
        d_far = -(bb.corner(_cvs->_bbCornerFar)).z();
    }

    if (d_near<=d_far)
    {
        if (d_near<_calculated_znear) _calculated_znear = d_near;
        if (d_far>_calculated_zfar) _calculated_zfar = d_far;
    }
    else
    {
        if ( !EQUAL_F(d_near, d_far) ) 
        {
            osg::notify(osg::WARN)<<"Warning: CullVisitor::updateCalculatedNearFar(.) near>far in range calculation,"<< std::endl;
            osg::notify(osg::WARN)<<"         correcting by swapping values d_near="<<d_near<<" dfar="<<d_far<< std::endl;
        }
        // note, need to reverse the d_near/d_far association because they are
        // the wrong way around...
        if (d_far<_calculated_znear) _calculated_znear = d_far;
        if (d_near>_calculated_zfar) _calculated_zfar = d_near;
    }
}

void CullVisitor::updateCalculatedNearFar(const osg::Vec3& pos)
{
    const osg::Vec3& eyePoint = _cvs->_eyePoint; // note local eye point.
    const osg::Vec3& lookVector = _tvs->_lookVector; // world look vector.

    float d;

    if (_cvs->_matrix.valid())
    {
        const osg::Matrix& matrix = *(_cvs->_matrix);

        // calculate the offset from the eye in local coords then transform
        // the offset into world and then compare against the world look vector.
        d = osg::Matrix::transform3x3(pos-eyePoint,matrix)*lookVector;
    }
    else
    {
        d = (pos-eyePoint)*lookVector;
    }

    if (d<_calculated_znear) _calculated_znear = d;
    if (d>_calculated_zfar) _calculated_zfar = d;
}   

void CullVisitor::setCullingMode(CullViewState::CullingMode mode)
{
    _cullingModeStack.back()=mode;
}


CullViewState::CullingMode CullVisitor::getCullingMode() const
{
    return _cullingModeStack.back();
}

void CullVisitor::apply(Node& node)
{
    CullViewState::CullingMode mode = _cullingModeStack.back();
    
    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    // push the culling mode.
    _cullingModeStack.push_back(mode);
    
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    traverse(node);

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();
    
    // pop the culling mode.
    _cullingModeStack.pop_back();
}


void CullVisitor::apply(Geode& node)
{

    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    Matrix* matrix = getCurrentMatrix();
    for(int i=0;i<node.getNumDrawables();++i)
    {

        Drawable* drawable = node.getDrawable(i);
        const BoundingBox &bb =drawable->getBound();

        if (isCulled(bb,mode)) continue;


        //SandB change:      
//        updateCalculatedNearFar(bb);
        if(_detailedCulling)
        {
            updateCalculatedNearFar(drawable);
        }
        else
        {
            updateCalculatedNearFar(bb);
        }
        //end of SandB change

        // push the geoset's state on the geostate stack.    
        StateSet* stateset = drawable->getStateSet();
        
        bool isTransparent = stateset && stateset->getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        if (isTransparent)
        {

            Vec3 center;
            if (matrix)
            {
                center = (drawable->getBound().center())*(*matrix);
            }
            else
            {
                center = drawable->getBound().center();
            }
            Vec3 delta_center = center-_tvs->_eyePoint;

            float depth;
            switch(_tsm)
            {
                case(LOOK_VECTOR_DISTANCE):depth = _tvs->_lookVector*delta_center;break;
                case(OBJECT_EYE_POINT_DISTANCE):
                default: depth = delta_center.length2();break;
            }

            if (stateset) pushStateSet(stateset);
            addDrawableAndDepth(drawable,matrix,depth);
            if (stateset) popStateSet();

        }
        else
        {
            if (stateset) pushStateSet(stateset);
            addDrawable(drawable,matrix);
            if (stateset) popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(Billboard& node)
{
    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    const Vec3& eye_local = getEyeLocal();
    const Vec3& up_local = getUpLocal();
    Matrix* matrix = getCurrentMatrix();

    for(int i=0;i<node.getNumDrawables();++i)
    {
        const Vec3& pos = node.getPos(i);

        Drawable* drawable = node.getDrawable(i);
        // need to modify isCulled to handle the billboard offset.
        // if (isCulled(drawable->getBound())) continue;

        //SandB change:
        updateCalculatedNearFar(pos);
        /*
        if(_detailedCulling)
        {
            updateCalculatedNearFar(drawable);
        }
        else
        {
            updateCalculatedNearFar(pos);
        }
        //end of SandB change
        */

        Matrix* billboard_matrix = createOrReuseMatrix();
        node.getMatrix(*billboard_matrix,eye_local,up_local,pos);

        if (matrix)
        {
            billboard_matrix->postMult(*matrix);
        }

        StateSet* stateset = drawable->getStateSet();
        
        bool isTransparent = stateset && stateset->getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        if (isTransparent)
        {

            Vec3 center;
            if (matrix)
            {
                center = pos*(*matrix);
            }
            else
            {
                center = pos;
            }
            Vec3 delta_center = center-_tvs->_eyePoint;

            float depth;
            switch(_tsm)
            {
                case(LOOK_VECTOR_DISTANCE):depth = _tvs->_lookVector*delta_center;break;
                case(OBJECT_EYE_POINT_DISTANCE):
                default: depth = delta_center.length2();break;
            }

            if (stateset) pushStateSet(stateset);
            addDrawableAndDepth(drawable,billboard_matrix,depth);
            if (stateset) popStateSet();

        }
        else
        {
            if (stateset) pushStateSet(stateset);
            addDrawable(drawable,billboard_matrix);
            if (stateset) popStateSet();
        }

    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(LightSource& node)
{
    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    Matrix* matrix = getCurrentMatrix();
    Light* light = node.getLight();
    if (light)
    {
        addLight(light,matrix);
    }

    // pop the node's state off the geostate stack.    
    if (node_state) popStateSet();
}


void CullVisitor::apply(Group& node)
{
    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    // push the culling mode.
    _cullingModeStack.push_back(mode);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    traverse(node);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    _cullingModeStack.pop_back();
}

void CullVisitor::apply(Transform& node)
{
    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    // push the culling mode.
    _cullingModeStack.push_back(mode);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    bool isModelView = node.isModelViewTransform();
    if (isModelView)
    {

        ref_ptr<osg::Matrix> matrix = createOrReuseMatrix();
        ref_ptr<osg::Matrix> inverse = createOrReuseMatrix();
        node.getLocalToWorldMatrix(*matrix,this);
        node.getWorldToLocalMatrix(*inverse,this);
        pushCullViewState(matrix.get(),inverse.get());
    }
    else
    {
        osg::notify(osg::WARN)<<"Warning: Projection Transform in scene graph has been ignored, not fully implemented yet."<<std::endl;
    }
    
    traverse(node);

    if (isModelView)
    {
        popCullViewState();
    }

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    _cullingModeStack.pop_back();
}

void CullVisitor::apply(Switch& node)
{
    apply((Group&)node);
}


void CullVisitor::apply(LOD& node)
{
    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    int eval = node.evaluate(getEyeLocal(),_LODBias);
    if (eval<0) return;

    // push the culling mode.
    _cullingModeStack.push_back(mode);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    //notify(INFO) << "selecting child "<<eval<< std::endl;
    node.getChild(eval)->accept(*this);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    _cullingModeStack.pop_back();
}

void CullVisitor::apply(osg::EarthSky& node)
{
    // simply override the current earth sky.
    setEarthSky(&node);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);

    traverse(node);

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

}


void CullVisitor::apply(Impostor& node)
{
    const BoundingSphere& bs = node.getBound();

    // return if object's bounding sphere is culled.
    CullViewState::CullingMode mode = _cullingModeStack.back();

    if (!node.getCullingActive()) mode = 0;
    else if (node.getNumChildrenWithCullingDisabled()==0 && 
             isCulled(node.getBound(),mode)) return;

    osg::Vec3 eyeLocal = getEyeLocal();

    int eval = node.evaluate(eyeLocal,_LODBias);
    if (eval<0){
        return;
    }

    // push the culling mode.
    _cullingModeStack.push_back(mode);

    // push the node's state.
    StateSet* node_state = node.getStateSet();
    if (node_state) pushStateSet(node_state);


    float distance2 = (eyeLocal-bs.center()).length2();
    if (!_impostorActive ||
        distance2*_LODBias*_LODBias<node.getImpostorThreshold2() ||
        distance2<bs.radius2()*2.0f)
    {
        // outwith the impostor distance threshold therefore simple
        // traverse the appropriate child of the LOD.
        node.getChild(eval)->accept(*this);
    }
    else if (!_viewport.valid())
    {
        // need to use impostor but no valid viewport is defined to simply
        // default to using the LOD child as above.
        node.getChild(eval)->accept(*this);
    }
    else
    {    
        // within the impostor distance threshold therefore attempt
        // to use impostor instead.
        
        Matrix* matrix = getCurrentMatrix();

        // search for the best fit ImpostorSprite;
        ImpostorSprite* impostorSprite = node.findBestImpostorSprite(eyeLocal);
        
        if (impostorSprite)
        {
            // impostor found, now check to see if it is good enough to use
            float error = impostorSprite->calcPixelError(*_camera,*_viewport,matrix);
            
            if (error>_impostorPixelErrorThreshold)
            {
                // chosen impostor sprite pixel error is too great to use
                // from this eye point, therefore invalidate it.
                impostorSprite=NULL;
            }
        }
        

// need to think about sprite reuse and support for multiple context's.

        if (impostorSprite==NULL)
        {
            // no appropriate sprite has been found therefore need to create
            // one for use.
            
            // create the impostor sprite.
            impostorSprite = createImpostorSprite(node);

        }
        
        if (impostorSprite)
        {
            
            updateCalculatedNearFar(impostorSprite->getBound());

            StateSet* stateset = impostorSprite->getStateSet();
            
            if (stateset) pushStateSet(stateset);
            
            if (_depthSortImpostorSprites)
            {
                Vec3 center;
                if (matrix)
                {
                    center = node.getCenter()*(*matrix);
                }
                else
                {
                    center = node.getCenter();
                }
                Vec3 delta_center = center-_tvs->_eyePoint;

                float depth;
                switch(_tsm)
                {
                    case(LOOK_VECTOR_DISTANCE):depth = _tvs->_lookVector*delta_center;break;
                    case(OBJECT_EYE_POINT_DISTANCE):
                    default: depth = delta_center.length2();break;
                }

                addDrawableAndDepth(impostorSprite,matrix,depth);
            }
            else
            {
                addDrawable(impostorSprite,matrix);
            }

            if (stateset) popStateSet();
            
            // update frame number to show that impostor is in action.
            impostorSprite->setLastFrameUsed(getTraversalNumber());
            
        }
        else
        {
            // no impostor has been selected or created so default to 
            // traversing the usual LOD selected child.
            node.getChild(eval)->accept(*this);
        }
                
    }

    // pop the node's state off the render graph stack.    
    if (node_state) popStateSet();

    // pop the culling mode.
    _cullingModeStack.pop_back();
}

ImpostorSprite* CullVisitor::createImpostorSprite(Impostor& node)
{
    if (!_camera.valid()) return NULL;

    bool isPerspectiveCamera = _camera->getProjectionType()==Camera::FRUSTUM ||
                               _camera->getProjectionType()==Camera::PERSPECTIVE;

    Matrix* matrix = getCurrentMatrix();
    const BoundingSphere& bs = node.getBound();
    osg::Vec3 eye_local = getEyeLocal();
    int eval = node.evaluate(eye_local,_LODBias);

    if (!bs.isValid())
    {
        return NULL;
    }


    // no appropriate sprite has been found therefore need to create
    // one for use.

    // create the render to texture stage.
    ref_ptr<RenderToTextureStage> rtts = new RenderToTextureStage;


    // set up the camera, note, in world coordinates, as if
    // at the root of the scene graph as in the cullvisitor's original.
    // this is required since we are simply inheriting all the
    // transforms above this impostor node, rather than creating
    // a local version and starting with a clean slate. We could
    // do this, but then lights and clipping planes would need to
    // transformed accordingly.  This way we avoid this.
    Vec3 eye_world = _tvs->_eyePoint;

    Vec3 center_world = bs.center();
    if (matrix) center_world = center_world*(*matrix);

    Vec3 lv_world = center_world-eye_world;

    Vec3 side_world = lv_world ^ _tvs->_upVector;
    Vec3 up_world = side_world ^ lv_world;
    up_world.normalize();

    ref_ptr<Camera> camera = new Camera;
    camera->setLookAt(eye_world,center_world,up_world);

    rtts->setCamera(camera.get());


    // set up lighting.
    // currently ignore lights in the scene graph itself..
    // will do later.
    RenderStage* previous_stage = _currentRenderBin->_stage;

    // set up the background color and clear mask.
    osg::Vec4 clear_color = previous_stage->getClearColor();
    clear_color[3] = 0.0f; // set the alpha to zero.
    rtts->setClearColor(clear_color);
    rtts->setClearMask(previous_stage->getClearMask());
    
    // set up to charge the same RenderStageLighting is the parent previous stage.
    rtts->setRenderStageLighting(previous_stage->getRenderStageLighting());

    // record the render bin, to be restored after creation
    // of the render to text
    RenderBin* previousRenderBin = _currentRenderBin;

    // set the current renderbin to be the newly created stage.
    _currentRenderBin = rtts.get();

    // what shall we do about the cull view stack?
    // need to craete a new _tvs and save the previous one.
    // need to create a push a new _cvs to set up for new camera position.

    ref_ptr<CullViewState> previous_tvs = _tvs;

    _tvs = new CullViewState;

    // store the previous camera setting

    ref_ptr<const Camera> previous_camera = _camera;

    // sets up the _tvs to reflect the new camera.
    setCamera(*camera);

    // pushing the cull view state will update it so it takes
    // into account the new camera orientation.
    pushCullViewState();

    // what shall we do about the near far?
    // we could need to save the near and far, or switch it off.
    // simplicist to save near and far.  will do this for now.

    float previous_znear = _calculated_znear;
    float previous_zfar = _calculated_zfar;

    _calculated_znear = FLT_MAX;
    _calculated_zfar = -FLT_MAX;

    ref_ptr<StateSet> dummyState = new StateSet;
    
    
//    dummyState->setMode(GL_BLEND,osg::StateAttribute::OVERRIDE_OFF);
    
    pushStateSet(dummyState.get());


    // switch off the view frustum culling, since we will have
    // the whole subgraph in view.
    _cullingModeStack.push_back((_cullingModeStack.back() & ~CullViewState::VIEW_FRUSTUM_CULLING));

    {

        // traversing the usual LOD selected child.
        node.getChild(eval)->accept(*this);

    }

    popStateSet();

    // restore the culling mode.
    _cullingModeStack.pop_back();

    float local_znear = _calculated_znear;
    float local_zfar = _calculated_zfar;

    // restore the previous near and far.            
    _calculated_znear = previous_znear;
    _calculated_zfar = previous_zfar;

    // restor the previous renderbin.
    _currentRenderBin = previousRenderBin;

    // restore the previous _tvs and _cvs;
    _tvs = previous_tvs;
    popCullViewState();


    // restore the previous camera.
    _camera = previous_camera;


    if (rtts->_renderGraphList.size()==0 && rtts->_bins.size()==0)
    {
        // getting to this point means that all the subgraph has been
        // culled by small feature culling or is beyond LOD ranges.
        return NULL;
    }

    if (local_znear>local_zfar)
    {
        notify(WARN) << "Warning : problem with osg::CullVisitor::creatImpostorSprite() local_znear ("<<local_znear<<") "<<" > ("<<local_zfar<<") local_zfar"<< std::endl;
        return NULL;        
    }


// create texture quad coords (in local coords)

    Vec3 center_local = bs.center();
    Vec3 camera_up_local = _cvs->_upVector;
    Vec3 lv_local = center_local-eye_local;

    float distance_local = lv_local.length();
    lv_local /= distance_local;
   
    Vec3 sv_local = lv_local^camera_up_local;
    sv_local.normalize();
    
    Vec3 up_local = sv_local^lv_local;
    

    
    float width = bs.radius();
    if (isPerspectiveCamera)
    {
        // expand the width to account for projection onto sprite.
        width *= (distance_local/sqrtf(distance_local*distance_local-bs.radius2()));
    }
    
    // scale up and side vectors to sprite width.
    up_local *= width;
    sv_local *= width;
    
    // create the corners of the sprite.
    Vec3 c00(center_local - sv_local - up_local);
    Vec3 c10(center_local + sv_local - up_local);
    Vec3 c01(center_local - sv_local + up_local);
    Vec3 c11(center_local + sv_local + up_local);
    
// adjust camera left,right,up,down to fit (in world coords)

#define USE_SPHERE_NEAR_FAR    

#ifdef USE_SPHERE_NEAR_FAR    
    Vec3 near_local  ( center_local-lv_local*width );
    Vec3 far_local   ( center_local+lv_local*width );
#endif
    Vec3 top_local   ( center_local+up_local);
    Vec3 right_local ( center_local+sv_local);
    
#ifdef USE_SPHERE_NEAR_FAR    
    Vec3 near_world;
    Vec3 far_world;
#endif
    Vec3 top_world;
    Vec3 right_world;
    
    if (matrix)
    {
#ifdef USE_SPHERE_NEAR_FAR    
        near_world  = near_local * (*matrix);
        far_world   = far_local * (*matrix);
#endif
        top_world   = top_local * (*matrix);
        right_world = right_local * (*matrix);
    }
    else
    {
#ifdef USE_SPHERE_NEAR_FAR    
        near_world  = near_local;
        far_world   = far_local;
#endif        
        top_world   = top_local;
        right_world = right_local;
    }
    
#ifdef USE_SPHERE_NEAR_FAR
    float znear = (near_world-eye_world).length();
    float zfar  = (far_world-eye_world).length();
#else    
    float znear = local_znear;
    float zfar = local_zfar;
#endif

    if (local_zfar>=local_znear)
    {
        znear = local_znear;
        zfar = local_zfar;
    }
    
    

    
    float top   = (top_world-center_world).length();
    float right = (right_world-center_world).length();

    znear *= 0.9f;
    zfar *= 1.1f;

    if (isPerspectiveCamera)
    {
        // deal with projection issue move the top and right points
        // onto the near plane.
    float ratio = znear/(center_world-eye_world).length();
        top *= ratio;
        right *= ratio;
        camera->setFrustum(-right,right,-top,top,znear,zfar);
    
    }
    else
    {
        // othographic projection.

        camera->setOrtho(-right,right,-top,top,znear,zfar);
    }
    
    if (local_znear<znear)
    {
        znear = local_znear;
    }
    
    if (local_zfar>zfar)
    {
        zfar = local_zfar;
    }

    // restore the previous near and far.            
    local_znear = previous_znear;
    local_zfar = previous_zfar;


// calc texture size for eye, bs.

    Vec3 c00_world;
    Vec3 c11_world;
    
    if (matrix)
    {    
        c00_world = c00 * (*matrix);
        c11_world = c11 * (*matrix);
    }
    else
    {
        c00_world = c00;
        c11_world = c11;
    }
    

    // convert the corners of the sprite (in world coords) into their
    // equivilant window coordinates by using the camera's project method.
    Vec3 c00_win;
    Vec3 c11_win;
    _camera->project(c00_world,*_viewport,c00_win);
    _camera->project(c11_world,*_viewport,c11_win);    


// adjust texture size to be nearest power of 2.

    float s  = c11_win.x()-c00_win.x();
    float t  = c11_win.y()-c00_win.y();

    // may need to reverse sign of width or height if a matrix has
    // been applied which flips the orientation of this subgraph.
    if (s<0.0f) s = -s;
    if (t<0.0f) t = -t;

    // bias value used to assist the rounding up or down of
    // the texture dimensions to the nearest power of two.
    // bias near 0.0 will almost always round down.
    // bias near 1.0 will almost always round up. 
    float bias = 0.7f;

    float sp2 = logf((float)s)/logf(2.0f);
    float rounded_sp2 = floorf(sp2+bias);
    int new_s = (int)(powf(2.0f,rounded_sp2));

    float tp2 = logf((float)t)/logf(2.0f);
    float rounded_tp2 = floorf(tp2+bias);
    int new_t = (int)(powf(2.0f,rounded_tp2));

    // if dimension is bigger than window divide it down.    
    while (new_s>_viewport->width()) new_s /= 2;

    // if dimension is bigger than window divide it down.    
    while (new_t>_viewport->height()) new_t /= 2;


    // offset the impostor viewport from the center of the main window
    // viewport as often the edges of the viewport might be obscured by
    // other windows, which can cause image/reading writing problems.
    int center_x = _viewport->x()+_viewport->width()/2;
    int center_y = _viewport->y()+_viewport->height()/2;

    Viewport* viewport = new Viewport;
    viewport->setViewport(center_x-new_s/2,center_y-new_t/2,new_s,new_t);
    rtts->setViewport(viewport);

// create the impostor sprite.

    ImpostorSprite* impostorSprite = 
        _impostorSpriteManager->createOrReuseImpostorSprite(new_s,new_t,getTraversalNumber()-_numFramesToKeepImpostorSprites);

    if (impostorSprite==NULL) return NULL;

    // have successfully created an impostor sprite so now need to
    // add it into the impostor.
    node.addImpostorSprite(impostorSprite);

    if (_depthSortImpostorSprites)
    {
        // the depth sort bin should probably be user definable,
        // will look into this later. RO July 2001.
        StateSet* stateset = impostorSprite->getStateSet();
        stateset->setRenderBinDetails(1,"DepthSortedBin");
    }
    
    Texture* texture = impostorSprite->getTexture();

    // update frame number to show that impostor is in action.
    impostorSprite->setLastFrameUsed(getTraversalNumber());

    Vec3* coords = impostorSprite->getCoords();
    Vec2* texcoords = impostorSprite->getTexCoords();
    
    coords[0] = c01;
    texcoords[0].set(0.0f,1.0f);

    coords[1] = c00;
    texcoords[1].set(0.0f,0.0f);

    coords[2] = c10;
    texcoords[2].set(1.0f,0.0f);

    coords[3] = c11;
    texcoords[3].set(1.0f,1.0f);
    
    impostorSprite->dirtyBound();
    
    Vec3* controlcoords = impostorSprite->getControlCoords();
    
    if (isPerspectiveCamera)
    {
        // deal with projection issue by moving the coorners of the quad
        // towards the eye point.
        float ratio = width/(center_local-eye_local).length();
        float one_minus_ratio = 1.0f-ratio;
        Vec3 eye_local_ratio = eye_local*ratio;
        
        controlcoords[0] = coords[0]*one_minus_ratio + eye_local_ratio;
        controlcoords[1] = coords[1]*one_minus_ratio + eye_local_ratio;
        controlcoords[2] = coords[2]*one_minus_ratio + eye_local_ratio;
        controlcoords[3] = coords[3]*one_minus_ratio + eye_local_ratio;
    }
    else
    {
        // project the control points forward towards the eyepoint,
        // but since this an othographics projection this projection is
        // parallel.
        Vec3 dv = lv_local*width;

        controlcoords[0] = coords[0]-dv;
        controlcoords[1] = coords[1]-dv;
        controlcoords[2] = coords[2]-dv;
        controlcoords[3] = coords[3]-dv;
    }

    impostorSprite->setStoredLocalEyePoint(eye_local);


    // and the render to texture stage to the current stages
    // dependancy list.
    _currentRenderBin->_stage->addToDependencyList(rtts.get());

    // attach texture to the RenderToTextureStage.
    rtts->setTexture(texture);

    // must sort the RenderToTextureStage so that all leaves are
    // accounted correctly in all renderbins i.e depth sorted bins.    
    rtts->sort();

    return impostorSprite;

}
