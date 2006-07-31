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
#include <osg/AutoTransform>
#include <osg/CullStack>

using namespace osg;

AutoTransform::AutoTransform():
    _autoUpdateEyeMovementTolerance(0.0f),
    _autoRotateMode(NO_ROTATION),
    _autoScaleToScreen(false),
    _scale(1.0f,1.0f,1.0f),
    _firstTimeToInitEyePoint(true),
    _matrixDirty(true)
{
//    setNumChildrenRequiringUpdateTraversal(1);
}

AutoTransform::AutoTransform(const AutoTransform& pat,const CopyOp& copyop):
    Transform(pat,copyop),
    _position(pat._position),
    _pivotPoint(pat._pivotPoint),
    _autoUpdateEyeMovementTolerance(pat._autoUpdateEyeMovementTolerance),
    _autoRotateMode(pat._autoRotateMode),
    _autoScaleToScreen(pat._autoScaleToScreen),
    _rotation(pat._rotation),
    _scale(pat._scale),
    _firstTimeToInitEyePoint(true),
    _matrixDirty(true)
{
//    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

bool AutoTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_matrixDirty) computeMatrix();
    
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(_cachedMatrix);
    }
    else // absolute
    {
        matrix = _cachedMatrix;
    }
    return true;
}


bool AutoTransform::computeWorldToLocalMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMult(osg::Matrix::translate(-_position)*
                        osg::Matrix::rotate(_rotation.inverse())*
                        osg::Matrix::scale(1.0f/_scale.x(),1.0f/_scale.y(),1.0f/_scale.z())*
                        osg::Matrix::translate(_pivotPoint));
    }
    else // absolute
    {
        matrix = osg::Matrix::translate(-_position)*
                 osg::Matrix::rotate(_rotation.inverse())*
                 osg::Matrix::scale(1.0f/_scale.x(),1.0f/_scale.y(),1.0f/_scale.z())*
                 osg::Matrix::translate(_pivotPoint);
    }
    return true;
}

void AutoTransform::computeMatrix() const
{
    if (!_matrixDirty) return;
    
    _cachedMatrix.set(osg::Matrix::translate(-_pivotPoint)*
                      osg::Matrix::scale(_scale)*
                      osg::Matrix::rotate(_rotation)*
                      osg::Matrix::translate(_position));
    
    _matrixDirty = false;
}

void AutoTransform::accept(NodeVisitor& nv)
{
    // if app traversal update the frame count.
    if (nv.getVisitorType()==NodeVisitor::UPDATE_VISITOR)
    {
    }
    else
    if (nv.getVisitorType()==NodeVisitor::CULL_VISITOR)
    {

        CullStack* cs = dynamic_cast<CullStack*>(&nv);
        if (cs)
        {

            int width = _previousWidth;
            int height = _previousHeight;

            osg::Viewport* viewport = cs->getViewport();
            if (viewport)
            {
                width = viewport->width();
                height = viewport->height();
            }

            osg::Vec3 eyePoint = cs->getEyeLocal(); 
            osg::Vec3 localUp = cs->getUpLocal(); 
            osg::Vec3 position = getPosition();

            const osg::Matrix& projection = cs->getProjectionMatrix();

            bool doUpdate = _firstTimeToInitEyePoint;
            if (!_firstTimeToInitEyePoint)
            {
                osg::Vec3 dv = _previousEyePoint-eyePoint;
                if (dv.length2()>getAutoUpdateEyeMovementTolerance()*(eyePoint-getPosition()).length2())
                {
                    doUpdate = true;
                }
                osg::Vec3 dupv = _previousLocalUp-localUp;
                // rotating the camera only affects ROTATE_TO_*
                if (_autoRotateMode &&
                    dupv.length2()>getAutoUpdateEyeMovementTolerance())
                {
                    doUpdate = true;
                }
                else if (width!=_previousWidth || height!=_previousHeight)
                {
                    doUpdate = true;
                }
                else if (projection != _previousProjection) 
                {
                    doUpdate = true;
                }                
                else if (position != _previousPosition) 
                { 
                    doUpdate = true; 
                } 
            }
            _firstTimeToInitEyePoint = false;

            if (doUpdate)
            {            

                if (getAutoScaleToScreen())
                {
                    float size = 1.0f/cs->pixelSize(getPosition(),0.48f);
                    setScale(size);
                }

                if (_autoRotateMode==ROTATE_TO_SCREEN)
                {
                    osg::Quat rotation = cs->getModelViewMatrix().getRotate();
                    setRotation(rotation.inverse());
                }
                else if (_autoRotateMode==ROTATE_TO_CAMERA)
                {
                    osg::Vec3 PosToEye = _position - eyePoint;
                    osg::Matrix lookto = osg::Matrix::lookAt(
                        osg::Vec3(0,0,0), PosToEye, localUp);
                    Quat q;
                    q.set(osg::Matrix::inverse(lookto));
                    setRotation(q);
                }

                _previousEyePoint = eyePoint;
                _previousLocalUp = localUp;
                _previousWidth = width;
                _previousHeight = height;
                _previousProjection = projection;
                _previousPosition = position;

                _matrixDirty = true;
            }

        }
    }
    
    // now do the proper accept
    Transform::accept(nv);
}

BoundingSphere AutoTransform::computeBound() const
{
    BoundingSphere bsphere;

    if ( getAutoScaleToScreen() && _firstTimeToInitEyePoint )
        return bsphere;

    bsphere = Transform::computeBound();

    return bsphere;
}
