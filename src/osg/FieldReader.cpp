#include "osg/FieldReader"
#include "osg/Field"

using namespace osg;

FieldReader::FieldReader()
{
    _init();
}


FieldReader::FieldReader(const FieldReader& ic)
{
    _copy(ic);
}


FieldReader::~FieldReader()
{
    _free();
}


FieldReader& FieldReader::operator = (const FieldReader& ic)
{
    if (this==&ic) return *this;
    _free();
    _copy(ic);
    return *this;
}


void FieldReader::_free()
{
// free all data

    _init();

}


void FieldReader::_init()
{
    _fin = NULL;
    _eof = true;

    _noNestedBrackets = 0;

    int i;
    for(i=0;i<256;++i) _delimatorEatLookUp[i]=false;
    _delimatorEatLookUp[' '] = true;
    _delimatorEatLookUp['\t'] = true;
    _delimatorEatLookUp['\n'] = true;
    _delimatorEatLookUp['\r'] = true;

    for(i=0;i<256;++i) _delimatorKeepLookUp[i]=false;
    _delimatorKeepLookUp['{'] = true;
    _delimatorKeepLookUp['}'] = true;
    _delimatorKeepLookUp['"'] = true;
    _delimatorKeepLookUp['\''] = true;

}


void FieldReader::_copy(const FieldReader& ic)
{

    _fin = ic._fin;
    _eof = ic._eof;

    _noNestedBrackets = ic._noNestedBrackets;

    int i;
    for(i=0;i<256;++i) _delimatorEatLookUp[i]=ic._delimatorEatLookUp[i];
    for(i=0;i<256;++i) _delimatorKeepLookUp[i]=ic._delimatorKeepLookUp[i];
}


void FieldReader::attach(istream* input)
{
    _fin = input;

    if (_fin)
    {
        _eof = _fin->eof()!=0;
    }
    else
    {
        _eof = true;
    }
}


void FieldReader::detach()
{
    _fin = NULL;
    _eof = true;
}


bool FieldReader::eof() const
{
    return _eof;
}


bool FieldReader::findStartOfNextField()
{
    int ch = 0;
    while (true)
    {
        ch = _fin->peek();
        if (ch==EOF)
        {
            _eof = true;
            return false;
        }
        else if (_delimatorEatLookUp[ch])
        {
            _fin->ignore(1);
        }
        else
        {
            return true;
        }
    }
}


bool FieldReader::readField(Field& fieldPtr)
{
    return _readField(&fieldPtr);
}


void FieldReader::ignoreField()
{
    _readField(NULL);
}


bool FieldReader::_readField(Field* fieldPtr)
{
    if (fieldPtr) fieldPtr->reset();

    if (!eof() && findStartOfNextField())
    {

        int ch = _fin->peek();
        if (ch==EOF)
        {
            _eof = true;
            if (fieldPtr) fieldPtr->setNoNestedBrackets(getNoNestedBrackets());
            return fieldPtr && fieldPtr->getNoCharacters()!=0;
        }
        else if (ch=='"')
        {
            if (fieldPtr)
            {
                fieldPtr->setWithinQuotes(true);
                fieldPtr->setNoNestedBrackets(getNoNestedBrackets());
            }
            _fin->ignore(1);
            char c;
            while (true)
            {
                ch = _fin->peek();
                if (ch==EOF)
                {
                    _eof = true;
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                c = ch;
                if (ch=='"')
                {
                    _fin->ignore(1);
                    //return fieldPtr && fieldPtr->getNoCharacters()!=0;
                    return fieldPtr!=NULL;
                }
                else
                {
                    _fin->get(c);
                    if (fieldPtr) fieldPtr->addChar(c);
                }
            }
        }
        else if (ch=='\'')
        {
            if (fieldPtr)
            {
                fieldPtr->setWithinQuotes(true);
                fieldPtr->setNoNestedBrackets(getNoNestedBrackets());
            }
            _fin->ignore(1);
            char c;
            while (true)
            {
                ch = _fin->peek();
                if (ch==EOF)
                {
                    _eof = true;
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                c = ch;
                if (ch=='\'')
                {
                    _fin->ignore(1);
                    //return fieldPtr && fieldPtr->getNoCharacters()!=0;
                    return fieldPtr!=NULL;
                }
                else
                {
                    _fin->get(c);
                    if (fieldPtr) fieldPtr->addChar(c);
                }
            }
        }
        else if (_delimatorKeepLookUp[ch])
        {
            char c;
            _fin->get(c);
            if (fieldPtr) fieldPtr->addChar(c);
            if (c=='{') ++_noNestedBrackets;
            else if (c=='}') --_noNestedBrackets;
            if (fieldPtr) fieldPtr->setNoNestedBrackets(getNoNestedBrackets());
            return fieldPtr && fieldPtr->getNoCharacters()!=0;
        }
        else
        {
            if (fieldPtr) fieldPtr->setNoNestedBrackets(getNoNestedBrackets());
            char c;
            while (true)
            {
                ch = _fin->peek();
                if (ch==EOF)
                {
                    _eof = true;
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                c = ch;
                if (_delimatorEatLookUp[c])
                {
                    _fin->ignore(1);
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                if (_delimatorKeepLookUp[c])
                {
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                else
                {
                    _fin->get(c);
                    if (fieldPtr) fieldPtr->addChar(c);
                }
            }
        }

    }
    else
    {
        return false;
    }
}


int FieldReader::getNoNestedBrackets() const
{
    return _noNestedBrackets;
}
