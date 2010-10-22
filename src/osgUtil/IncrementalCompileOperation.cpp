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

#include <OpenThreads/ScopedLock>

#include <algorithm>
#include <stdlib.h>
#include <iterator>

namespace osgUtil 
{


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
// CompileOperator
//
CompileOperator::CompileOperator():
    _timingTestsCompleted(false)
{
    _compileStats = new CompileStats;
}

void CompileOperator::assignForceTextureDownloadGeometry()
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

double CompileOperator::timeCompile(osg::RenderInfo& renderInfo, osg::Geometry* geometry) const
{
    osg::ElapsedTime timer;
    geometry->compileGLObjects(renderInfo);
    return timer.elapsedTime();
}

double CompileOperator::timeCompile(osg::RenderInfo& renderInfo, osg::StateSet* stateset) const
{
    osg::ElapsedTime timer;
    stateset->compileGLObjects(*renderInfo.getState());
    return timer.elapsedTime();
}

osg::Geometry* CompileOperator::createTestGeometry(unsigned int numVertices, bool vbo)  const
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(numVertices);
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(numVertices);
    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array(numVertices);
    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array(numVertices);

    geometry->setVertexArray(vertices.get());
    geometry->setNormalArray(normals.get());
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setColorArray(colours.get());
    geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    geometry->setTexCoordArray(0, texcoords.get());

    for(unsigned int i=0; i<numVertices; ++i)
    {
        (*vertices)[i] = osg::Vec3(0.0f,0.0f,0.0f);
        (*normals)[i] = osg::Vec3(0.0f,0.0f,0.0f);
        (*colours)[i] = osg::Vec4(1.0f,1.0f,1.0f,1.0f);
        (*texcoords)[i] = osg::Vec2(0.0f,0.0f);
    }

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0,numVertices));

    geometry->setUseDisplayList(true);
    geometry->setUseVertexBufferObjects(vbo);

    return geometry.release();
}

osg::StateSet* CompileOperator::createTestStateSet(unsigned int imageSize, bool mipmapped)  const
{
    return 0;
}

void CompileOperator::runTimingTests(osg::RenderInfo& renderInfo)
{
    OSG_NOTICE<<"runTimingTests()"<<std::endl;
    _timingTestsCompleted = true;

    unsigned int mx = 18;
    double Mbsec = 1.0/(1024.0*1024.0);
    for(unsigned int j=0; j<4; ++j)
    {
        OSG_NOTICE<<"Using display lists"<<std::endl;
        bool useVBO = false;
        for(unsigned int i=0; i<mx; ++i)
        {
            unsigned int numVertices = pow(2.0,double(i));
            osg::ref_ptr<osg::Geometry> geometry = createTestGeometry(numVertices, useVBO);
            double size = geometry->getGLObjectSizeHint();
            double time = timeCompile(renderInfo, geometry);
            OSG_NOTICE<<"   numVertices = "<<numVertices<<" size = "<<size<<", time = "<<time*1000.0<<"ms rate= "<<(size/time)*Mbsec<<"Mb/sec"<<std::endl;
        }
    }
    for(unsigned int j=0; j<4; ++j)
    {
        OSG_NOTICE<<"Using VBOs"<<std::endl;
        bool useVBO = true;
        for(unsigned int i=0; i<mx; ++i)
        {
            unsigned int numVertices = pow(2.0,double(i));
            osg::ref_ptr<osg::Geometry> geometry = createTestGeometry(numVertices, useVBO);
            double size = geometry->getGLObjectSizeHint();
            double time = timeCompile(renderInfo, geometry);
            OSG_NOTICE<<"   numVertices = "<<numVertices<<" size = "<<size<<", time = "<<time*1000.0<<"ms rate= "<<(size/time)*Mbsec<<"Mb/sec"<<std::endl;
        }
    }
}


bool CompileOperator::compile(osg::RenderInfo& renderInfo, CompileData& cd, unsigned int& maxNumObjectsToCompile, double& compileTime)
{
    osg::Timer_t startTick = osg::Timer::instance()->tick();

    if (!_timingTestsCompleted)
    {
        runTimingTests(renderInfo);
    }

    unsigned int totalDataSizeCompiled = 0;
    unsigned int drawablesCompiled = 0;
    unsigned int texturesCompiled = 0;
    unsigned int programsCompiled = 0;

    if (!cd.empty() && compileTime>0.0)
    {

        osg::Timer_t previousTick = osg::Timer::instance()->tick();

        const std::string vboDrawablesName("VBO Drawables");
        const std::string dlDawablesName("DisplayList Drawables");

        while(!cd._drawables.empty() &&
                maxNumObjectsToCompile>0 &&
                osg::Timer::instance()->delta_s(startTick, previousTick) < compileTime)
        {
            CompileData::Drawables::iterator itr = cd._drawables.begin();
            const osg::Drawable* drawable = itr->get();
            unsigned int size = drawable->getGLObjectSizeHint();
            const std::string& nameOfDrawableType = drawable->getUseVertexBufferObjects() ? vboDrawablesName : dlDawablesName;
            double estimatedTime = _compileStats->estimateTime(nameOfDrawableType, double(size));
            double estimatedTime2 = _compileStats->estimateTime2(nameOfDrawableType, double(size));
            double estimatedTime3 = _compileStats->estimateTime3(nameOfDrawableType, double(size));
            double estimatedTime4 = _compileStats->estimateTime4(nameOfDrawableType, double(size));

            drawable->compileGLObjects(renderInfo);
            osg::Timer_t currTick = osg::Timer::instance()->tick();
            double timeForCompile = osg::Timer::instance()->delta_s(previousTick, currTick);
            previousTick = currTick;

            OSG_NOTICE<<"Drawable size = "<<size<<std::endl;
            OSG_NOTICE<<"  Estimated time   ="<<estimatedTime<<", actual time="<<timeForCompile<<" ratio = "<<timeForCompile/estimatedTime<<std::endl;
            OSG_NOTICE<<"  Estimated time2="<<estimatedTime2<<", actual time="<<timeForCompile<<" ratio = "<<timeForCompile/estimatedTime2<<std::endl;
            OSG_NOTICE<<"  Estimated time3="<<estimatedTime3<<", actual time="<<timeForCompile<<" ratio = "<<timeForCompile/estimatedTime3<<std::endl;
            OSG_NOTICE<<"  Estimated time4="<<estimatedTime4<<", actual time="<<timeForCompile<<" ratio = "<<timeForCompile/estimatedTime4<<std::endl;

            _compileStats->add(nameOfDrawableType, double(size), timeForCompile);


            totalDataSizeCompiled += size;
            // OSG_NOTICE<<"Compiled drawable, getGLObjectSizeHint()="<<(*itr)->getGLObjectSizeHint()<<std::endl;

            ++drawablesCompiled;
            --maxNumObjectsToCompile;
            cd._drawables.erase(itr);


        }

        while(!cd._textures.empty() &&
                maxNumObjectsToCompile>0 &&
                osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick()) < compileTime)
        {
            CompileData::Textures::iterator itr = cd._textures.begin();

            osg::Texture* texture = itr->get();

            osg::Texture::FilterMode minFilter = texture->getFilter(osg::Texture::MIN_FILTER);
            bool textureMipmapped = (minFilter!=osg::Texture::LINEAR && minFilter!=osg::Texture::NEAREST);
            bool textureCompressedFormat = texture->isCompressedInternalFormat();
            bool needtoBuildMipmaps = false;
            bool needtoCompress = false;

            OSG_NOTICE<<"  texture->isCompressedInternalFormat()="<<textureCompressedFormat<<std::endl;

            for(unsigned int i=0; i<texture->getNumImages();++i)
            {
                osg::Image* image = texture->getImage(i);
                if (image)
                {
                    totalDataSizeCompiled += texture->getImage(i)->getTotalSizeInBytesIncludingMipmaps();
                    if (textureMipmapped && !image->isMipmap()) needtoBuildMipmaps = true;
                    if (textureCompressedFormat && !image->isCompressed()) needtoCompress = true;
                }
            }


            OSG_NOTICE<<"compiling texture, textureMipmapped="<<textureMipmapped<<", needtoBuildMipmaps="<<needtoBuildMipmaps<<std::endl;
            OSG_NOTICE<<"                   textureCompressedFormat="<<textureCompressedFormat<<", needtoCompress="<<needtoCompress<<std::endl;

            // if (needtoBuildMipmaps) texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);


            if (_forceTextureDownloadGeometry.get())
            {

                if (_forceTextureDownloadGeometry->getStateSet())
                {
                    renderInfo.getState()->apply(_forceTextureDownloadGeometry->getStateSet());
                }

                renderInfo.getState()->applyTextureMode(0, texture->getTextureTarget(), true);
                renderInfo.getState()->applyTextureAttribute(0, texture);

                _forceTextureDownloadGeometry->draw(renderInfo);
            }
            else
            {
                texture->apply(*renderInfo.getState());
            }

            ++texturesCompiled;
            --maxNumObjectsToCompile;

            cd._textures.erase(itr);
        }


        if (!cd._programs.empty())
        {
            osg::GL2Extensions* extensions = osg::GL2Extensions::Get(renderInfo.getState()->getContextID(), true);
            if (extensions && extensions->isGlslSupported())
            {
                // compile programs
                while(!cd._programs.empty() &&
                    maxNumObjectsToCompile>0 &&
                    osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick()) < compileTime)
                {
                    CompileData::Programs::iterator itr = cd._programs.begin();
                    (*itr)->apply(*renderInfo.getState());

                    ++programsCompiled;
                    --maxNumObjectsToCompile;

                    cd._programs.erase(itr);
                }

#if 0

// what shall we do about uniforms???

        osg::Program* program = dynamic_cast<osg::Program*>(stateset.getAttribute(osg::StateAttribute::PROGRAM));
        if (program) {
            if( program->isFixedFunction() )
                _lastCompiledProgram = NULL; // It does not make sense to apply uniforms on fixed pipe
            else
                _lastCompiledProgram = program;
        }

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

            osg::GL2Extensions* extensions = osg::GL2Extensions::Get(_renderInfo.getState()->getContextID(), true);
            extensions->glUseProgram(0);
            _renderInfo.getState()->setLastAppliedProgramObject(0);
        }

#endif

                // switch off Program,
                extensions->glUseProgram(0);
                renderInfo.getState()->setLastAppliedProgramObject(0);
            }
        }
    }

    double timeUsed = osg::Timer::instance()->delta_s(startTick, osg::Timer::instance()->tick());

    OSG_NOTICE<<"compile time, texturesCompiled="<<texturesCompiled<<", drawablesCompiled="<<drawablesCompiled<<", programsCompiled="<<programsCompiled<<", timeUsed="<<timeUsed*1000.0<<" totalDataSizeCompiled="<<totalDataSizeCompiled<<" bytes, download rate="<<double(totalDataSizeCompiled)/(1024.0*1024*timeUsed)<<"Mb/sec"<<std::endl;

    compileTime -= timeUsed;

    return cd.empty();
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

    _compileOperator = new CompileOperator;
}

IncrementalCompileOperation::~IncrementalCompileOperation()
{
}

void IncrementalCompileOperation::assignForceTextureDownloadGeometry()
{
    _compileOperator->assignForceTextureDownloadGeometry();
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

void IncrementalCompileOperation::mergeCompiledSubgraphs()
{
    // OSG_INFO<<"IncrementalCompileOperation::mergeCompiledSubgraphs()"<<std::endl;

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

        if (_mode&GLObjectsVisitor::COMPILE_DISPLAY_LISTS &&
            (drawable.getUseDisplayList() || drawable.getUseVertexBufferObjects()))
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
        cd._drawables.insert(cstc._drawables.begin(), cstc._drawables.end());
        cd._textures.insert(cstc._textures.begin(), cstc._textures.end());
        cd._programs.insert(cstc._programs.begin(), cstc._programs.end());
    }
    
}

void IncrementalCompileOperation::operator () (osg::GraphicsContext* context)
{
    // OSG_NOTICE<<"IncrementalCompileOperation::operator () ("<<context<<")"<<std::endl;

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


    osg::RenderInfo renderInfo;
    renderInfo.setState(context->getState());

    CompileSets toCompileCopy;
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);
        std::copy(_toCompile.begin(),_toCompile.end(),std::back_inserter<CompileSets>(toCompileCopy));
    }

    for(CompileSets::iterator itr = toCompileCopy.begin();
        itr != toCompileCopy.end() && compileTime>0.0;
        ++itr)
    {
        CompileSet* cs = itr->get();
        CompileMap& cm = cs->_compileMap;
        CompileData& cd = cm[context];

        if (_compileOperator->compile(renderInfo, cd, maxNumOfObjectsToCompilePerFrame, compileTime))
        {
            bool csCompleted = false;
            {
                OpenThreads::ScopedLock<OpenThreads::Mutex>  toCompile_lock(_toCompileMutex);

                if (cs->compileCompleted())
                {                    
                    CompileSets::iterator cs_itr = std::find(_toCompile.begin(), _toCompile.end(), *itr);
                    if (cs_itr != _toCompile.end())
                    {
                        OSG_NOTIFY(level)<<"Erasing from list"<<std::endl;

                        // remove from the _toCompile list, note cs won't be deleted here as the tempoary
                        // toCompile_Copy list will retain a reference.
                        _toCompile.erase(cs_itr);

                        // signal that we need to do clean up operations/pass cs on to _compile list.
                        csCompleted = true;
                    }
                }
            }

            if (csCompleted)
            {
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
    }
}

} // end of namespace osgUtil
