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
#include <osgTerrain/TerrainNode>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/FileUtils>

#include <osg/io_utils>
#include <osg/Texture2D>
#include <osg/Texture1D>
#include <osg/TexEnvCombine>
#include <osg/Program>
#include <osg/Math>

using namespace osgTerrain;

GeometryTechnique::GeometryTechnique()
{
}

GeometryTechnique::GeometryTechnique(const GeometryTechnique& gt,const osg::CopyOp& copyop):
    TerrainTechnique(gt,copyop)
{
}

GeometryTechnique::~GeometryTechnique()
{
}

void GeometryTechnique::init()
{
    osg::notify(osg::NOTICE)<<"Doing init()"<<std::endl;
    
    if (!_terrainNode) return;


    osgTerrain::Layer* elevationLayer = _terrainNode->getElevationLayer();
    osgTerrain::Layer* colorLayer = _terrainNode->getColorLayer(0);
    osg::TransferFunction* colorTF = _terrainNode->getColorTransferFunction(0);
    osgTerrain::TerrainNode::Filter filter = _terrainNode->getColorFilter(0);

    // if the elevationLayer and colorLayer are the same, and there is colorTF then
    // simply assing as a texture coordinate.
    if ((elevationLayer==colorLayer) && colorTF) colorLayer = 0;
    
    osg::notify(osg::NOTICE)<<"elevationLayer = "<<elevationLayer<<std::endl;
    osg::notify(osg::NOTICE)<<"colorLayer = "<<colorLayer<<std::endl;
    osg::notify(osg::NOTICE)<<"colorTF = "<<colorTF<<std::endl;

    Locator* elevationLocator = elevationLayer ? elevationLayer->getLocator() : 0;
    Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;
    
    Locator* masterLocator = elevationLocator ? elevationLocator : colorLocator;
    if (!masterLocator)
    {
        osg::notify(osg::NOTICE)<<"Problem, no locator found in any of the terrain layers"<<std::endl;
        return;
    }
    
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

    osg::notify(osg::NOTICE)<<"bottomLeftNDC = "<<bottomLeftNDC<<std::endl;
    osg::notify(osg::NOTICE)<<"topRightNDC = "<<topRightNDC<<std::endl;

    
    _geode = new osg::Geode;

    _transform = new osg::MatrixTransform;
    _transform->addChild(_geode.get());

    osg::Vec3d centerNDC = (bottomLeftNDC + topRightNDC)*0.5;
    osg::Vec3d centerModel = (bottomLeftNDC + topRightNDC)*0.5;
    masterLocator->convertLocalToModel(centerNDC, centerModel);
    
    _transform->setMatrix(osg::Matrix::translate(centerModel));
    

    _terrainGeometry = 0; // new osgTerrain::TerrainGeometry;
    if (_terrainGeometry.valid()) _geode->addDrawable(_terrainGeometry.get());

    _geometry = new osg::Geometry;
    if (_geometry.valid()) _geode->addDrawable(_geometry.get());

    
    unsigned int numRows = 100;
    unsigned int numColumns = 100;
    
    if (elevationLayer)
    {
        numColumns = elevationLayer->getNumColumns();
        numRows = elevationLayer->getNumRows();
    }
    
    
    unsigned int numVertices = numRows * numColumns;

    // allocate and assign vertices
    osg::Vec3Array* _vertices = new osg::Vec3Array(numVertices);
    if (_terrainGeometry.valid()) _terrainGeometry->setVertices(_vertices);
    if (_geometry.valid()) _geometry->setVertexArray(_vertices);

    // allocate and assign normals
    osg::Vec3Array* _normals = new osg::Vec3Array(numVertices);
    if (_terrainGeometry.valid()) _terrainGeometry->setNormals(_normals);
    if (_geometry.valid())
    {
        _geometry->setNormalArray(_normals);
        _geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
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

        _texcoords = new osg::Vec2Array(numVertices);
        
        if (_terrainGeometry.valid()) _terrainGeometry->setTexCoords(color_index, _texcoords);

        if (_geometry.valid()) _geometry->setTexCoordArray(color_index, _texcoords);
    }

    osg::FloatArray* _elevations = new osg::FloatArray(numVertices);
    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
    if (tf)
    {
        tf_index = texcoord_index;
        ++texcoord_index;

        if (!colorLayer)
        {
            // _elevations = new osg::FloatArray(numVertices);
            if (_terrainGeometry.valid()) _terrainGeometry->setTexCoords(tf_index, _elevations);
            if (_geometry.valid()) _geometry->setTexCoordArray(tf_index, _elevations);

            minHeight = tf->getMinimum();
            scaleHeight = 1.0f/(tf->getMaximum()-tf->getMinimum());
        }
    }

        
    // allocate and assign color
    osg::Vec4Array* _colors = new osg::Vec4Array(1);
    (*_colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    
    if (_terrainGeometry.valid()) _terrainGeometry->setColors(_colors);
    if (_geometry.valid())
    {
        _geometry->setColorArray(_colors);
        _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    }
    
    // populate vertex and tex coord arrays
    unsigned int j;
    for(j=0; j<numRows; ++j)
    {
        for(unsigned int i=0; i<numColumns; ++i)
        {
            unsigned int iv = j*numColumns + i;
            osg::Vec3d ndc( ((double)i)/(double)(numColumns-1), ((double)j)/(double)(numRows-1), 0.0);
            
            if (elevationLayer)
            {
                float value = 0.0f;
                elevationLayer->getValue(i,j, value);
                // osg::notify(osg::NOTICE)<<"i="<<i<<" j="<<j<<" z="<<value<<std::endl;
                ndc.z() = value;
            }
            
            osg::Vec3d model;
            masterLocator->convertLocalToModel(ndc, model);

            (*_vertices)[iv] = model - centerModel;

            if (colorLayer)
            {
                if (colorLocator!= masterLocator)
                {
                    osg::Vec3d color_ndc;
                    Locator::convertLocalCoordBetween(*masterLocator, ndc, *colorLocator, color_ndc);
                    (*_texcoords)[iv].set(color_ndc.x(), color_ndc.y());
                }
                else
                {
                    (*_texcoords)[iv].set(ndc.x(), ndc.y());
                }

            }

            if (_elevations)
            {
                (*_elevations)[iv] = (ndc.z()-minHeight)*scaleHeight;
            }

            // compute the local normal
            osg::Vec3d ndc_one( (double)i/(double)(numColumns-1), (double)j/(double)(numColumns-1), 1.0);
            osg::Vec3d model_one;
            masterLocator->convertLocalToModel(ndc_one, model_one);
            model_one = model_one - model;
            model_one.normalize();            
            (*_normals)[iv] = model_one;
        }
    }


    // populate primitive sets
    bool optimizeOrientations = _elevations!=0;
    bool swapOrientation = !(masterLocator->orientationOpenGL());
    
    if (!optimizeOrientations)
    {
        osg::notify(osg::NOTICE)<<"Old tesselation"<<std::endl;
        for(j=0; j<numRows-1; ++j)
        {
            osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP, numColumns*2);
            for(unsigned int i=0; i<numColumns; ++i)
            {
                unsigned int iv = j*numColumns + i;
                if (swapOrientation)
                {
                    (*elements)[i*2] = iv + numColumns;
                    (*elements)[i*2+1] = iv;
                }
                else
                {
                    (*elements)[i*2+1] = iv + numColumns;
                    (*elements)[i*2] = iv;
                }
            }

            if (_terrainGeometry.valid()) _terrainGeometry->addPrimitiveSet(elements);

            if (_geometry.valid()) _geometry->addPrimitiveSet(elements);
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"New tesselation"<<std::endl;
    
        osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLES);
        elements->reserve((numRows-1) * (numColumns-1) * 6);
        
        if (_terrainGeometry.valid()) _terrainGeometry->addPrimitiveSet(elements);
        if (_geometry.valid()) _geometry->addPrimitiveSet(elements);

        for(j=0; j<numRows-1; ++j)
        {
            for(unsigned int i=0; i<numColumns-1; ++i)
            {
                unsigned int i00;
                unsigned int i01;
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

                unsigned int i10 = i00+1;
                unsigned int i11 = i01+1;

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
        }
    }

    bool containsTransparency = false;

    if (colorLayer)
    {
        osgTerrain::ImageLayer* imageLayer = dynamic_cast<osgTerrain::ImageLayer*>(colorLayer);
        if (imageLayer)
        {
            osg::Image* image = imageLayer->getImage();
            osg::StateSet* stateset = _geode->getOrCreateStateSet();

            osg::Texture2D* texture2D = new osg::Texture2D;
            texture2D->setImage(image);
            texture2D->setResizeNonPowerOfTwoHint(false);
            stateset->setTextureAttributeAndModes(color_index, texture2D, osg::StateAttribute::ON);

            texture2D->setFilter(osg::Texture::MAG_FILTER, filter==TerrainNode::LINEAR ? osg::Texture::LINEAR :  osg::Texture::NEAREST);
            
            if (tf)
            {
                // up the precision of hte internal texture format to its maximum.
                //image->setInternalTextureFormat(GL_LUMINANCE32F_ARB);
                image->setInternalTextureFormat(GL_LUMINANCE16);
            }
            else
            {
                containsTransparency = image->isImageTranslucent();
            }
        }
        
    }


    if (tf)
    {
        osg::notify(osg::NOTICE)<<"Requires TransferFunction"<<std::endl;
        osg::Image* image = tf->getImage();
        osg::StateSet* stateset = _geode->getOrCreateStateSet();
        osg::Texture1D* texture1D = new osg::Texture1D;
        texture1D->setImage(image);
        texture1D->setResizeNonPowerOfTwoHint(false);
        texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
        texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
        stateset->setTextureAttributeAndModes(tf_index, texture1D, osg::StateAttribute::ON);

        containsTransparency = image->isImageTranslucent();

        if (colorLayer)
        {
            osg::notify(osg::NOTICE)<<"Using fragment program"<<std::endl;
        
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

            // get shaders from source
            std::string vertexShaderFile = osgDB::findDataFile("lookup.vert");
            if (!vertexShaderFile.empty())
            {
                program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, vertexShaderFile));
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Not found lookup.vert"<<std::endl;
            }

            std::string fragmentShaderFile = osgDB::findDataFile("lookup.frag");
            if (!fragmentShaderFile.empty())
            {
                program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragmentShaderFile));
            }
            else
            {
                osg::notify(osg::NOTICE)<<"Not found lookup.frag"<<std::endl;
            }

            osg::Uniform* sourceSampler = new osg::Uniform("sourceTexture",color_index);
            stateset->addUniform(sourceSampler);

            osg::Uniform* lookupTexture = new osg::Uniform("lookupTexture",tf_index);
            stateset->addUniform(lookupTexture);

            osg::Uniform* minValue = new osg::Uniform("minValue", tf->getMinimum());
            stateset->addUniform(minValue);

            osg::Uniform* inverseRange = new osg::Uniform("inverseRange", 1.0f/(tf->getMaximum()-tf->getMinimum()));
            stateset->addUniform(inverseRange);
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Using standard OpenGL fixed function pipeline"<<std::endl;
        }
    }
    
    if (containsTransparency)
    {
        osg::StateSet* stateset = _geode->getOrCreateStateSet();
        stateset->setMode(GL_BLEND, osg::StateAttribute::ON);
        stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    }

    // if (_terrainGeometry.valid()) _terrainGeometry->setUseDisplayList(false);
    if (_geometry.valid()) _geometry->setUseVertexBufferObjects(true);

    if (_geometry.valid())
    {
        osgUtil::SmoothingVisitor smoother;
        smoother.smooth(*_geometry);    
    }

    _dirty = false;    
}


void GeometryTechnique::update(osgUtil::UpdateVisitor* nv)
{
}


void GeometryTechnique::cull(osgUtil::CullVisitor* nv)
{
    if (_transform.valid())
    {
        _transform->accept(*nv);
    }
}

void GeometryTechnique::cleanSceneGraph()
{
}

void GeometryTechnique::dirty()
{
    TerrainTechnique::dirty();
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  TerrainGeometry
//
TerrainGeometry::TerrainGeometry()
{
}


TerrainGeometry::TerrainGeometry(const TerrainGeometry& geometry,const osg::CopyOp& copyop):
    osg::Drawable(geometry, copyop),
    _vertices(geometry._vertices),
    _normals(geometry._normals),
    _colors(geometry._colors),
    _texcoords(geometry._texcoords),
    _primitiveSets(geometry._primitiveSets)

{
}

osg::BoundingBox TerrainGeometry::computeBound() const
{
    osg::BoundingBox bb;
    
    if (_vertices.first.valid())
    {
        for(osg::Vec3Array::const_iterator itr = _vertices.first->begin();
            itr != _vertices.first->end();
            ++itr)
        {
            bb.expandBy(*itr);
        }
    }
    return bb;
}

void TerrainGeometry::drawImplementation(osg::RenderInfo& renderInfo) const
{
#if 0
    osg::notify(osg::NOTICE)<<"TerrainGeometry::drawImplementation"<<std::endl;
    
    if (_vertices.first.valid() && _vertices.first->getDataVariance()==DYNAMIC)
    {
        osg::notify(osg::NOTICE)<<"  Vertices DYNAMIC"<<std::endl;
    }
    else
    {
        osg::notify(osg::NOTICE)<<"  Vertices STATIC"<<std::endl;
    }
#endif
    
    osg::State& state = *renderInfo.getState();
    const osg::Geometry::Extensions* extensions = osg::Geometry::getExtensions(state.getContextID(),true);
    
    //
    // Non Vertex Buffer Object path for defining vertex arrays.
    //            
    if( _vertices.first.valid() )
        state.setVertexPointer(_vertices.first->getDataSize(), _vertices.first->getDataType(), 0, _vertices.first->getDataPointer());
    else
        state.disableVertexPointer();

    if (_normals.first.valid())
    {
        state.setNormalPointer(_normals.first->getDataType(),0,_normals.first->getDataPointer());
    }
    else
    {
        state.disableNormalPointer();
        glNormal3f(0.0f,0.0f,1.0f);
    }

    if (_colors.first.valid() && _colors.first->getNumElements()==_vertices.first->getNumElements())
    {
        state.setColorPointer(_colors.first->getDataSize(),_colors.first->getDataType(),0,_colors.first->getDataPointer());
    }
    else
    {
        state.disableColorPointer();

        if (_colors.first.valid() && _colors.first->getNumElements()>=1)
        {
            glColor4fv(static_cast<const GLfloat*>(_colors.first->getDataPointer()));
        }
        else
        {
            glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        } 
    }

    unsigned int unit;
    for(unit=0;unit<_texcoords.size();++unit)
    {
        const osg::Array* array = _texcoords[unit].first.get();
        if (array)
            state.setTexCoordPointer(unit,array->getDataSize(),array->getDataType(),0,array->getDataPointer());
        else
            state.disableTexCoordPointer(unit);
    }
    state.disableTexCoordPointersAboveAndIncluding(unit);

    bool usingVertexBufferObjects = false;

    for(PrimitiveSetList::const_iterator itr = _primitiveSets.begin();
        itr != _primitiveSets.end();
        ++itr)
    {
        (*itr)->draw(state, usingVertexBufferObjects);

    }
}
