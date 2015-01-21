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

#include <osgTerrain/GeometryTechnique>
#include <osgTerrain/TerrainTile>
#include <osgTerrain/Terrain>

#include <osgUtil/MeshOptimizers>

#include <osgDB/FileUtils>

#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/Program>
#include <osg/Math>
#include <osg/Timer>

using namespace osgTerrain;

GeometryTechnique::GeometryTechnique()
{
    setFilterBias(0);
    setFilterWidth(0.1);
    setFilterMatrixAs(GAUSSIAN);

}

GeometryTechnique::GeometryTechnique(const GeometryTechnique& gt,const osg::CopyOp& copyop):
    TerrainTechnique(gt,copyop)
{
    setFilterBias(gt._filterBias);
    setFilterWidth(gt._filterWidth);
    setFilterMatrix(gt._filterMatrix);
}

GeometryTechnique::~GeometryTechnique()
{
}

void GeometryTechnique::setFilterBias(float filterBias)
{
    _filterBias = filterBias;
    if (!_filterBiasUniform) _filterBiasUniform = new osg::Uniform("filterBias",_filterBias);
    else _filterBiasUniform->set(filterBias);
}

void GeometryTechnique::setFilterWidth(float filterWidth)
{
    _filterWidth = filterWidth;
    if (!_filterWidthUniform) _filterWidthUniform = new osg::Uniform("filterWidth",_filterWidth);
    else _filterWidthUniform->set(filterWidth);
}

void GeometryTechnique::setFilterMatrix(const osg::Matrix3& matrix)
{
    _filterMatrix = matrix;
    if (!_filterMatrixUniform) _filterMatrixUniform = new osg::Uniform("filterMatrix",_filterMatrix);
    else _filterMatrixUniform->set(_filterMatrix);
}

void GeometryTechnique::setFilterMatrixAs(FilterType filterType)
{
    switch(filterType)
    {
        case(SMOOTH):
            setFilterMatrix(osg::Matrix3(0.0, 0.5/2.5, 0.0,
                                         0.5/2.5, 0.5/2.5, 0.5/2.5,
                                         0.0, 0.5/2.5, 0.0));
            break;
        case(GAUSSIAN):
            setFilterMatrix(osg::Matrix3(0.0, 1.0/8.0, 0.0,
                                         1.0/8.0, 4.0/8.0, 1.0/8.0,
                                         0.0, 1.0/8.0, 0.0));
            break;
        case(SHARPEN):
            setFilterMatrix(osg::Matrix3(0.0, -1.0, 0.0,
                                         -1.0, 5.0, -1.0,
                                         0.0, -1.0, 0.0));
            break;

    };
}

void GeometryTechnique::init(int dirtyMask, bool assumeMultiThreaded)
{
    OSG_INFO<<"Doing GeometryTechnique::init()"<<std::endl;

    if (!_terrainTile) return;

    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_writeBufferMutex);

    // take a temporary referecen
    osg::ref_ptr<TerrainTile> tile = _terrainTile;

    if (dirtyMask==0) return;

    osg::ref_ptr<BufferData> buffer = new BufferData;

    Locator* masterLocator = computeMasterLocator();

    osg::Vec3d centerModel = computeCenterModel(*buffer, masterLocator);

    if ((dirtyMask & TerrainTile::IMAGERY_DIRTY)==0)
    {
        generateGeometry(*buffer, masterLocator, centerModel);

        osg::ref_ptr<BufferData> read_buffer = _currentBufferData;

        osg::StateSet* stateset = read_buffer->_geode->getStateSet();
        if (stateset)
        {
            // OSG_NOTICE<<"Reusing StateSet"<<std::endl;
            buffer->_geode->setStateSet(stateset);
        }
        else
        {
            applyColorLayers(*buffer);
            applyTransparency(*buffer);
        }
    }
    else
    {
        generateGeometry(*buffer, masterLocator, centerModel);
        applyColorLayers(*buffer);
        applyTransparency(*buffer);
    }

    if (buffer->_transform.valid()) buffer->_transform->setThreadSafeRefUnref(true);

    if (!_currentBufferData || !assumeMultiThreaded)
    {
        // no currentBufferData so we must be the first init to be applied
        _currentBufferData = buffer;
    }
    else
    {
        // there is already an active _currentBufferData so we'll request that this gets swapped on next frame.
        _newBufferData = buffer;
        if (_terrainTile->getTerrain()) _terrainTile->getTerrain()->updateTerrainTileOnNextFrame(_terrainTile);
    }

    _terrainTile->setDirtyMask(0);
}

Locator* GeometryTechnique::computeMasterLocator()
{
    osgTerrain::Layer* elevationLayer = _terrainTile->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(0);

    Locator* elevationLocator = elevationLayer ? elevationLayer->getLocator() : 0;
    Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;

    Locator* masterLocator = elevationLocator ? elevationLocator : colorLocator;
    if (!masterLocator)
    {
        OSG_NOTICE<<"Problem, no locator found in any of the terrain layers"<<std::endl;
        return 0;
    }

    return masterLocator;
}

osg::Vec3d GeometryTechnique::computeCenterModel(BufferData& buffer, Locator* masterLocator)
{
    if (!masterLocator) return osg::Vec3d(0.0,0.0,0.0);

    osgTerrain::Layer* elevationLayer = _terrainTile->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(0);

    Locator* elevationLocator = elevationLayer ? elevationLayer->getLocator() : 0;
    Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;

    if (!elevationLocator) elevationLocator = masterLocator;
    if (!colorLocator) colorLocator = masterLocator;

    osg::Vec3d bottomLeftNDC(DBL_MAX, DBL_MAX, 0.0);
    osg::Vec3d topRightNDC(-DBL_MAX, -DBL_MAX, 0.0);

    if (elevationLayer)
    {
        if (elevationLocator!= masterLocator)
        {
            masterLocator->computeLocalBounds(*elevationLocator, bottomLeftNDC, topRightNDC);
        }
        else
        {
            bottomLeftNDC.x() = osg::minimum(bottomLeftNDC.x(), 0.0);
            bottomLeftNDC.y() = osg::minimum(bottomLeftNDC.y(), 0.0);
            topRightNDC.x() = osg::maximum(topRightNDC.x(), 1.0);
            topRightNDC.y() = osg::maximum(topRightNDC.y(), 1.0);
        }
    }

    if (colorLayer)
    {
        if (colorLocator!= masterLocator)
        {
            masterLocator->computeLocalBounds(*colorLocator, bottomLeftNDC, topRightNDC);
        }
        else
        {
            bottomLeftNDC.x() = osg::minimum(bottomLeftNDC.x(), 0.0);
            bottomLeftNDC.y() = osg::minimum(bottomLeftNDC.y(), 0.0);
            topRightNDC.x() = osg::maximum(topRightNDC.x(), 1.0);
            topRightNDC.y() = osg::maximum(topRightNDC.y(), 1.0);
        }
    }

    OSG_INFO<<"bottomLeftNDC = "<<bottomLeftNDC<<std::endl;
    OSG_INFO<<"topRightNDC = "<<topRightNDC<<std::endl;

    buffer._transform = new osg::MatrixTransform;

    osg::Vec3d centerNDC = (bottomLeftNDC + topRightNDC)*0.5;
    osg::Vec3d centerModel = (bottomLeftNDC + topRightNDC)*0.5;
    masterLocator->convertLocalToModel(centerNDC, centerModel);

    buffer._transform->setMatrix(osg::Matrix::translate(centerModel));

    return centerModel;
}

class VertexNormalGenerator
{
    public:

        typedef std::vector<int> Indices;
        typedef std::pair< osg::ref_ptr<osg::Vec2Array>, Locator* > TexCoordLocatorPair;
        typedef std::map< Layer*, TexCoordLocatorPair > LayerToTexCoordMap;

        VertexNormalGenerator(Locator* masterLocator, const osg::Vec3d& centerModel, int numRows, int numColmns, float scaleHeight, bool createSkirt);

        void populateCenter(osgTerrain::Layer* elevationLayer, LayerToTexCoordMap& layerToTexCoordMap);
        void populateLeftBoundary(osgTerrain::Layer* elevationLayer);
        void populateRightBoundary(osgTerrain::Layer* elevationLayer);
        void populateAboveBoundary(osgTerrain::Layer* elevationLayer);
        void populateBelowBoundary(osgTerrain::Layer* elevationLayer);

        void computeNormals();

        unsigned int capacity() const { return _vertices->capacity(); }

        inline void setVertex(int c, int r, const osg::Vec3& v, const osg::Vec3& n)
        {
            int& i = index(c,r);
            if (i==0)
            {
                if (r<0 || r>=_numRows || c<0 || c>=_numColumns)
                {
                    i = -(1+static_cast<int>(_boundaryVertices->size()));
                    _boundaryVertices->push_back(v);
                    // OSG_NOTICE<<"setVertex("<<c<<", "<<r<<", ["<<v<<"], ["<<n<<"]), i="<<i<<" _boundaryVertices["<<-i-1<<"]="<<(*_boundaryVertices)[-i-1]<<"]"<<std::endl;
                }
                else
                {
                    i = _vertices->size() + 1;
                    _vertices->push_back(v);
                    _normals->push_back(n);
                    // OSG_NOTICE<<"setVertex("<<c<<", "<<r<<", ["<<v<<"], ["<<n<<"]), i="<<i<<" _vertices["<<i-1<<"]="<<(*_vertices)[i-1]<<"]"<<std::endl;
                }
            }
            else if (i<0)
            {
                (*_boundaryVertices)[-i-1] = v;
                // OSG_NOTICE<<"setVertex("<<c<<", "<<r<<", ["<<v<<"], ["<<n<<"] _boundaryVertices["<<-i-1<<"]="<<(*_boundaryVertices)[-i-1]<<"]"<<std::endl;
            }
            else
            {
                // OSG_NOTICE<<"Overwriting setVertex("<<c<<", "<<r<<", ["<<v<<"], ["<<n<<"]"<<std::endl;
                // OSG_NOTICE<<"     previous values ( vertex ["<<(*_vertices)[i-1]<<"], normal (*_normals)[i-1] ["<<n<<"]"<<std::endl;
                // (*_vertices)[i-1] = v;

                // average the vertex positions
                (*_vertices)[i-1] = ((*_vertices)[i-1] + v)*0.5f;

                (*_normals)[i-1] = n;
            }
        }

        inline int& index(int c, int r) { return _indices[(r+1)*(_numColumns+2)+c+1]; }

        inline int index(int c, int r) const { return _indices[(r+1)*(_numColumns+2)+c+1]; }

        inline int vertex_index(int c, int r) const { int i = _indices[(r+1)*(_numColumns+2)+c+1]; return i-1; }

        inline bool vertex(int c, int r, osg::Vec3& v) const
        {
            int i = index(c,r);
            if (i==0) return false;
            if (i<0) v = (*_boundaryVertices)[-i-1];
            else v = (*_vertices)[i-1];
            return true;
        }

        inline bool computeNormal(int c, int r, osg::Vec3& n) const
        {
#if 1
            return computeNormalWithNoDiagonals(c,r,n);
#else
            return computeNormalWithDiagonals(c,r,n);
#endif
        }

        inline bool computeNormalWithNoDiagonals(int c, int r, osg::Vec3& n) const
        {
            osg::Vec3 center;
            bool center_valid  = vertex(c, r,  center);
            if (!center_valid) return false;

            osg::Vec3 left, right, top,  bottom;
            bool left_valid  = vertex(c-1, r,  left);
            bool right_valid = vertex(c+1, r,   right);
            bool bottom_valid = vertex(c,   r-1, bottom);
            bool top_valid = vertex(c,   r+1, top);

            osg::Vec3 dx(0.0f,0.0f,0.0f);
            osg::Vec3 dy(0.0f,0.0f,0.0f);
            osg::Vec3 zero(0.0f,0.0f,0.0f);
            if (left_valid)
            {
                dx = center-left;
            }
            if (right_valid)
            {
                dx = right-center;
            }
            if (bottom_valid)
            {
                dy += center-bottom;
            }
            if (top_valid)
            {
                dy += top-center;
            }

            if (dx==zero || dy==zero) return false;

            n = dx ^ dy;
            return n.normalize() != 0.0f;
        }

        inline bool computeNormalWithDiagonals(int c, int r, osg::Vec3& n) const
        {
            osg::Vec3 center;
            bool center_valid  = vertex(c, r,  center);
            if (!center_valid) return false;

            osg::Vec3 top_left, top_right, bottom_left, bottom_right;
            bool top_left_valid  = vertex(c-1, r+1,  top_left);
            bool top_right_valid  = vertex(c+1, r+1,  top_right);
            bool bottom_left_valid  = vertex(c-1, r-1,  bottom_left);
            bool bottom_right_valid  = vertex(c+1, r-1,  bottom_right);

            osg::Vec3 left, right, top,  bottom;
            bool left_valid  = vertex(c-1, r,  left);
            bool right_valid = vertex(c+1, r,   right);
            bool bottom_valid = vertex(c,   r-1, bottom);
            bool top_valid = vertex(c,   r+1, top);

            osg::Vec3 dx(0.0f,0.0f,0.0f);
            osg::Vec3 dy(0.0f,0.0f,0.0f);
            osg::Vec3 zero(0.0f,0.0f,0.0f);
            const float ratio = 0.5f;
            if (left_valid)
            {
                dx = center-left;
                if (top_left_valid) dy += (top_left-left)*ratio;
                if (bottom_left_valid) dy += (left-bottom_left)*ratio;
            }
            if (right_valid)
            {
                dx = right-center;
                if (top_right_valid) dy += (top_right-right)*ratio;
                if (bottom_right_valid) dy += (right-bottom_right)*ratio;
            }
            if (bottom_valid)
            {
                dy += center-bottom;
                if (bottom_left_valid) dx += (bottom-bottom_left)*ratio;
                if (bottom_right_valid) dx += (bottom_right-bottom)*ratio;
            }
            if (top_valid)
            {
                dy += top-center;
                if (top_left_valid) dx += (top-top_left)*ratio;
                if (top_right_valid) dx += (top_right-top)*ratio;
            }

            if (dx==zero || dy==zero) return false;

            n = dx ^ dy;
            return n.normalize() != 0.0f;
        }

        Locator*                        _masterLocator;
        const osg::Vec3d                _centerModel;
        int                             _numRows;
        int                             _numColumns;
        float                           _scaleHeight;

        Indices                         _indices;

        osg::ref_ptr<osg::Vec3Array>    _vertices;
        osg::ref_ptr<osg::Vec3Array>    _normals;
        osg::ref_ptr<osg::FloatArray>   _elevations;

        osg::ref_ptr<osg::Vec3Array>    _boundaryVertices;

};

VertexNormalGenerator::VertexNormalGenerator(Locator* masterLocator, const osg::Vec3d& centerModel, int numRows, int numColumns, float scaleHeight, bool createSkirt):
    _masterLocator(masterLocator),
    _centerModel(centerModel),
    _numRows(numRows),
    _numColumns(numColumns),
    _scaleHeight(scaleHeight)
{
    int numVerticesInBody = numColumns*numRows;
    int numVerticesInSkirt = createSkirt ? numColumns*2 + numRows*2 - 4 : 0;
    int numVertices = numVerticesInBody+numVerticesInSkirt;

    _indices.resize((_numRows+2)*(_numColumns+2),0);

    _vertices = new osg::Vec3Array;
    _vertices->reserve(numVertices);

    _normals = new osg::Vec3Array;
    _normals->reserve(numVertices);

    _elevations = new osg::FloatArray;
    _elevations->reserve(numVertices);

    _boundaryVertices = new osg::Vec3Array;
    _boundaryVertices->reserve(_numRows*2 + _numColumns*2 + 4);
}

void VertexNormalGenerator::populateCenter(osgTerrain::Layer* elevationLayer, LayerToTexCoordMap& layerToTexCoordMap)
{
    // OSG_NOTICE<<std::endl<<"VertexNormalGenerator::populateCenter("<<elevationLayer<<")"<<std::endl;

    bool sampled = elevationLayer &&
                   ( (elevationLayer->getNumRows()!=static_cast<unsigned int>(_numRows)) ||
                     (elevationLayer->getNumColumns()!=static_cast<unsigned int>(_numColumns)) );

    for(int j=0; j<_numRows; ++j)
    {
        for(int i=0; i<_numColumns; ++i)
        {
            osg::Vec3d ndc( ((double)i)/(double)(_numColumns-1), ((double)j)/(double)(_numRows-1), 0.0);

            bool validValue = true;
            if (elevationLayer)
            {
                float value = 0.0f;
                if (sampled) validValue = elevationLayer->getInterpolatedValidValue(ndc.x(), ndc.y(), value);
                else validValue = elevationLayer->getValidValue(i,j,value);
                ndc.z() = value*_scaleHeight;
            }

            if (validValue)
            {
                osg::Vec3d model;
                _masterLocator->convertLocalToModel(ndc, model);

                for(VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    osg::Vec2Array* texcoords = itr->second.first.get();
                    osgTerrain::ImageLayer* imageLayer(dynamic_cast<osgTerrain::ImageLayer*>(itr->first));

                    if (imageLayer != NULL)
                    {
                        Locator* colorLocator = itr->second.second;
                        if (colorLocator != _masterLocator)
                        {
                            osg::Vec3d color_ndc;
                            Locator::convertLocalCoordBetween(*_masterLocator, ndc, *colorLocator, color_ndc);
                            (*texcoords).push_back(osg::Vec2(color_ndc.x(), color_ndc.y()));
                        }
                        else
                        {
                            (*texcoords).push_back(osg::Vec2(ndc.x(), ndc.y()));
                        }
                    }
                    else
                    {
                        osgTerrain::ContourLayer* contourLayer(dynamic_cast<osgTerrain::ContourLayer*>(itr->first));

                        bool texCoordSet = false;
                        if (contourLayer)
                        {
                            osg::TransferFunction1D* transferFunction = contourLayer->getTransferFunction();
                            if (transferFunction)
                            {
                                float difference = transferFunction->getMaximum()-transferFunction->getMinimum();
                                if (difference != 0.0f)
                                {
                                    osg::Vec3d           color_ndc;
                                    osgTerrain::Locator* colorLocator(itr->second.second);

                                    if (colorLocator != _masterLocator)
                                    {
                                        Locator::convertLocalCoordBetween(*_masterLocator,ndc,*colorLocator,color_ndc);
                                    }
                                    else
                                    {
                                        color_ndc = ndc;
                                    }

                                    color_ndc[2] /= _scaleHeight;

                                    (*texcoords).push_back(osg::Vec2((color_ndc[2]-transferFunction->getMinimum())/difference,0.0f));
                                    texCoordSet = true;
                                }
                            }
                        }
                        if (!texCoordSet)
                        {
                            (*texcoords).push_back(osg::Vec2(0.0f,0.0f));
                        }
                    }
                }

                if (_elevations.valid())
                {
                    (*_elevations).push_back(ndc.z());
                }

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                _masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();

                setVertex(i, j, osg::Vec3(model-_centerModel), model_one);
            }
        }
    }
}

void VertexNormalGenerator::populateLeftBoundary(osgTerrain::Layer* elevationLayer)
{
    // OSG_NOTICE<<"   VertexNormalGenerator::populateLeftBoundary("<<elevationLayer<<")"<<std::endl;

    if (!elevationLayer) return;

    bool sampled = elevationLayer &&
                   ( (elevationLayer->getNumRows()!=static_cast<unsigned int>(_numRows)) ||
                     (elevationLayer->getNumColumns()!=static_cast<unsigned int>(_numColumns)) );

    for(int j=0; j<_numRows; ++j)
    {
        for(int i=-1; i<=0; ++i)
        {
            osg::Vec3d ndc( ((double)i)/(double)(_numColumns-1), ((double)j)/(double)(_numRows-1), 0.0);
            osg::Vec3d left_ndc( 1.0+ndc.x(), ndc.y(), 0.0);

            bool validValue = true;
            if (elevationLayer)
            {
                float value = 0.0f;
                if (sampled) validValue = elevationLayer->getInterpolatedValidValue(left_ndc.x(), left_ndc.y(), value);
                else validValue = elevationLayer->getValidValue((_numColumns-1)+i,j,value);
                ndc.z() = value*_scaleHeight;

                ndc.z() += 0.f;
            }
            if (validValue)
            {
                osg::Vec3d model;
                _masterLocator->convertLocalToModel(ndc, model);

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                _masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();

                setVertex(i, j, osg::Vec3(model-_centerModel), model_one);
                // OSG_NOTICE<<"       setVertex("<<i<<", "<<j<<"..)"<<std::endl;
            }
        }
    }
}

void VertexNormalGenerator::populateRightBoundary(osgTerrain::Layer* elevationLayer)
{
    // OSG_NOTICE<<"   VertexNormalGenerator::populateRightBoundary("<<elevationLayer<<")"<<std::endl;

    if (!elevationLayer) return;

    bool sampled = elevationLayer &&
                   ( (elevationLayer->getNumRows()!=static_cast<unsigned int>(_numRows)) ||
                     (elevationLayer->getNumColumns()!=static_cast<unsigned int>(_numColumns)) );

    for(int j=0; j<_numRows; ++j)
    {
        for(int i=_numColumns-1; i<_numColumns+1; ++i)
        {
            osg::Vec3d ndc( ((double)i)/(double)(_numColumns-1), ((double)j)/(double)(_numRows-1), 0.0);
            osg::Vec3d right_ndc(ndc.x()-1.0, ndc.y(), 0.0);

            bool validValue = true;
            if (elevationLayer)
            {
                float value = 0.0f;
                if (sampled) validValue = elevationLayer->getInterpolatedValidValue(right_ndc.x(), right_ndc.y(), value);
                else validValue = elevationLayer->getValidValue(i-(_numColumns-1),j,value);
                ndc.z() = value*_scaleHeight;
            }

            if (validValue)
            {
                osg::Vec3d model;
                _masterLocator->convertLocalToModel(ndc, model);

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                _masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();

                setVertex(i, j, osg::Vec3(model-_centerModel), model_one);
                // OSG_NOTICE<<"       setVertex("<<i<<", "<<j<<"..)"<<std::endl;
            }
        }
    }
}

void VertexNormalGenerator::populateAboveBoundary(osgTerrain::Layer* elevationLayer)
{
    // OSG_NOTICE<<"   VertexNormalGenerator::populateAboveBoundary("<<elevationLayer<<")"<<std::endl;

    if (!elevationLayer) return;

    bool sampled = elevationLayer &&
                   ( (elevationLayer->getNumRows()!=static_cast<unsigned int>(_numRows)) ||
                     (elevationLayer->getNumColumns()!=static_cast<unsigned int>(_numColumns)) );

    for(int j=_numRows-1; j<_numRows+1; ++j)
    {
        for(int i=0; i<_numColumns; ++i)
        {
            osg::Vec3d ndc( ((double)i)/(double)(_numColumns-1), ((double)j)/(double)(_numRows-1), 0.0);
            osg::Vec3d above_ndc( ndc.x(), ndc.y()-1.0, 0.0);

            bool validValue = true;
            if (elevationLayer)
            {
                float value = 0.0f;
                if (sampled) validValue = elevationLayer->getInterpolatedValidValue(above_ndc.x(), above_ndc.y(), value);
                else validValue = elevationLayer->getValidValue(i,j-(_numRows-1),value);
                ndc.z() = value*_scaleHeight;
            }

            if (validValue)
            {
                osg::Vec3d model;
                _masterLocator->convertLocalToModel(ndc, model);

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                _masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();

                setVertex(i, j, osg::Vec3(model-_centerModel), model_one);
                // OSG_NOTICE<<"       setVertex("<<i<<", "<<j<<"..)"<<std::endl;
            }
        }
    }
}

void VertexNormalGenerator::populateBelowBoundary(osgTerrain::Layer* elevationLayer)
{
    // OSG_NOTICE<<"   VertexNormalGenerator::populateBelowBoundary("<<elevationLayer<<")"<<std::endl;

    if (!elevationLayer) return;

    bool sampled = elevationLayer &&
                   ( (elevationLayer->getNumRows()!=static_cast<unsigned int>(_numRows)) ||
                     (elevationLayer->getNumColumns()!=static_cast<unsigned int>(_numColumns)) );

    for(int j=-1; j<=0; ++j)
    {
        for(int i=0; i<_numColumns; ++i)
        {
            osg::Vec3d ndc( ((double)i)/(double)(_numColumns-1), ((double)j)/(double)(_numRows-1), 0.0);
            osg::Vec3d below_ndc( ndc.x(), 1.0+ndc.y(), 0.0);

            bool validValue = true;
            if (elevationLayer)
            {
                float value = 0.0f;
                if (sampled) validValue = elevationLayer->getInterpolatedValidValue(below_ndc.x(), below_ndc.y(), value);
                else validValue = elevationLayer->getValidValue(i,(_numRows-1)+j,value);
                ndc.z() = value*_scaleHeight;
            }

            if (validValue)
            {
                osg::Vec3d model;
                _masterLocator->convertLocalToModel(ndc, model);

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                _masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();

                setVertex(i, j, osg::Vec3(model-_centerModel), model_one);
                // OSG_NOTICE<<"       setVertex("<<i<<", "<<j<<"..)"<<std::endl;
            }
        }
    }
}


void VertexNormalGenerator::computeNormals()
{
    // compute normals for the center section
    for(int j=0; j<_numRows; ++j)
    {
        for(int i=0; i<_numColumns; ++i)
        {
            int vi = vertex_index(i, j);
            if (vi>=0) computeNormal(i, j, (*_normals)[vi]);
            else OSG_NOTICE<<"Not computing normal, vi="<<vi<<std::endl;
        }
    }
}

void GeometryTechnique::generateGeometry(BufferData& buffer, Locator* masterLocator, const osg::Vec3d& centerModel)
{
    Terrain* terrain = _terrainTile->getTerrain();
    osgTerrain::Layer* elevationLayer = _terrainTile->getElevationLayer();


    buffer._geode = new osg::Geode;
    if(buffer._transform.valid())
        buffer._transform->addChild(buffer._geode.get());

    buffer._geometry = new osg::Geometry;
    buffer._geode->addDrawable(buffer._geometry.get());

    osg::Geometry* geometry = buffer._geometry.get();

    unsigned int numRows = 20;
    unsigned int numColumns = 20;

    if (elevationLayer)
    {
        numColumns = elevationLayer->getNumColumns();
        numRows = elevationLayer->getNumRows();
    }

    float sampleRatio = terrain ? terrain->getSampleRatio() : 1.0f;

    // OSG_NOTICE<<"Sample ratio="<<sampleRatio<<std::endl;

    unsigned int minimumNumColumns = 16u;
    unsigned int minimumNumRows = 16u;

    if ((sampleRatio!=1.0f) && (numColumns>minimumNumColumns) && (numRows>minimumNumRows))
    {
        unsigned int originalNumColumns = numColumns;
        unsigned int originalNumRows = numRows;

        numColumns = std::max((unsigned int) (float(originalNumColumns)*sqrtf(sampleRatio)), minimumNumColumns);
        numRows = std::max((unsigned int) (float(originalNumRows)*sqrtf(sampleRatio)),minimumNumRows);
    }



    bool treatBoundariesToValidDataAsDefaultValue = _terrainTile->getTreatBoundariesToValidDataAsDefaultValue();
    OSG_INFO<<"TreatBoundariesToValidDataAsDefaultValue="<<treatBoundariesToValidDataAsDefaultValue<<std::endl;

    float skirtHeight = 0.0f;
    HeightFieldLayer* hfl = dynamic_cast<HeightFieldLayer*>(elevationLayer);
    if (hfl && hfl->getHeightField())
    {
        skirtHeight = hfl->getHeightField()->getSkirtHeight();
    }

    bool createSkirt = skirtHeight != 0.0f;


    float scaleHeight = terrain ? terrain->getVerticalScale() : 1.0f;

    // construct the VertexNormalGenerator which will manage the generation and the vertices and normals
    VertexNormalGenerator VNG(masterLocator, centerModel, numRows, numColumns, scaleHeight, createSkirt);

    unsigned int numVertices = VNG.capacity();

    // allocate and assign vertices
    geometry->setVertexArray(VNG._vertices.get());

    // allocate and assign normals
    geometry->setNormalArray(VNG._normals.get(), osg::Array::BIND_PER_VERTEX);


    // allocate and assign tex coords
    // typedef std::pair< osg::ref_ptr<osg::Vec2Array>, Locator* > TexCoordLocatorPair;
    // typedef std::map< Layer*, TexCoordLocatorPair > LayerToTexCoordMap;

    VertexNormalGenerator::LayerToTexCoordMap layerToTexCoordMap;
    for(unsigned int layerNum=0; layerNum<_terrainTile->getNumColorLayers(); ++layerNum)
    {
        osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(layerNum);
        if (colorLayer)
        {
            VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.find(colorLayer);
            if (itr!=layerToTexCoordMap.end())
            {
                geometry->setTexCoordArray(layerNum, itr->second.first.get());
            }
            else
            {

                Locator* locator = colorLayer->getLocator();
                if (!locator)
                {
                    osgTerrain::SwitchLayer* switchLayer = dynamic_cast<osgTerrain::SwitchLayer*>(colorLayer);
                    if (switchLayer)
                    {
                        if (switchLayer->getActiveLayer()>=0 &&
                            static_cast<unsigned int>(switchLayer->getActiveLayer())<switchLayer->getNumLayers() &&
                            switchLayer->getLayer(switchLayer->getActiveLayer()))
                        {
                            locator = switchLayer->getLayer(switchLayer->getActiveLayer())->getLocator();
                        }
                    }
                }

                VertexNormalGenerator::TexCoordLocatorPair& tclp = layerToTexCoordMap[colorLayer];
                tclp.first = new osg::Vec2Array;
                tclp.first->reserve(numVertices);
                tclp.second = locator ? locator : masterLocator;
                geometry->setTexCoordArray(layerNum, tclp.first.get());
            }
        }
    }

    // allocate and assign color
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);

    geometry->setColorArray(colors.get(), osg::Array::BIND_OVERALL);

    //
    // populate vertex and tex coord arrays
    //
    VNG.populateCenter(elevationLayer, layerToTexCoordMap);

    if (terrain && terrain->getEqualizeBoundaries())
    {
        TileID tileID = _terrainTile->getTileID();

        osg::ref_ptr<TerrainTile> left_tile  = terrain->getTile(TileID(tileID.level, tileID.x-1, tileID.y));
        osg::ref_ptr<TerrainTile> right_tile = terrain->getTile(TileID(tileID.level, tileID.x+1, tileID.y));
        osg::ref_ptr<TerrainTile> top_tile = terrain->getTile(TileID(tileID.level, tileID.x, tileID.y+1));
        osg::ref_ptr<TerrainTile> bottom_tile = terrain->getTile(TileID(tileID.level, tileID.x, tileID.y-1));

#if 0
        osg::ref_ptr<TerrainTile> top_left_tile  = terrain->getTile(TileID(tileID.level, tileID.x-1, tileID.y+1));
        osg::ref_ptr<TerrainTile> top_right_tile = terrain->getTile(TileID(tileID.level, tileID.x+1, tileID.y+1));
        osg::ref_ptr<TerrainTile> bottom_left_tile = terrain->getTile(TileID(tileID.level, tileID.x-1, tileID.y-1));
        osg::ref_ptr<TerrainTile> bottom_right_tile = terrain->getTile(TileID(tileID.level, tileID.x+1, tileID.y-1));
#endif
        VNG.populateLeftBoundary(left_tile.valid() ? left_tile->getElevationLayer() : 0);
        VNG.populateRightBoundary(right_tile.valid() ? right_tile->getElevationLayer() : 0);
        VNG.populateAboveBoundary(top_tile.valid() ? top_tile->getElevationLayer() : 0);
        VNG.populateBelowBoundary(bottom_tile.valid() ? bottom_tile->getElevationLayer() : 0);

        _neighbours.clear();

        bool updateNeighboursImmediately = false;

        if (left_tile.valid())   addNeighbour(left_tile.get());
        if (right_tile.valid())  addNeighbour(right_tile.get());
        if (top_tile.valid())    addNeighbour(top_tile.get());
        if (bottom_tile.valid()) addNeighbour(bottom_tile.get());

#if 0
        if (bottom_left_tile.valid()) addNeighbour(bottom_left_tile.get());
        if (bottom_right_tile.valid()) addNeighbour(bottom_right_tile.get());
        if (top_left_tile.valid()) addNeighbour(top_left_tile.get());
        if (top_right_tile.valid()) addNeighbour(top_right_tile.get());
#endif

        if (left_tile.valid())
        {
            if (!(left_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = left_tile->getDirtyMask() | TerrainTile::LEFT_EDGE_DIRTY;
                if (updateNeighboursImmediately) left_tile->init(dirtyMask, true);
                else left_tile->setDirtyMask(dirtyMask);
            }
        }
        if (right_tile.valid())
        {
            if (!(right_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = right_tile->getDirtyMask() | TerrainTile::RIGHT_EDGE_DIRTY;
                if (updateNeighboursImmediately) right_tile->init(dirtyMask, true);
                else right_tile->setDirtyMask(dirtyMask);
            }
        }
        if (top_tile.valid())
        {
            if (!(top_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = top_tile->getDirtyMask() | TerrainTile::TOP_EDGE_DIRTY;
                if (updateNeighboursImmediately) top_tile->init(dirtyMask, true);
                else top_tile->setDirtyMask(dirtyMask);
            }
        }

        if (bottom_tile.valid())
        {
            if (!(bottom_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = bottom_tile->getDirtyMask() | TerrainTile::BOTTOM_EDGE_DIRTY;
                if (updateNeighboursImmediately) bottom_tile->init(dirtyMask, true);
                else bottom_tile->setDirtyMask(dirtyMask);
            }
        }

#if 0
        if (bottom_left_tile.valid())
        {
            if (!(bottom_left_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = bottom_left_tile->getDirtyMask() | TerrainTile::BOTTOM_LEFT_CORNER_DIRTY;
                if (updateNeighboursImmediately) bottom_left_tile->init(dirtyMask, true);
                else bottom_left_tile->setDirtyMask(dirtyMask);
            }
        }

        if (bottom_right_tile.valid())
        {
            if (!(bottom_right_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = bottom_right_tile->getDirtyMask() | TerrainTile::BOTTOM_RIGHT_CORNER_DIRTY;
                if (updateNeighboursImmediately) bottom_right_tile->init(dirtyMask, true);
                else bottom_right_tile->setDirtyMask(dirtyMask);
            }
        }

        if (top_right_tile.valid())
        {
            if (!(top_right_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = top_right_tile->getDirtyMask() | TerrainTile::TOP_RIGHT_CORNER_DIRTY;
                if (updateNeighboursImmediately) top_right_tile->init(dirtyMask, true);
                else top_right_tile->setDirtyMask(dirtyMask);
            }
        }

        if (top_left_tile.valid())
        {
            if (!(top_left_tile->getTerrainTechnique()->containsNeighbour(_terrainTile)))
            {
                int dirtyMask = top_left_tile->getDirtyMask() | TerrainTile::TOP_LEFT_CORNER_DIRTY;
                if (updateNeighboursImmediately) top_left_tile->init(dirtyMask, true);
                else top_left_tile->setDirtyMask(dirtyMask);
            }
        }
#endif
    }


    osg::ref_ptr<osg::Vec3Array> skirtVectors = new osg::Vec3Array((*VNG._normals));
    VNG.computeNormals();

    //
    // populate the primitive data
    //
    bool swapOrientation = !(masterLocator->orientationOpenGL());
    bool smallTile = numVertices <= 16384;

    // OSG_NOTICE<<"smallTile = "<<smallTile<<std::endl;

    osg::ref_ptr<osg::DrawElements> elements = smallTile ?
        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_TRIANGLES)) :
        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_TRIANGLES));

    elements->reserveElements((numRows-1) * (numColumns-1) * 6);

    geometry->addPrimitiveSet(elements.get());


    unsigned int i, j;
    for(j=0; j<numRows-1; ++j)
    {
        for(i=0; i<numColumns-1; ++i)
        {
            // remap indices to final vertex positions
            int i00 = VNG.vertex_index(i,   j);
            int i01 = VNG.vertex_index(i,   j+1);
            int i10 = VNG.vertex_index(i+1, j);
            int i11 = VNG.vertex_index(i+1, j+1);

            if (swapOrientation)
            {
                std::swap(i00,i01);
                std::swap(i10,i11);
            }

            unsigned int numValid = 0;
            if (i00>=0) ++numValid;
            if (i01>=0) ++numValid;
            if (i10>=0) ++numValid;
            if (i11>=0) ++numValid;

            if (numValid==4)
            {
                // optimize which way to put the diagonal by choosing to
                // place it between the two corners that have the least curvature
                // relative to each other.
                float dot_00_11 = (*VNG._normals)[i00] * (*VNG._normals)[i11];
                float dot_01_10 = (*VNG._normals)[i01] * (*VNG._normals)[i10];
                if (dot_00_11 > dot_01_10)
                {
                    elements->addElement(i01);
                    elements->addElement(i00);
                    elements->addElement(i11);

                    elements->addElement(i00);
                    elements->addElement(i10);
                    elements->addElement(i11);
                }
                else
                {
                    elements->addElement(i01);
                    elements->addElement(i00);
                    elements->addElement(i10);

                    elements->addElement(i01);
                    elements->addElement(i10);
                    elements->addElement(i11);
                }
            }
            else if (numValid==3)
            {
                if (i00>=0) elements->addElement(i00);
                if (i01>=0) elements->addElement(i01);
                if (i11>=0) elements->addElement(i11);
                if (i10>=0) elements->addElement(i10);
            }
        }
    }


    if (createSkirt)
    {
        osg::ref_ptr<osg::Vec3Array> vertices = VNG._vertices.get();
        osg::ref_ptr<osg::Vec3Array> normals = VNG._normals.get();

        osg::ref_ptr<osg::DrawElements> skirtDrawElements = smallTile ?
            static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
            static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));

        // create bottom skirt vertices
        int r,c;
        r=0;
        for(c=0;c<static_cast<int>(numColumns);++c)
        {
            int orig_i = VNG.vertex_index(c,r);
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);

                for(VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    itr->second.first->push_back((*itr->second.first)[orig_i]);
                }

                skirtDrawElements->addElement(orig_i);
                skirtDrawElements->addElement(new_i);
            }
            else
            {
                if (skirtDrawElements->getNumIndices()!=0)
                {
                    geometry->addPrimitiveSet(skirtDrawElements.get());
                    skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
                }

            }
        }

        if (skirtDrawElements->getNumIndices()!=0)
        {
            geometry->addPrimitiveSet(skirtDrawElements.get());
            skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
        }

        // create right skirt vertices
        c=numColumns-1;
        for(r=0;r<static_cast<int>(numRows);++r)
        {
            int orig_i = VNG.vertex_index(c,r); // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    itr->second.first->push_back((*itr->second.first)[orig_i]);
                }

                skirtDrawElements->addElement(orig_i);
                skirtDrawElements->addElement(new_i);
            }
            else
            {
                if (skirtDrawElements->getNumIndices()!=0)
                {
                    geometry->addPrimitiveSet(skirtDrawElements.get());
                    skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
                }

            }
        }

        if (skirtDrawElements->getNumIndices()!=0)
        {
            geometry->addPrimitiveSet(skirtDrawElements.get());
            skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
        }

        // create top skirt vertices
        r=numRows-1;
        for(c=numColumns-1;c>=0;--c)
        {
            int orig_i = VNG.vertex_index(c,r); // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    itr->second.first->push_back((*itr->second.first)[orig_i]);
                }

                skirtDrawElements->addElement(orig_i);
                skirtDrawElements->addElement(new_i);
            }
            else
            {
                if (skirtDrawElements->getNumIndices()!=0)
                {
                    geometry->addPrimitiveSet(skirtDrawElements.get());
                    skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
                }

            }
        }

        if (skirtDrawElements->getNumIndices()!=0)
        {
            geometry->addPrimitiveSet(skirtDrawElements.get());
            skirtDrawElements = smallTile ?
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
        }

        // create left skirt vertices
        c=0;
        for(r=numRows-1;r>=0;--r)
        {
            int orig_i = VNG.vertex_index(c,r); // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(VertexNormalGenerator::LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    itr->second.first->push_back((*itr->second.first)[orig_i]);
                }

                skirtDrawElements->addElement(orig_i);
                skirtDrawElements->addElement(new_i);
            }
            else
            {
                if (skirtDrawElements->getNumIndices()!=0)
                {
                    geometry->addPrimitiveSet(skirtDrawElements.get());
                    skirtDrawElements = new osg::DrawElementsUShort(GL_QUAD_STRIP);
                }
            }
        }

        if (skirtDrawElements->getNumIndices()!=0)
        {
            geometry->addPrimitiveSet(skirtDrawElements.get());
        }
    }


    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(true);

#if 0
    {
        osgUtil::VertexCacheMissVisitor vcmv_before;
        osgUtil::VertexCacheMissVisitor vcmv_after;
        osgUtil::VertexCacheVisitor vcv;
        osgUtil::VertexAccessOrderVisitor vaov;

        vcmv_before.doGeometry(*geometry);
        vcv.optimizeVertices(*geometry);
        vaov.optimizeOrder(*geometry);
        vcmv_after.doGeometry(*geometry);
#if 0
        OSG_NOTICE<<"vcmv_before.triangles="<<vcmv_before.triangles<<std::endl;
        OSG_NOTICE<<"vcmv_before.misses="<<vcmv_before.misses<<std::endl;
        OSG_NOTICE<<"vcmv_after.misses="<<vcmv_after.misses<<std::endl;
        OSG_NOTICE<<std::endl;
#endif
    }
#endif

    if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::ReaderWriter::Options::BUILD_KDTREES &&
        osgDB::Registry::instance()->getKdTreeBuilder())
    {

        //osg::Timer_t before = osg::Timer::instance()->tick();
        //OSG_NOTICE<<"osgTerrain::GeometryTechnique::build kd tree"<<std::endl;
        osg::ref_ptr<osg::KdTreeBuilder> builder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();
        buffer._geode->accept(*builder);
        //osg::Timer_t after = osg::Timer::instance()->tick();
        //OSG_NOTICE<<"KdTree build time "<<osg::Timer::instance()->delta_m(before, after)<<std::endl;
    }
}

void GeometryTechnique::applyColorLayers(BufferData& buffer)
{
    typedef std::map<osgTerrain::Layer*, osg::Texture*> LayerToTextureMap;
    LayerToTextureMap layerToTextureMap;

    for(unsigned int layerNum=0; layerNum<_terrainTile->getNumColorLayers(); ++layerNum)
    {
        osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(layerNum);
        if (!colorLayer) continue;

        osgTerrain::SwitchLayer* switchLayer = dynamic_cast<osgTerrain::SwitchLayer*>(colorLayer);
        if (switchLayer)
        {
            if (switchLayer->getActiveLayer()<0 ||
                static_cast<unsigned int>(switchLayer->getActiveLayer())>=switchLayer->getNumLayers())
            {
                continue;
            }

            colorLayer = switchLayer->getLayer(switchLayer->getActiveLayer());
            if (!colorLayer) continue;
        }

        osg::Image* image = colorLayer->getImage();
        if (!image) continue;

        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(colorLayer);
        osgTerrain::ContourLayer* contourLayer = dynamic_cast<osgTerrain::ContourLayer*>(colorLayer);
        if (imageLayer)
        {
            osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();

            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(layerToTextureMap[colorLayer]);
            if (!texture2D)
            {
                texture2D = new osg::Texture2D;
                texture2D->setImage(image);
                texture2D->setMaxAnisotropy(16.0f);
                texture2D->setResizeNonPowerOfTwoHint(false);

                texture2D->setFilter(osg::Texture::MIN_FILTER, colorLayer->getMinFilter());
                texture2D->setFilter(osg::Texture::MAG_FILTER, colorLayer->getMagFilter());

                texture2D->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
                texture2D->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

                bool mipMapping = !(texture2D->getFilter(osg::Texture::MIN_FILTER)==osg::Texture::LINEAR || texture2D->getFilter(osg::Texture::MIN_FILTER)==osg::Texture::NEAREST);
                bool s_NotPowerOfTwo = image->s()==0 || (image->s() & (image->s() - 1));
                bool t_NotPowerOfTwo = image->t()==0 || (image->t() & (image->t() - 1));

                if (mipMapping && (s_NotPowerOfTwo || t_NotPowerOfTwo))
                {
                    OSG_INFO<<"Disabling mipmapping for non power of two tile size("<<image->s()<<", "<<image->t()<<")"<<std::endl;
                    texture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
                }


                layerToTextureMap[colorLayer] = texture2D;

                // OSG_NOTICE<<"Creating new ImageLayer texture "<<layerNum<<" image->s()="<<image->s()<<"  image->t()="<<image->t()<<std::endl;

            }
            else
            {
                // OSG_NOTICE<<"Reusing ImageLayer texture "<<layerNum<<std::endl;
            }

            stateset->setTextureAttributeAndModes(layerNum, texture2D, osg::StateAttribute::ON);

        }
        else if (contourLayer)
        {
            osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();

            osg::Texture1D* texture1D = dynamic_cast<osg::Texture1D*>(layerToTextureMap[colorLayer]);
            if (!texture1D)
            {
                texture1D = new osg::Texture1D;
                texture1D->setImage(image);
                texture1D->setResizeNonPowerOfTwoHint(false);
                texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
                texture1D->setFilter(osg::Texture::MAG_FILTER, colorLayer->getMagFilter());

                layerToTextureMap[colorLayer] = texture1D;
            }

            stateset->setTextureAttributeAndModes(layerNum, texture1D, osg::StateAttribute::ON);

        }
    }
}

void GeometryTechnique::applyTransparency(BufferData& buffer)
{
    TerrainTile::BlendingPolicy blendingPolicy = _terrainTile->getBlendingPolicy();
    if (blendingPolicy == TerrainTile::INHERIT && _terrainTile->getTerrain())
    {
        OSG_INFO<<"GeometryTechnique::applyTransparency() inheriting policy from Terrain"<<std::endl;
        blendingPolicy = _terrainTile->getTerrain()->getBlendingPolicy();
    }

    if (blendingPolicy == TerrainTile::INHERIT)
    {
        OSG_INFO<<"GeometryTechnique::applyTransparency() policy is INHERIT, defaulting to ENABLE_BLENDING_WHEN_ALPHA_PRESENT"<<std::endl;
        blendingPolicy = TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT;
    }

    if (blendingPolicy == TerrainTile::DO_NOT_SET_BLENDING)
    {
        OSG_INFO<<"blendingPolicy == TerrainTile::DO_NOT_SET_BLENDING"<<std::endl;
        return;
    }

    bool enableBlending = false;

    if (blendingPolicy == TerrainTile::ENABLE_BLENDING)
    {
        OSG_INFO<<"blendingPolicy == TerrainTile::ENABLE_BLENDING"<<std::endl;
        enableBlending = true;
    }
    else if (blendingPolicy == TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT)
    {
        OSG_INFO<<"blendingPolicy == TerrainTile::ENABLE_BLENDING_WHEN_ALPHA_PRESENT"<<std::endl;
        for(unsigned int i=0; i<_terrainTile->getNumColorLayers(); ++i)
        {
            osg::Image* image = (_terrainTile->getColorLayer(i)!=0) ? _terrainTile->getColorLayer(i)->getImage() : 0;
            if (image)
            {
                enableBlending = image->isImageTranslucent();
                break;
            }
        }
    }

    if (enableBlending)
    {
        osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

}

void GeometryTechnique::update(osgUtil::UpdateVisitor* uv)
{
    if (_terrainTile) _terrainTile->osg::Group::traverse(*uv);

    if (_newBufferData.valid())
    {
        _currentBufferData = _newBufferData;
        _newBufferData = 0;
    }
}


void GeometryTechnique::cull(osgUtil::CullVisitor* cv)
{
    if (_currentBufferData.valid())
    {
        if (_currentBufferData->_transform.valid())
        {
            _currentBufferData->_transform->accept(*cv);
        }
    }
}


void GeometryTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_terrainTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_terrainTile->getDirty()) _terrainTile->init(_terrainTile->getDirtyMask(), false);

        osgUtil::UpdateVisitor* uv = dynamic_cast<osgUtil::UpdateVisitor*>(&nv);
        if (uv)
        {
            update(uv);
            return;
        }
    }
    else if (nv.getVisitorType()==osg::NodeVisitor::CULL_VISITOR)
    {
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            cull(cv);
            return;
        }
    }


    if (_terrainTile->getDirty())
    {
        OSG_INFO<<"******* Doing init ***********"<<std::endl;
        _terrainTile->init(_terrainTile->getDirtyMask(), false);
    }

    if (_currentBufferData.valid())
    {
        if (_currentBufferData->_transform.valid()) _currentBufferData->_transform->accept(nv);
    }
}


void GeometryTechnique::cleanSceneGraph()
{
}

void GeometryTechnique::releaseGLObjects(osg::State* state) const
{
    if (_currentBufferData.valid() && _currentBufferData->_transform.valid()) _currentBufferData->_transform->releaseGLObjects(state);
    if (_newBufferData.valid() && _newBufferData->_transform.valid()) _newBufferData->_transform->releaseGLObjects(state);
}

