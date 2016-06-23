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
#include <osgDB/Input>
#include <istream>

#include <stdio.h>

using namespace osgDB;

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
    for(i=0;i<256;++i) _delimiterEatLookUp[i]=false;
    _delimiterEatLookUp[int(' ')] = true;
    _delimiterEatLookUp[int('\t')] = true;
    _delimiterEatLookUp[int('\n')] = true;
    _delimiterEatLookUp[int('\r')] = true;

    for(i=0;i<256;++i) _delimiterKeepLookUp[i]=false;
    _delimiterKeepLookUp[int('{')] = true;
    _delimiterKeepLookUp[int('}')] = true;
    _delimiterKeepLookUp[int('"')] = true;
    _delimiterKeepLookUp[int('\'')] = true;

}


void FieldReader::_copy(const FieldReader& ic)
{

    _fin = ic._fin;
    _eof = ic._eof;

    _noNestedBrackets = ic._noNestedBrackets;

    int i;
    for(i=0;i<256;++i) _delimiterEatLookUp[i]=ic._delimiterEatLookUp[i];
    for(i=0;i<256;++i) _delimiterKeepLookUp[i]=ic._delimiterKeepLookUp[i];
}


void FieldReader::attach(std::istream* input)
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
        else if (_delimiterEatLookUp[ch])
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
            bool escape = false; // use the escape character sequence \" to allow " to included in strings.
            while (true)
            {
                ch = _fin->peek();
                if (ch==EOF)
                {
                    _eof = true;
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                c = ch;
                if (ch=='\\')
                {
                    if (escape)
                    {
                        escape = false;
                        _fin->get(c);
                        if (fieldPtr) fieldPtr->addChar(c);
                    }
                    else
                    {
                        escape = true;
                        _fin->ignore(1);
                    }
                }
                else if (ch=='"')
                {
                    if (escape)
                    {
                        escape = false;
                        _fin->get(c);
                        if (fieldPtr) fieldPtr->addChar(c);
                    }
                    else
                    {
                        _fin->ignore(1);
                        //return fieldPtr && fieldPtr->getNoCharacters()!=0;
                        return fieldPtr!=NULL;
                    }
                }
                else
                {
                    if (escape)
                    {
                        escape = false;
                        if (fieldPtr) fieldPtr->addChar('\\');
                    }
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
            bool escape = false; // use the escape character sequence \' to allow ' to included in strings.
            while (true)
            {
                ch = _fin->peek();
                if (ch==EOF)
                {
                    _eof = true;
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                c = ch;
                if (ch=='\\' && !escape)
                {
                    escape = true;
                    _fin->ignore(1);
                }
                else if (ch=='\'')
                {
                    if (escape)
                    {
                        escape = false;
                        _fin->get(c);
                        if (fieldPtr) fieldPtr->addChar(c);
                    }
                    else
                    {
                        _fin->ignore(1);
                        //return fieldPtr && fieldPtr->getNoCharacters()!=0;
                        return fieldPtr!=NULL;
                    }
                }
                else
                {
                    if (escape)
                    {
                        escape = false;
                        if (fieldPtr) fieldPtr->addChar('\\');
                    }
                    _fin->get(c);
                    if (fieldPtr) fieldPtr->addChar(c);
                }
            }
        }
        else if (_delimiterKeepLookUp[ch])
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
                if (_delimiterEatLookUp[int(c)])
                {
                    _fin->ignore(1);
                    return fieldPtr && fieldPtr->getNoCharacters()!=0;
                }
                if (_delimiterKeepLookUp[int(c)])
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
