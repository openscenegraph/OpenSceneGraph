
#ifndef HEADER_GUARD_OSGFFMPEG_BOUNDED_MESSAGE_QUEUE_H
#define HEADER_GUARD_OSGFFMPEG_BOUNDED_MESSAGE_QUEUE_H

#include <OpenThreads/Condition>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <cassert>
#include <algorithm>
#include <vector>



namespace osgFFmpeg {



template <class T>
class BoundedMessageQueue
{
public:

    typedef T value_type;
    typedef size_t size_type;

    explicit BoundedMessageQueue(size_type capacity);
    ~BoundedMessageQueue();

    void clear();

    template <class Destructor>
    void flush(const Destructor destructor);

    void push(const value_type & value);
    bool tryPush(const value_type & value);
    bool timedPush(const value_type & value, unsigned long ms);

    value_type pop();
    value_type tryPop(bool & is_empty);
    value_type timedPop(bool & is_empty, unsigned long ms);

private:

    BoundedMessageQueue(const BoundedMessageQueue &);
    BoundedMessageQueue & operator = (const BoundedMessageQueue &);

    typedef std::vector<T> Buffer;
    typedef OpenThreads::Condition Condition;
    typedef OpenThreads::Mutex Mutex;
    typedef OpenThreads::ScopedLock<Mutex> ScopedLock;

    bool isFull() const;
    bool isEmpty() const;

    void unsafePush(const value_type & value);
    value_type unsafePop();

    Buffer        m_buffer;
    size_type    m_begin;
    size_type    m_end;
    size_type    m_size;

    Mutex        m_mutex;
    Condition    m_not_empty;
    Condition    m_not_full;
};





template <class T>
BoundedMessageQueue<T>::BoundedMessageQueue(const size_type capacity) :
    m_buffer(capacity),
    m_begin(0),
    m_end(0),
    m_size(0)
{

}



template <class T>
BoundedMessageQueue<T>::~BoundedMessageQueue()
{

}



template <class T>
void BoundedMessageQueue<T>::clear()
{
    {
        ScopedLock lock(m_mutex);

        m_buffer.clear();
        m_begin = 0;
        m_end = 0;
        m_size = 0;
    }

    m_not_full.broadcast();
}



template <class T>
template <class Destructor>
void BoundedMessageQueue<T>::flush(const Destructor destructor)
{
    {
        ScopedLock lock(m_mutex);

        while (! isEmpty())
        {
            value_type value = unsafePop();
            destructor(value);
        }

        m_begin = 0;
        m_end = 0;
        m_size = 0;
    }

    m_not_full.broadcast();
}



template <class T>
void BoundedMessageQueue<T>::push(const value_type & value)
{
    {
        ScopedLock lock(m_mutex);

        while (isFull())
            m_not_full.wait(&m_mutex);

        unsafePush(value);
    }

    m_not_empty.signal();
}



template <class T>
bool BoundedMessageQueue<T>::tryPush(const value_type & value)
{
    {
        ScopedLock lock(m_mutex);

        if (isFull())
            return false;

        unsafePush(value);
    }

    m_not_empty.signal();

    return true;
}



template <class T>
bool BoundedMessageQueue<T>::timedPush(const value_type & value, const unsigned long ms)
{
    // We don't wait in a loop to avoid an infinite loop (as the ms timeout would not be decremented).
    // This means that timedPush() could return false before the timeout has been hit.

    {
        ScopedLock lock(m_mutex);

        if (isFull())
            m_not_full.wait(&m_mutex, ms);

        if (isFull())
            return false;

        unsafePush(value);
    }

    m_not_empty.signal();

    return true;
}



template <class T>
typename BoundedMessageQueue<T>::value_type BoundedMessageQueue<T>::pop()
{
    value_type value;

    {
        ScopedLock lock(m_mutex);

        while (isEmpty())
            m_not_empty.wait(&m_mutex);

        value = unsafePop();
    }

    m_not_full.signal();

    return value;
}



template <class T>
typename BoundedMessageQueue<T>::value_type BoundedMessageQueue<T>::tryPop(bool & is_empty)
{
    value_type value;

    {
        ScopedLock lock(m_mutex);

        is_empty = isEmpty();

        if (is_empty)
            return value_type();

        value = unsafePop();
    }

    m_not_full.signal();

    return value;
}



template <class T>
typename BoundedMessageQueue<T>::value_type BoundedMessageQueue<T>::timedPop(bool & is_empty, const unsigned long ms)
{
    value_type value;

    {
        ScopedLock lock(m_mutex);

        // We don't wait in a loop to avoid an infinite loop (as the ms timeout would not be decremented).
        // This means that timedPop() could return with (is_empty = true) before the timeout has been hit.

        if (isEmpty())
            m_not_empty.wait(&m_mutex, ms);

        is_empty = isEmpty();

        if (is_empty)
            return value_type();

        value = unsafePop();
    }

    m_not_full.signal();

    return value;
}



template <class T>
inline bool BoundedMessageQueue<T>::isFull() const
{
    return m_size == m_buffer.size();
}



template <class T>
inline bool BoundedMessageQueue<T>::isEmpty() const
{
    return m_size == 0;
}



template <class T>
inline void BoundedMessageQueue<T>::unsafePush(const value_type & value)
{
    // Note: this shall never be called if the queue is full.
    assert(! isFull());    

    m_buffer[m_end++] = value;

    if (m_end == m_buffer.size())
        m_end = 0;

    ++m_size;
}
    


template <class T>
inline typename BoundedMessageQueue<T>::value_type BoundedMessageQueue<T>::unsafePop()
{
    // Note: this shall never be called if the queue is empty.
    assert(! isEmpty());

    const size_t pos = m_begin;

    ++m_begin;
    --m_size;

    if (m_begin == m_buffer.size())
        m_begin = 0;

    return m_buffer[pos];
}



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_BOUNDED_MESSAGE_QUEUE_H
