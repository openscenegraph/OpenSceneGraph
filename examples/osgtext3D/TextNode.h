/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
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

#ifndef OSGTEXT_TEXTNODE
#define OSGTEXT_TEXTNODE 1


#include <osg/Group>
#include <osg/Quat>
#include <osgUtil/CullVisitor>

#include <osgText/Font>
#include <osgText/String>

namespace osgText {

// forward declare
class TextNode;
class Glyph;

class Bevel : public osg::Object
{
    public:

        Bevel();
        Bevel(const Bevel& bevel, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgText, Bevel)

        void setBevelThickness(float thickness) { _thickness = thickness; }
        float getBevelThickness() const { return _thickness; }

        void flatBevel(float width=0.25f);

        void roundedBevel(float width=0.5f, unsigned int numSteps=10);

        void roundedBevel2(float width=0.5f, unsigned int numSteps=10);

        typedef std::vector<osg::Vec2> Vertices;

        void setVertices(const Vertices& vertices) { _vertices = vertices; }
        Vertices& getVertices() { return _vertices; }
        const Vertices& getVertices() const { return _vertices; }

        void print(std::ostream& fout);

    protected:

        float           _thickness;
        Vertices        _vertices;
};


class Style : public osg::Object
{
    public:

        Style();
        Style(const Style& style, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgText, Style)

        /// default Layout implementation used if no other is specified on TextNode
        static osg::ref_ptr<Style>& getDefaultStyle();

        /// NULL is no bevel
        void setBevel(Bevel* bevel) { _bevel = bevel; }
        const Bevel* getBevel() const { return _bevel.get(); }


        /// 1 is the default width of the text
        void setWidthRatio(float widthRatio) { _widthRatio = widthRatio; }
        float getWidthRatio() const { return _widthRatio; }

        /// 0 is 2D text
        void setThicknessRatio(float thicknessRatio) { _thicknessRatio = thicknessRatio; }
        float getThicknessRatio() const { return _thicknessRatio; }

        /// 0 is off
        void setOutlineRatio(float outlineRatio) { _outlineRatio = outlineRatio; }
        float getOutlineRatio() const { return _outlineRatio; }

        /// 1.0 is default number of samples
        void setSampleDensity(float sd) { _sampleDensity = sd; }
        float getSampleDensity() const { return _sampleDensity; }

    protected:

        osg::ref_ptr<Bevel>     _bevel;

        float                   _widthRatio;
        float                   _thicknessRatio;
        float                   _outlineRatio;
        float                   _sampleDensity;
};

class Layout : public osg::Object
{
    public:

        Layout();
        Layout(const Layout& layout, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgText,Layout)

        /// default Layout implementation used if no other is specified on TextNode
        static osg::ref_ptr<Layout>& getDefaultLayout();

        virtual void layout(TextNode& text) const;

    protected:
};

class TextTechnique : public osg::Object
{
    public:

        TextTechnique();
        TextTechnique(const TextTechnique& technique, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgText, TextTechnique)

        TextNode* getTextNode() { return _textNode; }
        const TextNode* getTextNode() const { return _textNode; }

        /// default TextTechnique implementation used if no other is specified on TextNode
        static osg::ref_ptr<TextTechnique>& getDefaultTextTechinque();

        /// start building a new charater layout
        virtual void start();

        /// called by Layout engine to place individual characters
        virtual void addCharacter(const osg::Vec3& position, const osg::Vec3& size, Glyph* glyph, Style* style);

        /// called by Layout engine to place individual characters
        virtual void addCharacter(const osg::Vec3& position, const osg::Vec3& size, Glyph3D* glyph, Style* style);

        /// finish building new charater layout
        virtual void finish();

        /// provide traversal control
        virtual void traverse(osg::NodeVisitor& nv);

    protected:

        friend class TextNode;

        void setTextNode(TextNode* textNode) { _textNode = textNode; }

        TextNode* _textNode;
};

class TextNode : public osg::Group
{
    public:

        TextNode();
        TextNode(const TextNode& text, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osgText, TextNode)

        virtual void traverse(osg::NodeVisitor& nv);

        void setFont(Font* font) { _font = font; }
        Font* getFont() { return _font.get(); }
        const Font* getFont() const { return _font.get(); }
        Font* getActiveFont() { return _font.valid() ? _font.get() : Font::getDefaultFont().get(); }
        const Font* getActiveFont() const { return _font.valid() ? _font.get() : Font::getDefaultFont().get(); }

        void setStyle(Style* style) { _style = style; }
        Style* getStyle() { return _style.get(); }
        const Style* getStyle() const { return _style.get(); }
        Style* getActiveStyle() { return _style.valid() ? _style.get() : Style::getDefaultStyle().get(); }
        const Style* getActiveStyle() const { return _style.valid() ? _style.get() : Style::getDefaultStyle().get(); }

        void setLayout(Layout* layout) { _layout = layout; }
        Layout* getLayout() { return _layout.get(); }
        const Layout* getLayout() const { return _layout.get(); }
        const Layout* getActiveLayout() const { return _layout.valid() ? _layout.get() : Layout::getDefaultLayout().get(); }

        void setTextTechnique(TextTechnique* technique);
        TextTechnique* getTextTechnique() { return _technique.get(); }
        const TextTechnique* getTextTechnique() const { return _technique.get(); }

        void setText(const std::string& str);
        void setText(const String& str) { _string = str; }
        String& getText() { return _string; }
        const String& getText() const { return _string; }

        void setPosition(const osg::Vec3d& position) { _position  = position; }
        const osg::Vec3d& getPosition() const { return _position; }

        void setRotation(const osg::Quat& rotation) { _rotation  = rotation; }
        const osg::Quat& getRotation() const { return _rotation; }

        void setCharacterSize(float characterSize) { _characterSize = characterSize; }
        float getCharacterSize() const { return _characterSize; }

        /// force a regeneration of the rendering backend required to represent the text.
        virtual void update();

    protected:

        virtual ~TextNode();

        osg::ref_ptr<Font>              _font;
        osg::ref_ptr<Style>             _style;
        osg::ref_ptr<Layout>            _layout;
        osg::ref_ptr<TextTechnique>     _technique;

        String                          _string;
        osg::Vec3d                      _position;
        osg::Quat                       _rotation;
        float                           _characterSize;
};

}

#endif