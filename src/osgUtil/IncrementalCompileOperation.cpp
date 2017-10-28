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
#include <iterator>
#include <stdlib.h>
#include <string.h>

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
static osg::ApplicationUsageProxy UCO_e3(osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE,"OSG_FORCE_TEXTURE_DOWNLOAD <ON/OFF>","should the texture compiles be forced to download using a dummy Geometry.");

/////////////////////////////////////////////////////////////////
//
// CollectStateToCompile
//
StateToCompile::StateToCompile(GLObjectsVisitor::Mode mode, osg::Object* markerObject):
    osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
    _mode(mode),
    _assignPBOToImages(false),
    _markerObject(markerObject)
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

void StateToCompile::apply(osg::Drawable& drawable)
{
    if (_drawablesHandled.count(&drawable)!=0) return;

    _drawablesHandled.insert(&drawable);

    if (!_markerObject || _markerObject.get()!=drawable.getUserData())
    {
        if (drawable.getDataVariance()!=osg::Object::STATIC)
        {
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
        }

        if (_mode&GLObjectsVisitor::COMPILE_DISPLAY_LISTS &&
            (drawable.getUseDisplayList() || drawable.getUseVertexBufferObjects()))
        {
            _drawables.insert(&drawable);
        }

        if (drawable.getStateSet())
        {
            apply(*(drawable.getStateSet()));
        }

        // mark the drawable as visited
        if (_markerObject.valid() && drawable.getUserData()==0) drawable.setUserData(_markerObject.get());
    }
}

void StateToCompile::apply(osg::StateSet& stateset)
{
    if (_statesetsHandled.count(&stateset)!=0) return;

    _statesetsHandled.insert(&stateset);

    if ((_mode & GLObjectsVisitor::COMPILE_STATE_ATTRIBUTES)!=0 &&
        (!_markerObject || _markerObject.get()!=stateset.getUserData()))
    {
        osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
        if (program && (!_markerObject || _markerObject.get()!=program->getUserData()))
        {
            _programs.insert(program);

            // mark the stateset as visited
            if (_markerObject.valid() && program->getUserData()==0) program->setUserData(_markerObject.get());
        }

        const osg::StateSet::TextureAttributeList& tal = stateset.getTextureAttributeList();

        for(osg::StateSet::TextureAttributeList::const_iterator itr = tal.begin();
            itr != tal.end();
            ++itr)
        {
            const osg::StateSet::AttributeList& al = *itr;
            osg::StateAttribute::TypeMemberPair tmp(osg::StateAttribute::TEXTURE,0);
            osg::StateSet::AttributeList::const_iterator texItr = al.find(tmp);
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

        // mark the stateset as visited
        if (_markerObject.valid() && stateset.getUserData()==0) stateset.setUserData(_markerObject.get());
    }
}

void StateToCompile::apply(osg::Texture& texture)
{
    // don't make any changes if Texture already processed
    if (_markerObject.valid() && _markerObject.get()==texture.getUserData()) return;

    if (_assignPBOToImages)
    {
        unsigned int numRequringPBO = 0;
        osg::ref_ptr<osg::PixelBufferObject> pbo = 0;
        for(unsigned int i=0; i<texture.getNumImages(); ++i)
        {
            osg::Image* image = texture.getImage(i);
            if (image)
            {
                if (image->getPixelBufferObject())
                {
                    pbo = image->getPixelBufferObject();
                }
                else
                {
                    ++numRequringPBO;
                }
            }
        }
        if (numRequringPBO>0)
        {
            // assign pbo
            if (!pbo)
            {
                if (!_pbo) _pbo = new osg::PixelBufferObject;
                pbo = _pbo;
            }

            for(unsigned int i=0; i<texture.getNumImages(); ++i)
            {
                osg::Image* image = texture.getImage(i);
                if (image)
                {
                    if (!image->getPixelBufferObject())
                    {
                        //OSG_NOTICE<<"Assigning PBO"<<std::endl;
                        pbo->setCopyDataAndReleaseGLBufferObject(true);
                        pbo->setUsage(GL_DYNAMIC_DRAW_ARB);
                        image->setPixelBufferObject(pbo.get());
                    }
                }
            }
        }
    }

    if (_markerObject.valid() && texture.getUserData()==0) texture.setUserData(_markerObject.get());

    _textures.insert(&texture);
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
    osg::GraphicsCostEstimator* gce = compileInfo.getState()->getGraphicsCostEstimator();
    osg::Geometry* geometry = _drawable->asGeometry();
    if (gce && geometry)
    {
        return gce->estimateCompileCost(geometry).first;
    }
    else return 0.0;
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
    osg::GraphicsCostEstimator* gce = compileInfo.getState()->getGraphicsCostEstimator();
    if (gce) return gce->estimateCompileCost(_texture.get()).first;
    else return 0.0;
}

bool IncrementalCompileOperation::CompileTextureOp::compile(CompileInfo& compileInfo)
{
    //OSG_NOTICE<<"CompileTextureOp::compile(..)"<<std::endl;
    osg::Geometry* forceDownloadGeometry = compileInfo.incrementalCompileOperation->getForceTextureDownloadGeometry();
    if (forceDownloadGeometry)
    {

        //OSG_NOTICE<<"Force texture download"<<std::endl;
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
    osg::GraphicsCostEstimator* gce = compileInfo.getState()->getGraphicsCostEstimator();
    if (gce) return gce->estimateCompileCost(_program.get()).first;
    else return 0.0;
}

bool IncrementalCompileOperation::CompileProgramOp::compile(CompileInfo& compileInfo)
{
    //OSG_NOTICE<<"CompileProgramOp::compile(..)"<<std::endl;
    _program->compileGLObjects(*compileInfo.getState());
    return true;
}

IncrementalCompileOperation::CompileInfo::CompileInfo(osg::GraphicsContext* context, IncrementalCompileOperation* ico):
    compileAll(false),
    maxNumObjectsToCompile(0),
    allocatedTime(0)
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
        itr != _compileOps.end();
        ++itr)
    {
        estimateTime += (*itr)->estimatedTimeForCompile(compileInfo);
    }
    return estimateTime;
}

bool IncrementalCompileOperation::CompileList::compile(CompileInfo& compileInfo)
{
//#define USE_TIME_ESTIMATES

    for(CompileOps::iterator itr = _compileOps.begin();
        itr != _compileOps.end() && compileInfo.okToCompile();
    )
    {
        #ifdef USE_TIME_ESTIMATES
        double estimatedCompileCost = (*itr)->estimatedTimeForCompile(compileInfo);
        #endif

        --compileInfo.maxNumObjectsToCompile;

        #ifdef USE_TIME_ESTIMATES
        osg::ElapsedTime timer;
        #endif

        CompileOps::iterator saved_itr(itr);
        ++itr;
        if ((*saved_itr)->compile(compileInfo))
        {
            _compileOps.erase(saved_itr);
        }

        #ifdef USE_TIME_ESTIMATES
        double actualCompileCost = timer.elapsedTime();
        OSG_NOTICE<<"IncrementalCompileOperation::CompileList::compile() estimatedTimForCompile = "<<estimatedCompileCost*1000.0<<"ms, actual = "<<actualCompileCost*1000.0<<"ms";
        if (estimatedCompileCost>0.0) OSG_NOTICE<<", ratio="<<(actualCompileCost/estimatedCompileCost);
        OSG_NOTICE<<std::endl;
        #endif
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

    StateToCompile stc(mode, _markerObject.get());

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
    osg::Referenced(true),
    osg::GraphicsOperation("IncrementalCompileOperation",true),
    _flushTimeRatio(0.5),
    _conservativeTimeRatio(0.5),
    _currentFrameNumber(0),
    _compileAllTillFrameNumber(0)
{
    _markerObject = new osg::DummyObject;
    _markerObject->setName("HasBeenProcessedByStateToCompile");

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

    bool useForceTextureDownload = false;
    if( (ptr = getenv("OSG_FORCE_TEXTURE_DOWNLOAD")) != 0)
    {
        useForceTextureDownload = strcmp(ptr,"yes")==0 || strcmp(ptr,"YES")==0 ||
                                  strcmp(ptr,"on")==0 || strcmp(ptr,"ON")==0;

        OSG_NOTICE<<"OSG_FORCE_TEXTURE_DOWNLOAD set to "<<useForceTextureDownload<<std::endl;
    }

    if (useForceTextureDownload)
    {
        assignForceTextureDownloadGeometry();
    }

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

    // pass on the markerObject to the CompileSet
    compileSet->_markerObject = _markerObject;

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
                _compiled.erase(itr);
                return;
            }
        }
    }
}


void IncrementalCompileOperation::mergeCompiledSubgraphs(const osg::FrameStamp* frameStamp)
{
    // OSG_INFO<<"IncrementalCompileOperation::mergeCompiledSubgraphs()"<<std::endl;

    OpenThreads::ScopedLock<OpenThreads::Mutex>  compilded_lock(_compiledMutex);

    if (frameStamp) _currentFrameNumber = frameStamp->getFrameNumber();

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

    //glFinish();
    //glFlush();

    double targetFrameRate = _targetFrameRate;
    double minimumTimeAvailableForGLCompileAndDeletePerFrame = _minimumTimeAvailableForGLCompileAndDeletePerFrame;

    double targetFrameTime = 1.0/targetFrameRate;

    const osg::FrameStamp* fs = context->getState()->getFrameStamp();
    double currentTime = fs ? fs->getReferenceTime() : 0.0;

    double currentElapsedFrameTime = context->getTimeSinceLastClear();

    OSG_NOTIFY(level)<<"IncrementalCompileOperation()"<<std::endl;
    OSG_NOTIFY(level)<<"    currentTime = "<<currentTime<<std::endl;
    OSG_NOTIFY(level)<<"    currentElapsedFrameTime = "<<currentElapsedFrameTime<<std::endl;

    double availableTime = std::max((targetFrameTime - currentElapsedFrameTime)*_conservativeTimeRatio,
                                    minimumTimeAvailableForGLCompileAndDeletePerFrame);

    double flushTime = availableTime * _flushTimeRatio;
    double compileTime = availableTime - flushTime;

#if 1
    OSG_NOTIFY(level)<<"    availableTime = "<<availableTime*1000.0<<std::endl;
    OSG_NOTIFY(level)<<"    flushTime     = "<<flushTime*1000.0<<std::endl;
    OSG_NOTIFY(level)<<"    compileTime   = "<<compileTime*1000.0<<std::endl;
#endif

    //level = osg::NOTICE;

    CompileInfo compileInfo(context, this);
    compileInfo.maxNumObjectsToCompile = _maximumNumOfObjectsToCompilePerFrame;
    compileInfo.allocatedTime = compileTime;
    compileInfo.compileAll = (_compileAllTillFrameNumber > _currentFrameNumber);

    CompileSets toCompileCopy;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
        std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
    }

    if (!toCompileCopy.empty())
    {
        compileSets(toCompileCopy, compileInfo);
    }

    osg::flushDeletedGLObjects(context->getState()->getContextID(), currentTime, flushTime);

    if (!toCompileCopy.empty() && compileInfo.maxNumObjectsToCompile>0)
    {
        compileInfo.allocatedTime += flushTime;

        // if any time left over from flush add on this remaining time to a second pass of compiling.
        if (compileInfo.okToCompile())
        {
            OSG_NOTIFY(level)<<"    Passing on "<<flushTime<<" to second round of compileSets(..)"<<std::endl;
            compileSets(toCompileCopy, compileInfo);
        }
    }

    //glFush();
    //glFinish();
}

void IncrementalCompileOperation::compileSets(CompileSets& toCompile, CompileInfo& compileInfo)
{
    osg::NotifySeverity level = osg::INFO;

    for(CompileSets::iterator itr = toCompile.begin();
        itr != toCompile.end() && compileInfo.okToCompile();
        )
    {
        CompileSet* cs = itr->get();
        if (cs->compile(compileInfo))
        {
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);

                CompileSets::iterator cs_itr = std::find(_toCompile.begin(), _toCompile.end(), *itr);
                if (cs_itr != _toCompile.end())
                {
                    OSG_NOTIFY(level)<<"    Erasing from list"<<std::endl;

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

            // remove entry from list.
            itr = toCompile.erase(itr);
        }
        else
        {
            ++itr;
        }
    }

}


void IncrementalCompileOperation::compileAllForNextFrame(unsigned int numFramesToDoCompileAll)
{
    _compileAllTillFrameNumber = _currentFrameNumber+numFramesToDoCompileAll;
}


} // end of namespace osgUtil
