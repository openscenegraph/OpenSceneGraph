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
#include <osgTerrain/Terrain>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/FileUtils>

#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/TexEnvCombine>
#include <osg/Program>
#include <osg/Math>

using namespace osgTerrain;

GeometryTechnique::GeometryTechnique():
    _currentReadOnlyBuffer(1),
    _currentWriteBuffer(0)
    
{
    setFilterBias(0);
    setFilterWidth(0.1);
    setFilterMatrixAs(GAUSSIAN);
    
}

GeometryTechnique::GeometryTechnique(const GeometryTechnique& gt,const osg::CopyOp& copyop):
    TerrainTechnique(gt,copyop)
{
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
    osg::notify(osg::INFO)<<"Doing init()"<<std::endl;
    
    if (!_terrain) return;

    BufferData& buffer = getWriteBuffer();
    
    Locator* masterLocator = computeMasterLocator();
    
    osg::Vec3d centerModel = computeCenterModel(masterLocator);
    
    generateGeometry(masterLocator, centerModel);
    
    applyColorLayers();
    
    applyTransferFunctions();
    
    applyTransparency();
    
    smoothGeometry();

    if (buffer._transform.valid()) buffer._transform->setThreadSafeRefUnref(true);

    _dirty = false;    

    swapBuffers();
}

Locator* GeometryTechnique::computeMasterLocator()
{
    osgTerrain::Layer* elevationLayer = _terrain->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);

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
    
    osgTerrain::Layer* elevationLayer = _terrain->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrain->getColorTransferFunction(0);

    if ((elevationLayer==colorLayer) && colorTF) colorLayer = 0;

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
    
    osgTerrain::Layer* elevationLayer = _terrain->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrain->getColorTransferFunction(0);
    
    if ((elevationLayer==colorLayer) && colorTF) colorLayer = 0;

    Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;

    if (!colorLocator) colorLocator = masterLocator;
    
    buffer._geode = new osg::Geode;
    if(buffer._transform.valid())
        buffer._transform->addChild(buffer._geode.get());
    
    buffer._geometry = new osg::Geometry;
    if (buffer._geometry.valid()) buffer._geode->addDrawable(buffer._geometry.get());
    
    unsigned int numRows = 100;
    unsigned int numColumns = 100;
    
    if (elevationLayer)
    {
        numColumns = elevationLayer->getNumColumns();
        numRows = elevationLayer->getNumRows();
    }
    
    bool treatBoundariesToValidDataAsDefaultValue = _terrain->getTreatBoundariesToValidDataAsDefaultValue();
    osg::notify(osg::INFO)<<"TreatBoundariesToValidDataAsDefaultValue="<<treatBoundariesToValidDataAsDefaultValue<<std::endl;
    
    unsigned int numVertices = numRows * numColumns;

    // allocate and assign vertices
    osg::Vec3Array* _vertices = new osg::Vec3Array;
    if (buffer._geometry.valid()) buffer._geometry->setVertexArray(_vertices);

    // allocate and assign normals
    osg::Vec3Array* _normals = new osg::Vec3Array;
    if (buffer._geometry.valid())
    {
        buffer._geometry->setNormalArray(_normals);
        buffer._geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }
    
    int texcoord_index = 0;
    int color_index = -1;
    int tf_index = -1;

    float minHeight = 0.0;
    float scaleHeight = 1.0;

    // allocate and assign tex coords
    osg::Vec2Array* _texcoords = 0;
    if (colorLayer)
    {
        color_index = texcoord_index;
        ++texcoord_index;

        _texcoords = new osg::Vec2Array;
        
        if (buffer._geometry.valid()) buffer._geometry->setTexCoordArray(color_index, _texcoords);
    }

    osg::FloatArray* _elevations = new osg::FloatArray;
    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
    if (tf)
    {
        tf_index = texcoord_index;
        ++texcoord_index;

        if (!colorLayer)
        {
            // _elevations = new osg::FloatArray(numVertices);
            if (buffer._geometry.valid()) buffer._geometry->setTexCoordArray(tf_index, _elevations);

            minHeight = tf->getMinimum();
            scaleHeight = 1.0f/(tf->getMaximum()-tf->getMinimum());
        }
    }

    if (_vertices) _vertices->reserve(numVertices);
    if (_texcoords) _texcoords->reserve(numVertices);
    if (_elevations) _elevations->reserve(numVertices);
    if (_normals) _normals->reserve(numVertices);
        
    // allocate and assign color
    osg::Vec4Array* _colors = new osg::Vec4Array(1);
    (*_colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    
    if (buffer._geometry.valid())
    {
        buffer._geometry->setColorArray(_colors);
        buffer._geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    }


    typedef std::vector<int> Indices;
    Indices indices(numColumns*numRows, -1);
    
    // populate vertex and tex coord arrays
    unsigned int j;
    for(j=0; j<numRows; ++j)
    {
        for(unsigned int i=0; i<numColumns; ++i)
        {
            unsigned int iv = j*numColumns + i;
            osg::Vec3d ndc( ((double)i)/(double)(numColumns-1), ((double)j)/(double)(numRows-1), 0.0);
     
            bool validValue = true;
     
            
            if (elevationLayer)
            {
                float value = 0.0f;
                validValue = elevationLayer->getValidValue(i,j, value);
                // osg::notify(osg::INFO)<<"i="<<i<<" j="<<j<<" z="<<value<<std::endl;
                ndc.z() = value;
            }
            
            if (validValue)
            {
                indices[iv] = _vertices->size();
            
                osg::Vec3d model;
                masterLocator->convertLocalToModel(ndc, model);

                (*_vertices).push_back(model - centerModel);

                if (colorLayer)
                {
                    if (colorLocator!= masterLocator)
                    {
                        osg::Vec3d color_ndc;
                        Locator::convertLocalCoordBetween(*masterLocator, ndc, *colorLocator, color_ndc);
                        (*_texcoords).push_back(osg::Vec2(color_ndc.x(), color_ndc.y()));
                    }
                    else
                    {
                        (*_texcoords).push_back(osg::Vec2(ndc.x(), ndc.y()));
                    }

                }

                if (_elevations)
                {
                    (*_elevations).push_back((ndc.z()-minHeight)*scaleHeight);
                }

                // compute the local normal
                osg::Vec3d ndc_one( (double)i/(double)(numColumns-1), (double)j/(double)(numColumns-1), 1.0);
                osg::Vec3d model_one;
                masterLocator->convertLocalToModel(ndc_one, model_one);
                model_one = model_one - model;
                model_one.normalize();            
                (*_normals).push_back(model_one);
            }
            else
            {
                indices[iv] = -1;
            }
        }
    }

    // populate primitive sets
//    bool optimizeOrientations = _elevations!=0;
    bool swapOrientation = !(masterLocator->orientationOpenGL());
    
    osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLES);
    elements->reserve((numRows-1) * (numColumns-1) * 6);

    if (buffer._geometry.valid()) buffer._geometry->addPrimitiveSet(elements);

    for(unsigned int j=0; j<numRows-1; ++j)
    {
        for(unsigned int i=0; i<numColumns-1; ++i)
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
                float e00 = (*_elevations)[i00];
                float e10 = (*_elevations)[i10];
                float e01 = (*_elevations)[i01];
                float e11 = (*_elevations)[i11];

                if (fabsf(e00-e11)<fabsf(e01-e10))
                {
                    elements->push_back(i01);
                    elements->push_back(i00);
                    elements->push_back(i11);

                    elements->push_back(i00);
                    elements->push_back(i10);
                    elements->push_back(i11);
                }
                else
                {
                    elements->push_back(i01);
                    elements->push_back(i00);
                    elements->push_back(i10);

                    elements->push_back(i01);
                    elements->push_back(i10);
                    elements->push_back(i11);
                }
            }
            else if (numValid==3)
            {
                if (i00>=0) elements->push_back(i00);
                if (i01>=0) elements->push_back(i01);
                if (i11>=0) elements->push_back(i11);
                if (i10>=0) elements->push_back(i10);
            }
            
        }
    }
    
    // if (_terrainGeometry.valid()) _terrainGeometry->setUseDisplayList(false);
    if (buffer._geometry.valid()) buffer._geometry->setUseVertexBufferObjects(true);
}

void GeometryTechnique::applyColorLayers()
{
    BufferData& buffer = getWriteBuffer();
    
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrain->getColorTransferFunction(0);
    osgTerrain::Terrain::Filter filter = _terrain->getColorFilter(0);
    
    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
    
    int color_index = -1;
    
    if (colorLayer)
    {
        color_index++;
        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(colorLayer);
        if (imageLayer)
        {
            osg::Image* image = imageLayer->getImage();
            osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();

            osg::Texture2D* texture2D = new osg::Texture2D;
            texture2D->setImage(image);
            texture2D->setResizeNonPowerOfTwoHint(false);
            stateset->setTextureAttributeAndModes(color_index, texture2D, osg::StateAttribute::ON);

            texture2D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR);
            texture2D->setFilter(osg::Texture::MAG_FILTER, filter==Terrain::LINEAR ? osg::Texture::LINEAR :  osg::Texture::NEAREST);
            
            if (tf)
            {
                // up the precision of hte internal texture format to its maximum.
                //image->setInternalTextureFormat(GL_LUMINANCE32F_ARB);
                image->setInternalTextureFormat(GL_LUMINANCE16);
            }
        }
    }
}

void GeometryTechnique::applyTransferFunctions()
{
    BufferData& buffer = getWriteBuffer();
    
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrain->getColorTransferFunction(0);
    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
    
    int color_index = -1;
    int tf_index = -1;
    
    if (colorLayer) {
        color_index++;
        tf_index++;
    }
    
    if (tf)
    {
        osg::notify(osg::INFO)<<"Requires TransferFunction"<<std::endl;
        tf_index++;
        osg::Image* image = tf->getImage();
        osg::StateSet* stateset = buffer._geode->getOrCreateStateSet();
        osg::Texture1D* texture1D = new osg::Texture1D;
        texture1D->setImage(image);
        texture1D->setResizeNonPowerOfTwoHint(false);
        texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        stateset->setTextureAttributeAndModes(tf_index, texture1D, osg::StateAttribute::ON);

        if (colorLayer)
        {
            osg::notify(osg::INFO)<<"Using fragment program"<<std::endl;
        
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

            // get shaders from source
            std::string vertexShaderFile = osgDB::findDataFile("shaders/lookup.vert");
            if (!vertexShaderFile.empty())
            {
                program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, vertexShaderFile));
            }
            else
            {
                osg::notify(osg::INFO)<<"Not found lookup.vert"<<std::endl;
            }

            std::string fragmentShaderFile = osgDB::findDataFile("shaders/lookup.frag");
            if (!fragmentShaderFile.empty())
            {
                program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragmentShaderFile));
            }
            else
            {
                osg::notify(osg::INFO)<<"Not found lookup.frag"<<std::endl;
            }

            osg::Uniform* sourceSampler = new osg::Uniform("sourceTexture",color_index);
            stateset->addUniform(sourceSampler);

            osg::Uniform* lookupTexture = new osg::Uniform("lookupTexture",tf_index);
            stateset->addUniform(lookupTexture);

            stateset->addUniform(_filterWidthUniform.get());
            stateset->addUniform(_filterMatrixUniform.get());
            stateset->addUniform(_filterBiasUniform.get());

            osg::Uniform* lightingEnabled = new osg::Uniform("lightingEnabled",true);
            stateset->addUniform(lightingEnabled);

            osg::Uniform* minValue = new osg::Uniform("minValue", tf->getMinimum());
            stateset->addUniform(minValue);

            osg::Uniform* inverseRange = new osg::Uniform("inverseRange", 1.0f/(tf->getMaximum()-tf->getMinimum()));
            stateset->addUniform(inverseRange);
        }
        else
        {
            osg::notify(osg::INFO)<<"Using standard OpenGL fixed function pipeline"<<std::endl;
        }
    }
}

void GeometryTechnique::applyTransparency()
{
    BufferData& buffer = getWriteBuffer();
    
    osgTerrain::Layer* elevationLayer = _terrain->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrain->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrain->getColorTransferFunction(0);

    // if the elevationLayer and colorLayer are the same, and there is colorTF then
    // simply assing as a texture coordinate.
    if ((elevationLayer==colorLayer) && colorTF) colorLayer = 0;
    
    bool containsTransparency = false;
    
    if (colorLayer)
    {
        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(colorLayer);
        if (imageLayer) {
            osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
            if (tf) containsTransparency = tf->getImage()->isImageTranslucent();
            else containsTransparency = imageLayer->getImage()->isImageTranslucent();
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
    if (_terrain) _terrain->osg::Group::traverse(*uv);
}


void GeometryTechnique::cull(osgUtil::CullVisitor* cv)
{
    BufferData& buffer = getReadOnlyBuffer();

#if 0
    if (buffer._terrain) buffer._terrain->osg::Group::traverse(*cv);
#else
    if (buffer._transform.valid())
    {
        buffer._transform->accept(*cv);
    }
#endif    
}


void GeometryTechnique::traverse(osg::NodeVisitor& nv)
{
    if (!_terrain) return;

    // if app traversal update the frame count.
    if (nv.getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
    {
        if (_dirty) init();

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


    if (_dirty) 
    {
        osg::notify(osg::INFO)<<"******* Doing init ***********"<<std::endl;
        init();
    }

    BufferData& buffer = getReadOnlyBuffer();
    if (buffer._transform.valid()) buffer._transform->accept(nv);
}


void GeometryTechnique::cleanSceneGraph()
{
}

void GeometryTechnique::dirty()
{
    TerrainTechnique::dirty();
}

