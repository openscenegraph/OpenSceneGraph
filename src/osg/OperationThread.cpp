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


#include <osg/OperationThread>
#include <osg/GraphicsContext>
#include <osg/Notify>

using namespace osg;
using namespace OpenThreads;

struct BlockOperation : public Operation, public Block
{
    BlockOperation():
        Operation("Block",false)
    {
        reset();
    }

    virtual void release()
    {
        Block::release();
    }

    virtual void operator () (Object*)
    {
        glFlush();
        Block::release();
    }
};

/////////////////////////////////////////////////////////////////////////////
//
//  OperationsQueue
//

OperationQueue::OperationQueue():
    osg::Referenced(true)
{
    _currentOperationIterator = _operations.begin();
    _operationsBlock = new RefBlock;
}

OperationQueue::~OperationQueue()
{
}

ref_ptr<Operation> OperationQueue::getNextOperation(bool blockIfEmpty)
{
    if (blockIfEmpty && _operations.empty())
    {
        _operationsBlock->block();
    }

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    
    if (_operations.empty()) return osg::ref_ptr<Operation>();
    
    if (_currentOperationIterator == _operations.end())
    {
        // iterator at end of operations so reset to beginning.
        _currentOperationIterator = _operations.begin();
    }
    
    ref_ptr<Operation> currentOperation = *_currentOperationIterator;

    if (!currentOperation->getKeep())
    {
        // osg::notify(osg::INFO)<<"removing "<<currentOperation->getName()<<std::endl;

        // remove it from the operations queue
        _currentOperationIterator = _operations.erase(_currentOperationIterator);

        // osg::notify(osg::INFO)<<"size "<<_operations.size()<<std::endl;

        if (_operations.empty())
        {
           // osg::notify(osg::INFO)<<"setting block "<<_operations.size()<<std::endl;
           _operationsBlock->set(false);
        }
    }
    else
    {
        // osg::notify(osg::INFO)<<"increment "<<_currentOperation->getName()<<std::endl;

        // move on to the next operation in the list.
        ++_currentOperationIterator;
    }

    return currentOperation;
}

void OperationQueue::add(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // add the operation to the end of the list
    _operations.push_back(operation);

    _operationsBlock->set(true);
}

void OperationQueue::remove(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)==operation)
        {
            bool needToResetCurrentIterator = (_currentOperationIterator == itr);
            
            itr = _operations.erase(itr);
            
            if (needToResetCurrentIterator) _currentOperationIterator = itr;
            
        }
        else ++itr;
    }
}

void OperationQueue::remove(const std::string& name)
{
    osg::notify(osg::INFO)<<"Doing remove named operation"<<std::endl;
    
    // acquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specified name
    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)->getName()==name) 
        {
            bool needToResetCurrentIterator = (_currentOperationIterator == itr);
            
            itr = _operations.erase(itr);
            
            if (needToResetCurrentIterator) _currentOperationIterator = itr;
        }
        else ++itr;
    }

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void OperationQueue::removeAllOperations()
{
    osg::notify(osg::INFO)<<"Doing remove all operations"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    
    _operations.clear();
    
    // reset current operator.
    _currentOperationIterator = _operations.begin();

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void OperationQueue::runOperations(Object* callingObject)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // reset current operation iterator to beginning if at end.
    if (_currentOperationIterator==_operations.end()) _currentOperationIterator = _operations.begin();
    
    for(;
        _currentOperationIterator != _operations.end();
        )
    {
        ref_ptr<Operation> operation = *_currentOperationIterator;

        if (!operation->getKeep())
        {
            _currentOperationIterator = _operations.erase(_currentOperationIterator);
        }
        else
        {
            ++_currentOperationIterator;
        }
                
        // osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

        // call the graphics operation.
        (*operation)(callingObject);
    }

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void OperationQueue::releaseOperationsBlock()
{
    _operationsBlock->release();
}
 
 void OperationQueue::releaseAllOperations()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    
    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();
        ++itr)
    {
        (*itr)->release();
    }
}


void OperationQueue::addOperationThread(OperationThread* thread)
{
    _operationThreads.insert(thread);
}

void OperationQueue::removeOperationThread(OperationThread* thread)
{
    _operationThreads.erase(thread);
}

/////////////////////////////////////////////////////////////////////////////
//
//  OperationThread
//

OperationThread::OperationThread():
    osg::Referenced(true),
    _parent(0),
    _done(false)
{
    setOperationQueue(new OperationQueue);
}

OperationThread::~OperationThread()
{
    //osg::notify(osg::NOTICE)<<"Destructing graphics thread "<<this<<std::endl;

    cancel();

    //osg::notify(osg::NOTICE)<<"Done Destructing graphics thread "<<this<<std::endl;
}

void OperationThread::setOperationQueue(OperationQueue* opq)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);

    if (_operationQueue == opq) return;

    if (_operationQueue.valid()) _operationQueue->removeOperationThread(this);

    _operationQueue = opq;

    if (_operationQueue.valid()) _operationQueue->addOperationThread(this);
}

void OperationThread::setDone(bool done)
{
    if (_done==done) return;

    _done = true;

    if (done)
    {
        osg::notify(osg::INFO)<<"set done "<<this<<std::endl;
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
            if (_currentOperation.valid())
            {
                osg::notify(osg::INFO)<<"releasing "<<_currentOperation.get()<<std::endl;
                _currentOperation->release();
            }
        }

        if (_operationQueue.valid()) _operationQueue->releaseOperationsBlock();
    }
}

int OperationThread::cancel()
{
    osg::notify(osg::INFO)<<"Cancelling OperationThread "<<this<<" isRunning()="<<isRunning()<<std::endl;

    int result = 0;
    if( isRunning() )
    {
    
        _done = true;

        osg::notify(osg::INFO)<<"   Doing cancel "<<this<<std::endl;

        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);

            if (_operationQueue.valid()) 
            {
                 _operationQueue->releaseOperationsBlock();
                //_operationQueue->releaseAllOperations();
            }

            if (_currentOperation.valid()) _currentOperation->release();
        }
        
        // then wait for the the thread to stop running.
        while(isRunning())
        {

#if 1
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);

                if (_operationQueue.valid()) 
                {
                    _operationQueue->releaseOperationsBlock();
                    // _operationQueue->releaseAllOperations();
                }

                if (_currentOperation.valid()) _currentOperation->release();
            }
#endif
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            osg::notify(osg::DEBUG_INFO)<<"   Waiting for OperationThread to cancel "<<this<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    osg::notify(osg::INFO)<<"  OperationThread::cancel() thread cancelled "<<this<<" isRunning()="<<isRunning()<<std::endl;

    return result;
}

void OperationThread::add(Operation* operation)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
    if (!_operationQueue) _operationQueue = new OperationQueue;
    _operationQueue->add(operation);
}

void OperationThread::remove(Operation* operation)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
    if (_operationQueue.valid()) _operationQueue->remove(operation);
}

void OperationThread::remove(const std::string& name)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
    if (_operationQueue.valid()) _operationQueue->remove(name);
}

void OperationThread::removeAllOperations()
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
    if (_operationQueue.valid()) _operationQueue->removeAllOperations();
}

void OperationThread::run()
{
    osg::notify(osg::INFO)<<"Doing run "<<this<<" isRunning()="<<isRunning()<<std::endl;

    bool firstTime = true;

    do
    {
        // osg::notify(osg::NOTICE)<<"In thread loop "<<this<<std::endl;
        ref_ptr<Operation> operation;
        ref_ptr<OperationQueue> operationQueue;
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
            operationQueue = _operationQueue;
        }

        operation = operationQueue->getNextOperation(true);

        if (_done) break;

        if (operation.valid())
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
                _currentOperation = operation;
            }

            // osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

            // call the graphics operation.
            (*operation)(_parent.get());

            {            
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_threadMutex);
                _currentOperation = 0;
            }
        }

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain circumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }
        
        // osg::notify(osg::NOTICE)<<"operations.size()="<<_operations.size()<<" done="<<_done<<" testCancel()"<<testCancel()<<std::endl;

    } while (!testCancel() && !_done);

    osg::notify(osg::INFO)<<"exit loop "<<this<<" isRunning()="<<isRunning()<<std::endl;

}
