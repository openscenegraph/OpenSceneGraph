// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osg/io_utils>
#include <osgWidget/WindowManager>
#include <osgWidget/Input>

namespace osgWidget {

class BlinkCursorCallback: public osg::Drawable::DrawCallback
{
public:
    BlinkCursorCallback(const bool& insertMode)
        : _insertMode(insertMode)
    {
    }

    virtual void drawImplementation( osg::RenderInfo & ri,const osg::Drawable* drawable ) const
    {
        static bool on = true;
        static osg::Timer_t startTime = osg::Timer::instance()->tick();
        osg::Timer_t now = osg::Timer::instance()->tick();

        if (osg::Timer::instance()->delta_s(startTime,now)>(_insertMode?0.125:0.25))
        {
            on = !on;
            startTime = now;
        }
        if (on)
            drawable->drawImplementation(ri);
    }
protected:
    const bool&    _insertMode;
};

Input::Input(const std::string& name, const std::string& label, unsigned int size):
Label        (name, label),
_xoff        (0.0f),
_yoff        (0.0f),
_index       (0),
_size        (0),
_cursorIndex (0),
_textLength     (0),
_maxSize     (size),
_insertMode     (false),    
_cursor      (new Widget("cursor")) {
   _text->setAlignment(osgText::Text::LEFT_BOTTOM_BASE_LINE);
   _text->setKerningType(osgText::KERNING_NONE);

   // Make the cursor un-copyable.
   _cursor->setCanClone(false);
   _cursor->setDataVariance(osg::Object::DYNAMIC);
   _cursor->setColor(0.0f, 0.0f, 0.0f, 1.0f);

   setEventMask(
       // For showing/hiding the "cursor."
       EVENT_MASK_FOCUS |
       // For keypresses, obviously.
       EVENT_MASK_KEY |
       // For "click" focusing.
       EVENT_MOUSE_PUSH
   );

   _offsets.resize(size+1, 0.0f);
   _widths.resize(size+1, 1.0f);

   _text->getText().resize(size, ' ');
   _text->update();

   _cursor->setDrawCallback( new BlinkCursorCallback(_insertMode) );
}

void Input::_calculateSize(const XYCoord& size) {
   // An Input cannot currently set it's own size RELIABLY until the osgText implementation
   // is dratiscally improved. I'm getting wildly crazy results. :(
   // point_type height = size.y() > _cursor->getHeight() ? size.y() : _cursor->getHeight();

#if 0
   point_type width  = size.x() + _cursor->getWidth();
   point_type height = _cursor->getHeight();

   if(width > getWidth()) setWidth(osg::round(width));

   if(height > getHeight()) setHeight(osg::round(height));
#endif
}

void Input::_calculateCursorOffsets() {
   // Determine the "offset"

    if (_text->getText().size()==0) 
    {
        _offsets[0] = 0;
        _widths[0] = 0;
        return;
    }

    osg::Vec3 pos = _text->getPosition();

    osgText::Text::TextureGlyphQuadMap& tgqm = const_cast<osgText::Text::TextureGlyphQuadMap&>(_text->getTextureGlyphQuadMap());
    osgText::Text::TextureGlyphQuadMap::iterator tgqmi = tgqm.begin();

    std::vector<osg::Vec2>                coords;
    std::vector<osgText::Font::Glyph*>    glyphs;
    for ( ; tgqmi != tgqm.end(); tgqmi++ )
    {
        const osgText::Text::GlyphQuads& gq = tgqmi->second;

        //coords.insert(coords.end(),gq.getTransformedCoords(0).begin(),gq.getTransformedCoords(0).end());
        coords.insert(coords.end(),gq.getCoords().begin(),gq.getCoords().end());
        for (unsigned int i=0; i<gq.getGlyphs().size(); ++i)
        {
            glyphs.push_back(gq.getGlyphs().at(i));
        }
    }
    
    std::list<unsigned int> keys;
    for (unsigned int i=0; i<_text->getText().size(); ++i)
    {
        keys.push_back(_text->getText().at(i));
    }
    unsigned int idx=0;
    osg::Vec2 lr;
    osg::Vec2 ll;
    while (!keys.empty())
    {
        unsigned int key = keys.front();
        for (unsigned int i=0; i<glyphs.size(); ++i)
        {
            osgText::Font::Glyph* g = glyphs.at(i);
            if (g->getGlyphCode()==key)
            {
                lr = coords[2 + (i * 4)];
                ll = coords[1 + (i * 4)];

                point_type width = lr.x() - ll.x();
                _widths[idx] = width == 0 ? g->getHorizontalAdvance() : width;

                _offsets[idx] = lr.x() + pos.x();

                if (width == 0)
                    _offsets[idx] += g->getHorizontalAdvance();
                ++idx;

                glyphs.erase(glyphs.begin()+i);
                coords.erase(coords.begin()+i*4);
                coords.erase(coords.begin()+i*4);
                coords.erase(coords.begin()+i*4);
                coords.erase(coords.begin()+i*4);
                break;
            }
        }
        keys.pop_front();
    }

    _offsets[idx] = lr.x() + pos.x();
    _widths[idx]= 1.f;

    _wordsOffsets.clear();
    for ( unsigned int i=0; i<_text->getText().size(); ++i )
    {
        while (i<_text->getText().size() && _text->getText().at(i)==' ') ++i;
        if (i<_text->getText().size())_wordsOffsets.push_back(i);
        while (i<_text->getText().size() && _text->getText().at(i)!=' ') ++i;
    }

    positioned();
}

bool Input::focus(const WindowManager*) {
   _cursor->setColor(0.5f, 0.5f, 0.6f, 1.0f);

   return true;
}

bool Input::unfocus(const WindowManager*) {
   _cursor->setColor(0.0f, 0.0f, 0.0f, 0.0f);

   return true;
}

void Input::parented(Window* parent) {
   Label::parented(parent);

   _cursor->setSize(_widths[_index], getHeight());

   if(_cursorIndex) parent->getGeode()->setDrawable(_cursorIndex, _cursor.get());

   else _cursorIndex = parent->addDrawableAndGetIndex(_cursor.get());
}

void Input::positioned() {
   point_type ln = static_cast<point_type>(_text->getLineCount());

   ln = ln == 0.0f ? 1.0f : ln;

   // point_type th = (_text->getCharacterHeight() * ln) + (_text->getLineSpacing() * (ln - 1.0f));

   point_type x = getX() + _xoff;
   point_type y = getY() + _yoff;

   // XYCoord size = getTextSize();

   _text->setPosition(osg::Vec3(x, y, _calculateZ(LAYER_MIDDLE)));

   point_type xoffset = _index > 0 ? _offsets[_index - 1] : 0.0f;

   _cursor->setSize(_widths[_index], getHeight());
   _cursor->setOrigin(getX() + xoffset, getY() );
   _cursor->setZ(_calculateZ(LAYER_MIDDLE-1));
}

bool Input::keyUp(int key, int mask, const WindowManager*) {
   return false;
}

bool Input::keyDown(int key, int mask, const WindowManager*) {
   osgText::String& s = _text->getText();

   switch (key)
   {
    case osgGA::GUIEventAdapter::KEY_Left:
        if (mask & osgGA::GUIEventAdapter::MODKEY_CTRL)
        {
            bool found = false;
            for (unsigned int i=0; i<_wordsOffsets.size()-1; ++i)
            {
                if (_wordsOffsets.at(i) < _index && _index <= _wordsOffsets.at(i+1))
                {
                    found = true;
                    _index = _wordsOffsets.at(i);
                    break;
                }
            }
            if (!found && _wordsOffsets.size())
            {
                _index = _wordsOffsets.at(_wordsOffsets.size()-1);
            }
        }
        else
        if (_index>0)
        {
            --_index;
        }
        break;
    case osgGA::GUIEventAdapter::KEY_Right:
        if (mask & osgGA::GUIEventAdapter::MODKEY_CTRL)
        {
            bool found = false;
            for (unsigned int i=0; i<_wordsOffsets.size()-1; ++i)
            {
                if (_wordsOffsets.at(i) <= _index && _index < _wordsOffsets.at(i+1))
                {
                    found = true;
                    _index = _wordsOffsets.at(i+1);
                    break;
                }
            }
            if (!found && _wordsOffsets.size())
            {
                _index = _wordsOffsets.at(_wordsOffsets.size()-1);
            }
        }
        else
        if (_index<_textLength)
        {
            ++_index;
        }
        break;
    case osgGA::GUIEventAdapter::KEY_Home:
        _index = 0;
        break;
    case osgGA::GUIEventAdapter::KEY_End:
        _index = _textLength;
        break;
    case osgGA::GUIEventAdapter::KEY_Insert:
        _insertMode = !_insertMode;
        break;
    case osgGA::GUIEventAdapter::KEY_Delete:
        if (mask & osgGA::GUIEventAdapter::MODKEY_CTRL)
        {
            point_type    deleteToIdx = _textLength;
            for (unsigned int i=0; i<_wordsOffsets.size()-1; ++i)
            {
                if (_wordsOffsets.at(i) <= _index && _index < _wordsOffsets.at(i+1))
                {
                    deleteToIdx = _wordsOffsets.at(i+1);
                    break;
                }
            }
            for (unsigned int i=0; i < s.size()-_index; ++i)
            {
                s[_index+i] = deleteToIdx+i < s.size() ? s[deleteToIdx+i] : ' ';
            }

            _text->update();

            _calculateCursorOffsets();

            _textLength -= deleteToIdx-_index;
        }
        else
        if (_index < s.size()-1)
        {
            for (unsigned int i=_index; i < s.size()-1; ++i)
            {
                s[i] = s[i+1];
            }

            _text->update();

            _calculateCursorOffsets();

            --_textLength;
        }
        break;
    case osgGA::GUIEventAdapter::KEY_BackSpace:
       if(_index >= 1) {

           _index--;
            if (_index< s.size()-1)
            {
                for (unsigned int i=_index; i < s.size()-1; ++i)
                {
                    s[i] = s[i+1];
                    s[i+1] = ' ';
                }
            }
            else
            {
                s[s.size()-1] = ' ';
            }

           _text->update();

           _calculateCursorOffsets();

           --_textLength;
       }
       break;
   default:
        if(key > 255 || _index >= _maxSize) return false;

        if (!_insertMode)
        {
            for (unsigned int i=s.size()-1; i>_index; --i)
            {
                s[i] = s[i-1];
            }
        }

       s[_index] = key;

       _text->update();

       _calculateCursorOffsets();

       _index++;

       if (!_insertMode) ++_textLength;
   }

   // _text->update();

   _calculateSize(getTextSize());

   getParent()->resize();

   return false;
}

void Input::setCursor(Widget*) {
}

unsigned int Input::calculateBestYOffset(const std::string& s)
{

   const osgText::FontResolution fr(static_cast<unsigned int>(_text->getCharacterHeight()),
                                    static_cast<unsigned int>(_text->getCharacterHeight()));

   osgText::String utf(s);

   unsigned int descent = 0;

   for(osgText::String::iterator i = utf.begin(); i != utf.end(); i++) {
       osgText::Font*        font  = const_cast<osgText::Font*>(_text->getFont());
       osgText::Font::Glyph* glyph = font->getGlyph(fr, *i);
       unsigned int          d     = abs((int)glyph->getHorizontalBearing().y());

       if(d > descent) descent = d;
   }

   return descent;
}

}
