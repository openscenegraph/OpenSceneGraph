#include <osgText/Paragraph>

using namespace osgText;

Paragraph::Paragraph()
{
    _alignment = osgText::Text::LEFT_TOP;
    _maxCharsPerLine = 80;
}

Paragraph::Paragraph(const Paragraph& paragraph,const osg::CopyOp& copyop):
    osg::Geode(paragraph,copyop),
    _position(paragraph._position),
    _text(paragraph._text),
    _font(dynamic_cast<Font*>(copyop(paragraph._font.get()))),
    _alignment(paragraph._alignment),
    _maxCharsPerLine(paragraph._maxCharsPerLine)
{
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
    
    typedef std::vector<std::string> TextList;
    TextList formatedText;
    
    createFormatedText(_maxCharsPerLine,_text,formatedText);

    // now create the text drawables from the formate text list.
    for(TextList::iterator itr=formatedText.begin();
        itr!=formatedText.end();
        ++itr)
    {
    
        osgText::Text* textDrawable = osgNew osgText::Text(_font.get());
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

bool Paragraph::createFormatedText(unsigned int noCharsPerLine,const std::string& str,std::vector<std::string>& formatedText)
{
    if (str.empty()) return false;

    std::string::size_type start = 0;
    std::string::size_type last_space = 0;
    std::string::size_type current_line_length = 0;

    for(std::string::size_type current=0;
        current<str.size();
        ++current)
    {
        const char c = str[current];
        if (c==' ') last_space = current;
        
        if (c=='\n')
        {
            formatedText.push_back(std::string(str,start,current-start));
            start = current+1;
            
            last_space = start;
            current_line_length = 0;
        }
        else if (current_line_length==noCharsPerLine)
        {
            if (last_space>start)
            {
                formatedText.push_back(std::string(str,start,last_space-start));
                start = last_space+1;
                current_line_length = current-start;
            }
            
            else
            {
                formatedText.push_back(std::string(str,start,current-start));
                start = current+1;
                current_line_length = 0;
            }
            
            last_space = start;
        }
        else ++current_line_length;
    }
    if (start<str.size())
    {
        formatedText.push_back(std::string(str,start,str.size()-start));
    }

    return true;
}
