
#ifndef HEADER_GUARD_OSGFFMPEG_MESSAGE_QUEUE_H
#define HEADER_GUARD_OSGFFMPEG_MESSAGE_QUEUE_H

#include <OpenThreads/Condition>
#include <OpenThreads/Mutex>
#include <OpenThreads/ScopedLock>

#include <deque>



namespace osgFFmpeg {



template <class T>
class MessageQueue
{
public:

    typedef T value_type;
    typedef size_t size_type;

    MessageQueue();
    ~MessageQueue();

    void clear();

    void push(const T & value);

    value_type pop();
    value_type tryPop(bool & is_empty);
    value_type timedPop(bool & is_empty, unsigned long ms);

private:

    MessageQueue(const MessageQueue &);
    MessageQueue & operator = (const MessageQueue &);

    typedef std::deque<T> Queue;
    typedef OpenThreads::Condition Condition;
    typedef OpenThreads::Mutex Mutex;
    typedef OpenThreads::ScopedLock<Mutex> ScopedLock;

    Mutex        m_mutex;
    Condition    m_not_empty;
    Queue        m_queue;
};





template <class T>
MessageQueue<T>::MessageQueue()
{

}



template <class T>
MessageQueue<T>::~MessageQueue()
{

}



template <class T>
void MessageQueue<T>::clear()
{
    ScopedLock lock(m_mutex);

    m_queue.clear();
}



template <class T>
void MessageQueue<T>::push(const T & value)
{
    {
        ScopedLock lock(m_mutex);
        m_queue.push_back(value);
    }

    m_not_empty.signal();
}



template <class T>
typename MessageQueue<T>::value_type MessageQueue<T>::pop()
{
    ScopedLock lock(m_mutex);

    while (m_queue.empty())
        m_not_empty.wait(&m_mutex);

    const value_type value = m_queue.front();
    m_queue.pop_front();

    return value;
}



template <class T>
typename MessageQueue<T>::value_type MessageQueue<T>::tryPop(bool & is_empty)
{
    ScopedLock lock(m_mutex);

    is_empty = m_queue.empty();

    if (is_empty)
        return value_type();

    const value_type value = m_queue.front();
    m_queue.pop_front();

    return value;
}



template <class T>
typename MessageQueue<T>::value_type MessageQueue<T>::timedPop(bool & is_empty, const unsigned long ms)
{
    ScopedLock lock(m_mutex);

    // We don't wait in a loop to avoid an infinite loop (as the ms timeout would not be decremented).
    // This means that timedPop() could return with (is_empty = true) before the timeout has been hit.

    if (m_queue.empty())
        m_not_empty.wait(&m_mutex, ms);

    is_empty = m_queue.empty();

    if (is_empty)
        return value_type();

    const value_type value = m_queue.front();
    m_queue.pop_front();

    return value;
}



} // namespace osgFFmpeg



#endif // HEADER_GUARD_OSGFFMPEG_MESSAGE_QUEUE_H
