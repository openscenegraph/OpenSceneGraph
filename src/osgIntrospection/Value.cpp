#include <osgIntrospection/Value>
#include <osgIntrospection/Type>
#include <osgIntrospection/Exceptions>
#include <osgIntrospection/ReaderWriter>

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

bool Value::compare(const Value &v1, const Value &v2)
{
    if (v1.isEmpty() && v2.isEmpty()) return true;
    if (v1.isEmpty() || v2.isEmpty()) return false;
    if (v1.type_ == v2.type_) return v1.inbox_->equal(v2);
    Value temp(v2.convertTo(v1.getType()));
    return compare(v1, temp);
}

void Value::check_empty() const
{
    if (!type_ || !inbox_)
        throw EmptyValueException();
}
