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
#include <osgUtil/IncrementalCompileOperation>

#include <osg/Drawable>
#include <osg/Notify>
#include <osg/Timer>
#include <osg/GLObjects>

#include <OpenThreads/ScopedLock>

#include <algorithm>

namespace osgUtil 
{

IncrementalCompileOperation::IncrementalCompileOperation():
    osg::GraphicsOperation("IncrementalCompileOperation",true),
    _flushTimeRatio(0.5),
    _conservativeTimeRatio(0.5)
{
    _targetFrameRate = 100.0;
    _minimumTimeAvailableForGLCompileAndDeletePerFrame = 0.001; // 1ms.
    _maximumNumOfObjectsToCompilePerFrame = 20;
    const char* ptr = 0;
    if( (ptr = getenv("OSG_MINIMUM_COMPILE_TIME_PER_FRAME")) != 0)
    {
        _minimumTimeAvailableForGLCompileAndDeletePerFrame = osg::asciiToDouble(ptr);
    }

    if( (ptr = getenv("OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME")) != 0)
    {
        _maximumNumOfObjectsToCompilePerFrame = atoi(ptr);
    }
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
    osg::notify(osg::INFO)<<"IncrementalCompileOperation::add("<<subgraphToCompile<<")"<<std::endl;
    add(new CompileSet(subgraphToCompile));
}

void IncrementalCompileOperation::add(osg::Group* attachmentPoint, osg::Node* subgraphToCompile)
{
    osg::notify(osg::INFO)<<"IncrementalCompileOperation::add("<<attachmentPoint<<", "<<subgraphToCompile<<")"<<std::endl;
    add(new CompileSet(attachmentPoint, subgraphToCompile));
}


void IncrementalCompileOperation::add(CompileSet* compileSet, bool callBuildCompileMap)
{
    if (!compileSet) return;

    if (compileSet->_subgraphToCompile.valid())
    {
        // force a compute of the bound of the subgraph to avoid the update traversal from having to do this work
        // and reducing the change of frame drop.
        compileSet->_subgraphToCompile->getBound();
    }
    
    if (callBuildCompileMap) compileSet->buildCompileMap(_contexts);

    osg::notify(osg::INFO)<<"IncrementalCompileOperation::add(CompileSet = "<<compileSet<<", "<<", "<<callBuildCompileMap<<")"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
    _toCompile.push_back(compileSet);
}

void IncrementalCompileOperation::mergeCompiledSubgraphs()
{
    // osg::notify(osg::INFO)<<"IncrementalCompileOperation::mergeCompiledSubgraphs()"<<std::endl;

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

class IncrementalCompileOperation::CollectStateToCompile : public osg::NodeVisitor
{
public:

    CollectStateToCompile(GLObjectsVisitor::Mode mode):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _mode(mode) {}
    
    GLObjectsVisitor::Mode _mode;

    typedef std::set<osg::Drawable*> DrawableSet;
    typedef std::set<osg::StateSet*> StateSetSet;
    typedef std::set<osg::Texture*> TextureSet;
    typedef std::set<osg::Program*> ProgramSet;

    DrawableSet _drawablesHandled;
    StateSetSet _statesetsHandled;
    
    DrawableSet _drawables;
    TextureSet _textures;
    ProgramSet _programs;

    void apply(osg::Node& node)
    {
        if (node.getStateSet())
        {
            apply(*(node.getStateSet()));
        }

        traverse(node);
    }

    void apply(osg::Geode& node)
    {
        if (node.getStateSet())
        {
            apply(*(node.getStateSet()));
        }

        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
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

    void apply(osg::Drawable& drawable)
    {
        if (_drawablesHandled.count(&drawable)!=0) return;

        _drawablesHandled.insert(&drawable);

        if (_mode&GLObjectsVisitor::SWITCH_OFF_DISPLAY_LISTS)
        {
            drawable.setUseDisplayList(false);
        }

        if (_mode&GLObjectsVisitor::SWITCH_ON_DISPLAY_LISTS)
        {
            drawable.setUseDisplayList(true);
        }

        if (_mode&GLObjectsVisitor::SWITCH_ON_VERTEX_BUFFER_OBJECTS)
        {
            drawable.setUseVertexBufferObjects(true);
        }

        if (_mode&GLObjectsVisitor::SWITCH_OFF_VERTEX_BUFFER_OBJECTS)
        {
            drawable.setUseVertexBufferObjects(false);
        }

        if (_mode&GLObjectsVisitor::COMPILE_DISPLAY_LISTS)
        {
            _drawables.insert(&drawable);
        }

    }

    void apply(osg::StateSet& stateset)
    {
        if (_statesetsHandled.count(&stateset)!=0) return;

        _statesetsHandled.insert(&stateset);

        if (_mode & GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)
        {
            osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
            if (program)
            {
                _programs.insert(program);
            }
            
            osg::StateSet::TextureAttributeList& tal = stateset.getTextureAttributeList();
            for(osg::StateSet::TextureAttributeList::iterator itr = tal.begin();
                itr != tal.end();
                ++itr)
            {
                osg::StateSet::AttributeList& al = *itr;
                osg::StateAttribute::TypeMemberPair tmp(osg::StateAttribute::TEXTURE,0);
                osg::StateSet::AttributeList::iterator texItr = al.find(tmp);
                if (texItr != al.end())
                {
                    osg::Texture* texture = dynamic_cast<osg::Texture*>(texItr->second.first.get());
                    if (texture) _textures.insert(texture);
                }
            }
        }
    }
    
};


void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& contexts, GLObjectsVisitor::Mode mode)
{
    if (contexts.empty() || !_subgraphToCompile) return;
    
    CollectStateToCompile cstc(mode);
    _subgraphToCompile->accept(cstc);
    
    if (cstc._textures.empty() &&  cstc._programs.empty() && cstc._drawables.empty()) return;
    
    for(ContextSet::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        CompileData& cd = _compileMap[*itr];
        std::copy(cstc._textures.begin(), cstc._textures.end(), std::back_inserter<CompileData::Textures>(cd._textures));
        std::copy(cstc._programs.begin(), cstc._programs.end(), std::back_inserter<CompileData::Programs>(cd._programs));
        std::copy(cstc._drawables.begin(), cstc._drawables.end(), std::back_inserter<CompileData::Drawables>(cd._drawables));
    }
    
}

void IncrementalCompileOperation::operator () (osg::GraphicsContext* context)
{
    // osg::notify(osg::NOTICE)<<"IncrementalCompileOperation::operator () ("<<context<<")"<<std::endl;

    osg::NotifySeverity level = osg::INFO;

    double targetFrameRate = _targetFrameRate;
    double minimumTimeAvailableForGLCompileAndDeletePerFrame = _minimumTimeAvailableForGLCompileAndDeletePerFrame;

    double targetFrameTime = 1.0/targetFrameRate;
    
    const osg::FrameStamp* fs = context->getState()->getFrameStamp();
    double currentTime = fs ? fs->getReferenceTime() : 0.0;

    double currentElapsedFrameTime = context->getTimeSinceLastClear();
    
    osg::notify(level)<<"currentTime = "<<currentTime<<std::endl;
    osg::notify(level)<<"currentElapsedFrameTime = "<<currentElapsedFrameTime<<std::endl;
    
    double _flushTimeRatio(0.5);
    double _conservativeTimeRatio(0.5);

    double availableTime = std::max((targetFrameTime - currentElapsedFrameTime)*_conservativeTimeRatio,
                                    minimumTimeAvailableForGLCompileAndDeletePerFrame);

    double flushTime = availableTime * _flushTimeRatio;
    double compileTime = availableTime - flushTime;

#if 1
    osg::notify(level)<<"total availableTime = "<<availableTime*1000.0<<std::endl;
    osg::notify(level)<<"      flushTime     = "<<flushTime*1000.0<<std::endl;
    osg::notify(level)<<"      compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

    osg::flushDeletedGLObjects(context->getState()->getContextID(), currentTime, flushTime);

    // if any time left over from flush add this to compile time.        
    if (flushTime>0.0) compileTime += flushTime;

#if 1
    osg::notify(level)<<"      revised compileTime   = "<<compileTime*1000.0<<std::endl;
#endif


    osg::RenderInfo renderInfo;
    renderInfo.setState(context->getState());


    CompileSets toCompileCopy;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
        std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
    }

    osg::Timer_t startTick = osg::Timer::instance()->tick();

    for(CompileSets::iterator itr = toCompileCopy.begin(); 
        itr != toCompileCopy.end();
        ++itr)
    {
        CompileSet* cs = itr->get();
        CompileMap& cm = cs->_compileMap;
        CompileData& cd = cm[context];
        
        if (!cd.empty())
        {
            osg::notify(level)<<"cd._drawables.size()="<<cd._drawables.size()<<std::endl;
            osg::notify(level)<<"cd._textures.size()="<<cd._textures.size()<<std::endl;
            osg::notify(level)<<"cd._programs.size()="<<cd._programs.size()<<std::endl;
    
            
            while(!cd._drawables.empty() && 
                  osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick()) < compileTime)
            {
                cd._drawables.back()->compileGLObjects(renderInfo);
                cd._drawables.pop_back();
            }
            
            while(!cd._textures.empty() && 
                  osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick()) < compileTime)
            {
                cd._textures.back()->apply(*renderInfo.getState());
                cd._textures.pop_back();
            }

            
            if (!cd._programs.empty())
            {
                // compile programs
                while(!cd._programs.empty() && 
                      osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick()) < compileTime)
                {
                    cd._programs.back()->apply(*renderInfo.getState());
                    cd._programs.pop_back();
                }
                
                // switch off Program,
                osg::GL2Extensions* extensions = osg::GL2Extensions::Get(renderInfo.getState()->getContextID(), true);
                extensions->glUseProgram(0);
                renderInfo.getState()->setLastAppliedProgramObject(0);
            }
        }
                
        if (cd.empty())
        {
            bool csCompleted = false;
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);

                if (cs->compileCompleted())
                {                    
                    CompileSets::iterator cs_itr = std::find(_toCompile.begin(), _toCompile.end(), *itr);
                    if (cs_itr != _toCompile.end())
                    {
                        osg::notify(level)<<"Erasing from list"<<std::endl;
                        
                        // remove from the _toCompile list, note cs won't be deleted here as the tempoary
                        // toCompile_Copy list will retain a reference.
                        _toCompile.erase(cs_itr);
                        
                        // signal that we need to do clean up operations/pass cs on to _compile list.
                        csCompleted = true;
                    }
                }
            }
        
            if (csCompleted && cs->_compileCompletedCallback.valid())
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
            
        }
    }
}

} // end of namespace osgUtil
