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


#include <osg/GraphicsThread>
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

OperationQueue::OperationQueue()
{
}

OperationQueue::~OperationQueue()
{
}

void OperationQueue::add(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    {
        // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

        // add the operation to the end of the list
        _operations.push_back(operation);
        
        _operationsBlock->set(true);
    }
}

void OperationQueue::remove(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)==operation) itr = _operations.erase(itr);
        else ++itr;
    }
}

void OperationQueue::remove(const std::string& name)
{
    osg::notify(osg::INFO)<<"Doing remove named operation"<<std::endl;
    
    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specificed name
    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)->getName()==name) itr = _operations.erase(itr);
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

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}


/////////////////////////////////////////////////////////////////////////////
//
//  OperationsThread
//

OperationsThread::OperationsThread():
    _parent(0),
    _done(false)
{
    _operationsBlock = new RefBlock;
}

OperationsThread::~OperationsThread()
{
    //osg::notify(osg::NOTICE)<<"Destructing graphics thread "<<this<<std::endl;

    cancel();

    //osg::notify(osg::NOTICE)<<"Done Destructing graphics thread "<<this<<std::endl;
}

void OperationsThread::setDone(bool done)
{
    if (_done==done) return;

    _done = true;

    if (done)
    {
        osg::notify(osg::INFO)<<"set done "<<this<<std::endl;
        
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            if (_currentOperation.valid())
            {
                osg::notify(osg::INFO)<<"releasing "<<_currentOperation.get()<<std::endl;
                _currentOperation->release();
            }
        }

        _operationsBlock->release();
    }

}

int OperationsThread::cancel()
{
    osg::notify(osg::INFO)<<"Cancelling OperationsThred "<<this<<" isRunning()="<<isRunning()<<std::endl;

    int result = 0;
    if( isRunning() )
    {
    
        _done = true;

        osg::notify(osg::INFO)<<"   Doing cancel "<<this<<std::endl;

        for(Operations::iterator itr = _operations.begin();
            itr != _operations.end();
            ++itr)
        {
            (*itr)->release();
        }
        
        // release the frameBlock and _databasePagerThreadBlock incase its holding up thread cancelation.
        _operationsBlock->release();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            _operationsBlock->release();

            {
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

                for(Operations::iterator itr = _operations.begin();
                    itr != _operations.end();
                    ++itr)
                {
                    (*itr)->release();
                }

                if (_currentOperation.valid()) _currentOperation->release();
            }

            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            osg::notify(osg::INFO)<<"   Waiting for OperationsThread to cancel "<<this<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    osg::notify(osg::INFO)<<"  OperationsThread::cancel() thread cancelled "<<this<<" isRunning()="<<isRunning()<<std::endl;

    return result;
}

void OperationsThread::add(Operation* operation, bool waitForCompletion)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    ref_ptr<BlockOperation> block = 0;

    {
        // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

        // add the operation to the end of the list
        _operations.push_back(operation);

        if (waitForCompletion)
        {
            block = new BlockOperation;
            _operations.push_back(block.get());
        }
        
        _operationsBlock->set(true);
    }
    
    if (block.valid())
    {
        // now we wait till the barrier is joined by the graphics thread.
        block->block();
    }
}

void OperationsThread::remove(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)==operation) itr = _operations.erase(itr);
        else ++itr;
    }
}

void OperationsThread::remove(const std::string& name)
{
    osg::notify(osg::INFO)<<"Doing remove named operation"<<std::endl;
    
    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specificed name
    for(Operations::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)->getName()==name) itr = _operations.erase(itr);
        else ++itr;
    }

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}

void OperationsThread::removeAllOperations()
{
    osg::notify(osg::INFO)<<"Doing remove all operations"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    _operations.clear();

    if (_operations.empty())
    {
        _operationsBlock->set(false);
    }
}


void OperationsThread::run()
{
    // make the graphics context current.
    GraphicsContext* graphicsContext = dynamic_cast<GraphicsContext*>(_parent.get());
    if (graphicsContext)
    {        
        graphicsContext->makeCurrent();
    }

    osg::notify(osg::INFO)<<"Doing run "<<this<<" isRunning()="<<isRunning()<<std::endl;

    bool firstTime = true;

    Operations::iterator itr = _operations.begin();

    do
    {
        // osg::notify(osg::NOTICE)<<"In main loop "<<this<<std::endl;

        if (_operations.empty())
        {
            _operationsBlock->block();
            
            // exit from loop if _done is set.
            if (_done) break;
            
            itr = _operations.begin();
        }
        else
        {
            if  (itr == _operations.end()) itr = _operations.begin();
        }

        // osg::notify(osg::INFO)<<"get op "<<_done<<" "<<this<<std::endl;

        // get the front of the file request list.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            if (!_operations.empty())
            {
                // get the next item
                _currentOperation = *itr;
                
                if (!_currentOperation->getKeep())
                {
                    // osg::notify(osg::INFO)<<"removing "<<_currentOperation->getName()<<std::endl;

                    // remove it from the opeations queue
                    itr = _operations.erase(itr);

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
                    ++itr;
                }
                

            }
            
        }
        
        if (_currentOperation.valid())
        {
            // osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

            // call the graphics operation.
            (*_currentOperation)(_parent.get());

            {            
                OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
                _currentOperation = 0;
            }
        }

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain cirumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }
        
        // osg::notify(osg::NOTICE)<<"operations.size()="<<_operations.size()<<" done="<<_done<<" testCancel()"<<testCancel()<<std::endl;

    } while (!testCancel() && !_done);

    if (graphicsContext)
    {    
        graphicsContext->releaseContext();
    }

    osg::notify(osg::INFO)<<"exit loop "<<this<<" isRunning()="<<isRunning()<<std::endl;

}
 
void SwapBuffersOperation::operator () (Object* object)
{
    GraphicsContext* context = dynamic_cast<GraphicsContext*>(object);
    if (context)
    {
        context->swapBuffersImplementation();
        context->clear();
    }
}

void BarrierOperation::release()
{
    Barrier::release();
}

void BarrierOperation::operator () (Object*)
{
    if (_preBlockOp==GL_FLUSH) glFlush();
    if (_preBlockOp==GL_FINISH) glFinish();
    
    block();
}

void ReleaseContext_Block_MakeCurrentOperation::release()
{
    Block::release();
}


void ReleaseContext_Block_MakeCurrentOperation::operator () (Object* object)
{
    GraphicsContext* context = dynamic_cast<GraphicsContext*>(object);
    if (!context) return;
    
    // release the graphics context.
    context->releaseContext();
    
    // reset the block so that it the next call to block() 
    reset();
    
    // block this thread, untill the block is released externally.
    block();
    
    // re aquire the graphcis context.
    context->makeCurrent();
}
