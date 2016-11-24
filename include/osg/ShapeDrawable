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

#ifndef OSG_SHAPEDRAWABLE
#define OSG_SHAPEDRAWABLE 1

#include <osg/Geometry>

namespace osg {


/** Allow the use of <tt>Shape</tt>s as <tt>Drawable</tt>s, so that they can
 *  be rendered with reduced effort. The implementation of \c ShapeDrawable is
 *  not geared to efficiency; it's better to think of it as a convenience to
 *  render <tt>Shape</tt>s easily (perhaps for test or debugging purposes) than
 *  as the right way to render basic shapes in some efficiency-critical section
 *  of code.
 */
class OSG_EXPORT ShapeDrawable : public osg::Geometry
{
    public:

        ShapeDrawable();

        ShapeDrawable(Shape* shape, TessellationHints* hints=0);

        template<class T> ShapeDrawable(const ref_ptr<T>& shape, TessellationHints* hints=0):
            _tessellationHints(hints) { setShape(shape.get()); }

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        ShapeDrawable(const ShapeDrawable& pg,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        virtual Object* cloneType() const { return new ShapeDrawable(); }
        virtual Object* clone(const CopyOp& copyop) const { return new ShapeDrawable(*this,copyop); }
        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const ShapeDrawable*>(obj)!=NULL; }
        virtual const char* libraryName() const { return "osg"; }
        virtual const char* className() const { return "ShapeDrawable"; }

        virtual void setShape(Shape* shape);

        /** Set the color of the shape.*/
        void setColor(const Vec4& color);

        /** Get the color of the shape.*/
        const Vec4& getColor() const { return _color; }

        void setTessellationHints(TessellationHints* hints);

        TessellationHints* getTessellationHints() { return _tessellationHints.get(); }
        const TessellationHints* getTessellationHints() const { return _tessellationHints.get(); }

        /** method to invoke to rebuild the geometry that renders the shape.*/
        void build();

    protected:

        ShapeDrawable& operator = (const ShapeDrawable&) { return *this;}

        virtual ~ShapeDrawable();

        Vec4 _color;

        ref_ptr<TessellationHints> _tessellationHints;

};


}

#endif
