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
#include <osgUtil/GLObjectsVisitor>
#include <osg/Drawable>
#include <osg/Notify>
#include <OpenThreads/ScopedLock>

using namespace osg;
using namespace osgUtil;


/////////////////////////////////////////////////////////////////
//
// GLObjectsVisitor
//
GLObjectsVisitor::GLObjectsVisitor(Mode mode)
{
    setTraversalMode(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN);

    _mode = mode;

}


void GLObjectsVisitor::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    traverse(node);
}

void GLObjectsVisitor::apply(osg::Geode& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    for(unsigned int i=0;i<node.getNumDrawables();++i)
    {
        Drawable* drawable = node.getDrawable(i);
        if (drawable)
        {
            apply(*drawable);
            if (drawable->getStateSet())
            {
                apply(*(drawable->getStateSet()));
            }
        }
    }
}

void GLObjectsVisitor::apply(osg::Drawable& drawable)
{
    if (_drawablesAppliedSet.count(&drawable)!=0) return;
    
    _drawablesAppliedSet.insert(&drawable);

    if (_mode&SWITCH_OFF_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(false);
    }

    if (_mode&SWITCH_ON_DISPLAY_LISTS)
    {
        drawable.setUseDisplayList(true);
    }

    if (_mode&COMPILE_DISPLAY_LISTS && _renderInfo.getState())
    {
        drawable.compileGLObjects(_renderInfo);
    }

    if (_mode&RELEASE_DISPLAY_LISTS)
    {
        drawable.releaseGLObjects(_renderInfo.getState());
    }

    if (_mode&SWITCH_ON_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(true);
    }

    if (_mode&SWITCH_OFF_VERTEX_BUFFER_OBJECTS)
    {
        drawable.setUseVertexBufferObjects(false);
    }
}

void GLObjectsVisitor::apply(osg::StateSet& stateset)
{
    if (_stateSetAppliedSet.count(&stateset)!=0) return;
    
    _stateSetAppliedSet.insert(&stateset);

    if (_mode & COMPILE_STATE_ATTRIBUTES && _renderInfo.getState())
    {
        stateset.compileGLObjects(*_renderInfo.getState());
        
        osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
        if (program) _lastCompiledProgram = program;

        if (_lastCompiledProgram.valid() && !stateset.getUniformList().empty())
        {
            osg::Program::PerContextProgram* pcp = _lastCompiledProgram->getPCP(_renderInfo.getState()->getContextID());
            if (pcp)
            {
                pcp->useProgram();
                
                _renderInfo.getState()->setLastAppliedProgramObject(pcp);
            
                osg::StateSet::UniformList& ul = stateset.getUniformList();
                for(osg::StateSet::UniformList::iterator itr = ul.begin();
                    itr != ul.end();
                    ++itr)
                {
                    pcp->apply(*(itr->second.first));
                }
            }
        }
        else if(_renderInfo.getState()->getLastAppliedProgramObject()){
                            
            GL2Extensions* extensions = GL2Extensions::Get(_renderInfo.getState()->getContextID(), true);
            extensions->glUseProgram(0);
            _renderInfo.getState()->setLastAppliedProgramObject(0);
        }
        
    }

    if (_mode & RELEASE_STATE_ATTRIBUTES)
    {
        stateset.releaseGLObjects(_renderInfo.getState());
    }
    
    if (_mode & CHECK_BLACK_LISTED_MODES)
    {
        stateset.checkValidityOfAssociatedModes(*_renderInfo.getState());
    }
}

/////////////////////////////////////////////////////////////////
//
// GLObjectsVisitor
//

GLObjectsOperation::GLObjectsOperation(GLObjectsVisitor::Mode mode):
    osg::GraphicsOperation("GLObjectOperation",false),
    _mode(mode)
{
}

GLObjectsOperation::GLObjectsOperation(osg::Node* subgraph, GLObjectsVisitor::Mode mode):
    osg::GraphicsOperation("GLObjectOperation",false),
    _subgraph(subgraph),
    _mode(mode)
{
}

void GLObjectsOperation::operator () (osg::GraphicsContext* context)
{
    GLObjectsVisitor glObjectsVisitor(_mode);
    
    context->getState()->initializeExtensionProcs();

    glObjectsVisitor.setState(context->getState());
    
    // osg::notify(osg::NOTICE)<<"GLObjectsOperation::before <<<<<<<<<<<"<<std::endl;
    if (_subgraph.valid())
    {
        _subgraph->accept(glObjectsVisitor);
    }
    else
    {
        for(osg::GraphicsContext::Cameras::iterator itr = context->getCameras().begin();
            itr != context->getCameras().end();
            ++itr)
        {
            (*itr)->accept(glObjectsVisitor);
        }
    }
    // osg::notify(osg::NOTICE)<<"GLObjectsOperation::after >>>>>>>>>>> "<<std::endl;
}

/////////////////////////////////////////////////////////////////
//
// IncrementalCompileOperation
//
IncrementalCompileOperation::IncrementalCompileOperation():
    osg::GraphicsOperation("IncrementalCompileOperation",true)
{
}

IncrementalCompileOperation::~IncrementalCompileOperation()
{
}

void IncrementalCompileOperation::assignContexts(Contexts& contexts)
{
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        osg::GraphicsContext* gc = *itr;
        addGraphicsContext(gc);
    }
}

void IncrementalCompileOperation::removeContexts(Contexts& contexts)
{
    for(Contexts::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        osg::GraphicsContext* gc = *itr;
        removeGraphicsContext(gc);
    }
}


void IncrementalCompileOperation::addGraphicsContext(osg::GraphicsContext* gc)
{
    if (_contexts.count(gc)==0)
    {
        gc->add(this);
        _contexts.insert(gc);
    }
}

void IncrementalCompileOperation::removeGraphicsContext(osg::GraphicsContext* gc)
{
    if (_contexts.count(gc)!=0)
    {
        gc->remove(this);
        _contexts.erase(gc);
    }
}

void IncrementalCompileOperation::add(osg::Node* subgraphToCompile)
{
    osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::add("<<subgraphToCompile<<")"<<std::endl;
    add(new CompileSet(subgraphToCompile));
}

void IncrementalCompileOperation::add(osg::Group* attachmentPoint, osg::Node* subgraphToCompile)
{
    osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::add("<<attachmentPoint<<", "<<subgraphToCompile<<")"<<std::endl;
    add(new CompileSet(attachmentPoint, subgraphToCompile));
}


void IncrementalCompileOperation::add(CompileSet* compileSet, bool callBuildCompileMap)
{
    if (!compileSet) return;
    
    if (callBuildCompileMap) compileSet->buildCompileMap(_contexts);

    osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::add(CompileSet = "<<compileSet<<", "<<", "<<callBuildCompileMap<<")"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
    _toCompile.push_back(compileSet);
}

void IncrementalCompileOperation::mergeCompiledSubgraphs()
{
    osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::mergeCompiledSubgraphs()"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  compilded_lock(_compiledMutex);
    
    for(CompileSets::iterator itr = _compiled.begin(); 
        itr != _compiled.end();
        ++itr)
    {
        CompileSet* cs = itr->get();
        if (cs->_attachmentPoint.valid())
        {
            cs->_attachmentPoint->addChild(cs->_subgraphToCompile.get());
        }
    }
    
    _compiled.clear();
}

void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& context)
{
}

void IncrementalCompileOperation::operator () (osg::GraphicsContext* context)
{
    osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::operator () ("<<context<<")"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
    for(CompileSets::iterator itr = _toCompile.begin(); 
        itr != _toCompile.end();
        )
    {
        CompileSet* cs = itr->get();
        CompileMap& cm = cs->_compileMap;
        CompileData* cd = cm[context].get();
        
        if (cd)
        {        
            // compile textures
            cd->_textures.clear();

            // compile drawables
            cd->_drawables.clear();

            // compile programs
            cd->_programs.clear();
        }
                
        if (!cd || cd->empty())
        {
            if (cs->_compileCompletedCallback.valid())
            {
                if (cs->_compileCompletedCallback->compileCompleted(cs))
                {
                    // callback will handle merging of subgraph so no need to place CompileSet in merge.
                }
                else
                {
                    OpenThreads::ScopedLock<OpenThreads::Mutex>  compilded_lock(_compiledMutex);
                    _compiled.push_back(cs);
                }
            }
        
            // remove from toCompileSet;
            itr = _toCompile.erase(itr);
        }
        else
        {
            ++itr;
        }
    }
}
