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
#include <osg/View>
#include <osg/Notify>

using namespace osg;

// use this cull callback to allow the camera to traverse the View's children without
// actuall having them assigned as children to the camea itself.  This make the camera a
// decorator without ever directly being assigned to it. 
class ViewCameraTraverseNodeCallback : public osg::NodeCallback
{
public:

    ViewCameraTraverseNodeCallback(osg::View* view):_view(view) {}                                                       

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        _view->Group::traverse(*nv);
    }
    
    osg::View* _view;
};


View::View()
{
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
}

/** Copy constructor using CopyOp to manage deep vs shallow copy.*/
View::View(const View& view,const CopyOp& copyop):
    Transform(view,copyop),
    CullSettings(view),
    _projectionMatrix(view._projectionMatrix),
    _viewMatrix(view._viewMatrix)
{
    // need to clone the cameras.
    for(unsigned int i=0; i<view.getNumCameras(); ++i)
    {
        const CameraData& cd = view.getCameraData(i);
        addCamera(dynamic_cast<osg::CameraNode*>(cd._camera->clone(copyop)), cd._projectionOffset, cd._viewOffset);
    }
}


View::~View()
{
    // detatch the cameras from this View to prevent dangling pointers
    for(CameraList::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        CameraData& cd = *itr;
        cd._camera->setView(0);
        cd._camera->setCullCallback(0);
    }
}


Matrixd View::getInverseViewMatrix() const
{
    Matrixd inverse;
    inverse.invert(_viewMatrix);
    return inverse;
}

void View::setProjectionMatrixAsOrtho(double left, double right,
                                           double bottom, double top,
                                           double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::ortho(left, right,
                                           bottom, top,
                                           zNear, zFar));
}                                           

void View::setProjectionMatrixAsOrtho2D(double left, double right,
                                             double bottom, double top)
{
    setProjectionMatrix(osg::Matrixd::ortho2D(left, right,
                                             bottom, top));
}

void View::setProjectionMatrixAsFrustum(double left, double right,
                                             double bottom, double top,
                                             double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::frustum(left, right,
                                             bottom, top,
                                             zNear, zFar));
}

void View::setProjectionMatrixAsPerspective(double fovy,double aspectRatio,
                                                 double zNear, double zFar)
{
    setProjectionMatrix(osg::Matrixd::perspective(fovy,aspectRatio,
                                                 zNear, zFar));
}                                      

bool View::getProjectionMatrixAsOrtho(double& left, double& right,
                                           double& bottom, double& top,
                                           double& zNear, double& zFar)
{
    return _projectionMatrix.getOrtho(left, right,
                                       bottom, top,
                                       zNear, zFar);
}

bool View::getProjectionMatrixAsFrustum(double& left, double& right,
                                             double& bottom, double& top,
                                             double& zNear, double& zFar)
{
    return _projectionMatrix.getFrustum(left, right,
                                         bottom, top,
                                         zNear, zFar);
}                                  

bool View::getProjectionMatrixAsPerspective(double& fovy,double& aspectRatio,
                                                 double& zNear, double& zFar)
{
    return _projectionMatrix.getPerspective(fovy, aspectRatio, zNear, zFar);
}                                                 

void View::setViewMatrixAsLookAt(const Vec3& eye,const Vec3& center,const Vec3& up)
{
    setViewMatrix(osg::Matrixd::lookAt(eye,center,up));
}

void View::getViewMatrixAsLookAt(Vec3& eye,Vec3& center,Vec3& up,float lookDistance)
{
    _viewMatrix.getLookAt(eye,center,up,lookDistance);
}


bool View::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(_viewMatrix);
    }
    else // absolute
    {
        matrix = _viewMatrix;
    }
    return true;
}

bool View::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    const Matrixd& inverse = getInverseViewMatrix();

    if (_referenceFrame==RELATIVE_RF)
    {
        // note doing inverse so pre becomes post.
        matrix.postMult(inverse);
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}

void View::updateCameras()
{
    for(CameraList::iterator itr = _cameras.begin();
        itr != _cameras.end();
        ++itr)
    {
        CameraData& cd = *itr;
        cd._camera->setProjectionMatrix(cd._projectionOffset * _projectionMatrix);
        cd._camera->setViewMatrix(cd._viewOffset * _viewMatrix);
        cd._camera->inheritCullSettings(*this);
    }


}

bool View::addCamera(osg::CameraNode* camera, const osg::Matrix& projectionOffset, const osg::Matrix& viewOffset)
{
    if (!camera) return false;

    ViewCameraTraverseNodeCallback* cb = new ViewCameraTraverseNodeCallback(this);
    camera->setCullCallback(cb);
    camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    camera->setProjectionMatrix(projectionOffset * _projectionMatrix);
    camera->setViewMatrix(viewOffset * _viewMatrix);
    camera->inheritCullSettings(*this);
       
    _cameras.push_back(CameraData(camera, projectionOffset, viewOffset));

    // osg::notify(osg::NOTICE)<<"Added camera"<<std::endl;

    return true;
}

bool View::removeCamera(unsigned int pos)
{
    if (pos >= _cameras.size()) return false;

    _cameras[pos]._camera->setView(0);
    _cameras[pos]._camera->setCullCallback(0);
    
    _cameras.erase(_cameras.begin()+pos);
    
    osg::notify(osg::NOTICE)<<"Removed camera"<<std::endl;

    return true;
}


