#include <osgDB/FieldReaderIterator>

using namespace osgDB;

FieldReaderIterator::FieldReaderIterator()
{
    _init();
}


FieldReaderIterator::FieldReaderIterator(const FieldReaderIterator& ic)
{
    _copy(ic);
}


FieldReaderIterator::~FieldReaderIterator()
{
    _free();
}


FieldReaderIterator& FieldReaderIterator::operator = (const FieldReaderIterator& ic)
{
    if (this==&ic) return *this;
    _free();
    _copy(ic);
    return *this;
}


void FieldReaderIterator::_free()
{
    // free all data
    if (_previousField)
    {
        delete _previousField;
    }
    if (_fieldQueue)
    {
        for(int i=0;i<_fieldQueueCapacity;++i)
        {
            if (_fieldQueue[i]) delete _fieldQueue[i];
            _fieldQueue[i] = NULL;
        }
        delete [] _fieldQueue;
    }
    _init();

}


void FieldReaderIterator::_init()
{
    _previousField = NULL;
    _fieldQueue = NULL;
    _fieldQueueSize = 0;
    _fieldQueueCapacity = 0;

}


void FieldReaderIterator::_copy(const FieldReaderIterator& ic)
{
    _reader = ic._reader;

    if (ic._previousField)
    {
        _previousField = new Field(*ic._previousField);
    }

    if (ic._fieldQueue && ic._fieldQueueCapacity>0)
    {
        _fieldQueue = new Field* [ic._fieldQueueCapacity];
        for(int i=0;i<ic._fieldQueueCapacity;++i)
        {
            if (ic._fieldQueue[i])
            {
                _fieldQueue[i] = new Field(*ic._fieldQueue[i]);
            }
            else
            {
                _fieldQueue[i] = NULL;
            }
        }
        _fieldQueueSize = ic._fieldQueueSize;
        _fieldQueueCapacity = ic._fieldQueueCapacity;
    }
    else
    {
        _fieldQueue = NULL;
        _fieldQueueSize = 0;
        _fieldQueueCapacity = 0;
    }

}


void FieldReaderIterator::attach(istream* input)
{
    _reader.attach(input);
}


void FieldReaderIterator::detach()
{
    _reader.detach();
}


bool FieldReaderIterator::eof() const
{
    return _fieldQueueSize==0 && _reader.eof();
}


void FieldReaderIterator::insert(int pos,Field* field)
{
    if (field==NULL) return;

    if (pos<0) pos=0;
    if (pos>_fieldQueueSize) pos=_fieldQueueSize;

    int i;
                                 // need to reallocate the stack
    if (_fieldQueueSize>=_fieldQueueCapacity)
    {
        int newCapacity = _fieldQueueCapacity*2;
        if (newCapacity<MINIMUM_FIELD_READER_QUEUE_SIZE) newCapacity = MINIMUM_FIELD_READER_QUEUE_SIZE;
        while(_fieldQueueSize>=newCapacity) newCapacity*=2;
        Field** newFieldStack = new Field* [newCapacity];
        for(i=0;i<_fieldQueueCapacity;++i)
        {
            newFieldStack[i] = _fieldQueue[i];
        }
        for(;i<newCapacity;++i)
        {
            newFieldStack[i] = NULL;
        }
        _fieldQueue = newFieldStack;
        _fieldQueueCapacity = newCapacity;
    }

    for(i=_fieldQueueSize-1;i>=pos;++i)
    {
        _fieldQueue[i+1]=_fieldQueue[i];
    }
    _fieldQueue[pos] = field;
    ++_fieldQueueSize;
}


void FieldReaderIterator::insert(int pos,const char* str)
{
    if (str)
    {
        Field* field = new Field;
        while(*str!=0)
        {
            field->addChar(*str);
            ++str;
        }
        insert(pos,field);
    }
}


Field& FieldReaderIterator::operator [] (int pos)
{
    return field(pos);
}


Field& FieldReaderIterator::field (int pos)
{
    if (pos<0)
    {
        _blank.setNoNestedBrackets(_reader.getNoNestedBrackets());
        return _blank;
    }                            // can directly access field
    else if (pos<_fieldQueueSize)
    {
        return *_fieldQueue[pos];
    }                            // need to read the new fields.
    else
    {
                                 // need to reallocate the stack
        if (pos>=_fieldQueueCapacity)
        {
            int newCapacity = _fieldQueueCapacity*2;
            if (newCapacity<MINIMUM_FIELD_READER_QUEUE_SIZE) newCapacity = MINIMUM_FIELD_READER_QUEUE_SIZE;
            while(_fieldQueueSize>=newCapacity) newCapacity*=2;
            Field** newFieldStack = new Field* [newCapacity];
            int i;
            for(i=0;i<_fieldQueueCapacity;++i)
            {
                newFieldStack[i] = _fieldQueue[i];
            }
            for(;i<newCapacity;++i)
            {
                newFieldStack[i] = NULL;
            }
            _fieldQueue = newFieldStack;
            _fieldQueueCapacity = newCapacity;
        }
        while(!_reader.eof() && pos>=_fieldQueueSize)
        {
            if (_fieldQueue[_fieldQueueSize]==NULL) _fieldQueue[_fieldQueueSize] = new Field;
            if (_reader.readField(*_fieldQueue[_fieldQueueSize]))
            {
                ++_fieldQueueSize;
            }
        }
        if (pos<_fieldQueueSize)
        {
            return *_fieldQueue[pos];
        }
        else
        {
            _blank.setNoNestedBrackets(_reader.getNoNestedBrackets());
            return _blank;
        }
    }
}


FieldReaderIterator& FieldReaderIterator::operator ++ ()
{
    return (*this)+=1;
}


FieldReaderIterator& FieldReaderIterator::operator += (int no)
{
    if (no>_fieldQueueSize)
    {
        while (!_reader.eof() && no>_fieldQueueSize)
        {
            _reader.ignoreField();
            --no;
        }
        _fieldQueueSize=0;
    }
    else if (no>0)
    {
        Field** tmpFields = new Field* [no];
        int i;
        for(i=0;i<no;++i)
        {
            tmpFields[i] = _fieldQueue[i];
        }
        for(i=no;i<_fieldQueueSize;++i)
        {
            _fieldQueue[i-no] = _fieldQueue[i];
        }
        _fieldQueueSize -= no;
        for(i=0;i<no;++i)
        {
            _fieldQueue[_fieldQueueSize+i] = tmpFields[i];
        }
        delete [] tmpFields;
    }
    return *this;
}


// increments the itetor of the next simple field or
// whole block if the current field[0] is an open bracket
void FieldReaderIterator::advanceOverCurrentFieldOrBlock()
{
    if (field(0).isOpenBracket()) advanceToEndOfCurrentBlock();
    else ++(*this);
}


void FieldReaderIterator::advanceToEndOfCurrentBlock()
{
    int entry = field(0).getNoNestedBrackets();
    while(!eof() && field(0).getNoNestedBrackets()>=entry)
    {
        ++(*this);
    }
}


void FieldReaderIterator::advanceToEndOfBlock(int noNestedBrackets)
{
    while(!eof() && field(0).getNoNestedBrackets()>=noNestedBrackets)
    {
        ++(*this);
    }
}


bool FieldReaderIterator::matchSequence(const char* str)
{
    if (str==NULL) return false;
    if (*str==0) return false;
    int fieldCount = 0;
    const char* end = str;
    while((*end)!=0 && (*end)==' ') ++end;
    const char* start = end;
    while((*start)!=0)
    {
        if (*end!=' ' && *end!=0)
        {
            ++end;
        }
        else
        {
            if (start!=end)
            {
                if (end-start>1 && *start=='%')
                {
                    const char type = *(start+1);
                    switch(type)
                    {
                                 // expecting an integer
                        case('i') :
                        {
                            if (!field(fieldCount).isInt()) return false;
                            break;
                        }
                                 // expecting an floating point number
                        case('f') :
                        {
                            if (!field(fieldCount).isFloat()) return false;
                            break;
                        }
                                 // expecting an quoted string
                        case('s') :
                        {
                            if (!field(fieldCount).isQuotedString()) return false;
                            break;
                        }
                        default :// expecting an word
                        {
                            if (!field(fieldCount).isWord()) return false;
                            break;
                        }
                    }

                }
                else
                {
                    if (*start=='{')
                    {
                        if (!field(fieldCount).isOpenBracket()) return false;
                    }
                    else if (*start=='}')
                    {
                        if (!field(fieldCount).isCloseBracket()) return false;
                    }
                    else
                    {
                        if (!field(fieldCount).matchWord(start,end-start)) return false;
                    }
                }
                fieldCount++;
            }
            while((*end)==' ') ++end;
            start = end;
        }
    }
    return true;
}
