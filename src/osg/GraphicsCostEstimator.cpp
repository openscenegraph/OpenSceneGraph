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

#include <osg/GraphicsCostEstimator>

#include <osg/RenderInfo>
#include <osg/Geometry>
#include <osg/Texture>
#include <osg/Program>
#include <osg/Geode>

namespace osg
{

/////////////////////////////////////////////////////////////////////////////////////////////
//
// GeometryCostEstimator
//
GeometryCostEstimator::GeometryCostEstimator()
{
    setDefaults();
}

void GeometryCostEstimator::setDefaults()
{
    double transfer_bandwidth = 10000000000.0; // 1GB/sec
    double gpu_bandwidth = 50000000000.0; // 50 GB/second
    double min_time = 0.00001; // 10 nano seconds.
    _arrayCompileCost.set(min_time, 1.0/transfer_bandwidth, 256); // min time 1/10th of millisecond, min size 256
    _primtiveSetCompileCost.set(min_time, 1.0/transfer_bandwidth, 256); // min time 1/10th of millisecond, min size 256
    _arrayDrawCost.set(min_time, 1.0/gpu_bandwidth, 256); // min time 1/10th of millisecond, min size 256;
    _primtiveSetDrawCost.set(min_time, 1.0/gpu_bandwidth, 256); // min time 1/10th of millisecond, min size 256;

    _displayListCompileConstant = 0.0;
    _displayListCompileFactor = 10.0;
}

void GeometryCostEstimator::calibrate(osg::RenderInfo& /*renderInfo*/)
{
}

CostPair GeometryCostEstimator::estimateCompileCost(const osg::Geometry* geometry) const
{

    bool usesVBO = geometry->getUseVertexBufferObjects();
    bool usesDL = !usesVBO && geometry->getUseDisplayList() && geometry->getSupportsDisplayList();

    if (usesVBO || usesDL)
    {
        CostPair cost;
        if (geometry->getVertexArray()) { cost.first += _arrayCompileCost(geometry->getVertexArray()->getTotalDataSize()); }
        if (geometry->getNormalArray()) { cost.first += _arrayCompileCost(geometry->getNormalArray()->getTotalDataSize()); }
        if (geometry->getColorArray()) { cost.first += _arrayCompileCost(geometry->getColorArray()->getTotalDataSize()); }
        if (geometry->getSecondaryColorArray()) { cost.first += _arrayCompileCost(geometry->getSecondaryColorArray()->getTotalDataSize()); }
        if (geometry->getFogCoordArray()) { cost.first += _arrayCompileCost(geometry->getFogCoordArray()->getTotalDataSize()); }
        for(unsigned i=0; i<geometry->getNumTexCoordArrays(); ++i)
        {
            if (geometry->getTexCoordArray(i)) { cost.first += _arrayCompileCost(geometry->getTexCoordArray(i)->getTotalDataSize()); }
        }
        for(unsigned i=0; i<geometry->getNumVertexAttribArrays(); ++i)
        {
            if (geometry->getVertexAttribArray(i)) { cost.first += _arrayCompileCost(geometry->getVertexAttribArray(i)->getTotalDataSize()); }
        }
        for(unsigned i=0; i<geometry->getNumPrimitiveSets(); ++i)
        {
            const osg::PrimitiveSet* primSet = geometry->getPrimitiveSet(i);
            const osg::DrawElements* drawElements = primSet ? primSet->getDrawElements() : 0;
            if (drawElements) { cost.first += _primtiveSetCompileCost(drawElements->getTotalDataSize()); }
        }

        if (usesDL)
        {
            cost.first = _displayListCompileConstant + _displayListCompileFactor * cost.first ;
        }

        return cost;
    }
    else
    {
        return CostPair(0.0,0.0);
    }
}

CostPair GeometryCostEstimator::estimateDrawCost(const osg::Geometry* /*geometry*/) const
{
    return CostPair(0.0,0.0);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// TextureCostEstimator
//
TextureCostEstimator::TextureCostEstimator()
{
    setDefaults();
}

void TextureCostEstimator::setDefaults()
{
    double transfer_bandwidth = 10000000000.0; // 1GB/sec
    double gpu_bandwidth = 50000000000.0; // 50 GB/second
    double min_time = 0.00001; // 10 nano seconds.
    _compileCost.set(min_time, 1.0/transfer_bandwidth, 256); // min time 1/10th of millisecond, min size 256
    _drawCost.set(min_time, 1.0/gpu_bandwidth, 256); // min time 1/10th of millisecond, min size 256
}

void TextureCostEstimator::calibrate(osg::RenderInfo& /*renderInfo*/)
{
}

CostPair TextureCostEstimator::estimateCompileCost(const osg::Texture* texture) const
{
    CostPair cost;
    for(unsigned int i=0; i<texture->getNumImages(); ++i)
    {
        const osg::Image* image = texture->getImage(i);
        if (image) cost.first += _compileCost(image->getTotalDataSize());
    }
    OSG_NOTICE<<"TextureCostEstimator::estimateCompileCost(), size="<<cost.first<<std::endl;
    return cost;
}

CostPair TextureCostEstimator::estimateDrawCost(const osg::Texture* /*texture*/) const
{
    return CostPair(0.0,0.0);
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
// ProgramCostEstimator
//
ProgramCostEstimator::ProgramCostEstimator()
{
}

void ProgramCostEstimator::setDefaults()
{
}

void ProgramCostEstimator::calibrate(osg::RenderInfo& /*renderInfo*/)
{
}

CostPair ProgramCostEstimator::estimateCompileCost(const osg::Program* /*program*/) const
{
    return CostPair(0.0,0.0);
}

CostPair ProgramCostEstimator::estimateDrawCost(const osg::Program* /*program*/) const
{
    return CostPair(0.0,0.0);
}


/////////////////////////////////////////////////////////////////////////////////////////////
//
// GeometryCostEstimator
//
GraphicsCostEstimator::GraphicsCostEstimator()
{
    _geometryEstimator = new GeometryCostEstimator;
    _textureEstimator = new TextureCostEstimator;
    _programEstimator = new ProgramCostEstimator;
}

GraphicsCostEstimator::~GraphicsCostEstimator()
{
}

void GraphicsCostEstimator::setDefaults()
{
    _geometryEstimator->setDefaults();
    _textureEstimator->setDefaults();
    _programEstimator->setDefaults();
}


void GraphicsCostEstimator::calibrate(osg::RenderInfo& /*renderInfo*/)
{
    OSG_INFO<<"GraphicsCostEstimator::calibrate(..)"<<std::endl;
}

class CollectCompileCosts : public osg::NodeVisitor
{
public:
    CollectCompileCosts(const GraphicsCostEstimator* gce):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _gce(gce),
        _costs(0.0,0.0)
        {}

    virtual void apply(osg::Node& node)
    {
        apply(node.getStateSet());
        traverse(node);
    }

    virtual void apply(osg::Geode& geode)
    {
        apply(geode.getStateSet());
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(geode.getDrawable(i)->getStateSet());
            osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
            if (geometry) apply(geometry);
        }
    }

    void apply(osg::StateSet* stateset)
    {
        if (!stateset) return;
        if (_statesets.count(stateset)) return;
        _statesets.insert(stateset);

        const osg::Program* program = dynamic_cast<const osg::Program*>(stateset->getAttribute(osg::StateAttribute::PROGRAM));
        if (program)
        {
            CostPair cost = _gce->estimateCompileCost(program);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }

        for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i)
        {
            const osg::Texture* texture = dynamic_cast<const osg::Texture*>(stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            CostPair cost = _gce->estimateCompileCost(texture);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }
    }

    void apply(osg::Geometry* geometry)
    {
        if (!geometry) return;
        if (_geometries.count(geometry)) return;
        _geometries.insert(geometry);

        CostPair cost = _gce->estimateCompileCost(geometry);

        _costs.first += cost.first;
        _costs.second += cost.second;
    }


    typedef std::set<osg::StateSet*> StateSets;
    typedef std::set<osg::Texture*> Textures;
    typedef std::set<osg::Geometry*> Geometries;

    const GraphicsCostEstimator* _gce;
    StateSets   _statesets;
    Textures    _textures;
    Geometries  _geometries;
    CostPair    _costs;
};


class CollectDrawCosts : public osg::NodeVisitor
{
public:
    CollectDrawCosts(const GraphicsCostEstimator* gce):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN),
        _gce(gce),
        _costs(0.0,0.0)
        {}

    virtual void apply(osg::Node& node)
    {
        apply(node.getStateSet());
        traverse(node);
    }

    virtual void apply(osg::Geode& geode)
    {
        apply(geode.getStateSet());
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(geode.getDrawable(i)->getStateSet());
            osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
            if (geometry) apply(geometry);
        }
    }

    void apply(osg::StateSet* stateset)
    {
        if (!stateset) return;

        const osg::Program* program = dynamic_cast<const osg::Program*>(stateset->getAttribute(osg::StateAttribute::PROGRAM));
        if (program)
        {
            CostPair cost = _gce->estimateDrawCost(program);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }

        for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i)
        {
            const osg::Texture* texture = dynamic_cast<const osg::Texture*>(stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            CostPair cost = _gce->estimateDrawCost(texture);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }
    }

    void apply(osg::Geometry* geometry)
    {
        if (!geometry) return;

        CostPair cost = _gce->estimateDrawCost(geometry);
        _costs.first += cost.first;
        _costs.second += cost.second;
    }

    const GraphicsCostEstimator* _gce;
    CostPair    _costs;
};

CostPair GraphicsCostEstimator::estimateCompileCost(const osg::Node* node) const
{
    if (!node) return CostPair(0.0,0.0);
    CollectCompileCosts ccc(this);
    const_cast<osg::Node*>(node)->accept(ccc);
    return ccc._costs;
}

CostPair GraphicsCostEstimator::estimateDrawCost(const osg::Node* node) const
{
    if (!node) return CostPair(0.0,0.0);
    CollectDrawCosts cdc(this);
    const_cast<osg::Node*>(node)->accept(cdc);
    return cdc._costs;
}

}
