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

#ifndef OSG_CULLSTACK
#define OSG_CULLSTACK 1

#include <osg/CullingSet>
#include <osg/CullSettings>
#include <osg/Viewport>
#include <osg/fast_back_stack>
#include <osg/Transform>

namespace osg {

/** A CullStack class which accumulates the current project, modelview matrices
and the CullingSet. */
class OSG_EXPORT CullStack : public osg::CullSettings
{

    public:


        CullStack();
        CullStack(const CullStack& cs);

        ~CullStack();

        typedef std::vector<ShadowVolumeOccluder>   OccluderList;

        void reset();

        void pushCullingSet();
        void popCullingSet();

        void setOccluderList(const ShadowVolumeOccluderList& svol) { _occluderList = svol; }
        ShadowVolumeOccluderList& getOccluderList() { return _occluderList; }
        const ShadowVolumeOccluderList& getOccluderList() const { return _occluderList; }

        void pushViewport(osg::Viewport* viewport);
        void popViewport();

        void pushProjectionMatrix(osg::RefMatrix* matrix);
        void popProjectionMatrix();

        void pushModelViewMatrix(osg::RefMatrix* matrix, Transform::ReferenceFrame referenceFrame);
        void popModelViewMatrix();

        inline float getFrustumVolume() { if (_frustumVolume<0.0f) computeFrustumVolume(); return _frustumVolume; }


        /** Compute the pixel size of an object at position v, with specified radius.*/
        float pixelSize(const Vec3& v,float radius) const
        {
            return getCurrentCullingSet().pixelSize(v,radius);
        }

        /** Compute the pixel size of the bounding sphere.*/
        float pixelSize(const BoundingSphere& bs) const
        {
            return pixelSize(bs.center(),bs.radius());
        }

        /** Compute the pixel size of an object at position v, with specified radius. fabs()ed to always be positive. */
        float clampedPixelSize(const Vec3& v,float radius) const
        {
            return getCurrentCullingSet().clampedPixelSize(v,radius);
        }

        /** Compute the pixel size of the bounding sphere. fabs()ed to always be positive. */
        float clampedPixelSize(const BoundingSphere& bs) const
        {
            return clampedPixelSize(bs.center(),bs.radius());
        }

        inline void disableAndPushOccludersCurrentMask(NodePath& nodePath)
        {
            getCurrentCullingSet().disableAndPushOccludersCurrentMask(nodePath);
        }

        inline void popOccludersCurrentMask(NodePath& nodePath)
        {
            getCurrentCullingSet().popOccludersCurrentMask(nodePath);
        }

        inline bool isCulled(const std::vector<Vec3>& vertices)
        {
            return getCurrentCullingSet().isCulled(vertices);
        }

        inline bool isCulled(const BoundingBox& bb)
        {
            return bb.valid() && getCurrentCullingSet().isCulled(bb);
        }

        inline bool isCulled(const BoundingSphere& bs)
        {
            return getCurrentCullingSet().isCulled(bs);
        }

        inline bool isCulled(const osg::Node& node)
        {
            if (node.isCullingActive())
            {
                return getCurrentCullingSet().isCulled(node.getBound());
            }
            else
            {
                getCurrentCullingSet().resetCullingMask();
                return false;
            }
        }

        inline void pushCurrentMask()
        {
            getCurrentCullingSet().pushCurrentMask();
        }

        inline void popCurrentMask()
        {
            getCurrentCullingSet().popCurrentMask();
        }


        typedef std::vector< CullingSet > CullingStack;

        inline CullingStack& getClipSpaceCullingStack() { return _clipspaceCullingStack; }

        inline CullingStack& getProjectionCullingStack() { return _projectionCullingStack; }

        inline CullingStack& getModelViewCullingStack() { return _modelviewCullingStack; }

        inline CullingSet& getCurrentCullingSet() { return *_back_modelviewCullingStack; }
        inline const CullingSet& getCurrentCullingSet() const { return *_back_modelviewCullingStack; }

        inline osg::Viewport* getViewport();
        inline const osg::Viewport* getViewport() const;

        inline osg::RefMatrix* getModelViewMatrix();
        inline const osg::RefMatrix* getModelViewMatrix() const;

        inline osg::RefMatrix* getProjectionMatrix();
        inline const osg::RefMatrix* getProjectionMatrix() const;

        inline osg::Matrix getWindowMatrix() const;
        inline const osg::RefMatrix* getMVPW();

        inline const osg::Vec3& getReferenceViewPoint() const { return _referenceViewPoints.back(); }
        inline void pushReferenceViewPoint(const osg::Vec3& viewPoint) { _referenceViewPoints.push_back(viewPoint); }
        inline void popReferenceViewPoint() { _referenceViewPoints.pop_back(); }

        inline const osg::Vec3& getEyeLocal() const { return _eyePointStack.back(); }

        inline const osg::Vec3& getViewPointLocal() const { return _viewPointStack.back(); }

        inline const osg::Vec3 getUpLocal() const
        {
            const osg::Matrix& matrix = *_modelviewStack.back();
            return osg::Vec3(matrix(0,1),matrix(1,1),matrix(2,1));
        }

        inline const osg::Vec3 getLookVectorLocal() const
        {
            const osg::Matrix& matrix = *_modelviewStack.back();
            return osg::Vec3(-matrix(0,2),-matrix(1,2),-matrix(2,2));
        }

        typedef fast_back_stack< ref_ptr<RefMatrix> > MatrixStack;

        MatrixStack& getProjectionStack() { return _projectionStack; }
        const MatrixStack& getProjectionStack() const { return _projectionStack; }

        MatrixStack& getModelViewStack() { return _modelviewStack; }
        const MatrixStack& getModelViewStack() const { return _modelviewStack; }

        MatrixStack& getMVPWStack() { return _MVPW_Stack; }
        const MatrixStack& getMVPWStack() const { return _MVPW_Stack; }

    protected:

        // base set of shadow volume occluder to use in culling.
        ShadowVolumeOccluderList                                    _occluderList;


        MatrixStack                                                 _projectionStack;

        MatrixStack                                                 _modelviewStack;
        MatrixStack                                                 _MVPW_Stack;

        typedef fast_back_stack<ref_ptr<Viewport> >                 ViewportStack;
        ViewportStack                                               _viewportStack;

        typedef fast_back_stack<Vec3>                               EyePointStack;
        EyePointStack                                               _referenceViewPoints;
        EyePointStack                                               _eyePointStack;
        EyePointStack                                               _viewPointStack;

        CullingStack                                                _clipspaceCullingStack;
        CullingStack                                                _projectionCullingStack;

        CullingStack                                                _modelviewCullingStack;
        unsigned int                                                _index_modelviewCullingStack;
        CullingSet*                                                 _back_modelviewCullingStack;

        void computeFrustumVolume();
        float                                                       _frustumVolume;

        unsigned int                                                _bbCornerNear;
        unsigned int                                                _bbCornerFar;

        ref_ptr<osg::RefMatrix>                                     _identity;

        typedef std::vector< osg::ref_ptr<osg::RefMatrix> > MatrixList;
        MatrixList _reuseMatrixList;
        unsigned int _currentReuseMatrixIndex;

        inline osg::RefMatrix* createOrReuseMatrix(const osg::Matrix& value);


};

inline osg::Viewport* CullStack::getViewport()
{
    return _viewportStack.empty() ? 0 : _viewportStack.back().get();
}

inline const osg::Viewport* CullStack::getViewport() const
{
    return _viewportStack.empty() ? 0 : _viewportStack.back().get();
}

inline osg::RefMatrix* CullStack::getModelViewMatrix()
{
    return _modelviewStack.empty() ? _identity.get() : _modelviewStack.back().get();
}

inline const osg::RefMatrix* CullStack::getModelViewMatrix() const
{
    return _modelviewStack.empty() ? _identity.get() : _modelviewStack.back().get();
}

inline osg::RefMatrix* CullStack::getProjectionMatrix()
{
    return _projectionStack.empty() ? _identity.get() : _projectionStack.back().get();
}

inline const osg::RefMatrix* CullStack::getProjectionMatrix() const
{
    return _projectionStack.empty() ? _identity.get() : _projectionStack.back().get();
}

inline osg::Matrix CullStack::getWindowMatrix() const
{
    if (!_viewportStack.empty())
    {
        osg::Viewport* viewport = _viewportStack.back().get();
        return viewport->computeWindowMatrix();
    }
    else
    {
        return *_identity;
    }
}

inline const osg::RefMatrix* CullStack::getMVPW()
{
    if (!_MVPW_Stack.empty())
    {
        if (!_MVPW_Stack.back())
        {
            _MVPW_Stack.back() = createOrReuseMatrix(*getModelViewMatrix());
            (*_MVPW_Stack.back()) *= *(getProjectionMatrix());
            (*_MVPW_Stack.back()) *= getWindowMatrix();
        }
        return _MVPW_Stack.back().get();
    }
    else
    {
        return _identity.get();
    }
}

inline RefMatrix* CullStack::createOrReuseMatrix(const osg::Matrix& value)
{
    // skip of any already reused matrix.
    while (_currentReuseMatrixIndex<_reuseMatrixList.size() &&
           _reuseMatrixList[_currentReuseMatrixIndex]->referenceCount()>1)
    {
        ++_currentReuseMatrixIndex;
    }

    // if still within list, element must be singularly referenced
    // there return it to be reused.
    if (_currentReuseMatrixIndex<_reuseMatrixList.size())
    {
        RefMatrix* matrix = _reuseMatrixList[_currentReuseMatrixIndex++].get();
        matrix->set(value);
        return matrix;
    }

    // otherwise need to create new matrix.
    osg::RefMatrix* matrix = new RefMatrix(value);
    _reuseMatrixList.push_back(matrix);
    ++_currentReuseMatrixIndex;
    return matrix;
}

}    // end of namespace

#endif
