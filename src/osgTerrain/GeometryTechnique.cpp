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
    osgTerrain::Layer* colorLayer = _terrainNode->getColorLayer();
    osg::TransferFunction* colorTF = _terrainNode->getColorTransferFunction();

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
    _geometry = new osg::Geometry;
    _geode->addDrawable(_geometry.get());
    
    unsigned int numRows = 100;
    unsigned int numColumns = 100;
    
    if (elevationLayer)
    {
        numColumns = elevationLayer->getNumColumns();
        numRows = elevationLayer->getNumRows();
    }
    
    
    unsigned int numVertices = numRows * numColumns;

    // allocate and assign vertices
    osg::Vec3Array* vertices = new osg::Vec3Array(numVertices);
    _geometry->setVertexArray(vertices);

    // allocate and assign normals
    osg::Vec3Array* normals = new osg::Vec3Array(numVertices);
    _geometry->setNormalArray(normals);
    _geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

    
    int texcoord_index = 0;
    int color_index = -1;
    int tf_index = -1;

    float minHeight = 0.0;
    float scaleHeight = 1.0;

    // allocate and assign tex coords
    osg::Vec2Array* texcoords = 0;
    if (colorLayer)
    {
        color_index = texcoord_index;
        ++texcoord_index;

        texcoords = new osg::Vec2Array(numVertices);
        _geometry->setTexCoordArray(color_index, texcoords);
    }

    osg::FloatArray* heights = 0;
    osg::TransferFunction1D* tf = dynamic_cast<osg::TransferFunction1D*>(colorTF);
    if (tf)
    {
        tf_index = texcoord_index;
        ++texcoord_index;

        if (!colorLayer)
        {
            heights = new osg::FloatArray(numVertices);
            _geometry->setTexCoordArray(tf_index, heights);

            minHeight = tf->getMinimum();
            scaleHeight = 1.0f/(tf->getMaximum()-tf->getMinimum());
        }
    }

        
    // allocate and assign color
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    _geometry->setColorArray(colors);
    _geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    
    // populate vertex and tex coord arrays
    unsigned int j;
    for(j=0; j<numRows; ++j)
    {
        for(unsigned int i=0; i<numColumns; ++i)
        {
            unsigned int iv = j*numColumns + i;
            osg::Vec3d ndc( (double)i/(double)(numColumns-1), (double)j/(double)(numColumns-1), 0.0);
            
            if (elevationLayer)
            {
                float value = 0.0f;
                elevationLayer->getValue(i,j, value);
                // osg::notify(osg::NOTICE)<<"i="<<i<<" j="<<j<<" z="<<value<<std::endl;
                ndc.z() = value;
            }
            
            osg::Vec3d model;
            masterLocator->convertLocalToModel(ndc, model);

            (*vertices)[iv] = model;

            if (colorLayer)
            {
                if (colorLocator!= masterLocator)
                {
                    osg::Vec3d color_ndc;
                    colorLocator->computeLocalBounds(*masterLocator, ndc, color_ndc);
                    (*texcoords)[iv].set(color_ndc.x(), color_ndc.y());
                }
                else
                {
                    (*texcoords)[iv].set(ndc.x(), ndc.y());
                }

            }

            if (heights)
            {
                (*heights)[iv] = (ndc.z()-minHeight)*scaleHeight;
            }

            // compute the local normal
            osg::Vec3d ndc_one( (double)i/(double)(numColumns-1), (double)j/(double)(numColumns-1), 1.0);
            osg::Vec3d model_one;
            masterLocator->convertLocalToModel(ndc_one, model_one);
            model_one -= model;
            model_one.normalize();            
            (*normals)[iv] = model_one;
        }
    }

    // populate primitive sets
    for(j=0; j<numRows-1; ++j)
    {
        osg::DrawElementsUInt* elements = new osg::DrawElementsUInt(GL_TRIANGLE_STRIP, numColumns*2);
        for(unsigned int i=0; i<numColumns; ++i)
        {
            unsigned int iv = j*numColumns + i;
            (*elements)[i*2] = iv + numColumns;
            (*elements)[i*2+1] = iv;
        }
        _geometry->addPrimitiveSet(elements);
    }



    osgUtil::SmoothingVisitor smoother;
    _geode->accept(smoother);


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

            if (tf)
            {
                // up the precision of hte internal texture format to its maximum.
                //image->setInternalTextureFormat(GL_LUMINANCE32F_ARB);
                image->setInternalTextureFormat(GL_LUMINANCE16);
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

    _dirty = false;    
}


void GeometryTechnique::update(osgUtil::UpdateVisitor* nv)
{
}


void GeometryTechnique::cull(osgUtil::CullVisitor* nv)
{
    if (_geode.valid())
    {
        _geode->accept(*nv);
    }
}

void GeometryTechnique::cleanSceneGraph()
{
}

void GeometryTechnique::dirty()
{
    TerrainTechnique::dirty();
}
