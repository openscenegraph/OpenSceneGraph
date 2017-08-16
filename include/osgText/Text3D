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

#ifndef OSGTEXT_TEXT3D
#define OSGTEXT_TEXT3D 1


#include <osgText/TextBase>
#include <osgText/Style>

namespace osgText {


class OSGTEXT_EXPORT Text3D : public osgText::TextBase
{
    public:

        /** Deprecated.*/
        enum RenderMode
        {
            PER_FACE,
            PER_GLYPH
        };

        Text3D();
        Text3D(const Text3D& text,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgText,Text3D)

        /** Get the Charactere Depth of text. */
        float getCharacterDepth() const;

        /** Set the Charactere Depth of text. */
        void setCharacterDepth(float characterDepth);

        /** Deprecated, value is now ignored. */
        RenderMode getRenderMode() const { return _renderMode; }
        /** Deprecated, value is now ignored. */
        void setRenderMode(RenderMode renderMode) { _renderMode = renderMode; }


        /** Get the wall StateSet */
        osg::StateSet* getWallStateSet() { return _wallStateSet.get(); }
        /** Get the wall StateSet */
        const osg::StateSet* getWallStateSet() const { return _wallStateSet.get(); }
        /** Get or create the wall StateSet */
        osg::StateSet* getOrCreateWallStateSet()
        {
           if (_wallStateSet.valid() == false) _wallStateSet = new osg::StateSet;
           return _wallStateSet.get();
        }
        /** Set the wall StateSet */
        void setWallStateSet(osg::StateSet* wallStateSet)  { _wallStateSet = wallStateSet; }

        /** Get the back StateSet */
        osg::StateSet* getBackStateSet() { return _backStateSet.get(); }
        /** Get the back StateSet */
        osg::StateSet* getBackStateSet() const { return _backStateSet.get(); }
        /** Get or create the back StateSet */
        osg::StateSet* getOrCreateBackStateSet() { if (_backStateSet.valid() == false) _backStateSet = new osg::StateSet; return _backStateSet.get(); }
        /** Set the back StateSet */
        void setBackStateSet(osg::StateSet* backStateSet)  { _backStateSet = backStateSet; }



        /** Draw the text.*/
        virtual void drawImplementation(osg::RenderInfo& renderInfo) const;

        /** return false, osgText::Text does not support accept(AttributeFunctor&).*/
        virtual bool supports(const osg::Drawable::AttributeFunctor&) const { return false; }

        /** return true, osgText::Text does support accept(ConstAttributeFunctor&).*/
        virtual bool supports(const osg::Drawable::ConstAttributeFunctor&) const { return false; }

        /** accept an ConstAttributeFunctor and call its methods to tell it about the internal attributes that this Drawable has.*/
        virtual void accept(osg::Drawable::ConstAttributeFunctor& af) const;

        /** return true, osgText::Text does support accept(PrimitiveFunctor&) .*/
        virtual bool supports(const osg::PrimitiveFunctor&) const { return false; }

        /** accept a PrimtiveFunctor and call its methods to tell it about the internal primtives that this Drawable has.*/
        virtual void accept(osg::PrimitiveFunctor& pf) const;

        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize);

        /** If State is non-zero, this function releases OpenGL objects for
          * the specified graphics context. Otherwise, releases OpenGL objexts
          * for all graphics contexts. */
        virtual void releaseGLObjects(osg::State* state=0) const;

        // make Font a friend to allow it set the _font to 0 if the font is
        // forcefully unloaded.
        friend class Font;

        virtual osg::BoundingBox computeBoundingBox() const;


    protected:

        virtual ~Text3D() {}

        String::iterator computeLastCharacterOnLine(osg::Vec2& cursor, String::iterator first,String::iterator last);

        void computeGlyphRepresentation();

        void copyAndOffsetPrimitiveSets(osg::Geometry::PrimitiveSetList& dest_PrimitiveSetList, osg::Geometry::PrimitiveSetList& src_PrimitiveSetList, unsigned int offset);

        osg::Geometry::PrimitiveSetList _frontPrimitiveSetList;
        osg::Geometry::PrimitiveSetList _wallPrimitiveSetList;
        osg::Geometry::PrimitiveSetList _backPrimitiveSetList;

        // ** glyph and other information to render the glyph
        struct GlyphRenderInfo
        {
            GlyphRenderInfo(GlyphGeometry* glyphGeometry, osg::Vec3 & pos):
                _glyphGeometry(glyphGeometry),
                _position(pos) {}

            osg::ref_ptr<GlyphGeometry> _glyphGeometry;
            osg::Vec3 _position;
        };

        typedef std::vector<GlyphRenderInfo> LineRenderInfo;
        typedef std::vector<LineRenderInfo> TextRenderInfo;

        TextRenderInfo _textRenderInfo;

        // deprecated value no longer used.
        RenderMode _renderMode;

        osg::ref_ptr<osg::StateSet> _wallStateSet;
        osg::ref_ptr<osg::StateSet> _backStateSet;
};

}


#endif
