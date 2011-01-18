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
#include <osg/Depth>
#include <osg/ColorMask>
#include <osg/ApplicationUsage>

#include <OpenThreads/ScopedLock>

#include <algorithm>
#include <stdlib.h>
#include <iterator>

namespace osgUtil 
{


// TODO
// priority of CompileSets
// isCompiled
// time estimation
// early completion
// needs compile given time slot
// custom CompileData elements
// pruneOldRequestsAndCheckIfEmpty()
// Use? :
//                     #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
//                        GLint p;
//                        glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_RESIDENT, &p);
//                    #endif

static osg::ApplicationUsageProxy ICO_e1(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MINIMUM_COMPILE_TIME_PER_FRAME <float>","minimum compile time alloted to compiling OpenGL objects per frame in database pager.");
static osg::ApplicationUsageProxy UCO_e2(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_MAXIMUM_OBJECTS_TO_COMPILE_PER_FRAME <int>","maximum number of OpenGL objects to compile per frame in database pager.");

/////////////////////////////////////////////////////////////////
//
// CollectStateToCompile
//
StateToCompile::StateToCompile(GLObjectsVisitor::Mode mode):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _mode(mode)
{
}

void StateToCompile::apply(osg::Node& node)
{
    if (node.getStateSet())
    {
        apply(*(node.getStateSet()));
    }

    traverse(node);
}

void StateToCompile::apply(osg::Geode& node)
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

void StateToCompile::apply(osg::Drawable& drawable)
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

    if (_mode&GLObjectsVisitor::COMPILE_DISPLAY_LISTS &&
        (drawable.getUseDisplayList() || drawable.getUseVertexBufferObjects()))
    {
        _drawables.insert(&drawable);
    }
}

void StateToCompile::apply(osg::StateSet& stateset)
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
                if (texture)
                {
                    if (_textures.count(texture)==0)
                    {
                        apply(*texture);
                    }
                }
            }
        }
    }
}

void StateToCompile::apply(osg::Texture& texture)
{
    _textures.insert(&texture);
}

/////////////////////////////////////////////////////////////////
//
// CompileStats
//
CompileStats::CompileStats()
{
}

void CompileStats::add(const std::string& name,double size, double time)
{
    Values& values = _statsMap[name];
    values.add(size,time);
}

double CompileStats::estimateTime(const std::string& name, double size) const
{
    StatsMap::const_iterator itr = _statsMap.find(name);
    return (itr!=_statsMap.end()) ? itr->second.estimateTime(size) : 0.0;
}

double CompileStats::estimateTime2(const std::string& name, double size) const
{
    StatsMap::const_iterator itr = _statsMap.find(name);
    return (itr!=_statsMap.end()) ? itr->second.estimateTime2(size) : 0.0;
}

double CompileStats::estimateTime3(const std::string& name, double size) const
{
    StatsMap::const_iterator itr = _statsMap.find(name);
    return (itr!=_statsMap.end()) ? itr->second.estimateTime3(size) : 0.0;
}

double CompileStats::estimateTime4(const std::string& name, double size) const
{
    StatsMap::const_iterator itr = _statsMap.find(name);
    return (itr!=_statsMap.end()) ? itr->second.estimateTime4(size) : 0.0;
}

double CompileStats::averageTime(const std::string& name) const
{
    StatsMap::const_iterator itr = _statsMap.find(name);
    return (itr!=_statsMap.end()) ? itr->second.averageTime() : 0.0;
}

void CompileStats::print(std::ostream& out) const
{
    for(StatsMap::const_iterator itr = _statsMap.begin();
        itr != _statsMap.end();
        ++itr)
    {
        const Values& values = itr->second;
        out<<itr->first<<" : averageTime "<<values.averageTime()<<", a="<<values.a<<" b="<<values.b<<std::endl;
    }
}

void CompileStats::Values::add(double size, double time)
{
    if (totalNum==0.0)
    {
        minSize = size;
        minTime = time;
    }
    else
    {
        if (size<minSize) minSize = size;
        if (time<minTime) minTime = time;
    }

    totalSize += size;
    totalTime += time;
    totalNum += 1.0;

    m += time/(size*size); // sum of yi/xi^2
    n += time/size; // sum of yi/xi
    o += 1.0/(size*size); // sum of 1/xi^2
    p += 1.0/size; // sum of 1/xi

    double d = o + p*p;
    if (d!=0.0)
    {
        a = (n*p - m)/d;
        b = (n*o - m*p)/d;
    }
    else
    {
        a = time;
        b = 0.0;
    }
}


/////////////////////////////////////////////////////////////////
//
// CompileOps
//
IncrementalCompileOperation::CompileDrawableOp::CompileDrawableOp(osg::Drawable* drawable):
    _drawable(drawable)
{
}

double IncrementalCompileOperation::CompileDrawableOp::estimatedTimeForCompile(CompileInfo& compileInfo) const
{
    return 0.0;
}

bool IncrementalCompileOperation::CompileDrawableOp::compile(CompileInfo& compileInfo)
{
    //OSG_NOTICE<<"CompileDrawableOp::compile(..)"<<std::endl;
    _drawable->compileGLObjects(compileInfo);
    return true;
}

IncrementalCompileOperation::CompileTextureOp::CompileTextureOp(osg::Texture* texture):
    _texture(texture)
{
}

double IncrementalCompileOperation::CompileTextureOp::estimatedTimeForCompile(CompileInfo& compileInfo) const
{
    return 0.0;
}

bool IncrementalCompileOperation::CompileTextureOp::compile(CompileInfo& compileInfo)
{
    //OSG_NOTICE<<"CompileTextureOp::compile(..)"<<std::endl;
    osg::Geometry* forceDownloadGeometry = compileInfo.incrementalCompileOperation->getForceTextureDownloadGeometry();
    if (forceDownloadGeometry)
    {
        if (forceDownloadGeometry->getStateSet())
        {
            compileInfo.getState()->apply(forceDownloadGeometry->getStateSet());
        }

        compileInfo.getState()->applyTextureMode(0, _texture->getTextureTarget(), true);
        compileInfo.getState()->applyTextureAttribute(0, _texture.get());

        forceDownloadGeometry->draw(compileInfo);
    }
    else
    {
        _texture->apply(*compileInfo.getState());
    }
    return true;
}

IncrementalCompileOperation::CompileProgramOp::CompileProgramOp(osg::Program* program):
    _program(program)
{
}

double IncrementalCompileOperation::CompileProgramOp::estimatedTimeForCompile(CompileInfo& compileInfo) const
{
    return 0.0;
}

bool IncrementalCompileOperation::CompileProgramOp::compile(CompileInfo& compileInfo)
{
    //OSG_NOTICE<<"CompileProgramOp::compile(..)"<<std::endl;
    _program->apply(*compileInfo.getState());
    return true;
}

IncrementalCompileOperation::CompileInfo::CompileInfo(osg::GraphicsContext* context, IncrementalCompileOperation* ico)
{
    setState(context->getState());
    incrementalCompileOperation = ico;
}


/////////////////////////////////////////////////////////////////
//
// CompileList
//
IncrementalCompileOperation::CompileList::CompileList()
{
}

IncrementalCompileOperation::CompileList::~CompileList()
{
}

void IncrementalCompileOperation::CompileList::add(CompileOp* compileOp)
{
    _compileOps.push_back(compileOp);
}

double IncrementalCompileOperation::CompileList::estimatedTimeForCompile(CompileInfo& compileInfo) const
{
    double estimateTime = 0.0;
    for(CompileOps::const_iterator itr = _compileOps.begin();
        itr != _compileOps.begin();
        ++itr)
    {
        estimateTime += (*itr)->estimatedTimeForCompile(compileInfo);
    }
    return estimateTime;
}

bool IncrementalCompileOperation::CompileList::compile(CompileInfo& compileInfo)
{
    for(CompileOps::iterator itr = _compileOps.begin();
        itr != _compileOps.end() && compileInfo.availableTime()>0.0 && compileInfo.maxNumObjectsToCompile>0;
    )
    {
        --compileInfo.maxNumObjectsToCompile;

        CompileOps::iterator saved_itr(itr);
        ++itr;
        if ((*saved_itr)->compile(compileInfo))
        {
            _compileOps.erase(saved_itr);
        }
    }
    return empty();
}

/////////////////////////////////////////////////////////////////
//
// CompileSet
//
void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& contexts, StateToCompile& stc)
{
    if (contexts.empty() || stc.empty()) return;

    if (stc.empty()) return;

    for(ContextSet::iterator itr = contexts.begin();
        itr != contexts.end();
        ++itr)
    {
        // increment the number of compile lists that will need to compile
        ++_numberCompileListsToCompile;

        CompileList& cl = _compileMap[*itr];
        for(StateToCompile::DrawableSet::iterator ditr = stc._drawables.begin();
            ditr != stc._drawables.end();
            ++ditr)
        {
            cl.add(*ditr);
        }

        for(StateToCompile::TextureSet::iterator titr = stc._textures.begin();
            titr != stc._textures.end();
            ++titr)
        {
            cl.add(*titr);
        }

        for(StateToCompile::ProgramSet::iterator pitr = stc._programs.begin();
            pitr != stc._programs.end();
            ++pitr)
        {
            cl.add(*pitr);
        }
    }
}

void IncrementalCompileOperation::CompileSet::buildCompileMap(ContextSet& contexts, GLObjectsVisitor::Mode mode)
{
    if (contexts.empty() || !_subgraphToCompile) return;

    StateToCompile stc(mode);
    _subgraphToCompile->accept(stc);

    buildCompileMap(contexts, stc);
}

bool IncrementalCompileOperation::CompileSet::compile(CompileInfo& compileInfo)
{
    CompileList& compileList = _compileMap[compileInfo.getState()->getGraphicsContext()];
    if (!compileList.empty())
    {
        if (compileList.compile(compileInfo))
        {
            --_numberCompileListsToCompile;
            return _numberCompileListsToCompile==0;
        }
    }
    return _numberCompileListsToCompile==0;
}

/////////////////////////////////////////////////////////////////
//
// IncrementalCompileOperation
//
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

    _compileStats = new CompileStats;

    // assignForceTextureDownloadGeometry();
}

IncrementalCompileOperation::~IncrementalCompileOperation()
{
}

void IncrementalCompileOperation::assignForceTextureDownloadGeometry()
{
    osg::Geometry* geometry = new osg::Geometry;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    vertices->push_back(osg::Vec3(0.0f,0.0f,0.0f));
    geometry->setVertexArray(vertices);

    osg::Vec4Array* texcoords = new osg::Vec4Array;
    texcoords->push_back(osg::Vec4(0.0f,0.0f,0.0f,0.0f));
    geometry->setTexCoordArray(0, texcoords);

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS,0,1));

    osg::StateSet* stateset = geometry->getOrCreateStateSet();
    stateset->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::ON);

    osg::Depth* depth = new osg::Depth;
    depth->setWriteMask(false);
    stateset->setAttribute(depth);

    osg::ColorMask* colorMask = new osg::ColorMask(false,false,false,false);
    stateset->setAttribute(colorMask);

    _forceTextureDownloadGeometry = geometry;
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

bool IncrementalCompileOperation::requiresCompile(StateToCompile& stateToCompile)
{
    return isActive() && !stateToCompile.empty();
}

void IncrementalCompileOperation::add(osg::Node* subgraphToCompile)
{
    OSG_INFO<<"IncrementalCompileOperation::add("<<subgraphToCompile<<")"<<std::endl;
    add(new CompileSet(subgraphToCompile));
}

void IncrementalCompileOperation::add(osg::Group* attachmentPoint, osg::Node* subgraphToCompile)
{
    OSG_INFO<<"IncrementalCompileOperation::add("<<attachmentPoint<<", "<<subgraphToCompile<<")"<<std::endl;
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

    OSG_INFO<<"IncrementalCompileOperation::add(CompileSet = "<<compileSet<<", "<<", "<<callBuildCompileMap<<")"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
    _toCompile.push_back(compileSet);
}

void IncrementalCompileOperation::remove(CompileSet* compileSet)
{
    // OSG_NOTICE<<"IncrementalCompileOperation::remove(CompileSet* compileSet)"<<std::endl;
    
    if (!compileSet) return;

    // remove CompileSet from _toCompile list if it's present.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_toCompileMutex);
        for(CompileSets::iterator itr = _toCompile.begin();
            itr != _toCompile.end();
            ++itr)
        {
            if (*itr == compileSet)
            {
                _toCompile.erase(itr);
                return;
            }
        }
    }

    // remove CompileSet from _compiled list if it's present.
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_compiledMutex);
        for(CompileSets::iterator itr = _compiled.begin();
            itr != _compiled.end();
            ++itr)
        {
            if (*itr == compileSet)
            {
                _toCompile.erase(itr);
                return;
            }
        }
    }
}


void IncrementalCompileOperation::mergeCompiledSubgraphs()
{
    // OSG_INFO<<"IncrementalCompileOperation::mergeCompiledSubgraphs()"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  compilded_lock(_compiledMutex);
    
    for(CompileSets::iterator itr = _compiled.begin(); 
        itr != _compiled.end();
        ++itr)
    {
        CompileSet* cs = itr->get();
        osg::ref_ptr<osg::Group> group;
        if (cs->_attachmentPoint.lock(group))
        {
            group->addChild(cs->_subgraphToCompile.get());
        }
    }
    
    _compiled.clear();
}


void IncrementalCompileOperation::operator () (osg::GraphicsContext* context)
{
    osg::NotifySeverity level = osg::INFO;

    double targetFrameRate = _targetFrameRate;
    double minimumTimeAvailableForGLCompileAndDeletePerFrame = _minimumTimeAvailableForGLCompileAndDeletePerFrame;

    double targetFrameTime = 1.0/targetFrameRate;
    
    const osg::FrameStamp* fs = context->getState()->getFrameStamp();
    double currentTime = fs ? fs->getReferenceTime() : 0.0;

    double currentElapsedFrameTime = context->getTimeSinceLastClear();
    
    OSG_NOTIFY(level)<<"currentTime = "<<currentTime<<std::endl;
    OSG_NOTIFY(level)<<"currentElapsedFrameTime = "<<currentElapsedFrameTime<<std::endl;
    
    double _flushTimeRatio(0.5);
    double _conservativeTimeRatio(0.5);

    double availableTime = std::max((targetFrameTime - currentElapsedFrameTime)*_conservativeTimeRatio,
                                    minimumTimeAvailableForGLCompileAndDeletePerFrame);

    double flushTime = availableTime * _flushTimeRatio;
    double compileTime = availableTime - flushTime;
    unsigned int maxNumOfObjectsToCompilePerFrame = _maximumNumOfObjectsToCompilePerFrame;

#if 1
    OSG_NOTIFY(level)<<"total availableTime = "<<availableTime*1000.0<<std::endl;
    OSG_NOTIFY(level)<<"      flushTime     = "<<flushTime*1000.0<<std::endl;
    OSG_NOTIFY(level)<<"      compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

    osg::flushDeletedGLObjects(context->getState()->getContextID(), currentTime, flushTime);

    // if any time left over from flush add this to compile time.        
    if (flushTime>0.0) compileTime += flushTime;

#if 1
    OSG_NOTIFY(level)<<"      revised compileTime   = "<<compileTime*1000.0<<std::endl;
#endif


    //level = osg::NOTICE;

    CompileInfo compileInfo(context, this);
    compileInfo.maxNumObjectsToCompile = _maximumNumOfObjectsToCompilePerFrame;
    compileInfo.allocatedTime = compileTime;

    CompileSets toCompileCopy;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
        std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
    }

    for(CompileSets::iterator itr = toCompileCopy.begin();
        itr != toCompileCopy.end() && compileTime>0.0 && maxNumOfObjectsToCompilePerFrame>0;
        ++itr)
    {
        CompileSet* cs = itr->get();
        if (cs->compile(compileInfo))
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);

                CompileSets::iterator cs_itr = std::find(_toCompile.begin(), _toCompile.end(), *itr);
                if (cs_itr != _toCompile.end())
                {
                    OSG_NOTIFY(level)<<"Erasing from list"<<std::endl;

                    // remove from the _toCompile list, note cs won't be deleted here as the tempoary
                    // toCompile_Copy list will retain a reference.
                    _toCompile.erase(cs_itr);
                }
            }
            if (cs->_compileCompletedCallback.valid() && cs->_compileCompletedCallback->compileCompleted(cs))
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
    //glFush();
    //glFinish();
}

} // end of namespace osgUtil
