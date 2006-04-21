/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

#ifndef OSGPARTICLE_PRECIPITATIODRAWABLE
#define OSGPARTICLE_PRECIPITATIODRAWABLE

#include <osg/Geometry>
#include <osgParticle/Export>

#include "PrecipitationParameters.h"

namespace osgParticle
{

    class OSGPARTICLE_EXPORT PrecipitationDrawable : public osg::Drawable
    {
    public:
    
        PrecipitationDrawable();
        PrecipitationDrawable(const PrecipitationDrawable& copy, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY);

        META_Object(osgParticle, PrecipitationDrawable);

        virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveFunctor&) const {}
        virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return false; }
        virtual void accept(osg::PrimitiveIndexFunctor&) const {}

        void setParameters(PrecipitationParameters* parameters);
        PrecipitationParameters* getParameters() { return _parameters.get(); }
        const PrecipitationParameters* getParameters() const { return _parameters.get(); }
        
        void setGeometry(osg::Geometry* geom) { _geometry = geom; }
        osg::Geometry* getGeometry() { return _geometry.get(); }
        const osg::Geometry* getGeometry() const { return _geometry.get(); }
        
        virtual void compileGLObjects(osg::State& state) const;
        
        virtual void drawImplementation(osg::State& state) const;
        
        
        
        struct Cell
        {
            Cell(int in_i, int in_j, int in_k):
                i(in_i), j(in_j), k(in_k) {}
        
        
            inline bool operator == (const Cell& rhs) const
            {
                return i==rhs.i && j==rhs.j && k==rhs.k;
            }

            inline bool operator != (const Cell& rhs) const
            {
                return i!=rhs.i || j!=rhs.j || k!=rhs.k;
            }

            inline bool operator < (const Cell& rhs) const
            {
                if (i<rhs.i) return true;
                if (i>rhs.i) return false;
                if (j<rhs.j) return true;
                if (j>rhs.j) return false;
                if (k<rhs.k) return true;
                if (k>rhs.k) return false;
                return false;
            }
        
            int i;
            int j;
            int k;
        };
        
        typedef std::map< Cell, osg::Matrix >  CellMatrixMap;
        
        CellMatrixMap& getCurrentCellMatrixMap() { return _currentCellMatrixMap; }
        CellMatrixMap& getPreviousCellMatrixMap() { return _previousCellMatrixMap; }

        inline void newFrame()
        {
            _previousCellMatrixMap.swap(_currentCellMatrixMap);
            _currentCellMatrixMap.clear();
        }

    protected:
    
        virtual ~PrecipitationDrawable() {}
        
        osg::ref_ptr<PrecipitationParameters> _parameters;
        
        osg::ref_ptr<osg::Geometry> _geometry;
        
        
        mutable CellMatrixMap _currentCellMatrixMap;
        mutable CellMatrixMap _previousCellMatrixMap;

    };

}

#endif
