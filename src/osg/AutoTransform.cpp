/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
    _autoRotateToScreen(false),
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
    _rotation(pat._rotation),
    _scale(pat._scale)
{
//    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

bool AutoTransform::computeLocalToWorldMatrix(Matrix& matrix,NodeVisitor*) const
{
    if (_matrixDirty) computeMatrix();
    
    if (_referenceFrame==RELATIVE_TO_PARENTS)
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
    if (_referenceFrame==RELATIVE_TO_PARENTS)
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

            bool doUpdate = _firstTimeToInitEyePoint;
            if (!_firstTimeToInitEyePoint)
            {
                osg::Vec3 dv = _previousEyePoint-eyePoint;
                if (dv.length2()>getAutoUpdateEyeMovementTolerance()*(eyePoint-getPosition()).length2())
                {
                    doUpdate = true;
                }
                else if (width!=_previousWidth || height!=_previousHeight)
                {
                    doUpdate = true;
                } 
            }
            _firstTimeToInitEyePoint = false;

            if (doUpdate)
            {            

                if (getAutoScaleToScreen())
                {
                    float size = 1.0f/cs->pixelSize(getPosition(),1.0f);
                    setScale(size);
                }

                if (getAutoRotateToScreen())
                {
                    osg::Quat rotation;
                    cs->getModelViewMatrix().get(rotation);            
                    setRotation(rotation.inverse());
                }

                _previousEyePoint = eyePoint;
                _previousWidth = width;
                _previousHeight = height;

                _matrixDirty = true;
            }

        }
    }
    
    // now do the proper accept
    Transform::accept(nv);
}
