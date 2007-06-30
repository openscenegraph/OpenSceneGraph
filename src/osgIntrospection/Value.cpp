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
//osgIntrospection - Copyright (C) 2005 Marco Jez

#include <osgIntrospection/Value>
#include <osgIntrospection/Type>
#include <osgIntrospection/Exceptions>
#include <osgIntrospection/ReaderWriter>
#include <osgIntrospection/Comparator>
#include <osgIntrospection/Converter>
#include <osgIntrospection/Reflection>

#include <sstream>
#include <memory>

using namespace osgIntrospection;

Value Value::convertTo(const Type& outtype) const
{
    Value v = tryConvertTo(outtype);
    if (v.isEmpty())
        throw TypeConversionException(_type->getExtendedTypeInfo(), outtype.getExtendedTypeInfo());
    return v;
}

Value Value::tryConvertTo(const Type& outtype) const
{
    check_empty();

    if (_type == &outtype)
        return *this;

    if (_type->isConstPointer() && outtype.isNonConstPointer())
        return Value();

    // search custom converters
    ConverterList conv;
    if (Reflection::getConversionPath(*_type, outtype, conv))
    {
        std::auto_ptr<CompositeConverter> cvt(new CompositeConverter(conv));
        return cvt->convert(*this);
    }

    std::auto_ptr<ReaderWriter::Options> wopt;

    if (_type->isEnum() && (outtype.getQualifiedName() == "int" || outtype.getQualifiedName() == "unsigned int"))
    {
        wopt.reset(new ReaderWriter::Options);
        wopt->setForceNumericOutput(true);
    }


    // ** never converte a pointer to another pointer with the ReaderWriter method
    // **  in this case, the ReaderWriter method always work and
    // **  using a pointer with a bad type cause a SEGFAULT
    if (_type->isPointer() && outtype.isPointer()) return Value();

    const ReaderWriter* src_rw = _type->getReaderWriter();
    if (src_rw)
    {
        const ReaderWriter* dst_rw = outtype.getReaderWriter();
        if (dst_rw)
        {
            std::stringstream ss;
            if (src_rw->writeTextValue(ss, *this, wopt.get()))
            {
                Value v;
                if (dst_rw->readTextValue(ss, v))
                {
                    return v;
                }
            }
        }
    }

    return Value();
}

std::string Value::toString() const
{
  check_empty();

    const ReaderWriter* rw = _type->getReaderWriter();
    if (rw)
    {
        std::ostringstream oss;
        if (!rw->writeTextValue(oss, *this))
            throw StreamWriteErrorException();
     return oss.str();
    }
    throw StreamingNotSupportedException(StreamingNotSupportedException::ANY, _type->getExtendedTypeInfo());
    }

std::wstring Value::toWString() const
{
    check_empty();

    const ReaderWriter* rw = _type->getReaderWriter();
    if (rw)
    {
        std::wostringstream woss;
        if (!rw->writeTextValue(woss, *this))
            throw StreamWriteErrorException();
        return woss.str();
    }
    throw StreamingNotSupportedException(StreamingNotSupportedException::ANY, _type->getExtendedTypeInfo());
}

void Value::check_empty() const
{
    if (!_type || !_inbox)
        throw EmptyValueException();
}

void Value::swap(Value& v)
{
    std::swap(_inbox, v._inbox);
    std::swap(_type, v._type);
    std::swap(_ptype, v._ptype);
}

bool Value::operator ==(const Value& other) const
{
    if (isEmpty() && other.isEmpty())
        return true;

    if (isEmpty() ^ other.isEmpty())
        return false;

    const Comparator* cmp1 = _type->getComparator();
    const Comparator* cmp2 = other._type->getComparator();
    
    const Comparator* cmp = cmp1? cmp1: cmp2;
    
    if (!cmp)
        throw ComparisonNotPermittedException(_type->getExtendedTypeInfo());

    if (cmp1 == cmp2)
        return cmp->isEqualTo(*this, other);

    if (cmp1)
        return cmp1->isEqualTo(*this, other.convertTo(*_type));

    return cmp2->isEqualTo(convertTo(*other._type), other);
}

bool Value::operator <=(const Value& other) const
{
    const Comparator* cmp1 = _type->getComparator();
    const Comparator* cmp2 = other._type->getComparator();
    
    const Comparator* cmp = cmp1? cmp1: cmp2;
    
    if (!cmp)
        throw ComparisonNotPermittedException(_type->getExtendedTypeInfo());

    if (cmp1 == cmp2)
        return cmp->isLessThanOrEqualTo(*this, other);

    if (cmp1)
        return cmp1->isLessThanOrEqualTo(*this, other.convertTo(*_type));

    return cmp2->isLessThanOrEqualTo(convertTo(*other._type), other);
}

bool Value::operator !=(const Value& other) const
{
    return !operator==(other);
}

bool Value::operator >(const Value& other) const
{
    return !operator<=(other);
}

bool Value::operator <(const Value& other) const
{
    const Comparator* cmp1 = _type->getComparator();
    const Comparator* cmp2 = other._type->getComparator();
    
    const Comparator* cmp = cmp1? cmp1: cmp2;
    
    if (!cmp)
        throw ComparisonNotPermittedException(_type->getExtendedTypeInfo());

    if (cmp1 == cmp2)
        return cmp->isLessThanOrEqualTo(*this, other) && !cmp->isEqualTo(*this, other);

    if (cmp1)
    {
        Value temp(other.convertTo(*_type));
        return cmp1->isLessThanOrEqualTo(*this, temp) && !cmp1->isEqualTo(*this, temp);
    }

    Value temp(convertTo(*other._type));
    return cmp2->isLessThanOrEqualTo(temp, other) && !cmp2->isEqualTo(temp, other);
}

bool Value::operator >=(const Value& other) const
{
    const Comparator* cmp1 = _type->getComparator();
    const Comparator* cmp2 = other._type->getComparator();
    
    const Comparator* cmp = cmp1? cmp1: cmp2;
    
    if (!cmp)
        throw ComparisonNotPermittedException(_type->getExtendedTypeInfo());

    if (cmp1 == cmp2)
        return !cmp->isLessThanOrEqualTo(*this, other) || cmp->isEqualTo(*this, other);

    if (cmp1)
    {
        Value temp(other.convertTo(*_type));
        return !cmp1->isLessThanOrEqualTo(*this, temp) || cmp1->isEqualTo(*this, temp);
    }

    Value temp(convertTo(*other._type));
    return !cmp2->isLessThanOrEqualTo(temp, other) || cmp2->isEqualTo(temp, other);
}
