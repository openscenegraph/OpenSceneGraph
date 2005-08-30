/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield
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

struct BlockOperation : public GraphicsThread::Operation, public Block
{
    BlockOperation():
        GraphicsThread::Operation("Block",false)
    {
        reset();
    }

    virtual void operator () (GraphicsContext*)
    {
        glFlush();
        release();
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

int GraphicsThread::cancel()
{
    osg::notify(osg::INFO)<<"Cancelling graphics thread"<<std::endl;

    int result = 0;
    if( isRunning() )
    {
    
        _done = true;

        // cancel the thread..
        result = Thread::cancel();
        //join();

        // release the frameBlock and _databasePagerThreadBlock incase its holding up thread cancelation.
        _operationsBlock->release();

        // then wait for the the thread to stop running.
        while(isRunning())
        {
            // commenting out debug info as it was cashing crash on exit, presumable
            // due to osg::notify or std::cout destructing earlier than this destructor.
            osg::notify(osg::INFO)<<"Waiting for GraphicsThread to cancel"<<std::endl;
            OpenThreads::Thread::YieldCurrentThread();
        }
    }

    return result;
}

void GraphicsThread::add(Operation* operation, bool waitForCompletion)
{
    osg::notify(osg::INFO)<<"Doing add"<<std::endl;

    BlockOperation* block = 0;

    {
        // aquire the lock on the operations queue to prevent anyone else for modifying it at the same time
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);

        // add the operation to the end of the list
        _operations.push_back(operation);

        if (waitForCompletion)
        {
            block = new BlockOperation;
            _operations.push_back(block);
        }
        
        _operationsBlock->set(true);
    }
    
    if (block)
    {
        // now we wait till the barrier is joined by the graphics thread.
        block->block();
    }
}

void GraphicsThread::run()
{
    // make the graphics context current.
    if (_graphicsContext)
    {
        if (!_graphicsContext->isRealized())
        {
            _graphicsContext->realize();
        }
    
        osg::notify(osg::INFO)<<"Doing make current"<<std::endl;
        _graphicsContext->makeCurrent();
    }

    osg::notify(osg::INFO)<<"Doing run"<<std::endl;

    bool firstTime = true;

    do
    {
        // osg::notify(osg::NOTICE)<<"In main loop"<<std::endl;

        if (_operations.empty())
        {
            _operationsBlock->block();
        }

        // osg::notify(osg::NOTICE)<<"get op"<<std::endl;

        ref_ptr<Operation> operation;
    
        // get the front of the file request list.
        {
            OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_operationsMutex);
            if (!_operations.empty())
            {
                // get the front the queue
                operation = _operations.front();
                
                // remove it from the opeations queue
                _operations.erase(_operations.begin());
                
                if (_operations.empty())
                {
                    _operationsBlock->set(false);
                }

            }
            
        }
        
        if (operation.valid())
        {
            // osg::notify(osg::NOTICE)<<"Doing op"<<std::endl;

            // call the graphics operation.
            (*operation)(_graphicsContext);
        }

        if (firstTime)
        {
            // do a yield to get round a peculiar thread hang when testCancel() is called 
            // in certain cirumstances - of which there is no particular pattern.
            YieldCurrentThread();
            firstTime = false;
        }

    } while (!testCancel() && !_done);

    osg::notify(osg::NOTICE)<<"exit loop"<<std::endl;

    // release the graphics context so that others can aquire it.
    if (_graphicsContext) _graphicsContext->releaseContext();

}
 
void SwapBuffersOperation::operator () (GraphicsContext* context)
{
    if (context)
    {
        context->swapBuffersImplementation();
    }
}

void BarrierOperation::operator () (GraphicsContext*)
{
    if (_preBlockOp==GL_FLUSH) glFlush();
    if (_preBlockOp==GL_FINISH) glFinish();
    
    block();
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
