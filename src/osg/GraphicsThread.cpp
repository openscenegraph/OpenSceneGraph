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

struct ThreadExitTidyUp
{
    ThreadExitTidyUp(osg::GraphicsContext* context, bool closeContextOnExit):
        _context(context),
        _closeContextOnExit(closeContextOnExit)
    {
        osg::notify(osg::INFO)<<"starting thread context "<<_context<<std::endl;
    }

    ~ThreadExitTidyUp()
    {
        osg::notify(osg::INFO)<<"exit thread"<<std::endl;
        if (_context)
        {
            if (_closeContextOnExit)
            {
                osg::notify(osg::INFO)<<"    - close context "<<_context<<std::endl;

                _context->closeImplementation();

                osg::notify(osg::INFO)<<"    - done close context "<<_context<<std::endl;
            }
            else
            {
                osg::notify(osg::INFO)<<"    - releaseContext "<<_context<<std::endl;

                //_context->releaseContext();
            }
        }
    }
    
    osg::GraphicsContext* _context;
    bool _closeContextOnExit;
    
    
};

struct BlockOperation : public GraphicsThread::Operation, public Block
{
    BlockOperation():
        GraphicsThread::Operation("Block",false)
    {
        reset();
    }

    virtual void release()
    {
        Block::release();
    }

    virtual void operator () (GraphicsContext*)
    {
        glFlush();
        Block::release();
    }
};


GraphicsThread::GraphicsThread():
    _graphicsContext(0),
    _done(false)
{
    _operationsBlock = new Block;
}

GraphicsThread::~GraphicsThread()
{
    osg::notify(osg::INFO)<<"Destructing graphics thread"<<std::endl;

    cancel();
}

void GraphicsThread::setDone(bool done)
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

int GraphicsThread::cancel()
{
    osg::notify(osg::INFO)<<"Cancelling graphics thread "<<this<<std::endl;

    int result = 0;
    if( isRunning() )
    {
    
        _done = true;

        osg::notify(osg::INFO)<<"   Doing cancel "<<this<<std::endl;

        for(OperationQueue::iterator itr = _operations.begin();
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

                for(OperationQueue::iterator itr = _operations.begin();
                    itr != _operations.end();
                    ++itr)
                {
                    (*itr)->release();
                }

                if (_currentOperation.valid()) _currentOperation->release();
            }

            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            osg::notify(osg::INFO)<<"   Waiting for GraphicsThread to cancel "<<this<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    return result;
}

void GraphicsThread::add(Operation* operation, bool waitForCompletion)
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

void GraphicsThread::remove(Operation* operation)
{
    osg::notify(osg::INFO)<<"Doing remove operation"<<std::endl;

    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    for(OperationQueue::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)==operation) itr = _operations.erase(itr);
        else ++itr;
    }
}

void GraphicsThread::remove(const std::string& name)
{
    osg::notify(osg::INFO)<<"Doing remove named operation"<<std::endl;
    
    // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

    // find the remove all operations with specificed name
    for(OperationQueue::iterator itr = _operations.begin();
        itr!=_operations.end();)
    {
        if ((*itr)->getName()==name) itr = _operations.erase(itr);
        else ++itr;
    }
}

void GraphicsThread::removeAllOperations()
{
    osg::notify(osg::INFO)<<"Doing remove all operations"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
    _operations.clear();
}


void GraphicsThread::run()
{
    bool contextRealizedInThisThread = false;

    // make the graphics context current.
    if (_graphicsContext)
    {
        if (!_graphicsContext->isRealized())
        {
            _graphicsContext->realize();
            contextRealizedInThisThread = true;
        }
    
        osg::notify(osg::INFO)<<"Doing make current"<<std::endl;
        _graphicsContext->makeCurrent();
    }

    // create a local object to clean up once the thread is cancelled.
    ThreadExitTidyUp threadExitTypeUp(_graphicsContext, contextRealizedInThisThread);

    osg::notify(osg::INFO)<<"Doing run"<<std::endl;

    bool firstTime = true;

    OperationQueue::iterator itr = _operations.begin();

    do
    {
        osg::notify(osg::INFO)<<"In main loop "<<this<<std::endl;

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

        osg::notify(osg::INFO)<<"get op "<<_done<<" "<<this<<std::endl;

        // get the front of the file request list.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            if (!_operations.empty())
            {
                // get the next item
                _currentOperation = *itr;
                
                if (!_currentOperation->getKeep())
                {
                    osg::notify(osg::INFO)<<"removing "<<_currentOperation->getName()<<std::endl;

                    // remove it from the opeations queue
                    itr = _operations.erase(itr);

                    osg::notify(osg::INFO)<<"size "<<_operations.size()<<std::endl;

                    if (_operations.empty())
                    {
                       osg::notify(osg::INFO)<<"setting block "<<_operations.size()<<std::endl;
                       _operationsBlock->set(false);
                    }
                }
                else
                {
                    osg::notify(osg::INFO)<<"increment "<<_currentOperation->getName()<<std::endl;

                    // move on to the next operation in the list.
                    ++itr;
                }
                

            }
            
        }
        
        if (_currentOperation.valid())
        {
            osg::notify(osg::INFO)<<"Doing op "<<_currentOperation->getName()<<" "<<this<<std::endl;

            // call the graphics operation.
            (*_currentOperation)(_graphicsContext);

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

    } while (!testCancel() && !_done);

    osg::notify(osg::INFO)<<"exit loop "<<this<<std::endl;

}
 
void SwapBuffersOperation::operator () (GraphicsContext* context)
{
    if (context)
    {
        context->swapBuffersImplementation();
    }
}

void BarrierOperation::release()
{
    Barrier::release();
}

void BarrierOperation::operator () (GraphicsContext*)
{
    if (_preBlockOp==GL_FLUSH) glFlush();
    if (_preBlockOp==GL_FINISH) glFinish();
    
    block();
}

void ReleaseContext_Block_MakeCurrentOperation::release()
{
    Block::release();
}


void ReleaseContext_Block_MakeCurrentOperation::operator () (GraphicsContext* context)
{
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
