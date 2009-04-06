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

#include <osgUtil/SmoothingVisitor>

#include <osgDB/FileUtils>

#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/TexEnvCombine>
#include <osg/Program>
#include <osg/Math>
#include <osg/Timer>

using namespace osgTerrain;

#define NEW_COORD_CODE

GeometryTechnique::GeometryTechnique():
    _currentReadOnlyBuffer(1),
    _currentWriteBuffer(0)
    
{
    setFilterBias(0);
    setFilterWidth(0.1);
    setFilterMatrixAs(GAUSSIAN);
    
}

GeometryTechnique::GeometryTechnique(const GeometryTechnique& gt,const osg::CopyOp& copyop):
    TerrainTechnique(gt,copyop),
    _currentReadOnlyBuffer(1),
    _currentWriteBuffer(0)
{
    setFilterBias(gt._filterBias);
    setFilterWidth(gt._filterWidth);
    setFilterMatrix(gt._filterMatrix);
}

GeometryTechnique::~GeometryTechnique()
{
}

void GeometryTechnique::swapBuffers()
{
    std::swap(_currentReadOnlyBuffer,_currentWriteBuffer);
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

void GeometryTechnique::init()
{
    osg::notify(osg::INFO)<<"Doing GeometryTechnique::init()"<<std::endl;
    
    if (!_terrainTile) return;

    BufferData& buffer = getWriteBuffer();
    
    Locator* masterLocator = computeMasterLocator();
    
    osg::Vec3d centerModel = computeCenterModel(masterLocator);
    
    generateGeometry(masterLocator, centerModel);
    
    applyColorLayers();
    applyTransparency();
    
    // smoothGeometry();

    if (buffer._transform.valid()) buffer._transform->setThreadSafeRefUnref(true);

    swapBuffers();
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
        osg::notify(osg::NOTICE)<<"Problem, no locator found in any of the terrain layers"<<std::endl;
        return 0;
    }
    
    return masterLocator;
}

osg::Vec3d GeometryTechnique::computeCenterModel(Locator* masterLocator)
{
    if (!masterLocator) return osg::Vec3d(0.0,0.0,0.0);

    BufferData& buffer = getWriteBuffer();
    
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

    osg::notify(osg::INFO)<<"bottomLeftNDC = "<<bottomLeftNDC<<std::endl;
    osg::notify(osg::INFO)<<"topRightNDC = "<<topRightNDC<<std::endl;

    buffer._transform = new osg::MatrixTransform;

    osg::Vec3d centerNDC = (bottomLeftNDC + topRightNDC)*0.5;
    osg::Vec3d centerModel = (bottomLeftNDC + topRightNDC)*0.5;
    masterLocator->convertLocalToModel(centerNDC, centerModel);
    
    buffer._transform->setMatrix(osg::Matrix::translate(centerModel));
    
    return centerModel;
}

void GeometryTechnique::generateGeometry(Locator* masterLocator, const osg::Vec3d& centerModel)
{
    BufferData& buffer = getWriteBuffer();
    
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
    
    float sampleRatio = _terrainTile->getTerrain() ? _terrainTile->getTerrain()->getSampleRatio() : 1.0f;
    
    double i_sampleFactor = 1.0;
    double j_sampleFactor = 1.0;

    // osg::notify(osg::NOTICE)<<"Sample ratio="<<sampleRatio<<std::endl;

    if (sampleRatio!=1.0f)
    {
    
        unsigned int originalNumColumns = numColumns;
        unsigned int originalNumRows = numRows;
    
        numColumns = std::max((unsigned int) (float(originalNumColumns)*sqrtf(sampleRatio)), 4u);
        numRows = std::max((unsigned int) (float(originalNumRows)*sqrtf(sampleRatio)),4u);

        i_sampleFactor = double(originalNumColumns-1)/double(numColumns-1);
        j_sampleFactor = double(originalNumRows-1)/double(numRows-1);
    }
    
    

    bool treatBoundariesToValidDataAsDefaultValue = _terrainTile->getTreatBoundariesToValidDataAsDefaultValue();
    osg::notify(osg::INFO)<<"TreatBoundariesToValidDataAsDefaultValue="<<treatBoundariesToValidDataAsDefaultValue<<std::endl;
    
    float skirtHeight = 0.0f;
    HeightFieldLayer* hfl = dynamic_cast<HeightFieldLayer*>(elevationLayer);
    if (hfl && hfl->getHeightField()) 
    {
        skirtHeight = hfl->getHeightField()->getSkirtHeight();
    }
    
    bool createSkirt = skirtHeight != 0.0f;
  
    unsigned int numVerticesInBody = numColumns*numRows;
    unsigned int numVerticesInSkirt = createSkirt ? numColumns*2 + numRows*2 - 4 : 0;
    unsigned int numVertices = numVerticesInBody+numVerticesInSkirt;

    // allocate and assign vertices
    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    vertices->reserve(numVertices);
    geometry->setVertexArray(vertices.get());

    // allocate and assign normals
    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array;
    if (normals.valid()) normals->reserve(numVertices);
    geometry->setNormalArray(normals.get());
    geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    

    //float minHeight = 0.0;
    float scaleHeight = _terrainTile->getTerrain() ? _terrainTile->getTerrain()->getVerticalScale() : 1.0f;

    // allocate and assign tex coords
    typedef std::pair< osg::ref_ptr<osg::Vec2Array>, Locator* > TexCoordLocatorPair;
    typedef std::map< Layer*, TexCoordLocatorPair > LayerToTexCoordMap;

    LayerToTexCoordMap layerToTexCoordMap;
    for(unsigned int layerNum=0; layerNum<_terrainTile->getNumColorLayers(); ++layerNum)
    {
        osgTerrain::Layer* colorLayer = _terrainTile->getColorLayer(layerNum);
        if (colorLayer)
        {
            LayerToTexCoordMap::iterator itr = layerToTexCoordMap.find(colorLayer);
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
            
                TexCoordLocatorPair& tclp = layerToTexCoordMap[colorLayer];
                tclp.first = new osg::Vec2Array;
                tclp.first->reserve(numVertices);
                tclp.second = locator ? locator : masterLocator;
                geometry->setTexCoordArray(layerNum, tclp.first.get());
            }
        }
    }

    osg::ref_ptr<osg::FloatArray> elevations = new osg::FloatArray;
    if (elevations.valid()) elevations->reserve(numVertices);
        

    // allocate and assign color
    osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    
    geometry->setColorArray(colors.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);


    typedef std::vector<int> Indices;
    Indices indices(numVertices, -1);
    
    // populate vertex and tex coord arrays
    unsigned int i, j;
    for(j=0; j<numRows; ++j)
    {
        for(i=0; i<numColumns; ++i)
        {
            unsigned int iv = j*numColumns + i;
            osg::Vec3d ndc( ((double)i)/(double)(numColumns-1), ((double)j)/(double)(numRows-1), 0.0);
     
            bool validValue = true;
     
            
            unsigned int i_equiv = i_sampleFactor==1.0 ? i : (unsigned int) (double(i)*i_sampleFactor);
            unsigned int j_equiv = i_sampleFactor==1.0 ? j : (unsigned int) (double(j)*j_sampleFactor);
            
            if (elevationLayer)
            {
                float value = 0.0f;
                validValue = elevationLayer->getValidValue(i_equiv,j_equiv, value);
                // osg::notify(osg::INFO)<<"i="<<i<<" j="<<j<<" z="<<value<<std::endl;
                ndc.z() = value*scaleHeight;
            }
            
            if (validValue)
            {
                indices[iv] = vertices->size();
            
                osg::Vec3d model;
                masterLocator->convertLocalToModel(ndc, model);

                (*vertices).push_back(model - centerModel);

                for(LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
                    itr != layerToTexCoordMap.end();
                    ++itr)
                {
                    osg::Vec2Array* texcoords = itr->second.first.get();
                    Locator* colorLocator = itr->second.second;
                    if (colorLocator != masterLocator)
                    {
                        osg::Vec3d color_ndc;
                        Locator::convertLocalCoordBetween(*masterLocator, ndc, *colorLocator, color_ndc);
                        (*texcoords).push_back(osg::Vec2(color_ndc.x(), color_ndc.y()));
                    }
                    else
                    {
                        (*texcoords).push_back(osg::Vec2(ndc.x(), ndc.y()));
                    }
                }

                if (elevations.valid())
                {
                    (*elevations).push_back(ndc.z());
                }

                // compute the local normal
                osg::Vec3d ndc_one = ndc; ndc_one.z() += 1.0;
                osg::Vec3d model_one;
                masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();            
                (*normals).push_back(model_one);
            }
            else
            {
                indices[iv] = -1;
            }
        }
    }
    
    // populate primitive sets
//    bool optimizeOrientations = elevations!=0;
    bool swapOrientation = !(masterLocator->orientationOpenGL());

    bool smallTile = numVertices <= 16384;

    // osg::notify(osg::NOTICE)<<"smallTile = "<<smallTile<<std::endl;
    
    osg::ref_ptr<osg::DrawElements> elements = smallTile ? 
        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_TRIANGLES)) :
        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_TRIANGLES));
        
    elements->reserveElements((numRows-1) * (numColumns-1) * 6);

    geometry->addPrimitiveSet(elements.get());

    for(j=0; j<numRows-1; ++j)
    {
        for(i=0; i<numColumns-1; ++i)
        {
            int i00;
            int i01;
            if (swapOrientation)
            {
                i01 = j*numColumns + i;
                i00 = i01+numColumns;
            }
            else
            {
                i00 = j*numColumns + i;
                i01 = i00+numColumns;
            }

            int i10 = i00+1;
            int i11 = i01+1;

            // remap indices to final vertex positions
            i00 = indices[i00];
            i01 = indices[i01];
            i10 = indices[i10];
            i11 = indices[i11];
            
            unsigned int numValid = 0;
            if (i00>=0) ++numValid;
            if (i01>=0) ++numValid;
            if (i10>=0) ++numValid;
            if (i11>=0) ++numValid;
            
            if (numValid==4)
            {
                float e00 = (*elevations)[i00];
                float e10 = (*elevations)[i10];
                float e01 = (*elevations)[i01];
                float e11 = (*elevations)[i11];

                if (fabsf(e00-e11)<fabsf(e01-e10))
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
    
    osg::ref_ptr<osg::Vec3Array> skirtVectors = new osg::Vec3Array((*normals));
    
    if (elevationLayer)
    {
        smoothGeometry();
        
        normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
        
        if (!normals) createSkirt = false;
    }

    if (createSkirt)
    {
        osg::ref_ptr<osg::DrawElements> skirtDrawElements = smallTile ? 
            static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
            static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));

        // create bottom skirt vertices
        int r,c;
        r=0;
        for(c=0;c<static_cast<int>(numColumns);++c)
        {
            int orig_i = indices[(r)*numColumns+c]; // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);

                for(LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
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
            int orig_i = indices[(r)*numColumns+c]; // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
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
            int orig_i = indices[(r)*numColumns+c]; // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
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
            int orig_i = indices[(r)*numColumns+c]; // index of original vertex of grid
            if (orig_i>=0)
            {
                unsigned int new_i = vertices->size(); // index of new index of added skirt point
                osg::Vec3 new_v = (*vertices)[orig_i] - ((*skirtVectors)[orig_i])*skirtHeight;
                (*vertices).push_back(new_v);
                if (normals.valid()) (*normals).push_back((*normals)[orig_i]);
                for(LayerToTexCoordMap::iterator itr = layerToTexCoordMap.begin();
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
            smallTile ? 
                static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(GL_QUAD_STRIP)) :
                static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(GL_QUAD_STRIP));
        }
    }


    geometry->setUseDisplayList(false);
    geometry->setUseVertexBufferObjects(true);
    
    
    if (osgDB::Registry::instance()->getBuildKdTreesHint()==osgDB::ReaderWriter::Options::BUILD_KDTREES &&
        osgDB::Registry::instance()->getKdTreeBuilder())
    {
    
        
        //osg::Timer_t before = osg::Timer::instance()->tick();
        //osg::notify(osg::NOTICE)<<"osgTerrain::GeometryTechnique::build kd tree"<<std::endl;
        osg::ref_ptr<osg::KdTreeBuilder> builder = osgDB::Registry::instance()->getKdTreeBuilder()->clone();
        buffer._geode->accept(*builder);
        //osg::Timer_t after = osg::Timer::instance()->tick();
        //osg::notify(osg::NOTICE)<<"KdTree build time "<<osg::Timer::instance()->delta_m(before, after)<<std::endl;
    }
}

void GeometryTechnique::applyColorLayers()
{
    BufferData& buffer = getWriteBuffer();

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
                    osg::notify(osg::INFO)<<"Disabling mipmapping for non power of two tile size("<<image->s()<<", "<<image->t()<<")"<<std::endl;
                    texture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
                }


                layerToTextureMap[colorLayer] = texture2D;

                // osg::notify(osg::NOTICE)<<"Creating new ImageLayer texture "<<layerNum<<" image->s()="<<image->s()<<"  image->t()="<<image->t()<<std::endl;

            }
            else
            {
                // osg::notify(osg::NOTICE)<<"Reusing ImageLayer texture "<<layerNum<<std::endl;
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

void GeometryTechnique::applyTransparency()
{
    BufferData& buffer = getWriteBuffer();
    
    bool containsTransparency = false;
    for(unsigned int i=0; i<_terrainTile->getNumColorLayers(); ++i)
    {
        osg::Image* image = (_terrainTile->getColorLayer(i)!=0) ? _terrainTile->getColorLayer(i)->getImage() : 0;
        if (image)
        {
            containsTransparency = image->isImageTranslucent();
            break;
        }        
    }
    
    if (containsTransparency)
    {
        osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

}

void GeometryTechnique::smoothGeometry()
{
    BufferData& buffer = getWriteBuffer();
    
    if (buffer._geometry.valid())
    {
        osgUtil::SmoothingVisitor smoother;
        smoother.smooth(*buffer._geometry);
    }
}

void GeometryTechnique::update(osgUtil::UpdateVisitor* uv)
{
    if (_terrainTile) _terrainTile->osg::Group::traverse(*uv);
}


void GeometryTechnique::cull(osgUtil::CullVisitor* cv)
{
    BufferData& buffer = getReadOnlyBuffer();

#if 0
    if (buffer._terrainTile) buffer._terrainTile->osg::Group::traverse(*cv);
#else
    if (buffer._transform.valid())
    {
        buffer._transform->accept(*cv);
    }
#endif    
}


void GeometryTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_terrainTile) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_terrainTile->getDirty()) _terrainTile->init();

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
        osg::notify(osg::INFO)<<"******* Doing init ***********"<<std::endl;
        _terrainTile->init();
    }

    BufferData& buffer = getReadOnlyBuffer();
    if (buffer._transform.valid()) buffer._transform->accept(nv);
}


void GeometryTechnique::cleanSceneGraph()
{
}

