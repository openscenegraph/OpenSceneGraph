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


BlockAndFlushOperation::BlockAndFlushOperation():
    Operation("Block",false)
{
    reset();
}

void BlockAndFlushOperation::release()
{
    Block::release();
}

void BlockAndFlushOperation::operator () (Object*)
{
    glFlush();
    Block::release();
}
