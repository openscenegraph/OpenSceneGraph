#include <osgIntrospection/Value>
#include <osgIntrospection/Type>
#include <osgIntrospection/Exceptions>
#include <osgIntrospection/ReaderWriter>
#include <osgIntrospection/Comparator>

#include <sstream>
#include <memory>

using namespace osgIntrospection;

Value Value::convertTo(const Type &outtype) const
{
    Value v = tryConvertTo(outtype);
    if (v.isEmpty())
        throw TypeConversionException(type_->getStdTypeInfo(), outtype.getStdTypeInfo());
    return v;
}

Value Value::tryConvertTo(const Type &outtype) const
{
    check_empty();

    if (type_ == &outtype)
        return *this;

    if (type_->isConstPointer() && outtype.isNonConstPointer())
        return Value();

    std::auto_ptr<ReaderWriter::Options> wopt;

    if (type_->isEnum() && (outtype.getQualifiedName() == "int" || outtype.getQualifiedName() == "unsigned int"))
    {
        wopt.reset(new ReaderWriter::Options);
        wopt->setForceNumericOutput(true);
    }

    const ReaderWriter *src_rw = type_->getReaderWriter();
    if (src_rw)
    {
        const ReaderWriter *dst_rw = outtype.getReaderWriter();
        if (dst_rw)
        {
            std::ostringstream oss;
            if (src_rw->writeTextValue(oss, *this, wopt.get()))
            {
                Value v;
                std::istringstream iss(oss.str());
                if (dst_rw->readTextValue(iss, v))
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

    const ReaderWriter *rw = type_->getReaderWriter();
    if (rw)
    {
        std::ostringstream oss;
        if (!rw->writeTextValue(oss, *this))
            throw StreamWriteErrorException();
        return oss.str();
    }
    throw StreamingNotSupportedException(StreamingNotSupportedException::ANY, type_->getStdTypeInfo());
}

void Value::check_empty() const
{
    if (!type_ || !inbox_)
        throw EmptyValueException();
}

void Value::swap(Value &v)
{
	std::swap(inbox_, v.inbox_);
	std::swap(type_, v.type_);
	std::swap(ptype_, v.ptype_);
}

bool Value::operator ==(const Value &other) const
{
	if (isEmpty() && other.isEmpty())
		return true;

	if (isEmpty() ^ other.isEmpty())
		return false;

	const Comparator *cmp1 = type_->getComparator();
	const Comparator *cmp2 = other.type_->getComparator();
	
	const Comparator *cmp = cmp1? cmp1: cmp2;
	
	if (!cmp)
		throw ComparisonNotPermittedException(type_->getStdTypeInfo());

	if (cmp1 == cmp2)
		return cmp->isEqualTo(*this, other);

	if (cmp1)
		return cmp1->isEqualTo(*this, other.convertTo(*type_));

	return cmp2->isEqualTo(convertTo(*other.type_), other);
}

bool Value::operator <=(const Value &other) const
{
	const Comparator *cmp1 = type_->getComparator();
	const Comparator *cmp2 = other.type_->getComparator();
	
	const Comparator *cmp = cmp1? cmp1: cmp2;
	
	if (!cmp)
		throw ComparisonNotPermittedException(type_->getStdTypeInfo());

	if (cmp1 == cmp2)
		return cmp->isLessThanOrEqualTo(*this, other);

	if (cmp1)
		return cmp1->isLessThanOrEqualTo(*this, other.convertTo(*type_));

	return cmp2->isLessThanOrEqualTo(convertTo(*other.type_), other);
}

bool Value::operator !=(const Value &other) const
{
	return !operator==(other);
}

bool Value::operator >(const Value &other) const
{
	return !operator<=(other);
}

bool Value::operator <(const Value &other) const
{
	const Comparator *cmp1 = type_->getComparator();
	const Comparator *cmp2 = other.type_->getComparator();
	
	const Comparator *cmp = cmp1? cmp1: cmp2;
	
	if (!cmp)
		throw ComparisonNotPermittedException(type_->getStdTypeInfo());

	if (cmp1 == cmp2)
		return cmp->isLessThanOrEqualTo(*this, other) && !cmp->isEqualTo(*this, other);

	if (cmp1)
	{
		Value temp(other.convertTo(*type_));
		return cmp1->isLessThanOrEqualTo(*this, temp) && !cmp1->isEqualTo(*this, temp);
	}

	Value temp(convertTo(*other.type_));
	return cmp2->isLessThanOrEqualTo(temp, other) && !cmp2->isEqualTo(temp, other);
}

bool Value::operator >=(const Value &other) const
{
	const Comparator *cmp1 = type_->getComparator();
	const Comparator *cmp2 = other.type_->getComparator();
	
	const Comparator *cmp = cmp1? cmp1: cmp2;
	
	if (!cmp)
		throw ComparisonNotPermittedException(type_->getStdTypeInfo());

	if (cmp1 == cmp2)
		return !cmp->isLessThanOrEqualTo(*this, other) || cmp->isEqualTo(*this, other);

	if (cmp1)
	{
		Value temp(other.convertTo(*type_));
		return !cmp1->isLessThanOrEqualTo(*this, temp) || cmp1->isEqualTo(*this, temp);
	}

	Value temp(convertTo(*other.type_));
	return !cmp2->isLessThanOrEqualTo(temp, other) || cmp2->isEqualTo(temp, other);
}
