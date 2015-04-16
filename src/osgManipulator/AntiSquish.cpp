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
//osgManipulator - Copyright (C) 2007 Fugro-Jason B.V.


#include <osgManipulator/AntiSquish>

using namespace osgManipulator;


AntiSquish::AntiSquish() : _usePivot(true), _usePosition(false), _cacheDirty( true )
{
}

AntiSquish::AntiSquish(const osg::Vec3d& pivot) : _pivot(pivot), _usePivot(true), _usePosition(false), _cacheDirty( true )
{
}

AntiSquish::AntiSquish(const osg::Vec3d& pivot, const osg::Vec3d& pos)
    : _pivot(pivot), _usePivot(true), _position(pos), _usePosition(true), _cacheDirty( true )
{
}


AntiSquish::AntiSquish(const AntiSquish& pat,const osg::CopyOp& copyop) :
    Transform(pat,copyop),
    _pivot(pat._pivot),
    _usePivot(pat._usePivot),
    _position(pat._position),
    _usePosition(pat._usePosition),
    _cacheDirty(pat._cacheDirty),
    _cacheLocalToWorld(pat._cacheLocalToWorld),
    _cache(pat._cache)
{
}

AntiSquish::~AntiSquish()
{
}


bool AntiSquish::computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
{
    osg::Matrix unsquishedMatrix;
    if ( !computeUnSquishedMatrix( unsquishedMatrix ) )
    {
        return false;
    }

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.preMult(unsquishedMatrix);
    }
    else // absolute
    {
        matrix = unsquishedMatrix;
    }

    return true;
}


bool AntiSquish::computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor*) const
{
    osg::Matrix unsquishedMatrix;
    if ( !computeUnSquishedMatrix( unsquishedMatrix ) )
    {
        return false;
    }

    osg::Matrixd inverse;
    inverse.invert( unsquishedMatrix );

    if (_referenceFrame==RELATIVE_RF)
    {
        matrix.postMult(inverse);
    }
    else // absolute
    {
        matrix = inverse;
    }
    return true;
}


bool AntiSquish::computeUnSquishedMatrix(osg::Matrix& unsquished) const
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock( _cacheLock );

    osg::NodePathList nodePaths = getParentalNodePaths();
    if (nodePaths.empty()) return false;

    osg::NodePath np = nodePaths.front();
    if (np.empty()) return false;

    // Remove the last node which is the anti squish node itself.
    np.pop_back();

    // Get the accumulated modeling matrix.
    const osg::Matrix localToWorld = osg::computeLocalToWorld(np);

    // reuse cached value
    if ( !_cacheDirty && _cacheLocalToWorld==localToWorld )
    {
        unsquished = _cache;
        return true;
    }

    osg::Vec3d t, s;
    osg::Quat r, so;

    localToWorld.decompose(t, r, s, so);

    // Let's take an average of the scale.
    double av = (s[0] + s[1] + s[2])/3.0;
    s[0] = av; s[1] = av; s[2]=av;

    if (av == 0)
        return false;

    //
    // Final Matrix: [-Pivot][SO]^[S][SO][R][T][Pivot][LOCALTOWORLD]^[position]
    // OR [SO]^[S][SO][R][T][LOCALTOWORLD]^
    //
    if (_usePivot)
    {
        unsquished.postMultTranslate(-_pivot);

        osg::Matrix tmps, invtmps;
        so.get(tmps);
        if (!invtmps.invert(tmps))
            return false;

        //SO^
        unsquished.postMult(invtmps);
        //S
        unsquished.postMultScale(s);
        //SO
        unsquished.postMult(tmps);
        //R
        unsquished.postMultRotate(r);
        //T
        unsquished.postMultTranslate(t);

        osg::Matrix invltw;
        if (!invltw.invert(localToWorld))
            return false;

        // LTW^
        unsquished.postMult( invltw );

        // Position
        if (_usePosition)
            unsquished.postMultTranslate(_position);
        else
            unsquished.postMultTranslate(_pivot);
    }
    else
    {
        osg::Matrix tmps, invtmps;
        so.get(tmps);
        if (!invtmps.invert(tmps))
            return false;

        unsquished.postMult(invtmps);
        unsquished.postMultScale(s);
        unsquished.postMult(tmps);
        unsquished.postMultRotate(r);
        unsquished.postMultTranslate(t);
        osg::Matrix invltw;
        if (!invltw.invert(localToWorld))
            return false;
        unsquished.postMult( invltw );
    }

    if (unsquished.isNaN())
        return false;

    _cache = unsquished;
    _cacheLocalToWorld = localToWorld;
    _cacheDirty = false;

    //As Transform::computeBounde calls us without a node-path it relies on
    //The cache. Hence a new _cache affects the bound.
    const_cast<AntiSquish*>(this)->dirtyBound();

    return true;
}



