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

namespace
{
    class AntiSquishCallback: public osg::NodeCallback
    {
        public:
            AntiSquishCallback(AntiSquish* asq) : osg::NodeCallback(), _antiSquish(asq) {}
            virtual ~AntiSquishCallback() {};
            
            virtual void operator() (osg::Node*, osg::NodeVisitor* nv)
            {
                // Get the node path.
                osg::NodePath np = nv->getNodePath();

                // Remove the last node which is the anti squish node itself.
                np.pop_back(); 

                // Get the accumulated modeling matrix.
                osg::Matrix localToWorld = osg::computeLocalToWorld(np);

                // compute the unsquished matrix.
                bool flag = false;
                osg::Matrix _unsquishedMatrix = _antiSquish->computeUnSquishedMatrix(localToWorld, flag);
                if (flag)
                    _antiSquish->setMatrix(_unsquishedMatrix);
            }

        protected:
            AntiSquish* _antiSquish;
    };

}

AntiSquish::AntiSquish() : _usePivot(true), _usePosition(false)
{
    _asqCallback = new AntiSquishCallback(this);
    setUpdateCallback(_asqCallback);
}

AntiSquish::AntiSquish(const osg::Vec3d& pivot) : _pivot(pivot), _usePivot(true), _usePosition(false)
{
    _asqCallback = new AntiSquishCallback(this);
    setUpdateCallback(_asqCallback);
}

AntiSquish::AntiSquish(const osg::Vec3d& pivot, const osg::Vec3d& pos)
    : _pivot(pivot), _usePivot(true), _position(pos), _usePosition(true)
{
    _asqCallback = new AntiSquishCallback(this);
    setUpdateCallback(_asqCallback);
}


AntiSquish::AntiSquish(const AntiSquish& pat,const osg::CopyOp& copyop) :
    MatrixTransform(pat,copyop),
    _asqCallback(pat._asqCallback),
    _pivot(pat._pivot),
    _usePivot(pat._usePivot),
    _position(pat._position),
    _usePosition(pat._usePosition),
    _cachedLocalToWorld(pat._cachedLocalToWorld) 
{
}

AntiSquish::~AntiSquish()
{
}

osg::Matrix AntiSquish::computeUnSquishedMatrix(const osg::Matrix& LTW, bool& flag)
{
    osg::Vec3d t, s;
    osg::Quat r, so; 

    if (LTW == _cachedLocalToWorld && _dirty == false)
    {
        flag = false;
        return osg::Matrix::identity();
    }

    _cachedLocalToWorld = LTW; 

    LTW.decompose(t, r, s, so);

    // Let's take an average of the scale.
    double av = (s[0] + s[1] + s[2])/3.0; 
    s[0] = av; s[1] = av; s[2]=av;

    if (av == 0)
    {
        flag = false;
        return osg::Matrix::identity();
    }

    osg::Matrix unsquished;
    
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
        {
            flag = false;
            return osg::Matrix::identity();
        }
  
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
        if (!invltw.invert(LTW))
        {
            flag = false;
            return osg::Matrix::identity();
        }
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
        {
            flag = false;
            return osg::Matrix::identity();
        }
        unsquished.postMult(invtmps);
        unsquished.postMultScale(s);
        unsquished.postMult(tmps);
        unsquished.postMultRotate(r);
        unsquished.postMultTranslate(t);
        osg::Matrix invltw;
        if (!invltw.invert(LTW))
        {
            flag = false;
            return osg::Matrix::identity();
        }
        unsquished.postMult( invltw );
    }

    if (unsquished.isNaN())
    {
        flag = false;
        return  osg::Matrix::identity();
    } 

    flag = true;
    _dirty = false;
    return unsquished; 
}



