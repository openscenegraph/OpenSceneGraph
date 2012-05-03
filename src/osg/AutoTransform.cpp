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
#include <osg/Notify>
#include <osg/io_utils>

using namespace osg;

AutoTransform::AutoTransform():
    _autoUpdateEyeMovementTolerance(0.0),
    _autoRotateMode(NO_ROTATION),
    _autoScaleToScreen(false),
    _scale(1.0,1.0,1.0),
    _firstTimeToInitEyePoint(true),
    _previousWidth(0),
    _previousHeight(0),
    _minimumScale(0.0),
    _maximumScale(DBL_MAX),
    _autoScaleTransitionWidthRatio(0.25),
    _matrixDirty(true),
    _axis(0.0f,0.0f,1.0f),
    _normal(0.0f,-1.0f,0.0f),
    _cachedMode(NO_ROTATION),
    _side(1.0f,0.0,0.0f)
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
    _previousWidth(0),
    _previousHeight(0),
    _minimumScale(pat._minimumScale),
    _maximumScale(pat._maximumScale),
    _autoScaleTransitionWidthRatio(pat._autoScaleTransitionWidthRatio),
    _matrixDirty(true),
    _axis(pat._axis),
    _normal(pat._normal),
    _cachedMode(pat._cachedMode),
    _side(pat._side)
{
//    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);
}

void AutoTransform::setAutoRotateMode(AutoRotateMode mode)
{
    _autoRotateMode = mode;
    _firstTimeToInitEyePoint = true;
    _cachedMode = CACHE_DIRTY;
    updateCache();
}

void AutoTransform::setAxis(const Vec3& axis)
{
    _axis = axis;
    _axis.normalize();
    updateCache();
}

void AutoTransform::setNormal(const Vec3& normal)
{
    _normal = normal;
    _normal.normalize();
    updateCache();
}

void AutoTransform::updateCache()
{
    if (_autoRotateMode==ROTATE_TO_AXIS)
    {
        if      (_axis==Vec3(1.0f,0.0,0.0f) && _normal==Vec3(0.0f,-1.0,0.0f)) _cachedMode = AXIAL_ROT_X_AXIS;
        else if (_axis==Vec3(0.0f,1.0,0.0f) && _normal==Vec3(1.0f, 0.0,0.0f)) _cachedMode = AXIAL_ROT_Y_AXIS;
        else if (_axis==Vec3(0.0f,0.0,1.0f) && _normal==Vec3(0.0f,-1.0,0.0f)) _cachedMode = AXIAL_ROT_Z_AXIS;
        else                                                                  _cachedMode = ROTATE_TO_AXIS;
    }
    else _cachedMode = _autoRotateMode;

    _side = _axis^_normal;
    _side.normalize();
}

void AutoTransform::setScale(const Vec3d& scale)
{
    _scale = scale;
    if (_scale.x()<_minimumScale) _scale.x() = _minimumScale;
    if (_scale.y()<_minimumScale) _scale.y() = _minimumScale;
    if (_scale.z()<_minimumScale) _scale.z() = _minimumScale;

    if (_scale.x()>_maximumScale) _scale.x() = _maximumScale;
    if (_scale.y()>_maximumScale) _scale.y() = _maximumScale;
    if (_scale.z()>_maximumScale) _scale.z() = _maximumScale;

    _matrixDirty=true;
    dirtyBound();
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
    if (_scale.x() == 0.0 || _scale.y() == 0.0 || _scale.z() == 0.0)
        return false;

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMultTranslate(-_position);
        matrix.postMultRotate(_rotation.inverse());
        matrix.postMultScale(Vec3d(1.0/_scale.x(), 1.0/_scale.y(), 1.0/_scale.z()));
        matrix.postMultTranslate(_pivotPoint);
    }
    else // absolute
    {
        matrix.makeRotate(_rotation.inverse());
        matrix.preMultTranslate(-_position);
        matrix.postMultScale(Vec3d(1.0/_scale.x(), 1.0/_scale.y(), 1.0/_scale.z()));
        matrix.postMultTranslate(_pivotPoint);
    }
    return true;
}

void AutoTransform::computeMatrix() const
{
    if (!_matrixDirty) return;

    _cachedMatrix.makeRotate(_rotation);
    _cachedMatrix.postMultTranslate(_position);
    _cachedMatrix.preMultScale(_scale);
    _cachedMatrix.preMultTranslate(-_pivotPoint);

    _matrixDirty = false;
}

void AutoTransform::accept(NodeVisitor& nv)
{
    if (nv.validNodeMask(*this))
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

                Viewport::value_type width = _previousWidth;
                Viewport::value_type height = _previousHeight;

                osg::Viewport* viewport = cs->getViewport();
                if (viewport)
                {
                    width = viewport->width();
                    height = viewport->height();
                }

                osg::Vec3d eyePoint = cs->getEyeLocal();
                osg::Vec3d localUp = cs->getUpLocal();
                osg::Vec3d position = getPosition();

                const osg::Matrix& projection = *(cs->getProjectionMatrix());

                bool doUpdate = _firstTimeToInitEyePoint;
                if (!_firstTimeToInitEyePoint)
                {
                    osg::Vec3d dv = _previousEyePoint-eyePoint;
                    if (dv.length2()>getAutoUpdateEyeMovementTolerance()*(eyePoint-getPosition()).length2())
                    {
                        doUpdate = true;
                    }
                    osg::Vec3d dupv = _previousLocalUp-localUp;
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
                        double size = 1.0/cs->pixelSize(getPosition(),0.48f);

                        if (_autoScaleTransitionWidthRatio>0.0)
                        {
                            if (_minimumScale>0.0)
                            {
                                double j = _minimumScale;
                                double i = (_maximumScale<DBL_MAX) ?
                                            _minimumScale+(_maximumScale-_minimumScale)*_autoScaleTransitionWidthRatio :
                                            _minimumScale*(1.0+_autoScaleTransitionWidthRatio);
                                double c = 1.0/(4.0*(i-j));
                                double b = 1.0 - 2.0*c*i;
                                double a = j + b*b / (4.0*c);
                                double k = -b / (2.0*c);

                                if (size<k) size = _minimumScale;
                                else if (size<i) size = a + b*size + c*(size*size);
                            }

                            if (_maximumScale<DBL_MAX)
                            {
                                double n = _maximumScale;
                                double m = (_minimumScale>0.0) ?
                                            _maximumScale+(_minimumScale-_maximumScale)*_autoScaleTransitionWidthRatio :
                                            _maximumScale*(1.0-_autoScaleTransitionWidthRatio);
                                double c = 1.0 / (4.0*(m-n));
                                double b = 1.0 - 2.0*c*m;
                                double a = n + b*b/(4.0*c);
                                double p = -b / (2.0*c);

                                if (size>p) size = _maximumScale;
                                else if (size>m) size = a + b*size + c*(size*size);
                            }
                        }

                        setScale(size);
                    }

                    if (_autoRotateMode==ROTATE_TO_SCREEN)
                    {
                        osg::Vec3d translation;
                        osg::Quat rotation;
                        osg::Vec3d scale;
                        osg::Quat so;

                        cs->getModelViewMatrix()->decompose( translation, rotation, scale, so );

                        setRotation(rotation.inverse());
                    }
                    else if (_autoRotateMode==ROTATE_TO_CAMERA)
                    {
                        osg::Vec3d PosToEye = _position - eyePoint;
                        osg::Matrix lookto = osg::Matrix::lookAt(
                            osg::Vec3d(0,0,0), PosToEye, localUp);
                        Quat q;
                        q.set(osg::Matrix::inverse(lookto));
                        setRotation(q);
                    }
                    else if (_autoRotateMode==ROTATE_TO_AXIS)
                    {
                        Matrix matrix;
                        Vec3 ev(eyePoint - _position);

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
                            case(ROTATE_TO_AXIS): // need to implement
                            {
                                float ev_side = ev*_side;
                                float ev_normal = ev*_normal;
                                float rotation = atan2f(ev_side,ev_normal);
                                matrix.makeRotate(rotation,_axis);
                                break;
                            }
                        }
                        Quat q;
                        q.set(matrix);
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
}

BoundingSphere AutoTransform::computeBound() const
{
    BoundingSphere bsphere;

    if ( getAutoScaleToScreen() && _firstTimeToInitEyePoint )
        return bsphere;

    bsphere = Transform::computeBound();

    return bsphere;
}
