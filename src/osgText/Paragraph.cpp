#include <osgText/Paragraph>

using namespace osgText;

Paragraph::Paragraph()
{
    _alignment = osgText::Text::LEFT_TOP;
    _maxCharsPerLine = 80;
}

Paragraph::Paragraph(const osg::Vec3& position,const std::string& text,osgText::Font* font)
{
    _maxCharsPerLine = 80;
    _position = position;
    _font = font;
    setText(text);
}

void Paragraph::setPosition(const osg::Vec3& position)
{
    if (_position==position) return;
    
    osg::Vec3 delta = position-_position;
    
    _position = position;
    
    for(osg::Geode::DrawableList::iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        osgText::Text* text = dynamic_cast<osgText::Text*>(itr->get());
        if (text) text->setPosition(text->getPosition()+delta);
    }
    
}

void Paragraph::setFont(osgText::Font* font)
{
    if (_font==font) return;
    
    _font = font;
    for(osg::Geode::DrawableList::iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        osgText::Text* text = dynamic_cast<osgText::Text*>(itr->get());
        if (text) text->setFont(font);
    }

}

void Paragraph::setMaximumNoCharactersPerLine(unsigned int maxCharsPerLine)
{
    if (_maxCharsPerLine==maxCharsPerLine) return;

    if (maxCharsPerLine<1) maxCharsPerLine=1;
    else _maxCharsPerLine=maxCharsPerLine;
    
    createDrawables();
}

void Paragraph::setAlignment(int alignment)
{
    if (_alignment==alignment) return;
    
    _alignment=alignment;

    for(osg::Geode::DrawableList::iterator itr=_drawables.begin();
        itr!=_drawables.end();
        ++itr)
    {
        osgText::Text* text = dynamic_cast<osgText::Text*>(itr->get());
        if (text) text->setAlignment(_alignment);
    }
}

void Paragraph::setText(const std::string& text)
{
    if (text==_text) return;
    
    _text = text;
    
    createDrawables();
}

float Paragraph::getHeight() const
{
    if (_font.valid()) return (_font->getPointSize()+1)*getNumDrawables();
    else return 0;
}


void Paragraph::createDrawables()
{
    _drawables.clear();
        
    osg::Vec3 pos = _position;
    
    typedef vector<std::string> TextList;
    TextList formatedText;

    std::string::size_type start = 0;
    std::string::size_type last_space = 0;
    std::string::size_type current_line_length = 0;

    for(std::string::size_type current=0;
        current<_text.size();
        ++current)
    {
        const char c = _text[current];
        if (c==' ') last_space = current;
        
        if (c=='\n')
        {
            formatedText.push_back(std::string(_text,start,current-start));
            start = current+1;
            
            last_space = start;
            current_line_length = 0;
        }
        else if (current_line_length==_maxCharsPerLine)
        {
            if (last_space>start)
            {
                formatedText.push_back(std::string(_text,start,last_space-start));
                start = last_space+1;
            }
            
            else
            {
                formatedText.push_back(std::string(_text,start,current-start));
                start = current+1;
            }
            
            last_space = start;
            current_line_length = 0;
        }
        else ++current_line_length;
    }
    if (start<_text.size())
    {
        formatedText.push_back(std::string(_text,start,_text.size()-start));
    }

    // now create the text drawables from the formate text list.
    for(TextList::iterator itr=formatedText.begin();
        itr!=formatedText.end();
        ++itr)
    {
        osgText::Text* textDrawable = new osgText::Text(_font.get());
        textDrawable->setAlignment(_alignment);
        textDrawable->setPosition(pos);
        textDrawable->setText(*itr);
        //            textDrawable->setDrawMode( osgText::Text::TEXT |
        //                               osgText::Text::BOUNDINGBOX |
        //                               osgText::Text::ALIGNEMENT );

        addDrawable(textDrawable);

        pos.y() -= (_font->getPointSize()+1);

    }
    
}
