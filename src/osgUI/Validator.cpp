/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#include <osgUI/Validator>
#include <osg/ValueObject>
#include <osg/Callback>
#include <sstream>
#include <limits.h>

using namespace osgUI;

Validator::Validator()
{
}

Validator::Validator(const osgUI::Validator& validator, const osg::CopyOp& copyop):
    osg::Object(validator, copyop)
{
}

Validator::State Validator::validate(std::string& text, int& cursorpos) const
{
    const osg::CallbackObject* co = getCallbackObject(this, "validate");
    if (co)
    {
        osg::ref_ptr<osg::StringValueObject> textInput = new osg::StringValueObject("text",text);
        osg::ref_ptr<osg::IntValueObject> cursorposInput = new osg::IntValueObject("cursorpos",cursorpos);

        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(textInput.get());
        inputParameters.push_back(cursorposInput.get());
        if (co->run(const_cast<Validator*>(this), inputParameters, outputParameters))
        {
            if (textInput->getValue()!=text)
            {
                OSG_NOTICE<<"Updating text in CallbackObject "<<textInput->getValue()<<std::endl;
                text = textInput->getValue();
            }
            if (cursorposInput->getValue()!=cursorpos)
            {
                OSG_NOTICE<<"Updating cursor pos in CallbackObject "<<cursorposInput->getValue()<<std::endl;
                cursorpos = cursorposInput->getValue();
            }

            if (outputParameters.size()>=1)
            {
                osg::Object* object = outputParameters[0].get();
                osg::StringValueObject* svo = dynamic_cast<osg::StringValueObject*>(object);
                if (svo)
                {
                    OSG_NOTICE<<"Have string return value from validate "<<svo->getValue()<<std::endl;

                    std::string returnString = svo->getValue();
                    if (returnString=="INVALID") return INVALID;
                    else if (returnString=="INTERMEDITATE") return INTERMEDIATE;
                    else if (returnString=="ACCEPTABLE") return ACCEPTABLE;
                }
                OSG_NOTICE<<"Called validate CallbackObject but didn't get string return value."<<object->className()<<std::endl;
            }
        }
    }
    return validateImplementation(text, cursorpos);
}

Validator::State Validator::validateImplementation(std::string& text, int& cursorpos) const
{
    OSG_NOTICE<<"Validator::validateImplemetation("<<text<<", "<<cursorpos<<")"<<std::endl;
    return ACCEPTABLE;
}

void Validator::fixup(std::string& text) const
{
    const osg::CallbackObject* co = getCallbackObject(this, "fixup");
    if (co)
    {
        osg::ref_ptr<osg::StringValueObject> textInput = new osg::StringValueObject("text",text);

        osg::Parameters inputParameters, outputParameters;
        inputParameters.push_back(textInput.get());
        if (co->run(const_cast<Validator*>(this), inputParameters, outputParameters))
        {
            if (textInput->getValue()!=text)
            {
                OSG_NOTICE<<"Updating text in CallbackObject "<<textInput->getValue()<<std::endl;
                text = textInput->getValue();
            }
        }
    }
    return fixupImplementation(text);
}

void Validator::fixupImplementation(std::string& text) const
{
    OSG_NOTICE<<"Validator::fixupImplemetation("<<text<<")"<<std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// IntValidator
//
IntValidator::IntValidator():
    _bottom(-INT_MAX),
    _top(-INT_MAX)
{
}

IntValidator::IntValidator(const IntValidator& validator, const osg::CopyOp& copyop):
    Validator(validator, copyop),
    _bottom(validator._bottom),
    _top(validator._top)
{
}

IntValidator::State IntValidator::validateImplementation(std::string& str, int& cursorpos) const
{
    std::string newstring;
    bool canBeNegative = _bottom<0.0;

    int numNegative = 0;
    for(std::size_t pos = 0; pos<str.size(); ++pos)
    {
        char c = str[pos];

        bool validChar = false;
        if (c>='0' && c<='9')
        {
            validChar = true;
        }
        else if (c=='-')
        {
            if (canBeNegative)
            {
                if (numNegative==0) validChar = true;
                ++numNegative;
            }
        }

        if (validChar) newstring.push_back(c);
    }

    str = newstring;

    if (str.empty()) return INTERMEDIATE;


    int v = static_cast<int>(osg::asciiToDouble(str.c_str()));
    if (v<_bottom)
    {
        return INTERMEDIATE;
    }
    if (v>_top)
    {
        return INTERMEDIATE;
    }

    return ACCEPTABLE;
}

void IntValidator::fixupImplementation(std::string& str) const
{
    if (str.empty()) return;

    int v = static_cast<int>(osg::asciiToDouble(str.c_str()));
    if (v<_bottom)
    {
        v = _bottom;
    }
    if (v>_top)
    {
        v = _top;
    }

    std::stringstream buffer;
    buffer<<v<<std::endl;
    str = buffer.str();
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// DoubleValidator
//
DoubleValidator::DoubleValidator():
    _decimals(-1),
    _bottom(-DBL_MAX),
    _top(DBL_MAX)
{
}

DoubleValidator::DoubleValidator(const DoubleValidator& validator, const osg::CopyOp& copyop):
    Validator(validator, copyop),
    _decimals(validator._decimals),
    _bottom(validator._bottom),
    _top(validator._top)
{
}

DoubleValidator::State DoubleValidator::validateImplementation(std::string& str, int& cursorpos) const
{
    std::string newstring;
    bool canBeNegative = _bottom<0.0;
    int maxNumDecimalPlaces = _decimals>=0 ? _decimals : str.size();

    int numPlacesAfterDecimal = 0;
    int numNegative = 0;
    bool hasDecimal = false;
    for(std::size_t pos = 0; pos<str.size(); ++pos)
    {
        char c = str[pos];

        bool validChar = false;
        if (c>='0' && c<='9')
        {
            if (hasDecimal)
            {
                ++numPlacesAfterDecimal;
                if (numPlacesAfterDecimal<=maxNumDecimalPlaces) validChar = true;
            }
            else
            {
                validChar = true;
            }
        }
        else if (c=='-')
        {
            if (canBeNegative)
            {
                if (numNegative==0) validChar = true;
                ++numNegative;
            }
        }
        else if (c=='.')
        {
            if (!hasDecimal)
            {
                validChar = true;
                hasDecimal = true;
            }
        }

        if (validChar) newstring.push_back(c);
    }

    str = newstring;

    if (str.empty()) return INTERMEDIATE;


    double v = osg::asciiToDouble(str.c_str());
    if (v<_bottom)
    {
        return INTERMEDIATE;
    }
    if (v>_top)
    {
        return INTERMEDIATE;
    }

    return ACCEPTABLE;
}

void DoubleValidator::fixupImplementation(std::string& str) const
{
    if (str.empty()) return;

    double v = osg::asciiToDouble(str.c_str());
    if (v<_bottom)
    {
        v = _bottom;
    }
    if (v>_top)
    {
        v = _top;
    }

    std::stringstream buffer;
    buffer<<v<<std::endl;
    str = buffer.str();
}

