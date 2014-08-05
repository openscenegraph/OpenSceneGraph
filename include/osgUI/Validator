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

#ifndef OSGUI_VALIDATOR
#define OSGUI_VALIDATOR

#include <osg/Object>
#include <osgUI/Export>

namespace osgUI
{

class OSGUI_EXPORT Validator : public osg::Object
{
public:
    Validator();
    Validator(const Validator& validator, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, Validator);

    enum State
    {
        INVALID,
        INTERMEDIATE,
        ACCEPTABLE
    };

    /** entry point to validate(..) method, checks for "validate" CallbackObject and calls it if present, otherwise calls validateImplementation(..)
        str parameter is the string that needs to be validated
        cursorpos is the position of the cursor within the str string.
        return validatidy State. */
    virtual State validate(std::string& /*str*/, int& cursorpos) const;

    /// override in subclasses to proviude the validate implementation.
    virtual State validateImplementation(std::string& /*str*/, int& /*cursorpos*/) const;

    /** entry point to fixup, checks for "validate" Callbac Object and calls it if present, otherwise calls validateImplementation(..)
        fixup(..) is called when user pressers return/enter in a field being edited.
        str parameter is string that needs to be corrected.*/
    virtual void fixup(std::string& /*str*/) const;

    /// override in subclass to provide the fixup implementation.
    virtual void fixupImplementation(std::string& /*str*/) const;

protected:
    virtual ~Validator() {}
};

class OSGUI_EXPORT IntValidator : public Validator
{
public:
    IntValidator();
    IntValidator(const IntValidator& widget, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, IntValidator);

    /// set the bottom value that is accepted as valid, default -INT_MAX
    void setBottom(int bottom) { _bottom = bottom; }
    int getBottom() const { return _bottom; }

    /// set the top value that is accepted as valid, default INT_MAX
    void setTop(int top) { _top = top; }
    int getTop() const { return _top; }

    /// override validate implementation.
    virtual State validateImplementation(std::string& str, int& cursorpos) const;
    /// override validate implementation.
    virtual void fixupImplementation(std::string& str) const;

protected:
    virtual ~IntValidator() {}
    int _bottom;
    int _top;
};

class OSGUI_EXPORT DoubleValidator : public Validator
{
public:
    DoubleValidator();
    DoubleValidator(const DoubleValidator& widget, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
    META_Object(osgUI, DoubleValidator);

    /** set the number of decimal places to accept, default is -1,
        all negative values disable validation against maximum number of places thus allows any number of decimals places. */
    void setDecimals(int numDecimals) { _decimals = numDecimals; }
    int getDecimals() const { return _decimals; }

    /// set the bottom value that is accepted as valid, default -DBL_MAX
    void setBottom(double bottom) { _bottom = bottom; }
    double getBottom() const { return _bottom; }

    /// set the top value that is accepted as valid, default DBL_MAX
    void setTop(double top) { _top = top; }
    double getTop() const { return _top; }

    /// override validate implementation.
    virtual State validateImplementation(std::string& str, int& cursorpos) const;
    /// override validate implementation.
    virtual void fixupImplementation(std::string& str) const;

protected:
    virtual ~DoubleValidator() {}
    int         _decimals;
    double      _bottom;
    double      _top;
};

}

#endif
