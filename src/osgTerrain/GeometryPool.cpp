/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
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

#include <osgTerrain/GeometryPool>
#include <osg/VertexArrayState>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osgDB/ReadFile>

using namespace osgTerrain;

const osgTerrain::Locator* osgTerrain::computeMasterLocator(const osgTerrain::TerrainTile* tile)
{
    const osgTerrain::Layer* elevationLayer = tile->getElevationLayer();
    const osgTerrain::Layer* colorLayer = tile->getColorLayer(0);

    const Locator* elevationLocator = elevationLayer ? elevationLayer->getLocator() : 0;
    const Locator* colorLocator = colorLayer ? colorLayer->getLocator() : 0;

    const Locator* masterLocator = elevationLocator ? elevationLocator : colorLocator;
    if (!masterLocator)
    {
        OSG_NOTICE<<"Problem, no locator found in any of the terrain layers"<<std::endl;
        return 0;
    }

    return masterLocator;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  GeometryPool
//
GeometryPool::GeometryPool():
    _rootStateSetAssigned(false)

{
    _rootStateSet = new osg::StateSet;
}

GeometryPool::~GeometryPool()
{
}

bool GeometryPool::createKeyForTile(TerrainTile* tile, GeometryKey& key)
{
    const osgTerrain::Locator* masterLocator = computeMasterLocator(tile);
    if (masterLocator)
    {
        const osg::Matrixd& matrix = masterLocator->getTransform();
        osg::Vec3d bottom_left = osg::Vec3d(0.0,0.0,0.0) * matrix;
        osg::Vec3d bottom_right = osg::Vec3d(1.0,0.0,0.0) * matrix;
        osg::Vec3d top_left = osg::Vec3d(1.0,1.0,0.0) * matrix;
        key.sx = static_cast<float>((bottom_right-bottom_left).length());
        key.sy = static_cast<float>((top_left-bottom_left).length());

        if (masterLocator->getCoordinateSystemType()==osgTerrain::Locator::GEOCENTRIC)
        {
            // need to differentiate between tiles based of latitude, so use y position of bottom left corner.
            key.y = static_cast<float>(bottom_left.y());
        }
        else
        {
            // when the projection is linear there is no need to differentiate tiles according to their latitude
            key.y = 0.0;
        }

    }

    osgTerrain::HeightFieldLayer* layer = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
    if (layer)
    {
        osg::HeightField* hf = layer->getHeightField();
        if (hf)
        {
            key.nx = hf->getNumColumns();
            key.ny = hf->getNumRows();
        }
    }
    return true;
}

osg::ref_ptr<SharedGeometry> GeometryPool::getOrCreateGeometry(osgTerrain::TerrainTile* tile)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_geometryMapMutex);

    GeometryKey key;
    createKeyForTile(tile, key);

    GeometryMap::iterator itr = _geometryMap.find(key);
    if (itr != _geometryMap.end())
    {
        return itr->second.get();
    }

    osg::ref_ptr<SharedGeometry> geometry = new SharedGeometry;
    _geometryMap[key] = geometry;

    geometry->setUseVertexBufferObjects(true);

    osg::ref_ptr<osg::VertexBufferObject> vbo = new osg::VertexBufferObject;

    SharedGeometry::VertexToHeightFieldMapping& vthfm = geometry->getVertexToHeightFieldMapping();

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    vertices->setVertexBufferObject(vbo.get());
    geometry->setVertexArray(vertices.get());

    osg::ref_ptr<osg::Vec3Array> normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    normals->setVertexBufferObject(vbo.get());
    geometry->setNormalArray(normals.get());

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array(osg::Array::BIND_OVERALL);
    geometry->setColorArray(colours.get());
    colours->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));

    osg::ref_ptr<osg::Vec4Array> texcoords = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    texcoords->setVertexBufferObject(vbo.get());
    geometry->setTexCoordArray(texcoords.get());



    int nx = key.nx;
    int ny = key.nx;

    int numVerticesMainBody = nx * ny;
    int numVerticesSkirt = (nx)*2 + (ny)*2;
    int numVertices = numVerticesMainBody + numVerticesSkirt;

    vertices->reserve(numVertices);
    normals->reserve(numVertices);
    texcoords->reserve(numVertices);
    vthfm.reserve(numVertices);

    double c_mult = 1.0/static_cast<double>(nx-1);
    double r_mult = 1.0/static_cast<double>(ny-1);

    typedef std::vector<osg::Vec4d> LocationCoords;
    LocationCoords locationCoords;
    locationCoords.reserve(numVertices);


    // pass in the delta texcoord per texel via the color array
    (*colours)[0].x() = c_mult;
    (*colours)[0].y() = r_mult;



    osg::Matrixd matrix;
    const osgTerrain::Locator* locator = computeMasterLocator(tile);
    if (locator)
    {
        matrix = locator->getTransform();
    }

    double skirtHeight = -1.0;

    // compute the size of the skirtHeight
    {
        osg::Vec3d bottom_left(0.0,0.0,0.0);
        osg::Vec3d top_right(1.0,1.0,0.0);

        // transform for unit coords to local coords of the tile
        bottom_left = bottom_left * matrix;
        top_right = top_right * matrix;

        // if we have a geocentric database then transform into geocentric coords.
        const osg::EllipsoidModel* em = locator ? locator->getEllipsoidModel() : 0;
        if (em && locator->getCoordinateSystemType()==osgTerrain::Locator::GEOCENTRIC)
        {
            // note y axis maps to latitude, x axis to longitude
            em->convertLatLongHeightToXYZ(bottom_left.y(), bottom_left.x(), bottom_left.z(), bottom_left.x(), bottom_left.y(), bottom_left.z());
            em->convertLatLongHeightToXYZ(top_right.y(), top_right.x(), top_right.z(), top_right.x(), top_right.y(), top_right.z());
        }

        double diagonalLength = (top_right-bottom_left).length();
        double skirtRatio = 0.02;
        skirtHeight = -diagonalLength*skirtRatio;
    }

    // set up the vertex data
    {
        osg::Vec3d pos(0.0, 0.0, 0.0);
        osg::Vec3d normal(0.0, 0.0, 1.0);
        osg::Vec2 delta(1.0f/static_cast<float>(nx), 1.0f/static_cast<float>(ny));

        // bottom row for skirt
        pos.y () = static_cast<double>(0)*r_mult;
        pos.z() = skirtHeight;
        for(int c=0; c<nx; ++c)
        {
            pos.x() = static_cast<double>(c)*c_mult;
            vertices->push_back(pos);
            normals->push_back(normal);
            texcoords->push_back(osg::Vec4(pos.x(), pos.y(), 1.0f, 1.0f));
            locationCoords.push_back(osg::Vec4d(pos.x(), pos.y(),c_mult, r_mult));
            vthfm.push_back(0*nx + c);
        }

        // main body
        for(int r=0; r<ny; ++r)
        {
            pos.y () = static_cast<double>(r)*r_mult;

            // start skirt vertex
            pos.z() = skirtHeight;
            {
                pos.x() = static_cast<double>(0)*c_mult;
                vertices->push_back(pos);
                normals->push_back(normal);
                texcoords->push_back(osg::Vec4(pos.x(), pos.y(), 1.0f, 1.0f));
                locationCoords.push_back(osg::Vec4d(pos.x(), pos.y(),c_mult, r_mult));
                vthfm.push_back(r*nx + 0);
            }

            pos.z() = 0;
            for(int c=0; c<nx; ++c)
            {
                pos.x() = static_cast<double>(c)*c_mult;
                vertices->push_back(pos);
                normals->push_back(normal);
                texcoords->push_back(osg::Vec4(pos.x(), pos.y(), 1.0f, 1.0f));
                locationCoords.push_back(osg::Vec4d(pos.x(), pos.y(),c_mult, r_mult));
                vthfm.push_back(r*nx + c);
            }

            // end skirt vertex
            pos.z() = skirtHeight;
            {
                pos.x() = static_cast<double>(nx-1)*c_mult;
                vertices->push_back(pos);
                normals->push_back(normal);
                texcoords->push_back(osg::Vec4(pos.x(), pos.y(), 1.0f, 1.0f));
                locationCoords.push_back(osg::Vec4d(pos.x(), pos.y(),c_mult, r_mult));
                vthfm.push_back((r+1)*nx-1);
            }

        }

        // top row skirt
        pos.y () = static_cast<double>(ny-1)*r_mult;
        pos.z() = skirtHeight;
        for(int c=0; c<nx; ++c)
        {
            pos.x() = static_cast<double>(c)*c_mult;
            vertices->push_back(pos);
            normals->push_back(normal);
            texcoords->push_back(osg::Vec4(pos.x(), pos.y(), 1.0f, 1.0f));
            locationCoords.push_back(osg::Vec4d(pos.x(), pos.y(),c_mult, r_mult));
            vthfm.push_back((ny-1)*nx + c);
        }
    }

    bool smallTile = numVertices < 65536;

    GLenum primitiveTypes = GL_QUADS;

    osg::ref_ptr<osg::DrawElements> elements = smallTile ?
        static_cast<osg::DrawElements*>(new osg::DrawElementsUShort(primitiveTypes)) :
        static_cast<osg::DrawElements*>(new osg::DrawElementsUInt(primitiveTypes));

    elements->reserveElements( (nx-1) * (ny-1) * 4 + (nx-1)*2*4 + (ny-1)*2*4 );
    elements->setElementBufferObject(new osg::ElementBufferObject());
    geometry->setDrawElements(elements.get());


    // first row containing the skirt
    for(int c=0; c<nx-1; ++c)
    {
        int il = c;
        int iu = il+nx+1;
        elements->addElement(il);
        elements->addElement(il+1);
        elements->addElement(iu+1);
        elements->addElement(iu);
    }

    // center section
    for(int r=0; r<ny-1; ++r)
    {
        for(int c=0; c<nx+1; ++c)
        {
            int il = c+nx+r*(nx+2);
            int iu = il+nx+2;
            elements->addElement(il);
            elements->addElement(il+1);
            elements->addElement(iu+1);
            elements->addElement(iu);
        }
    }

    // top row containing skirt
    for(int c=0; c<nx-1; ++c)
    {
        int il = c+nx+(ny-1)*(nx+2)+1;
        int iu = il+nx+1;
        elements->addElement(il);
        elements->addElement(il+1);
        elements->addElement(iu+1);
        elements->addElement(iu);
    }

    if (locator)
    {
        matrix = locator->getTransform();

        osg::Vec3d center(0.5, 0.5, 0.0);

        osg::Vec3d bottom_left(0.0,0.0,0.0);
        osg::Vec3d bottom_right(1.0,0.0,0.0);
        osg::Vec3d top_left(0.0,1.0,0.0);

        center = center * matrix;
        bottom_left = bottom_left * matrix;
        bottom_right = bottom_right * matrix;
        top_left = top_left * matrix;

        // shift to center.x() to x=0 and carry all the corners with it.
        bottom_left.x() -= center.x();
        bottom_right.x() -= center.x();
        top_left.x() -= center.x();
        //center.x() = 0.0;

 //       OSG_NOTICE<<"   in lat/longs : bottom_left = "<<bottom_left<<std::endl;
 //       OSG_NOTICE<<"   in lat/longs : bottom_right = "<<bottom_right<<std::endl;
 //       OSG_NOTICE<<"   in lat/longs : top_left = "<<top_left<<std::endl;

        const osg::EllipsoidModel* em = locator->getEllipsoidModel();
        if (em && locator->getCoordinateSystemType()==osgTerrain::Locator::GEOCENTRIC)
        {
            osg::Matrixd localToWorldTransform;
            // note y axis maps to latitude, x axis to longitude
            em->computeLocalToWorldTransformFromLatLongHeight(center.y(), center.x(), center.z(), localToWorldTransform);
 //           OSG_NOTICE<<"We have a EllipsoidModel to take account of "<<localToWorldTransform<<std::endl;

            // note y axis maps to latitude, x axis to longitude
            em->convertLatLongHeightToXYZ(center.y(), center.x(), center.z(), center.x(), center.y(),center.z());
            em->convertLatLongHeightToXYZ(bottom_left.y(), bottom_left.x(), bottom_left.z(), bottom_left.x(), bottom_left.y(),bottom_left.z());
            em->convertLatLongHeightToXYZ(bottom_right.y(), bottom_right.x(), bottom_right.z(), bottom_right.x(), bottom_right.y(),bottom_right.z());
            em->convertLatLongHeightToXYZ(top_left.y(), top_left.x(), top_left.z(), top_left.x(), top_left.y(),top_left.z());

            osg::Matrixd worldToLocalTransform;
            worldToLocalTransform.invert(localToWorldTransform);

            center = center * worldToLocalTransform;
            bottom_left = bottom_left * worldToLocalTransform;
            bottom_right = bottom_right * worldToLocalTransform;
            top_left = top_left * worldToLocalTransform;


            for(int i=0; i<numVertices; ++i)
            {
                const osg::Vec4d& location = locationCoords[i];
                double height = (*vertices)[i].z();
                osg::Vec3d pos = osg::Vec3d(location.x(), location.y(), 0.0) * matrix;
                em->convertLatLongHeightToXYZ(pos.y(), pos.x(), height, pos.x(), pos.y(),pos.z());

                osg::Vec4& tc = (*texcoords)[i];

                osg::Vec3d pos_right = osg::Vec3d(location.x()+location[2], location.y(), 0.0) * matrix;
                em->convertLatLongHeightToXYZ(pos_right.y(), pos_right.x(), height, pos_right.x(), pos_right.y(),pos_right.z());

                osg::Vec3d pos_up = osg::Vec3d(location.x(), location.y()+location[3], 0.0) * matrix;
                em->convertLatLongHeightToXYZ(pos_up.y(), pos_up.x(), height, pos_up.x(), pos_up.y(),pos_up.z());

                double length_right = (pos_right-pos).length();
                double length_up = (pos_up-pos).length();
                tc[2] = 1.0/length_right;
                tc[3] = 1.0/length_up;


                osg::Vec3d normal(pos);
                normal = osg::Matrixd::transform3x3(localToWorldTransform, normal);
                normal.normalize();

                pos = pos * worldToLocalTransform;
                pos -= center;

                (*vertices)[i] = pos;
                (*normals)[i] = normal;

            }

        }
    }

    // double tileWidth = (bottom_right-bottom_left).length();
    // double skirtHeight = tileWidth*0.05;

 //   OSG_NOTICE<<"   in local coords : center = "<<center<<std::endl;
 //   OSG_NOTICE<<"   in local coords : bottom_left = "<<bottom_left<<std::endl;
 //   OSG_NOTICE<<"   in local coords : bottom_right = "<<bottom_right<<std::endl;
 //   OSG_NOTICE<<"   in local coords : top_left = "<<top_left<<std::endl;
 //   OSG_NOTICE<<"   skirtHeight = "<<skirtHeight<<std::endl;


    //osgDB::writeNodeFile(*geometry, "geometry.osgt");

//    OSG_NOTICE<<"Creating new geometry "<<geometry.get()<<std::endl;

    return geometry;
}

osg::ref_ptr<osg::MatrixTransform> GeometryPool::getTileSubgraph(osgTerrain::TerrainTile* tile)
{
    // create or reuse Geometry
    osg::ref_ptr<SharedGeometry> geometry = getOrCreateGeometry(tile);


    osg::ref_ptr<HeightFieldDrawable> hfDrawable = new HeightFieldDrawable();

    osgTerrain::HeightFieldLayer* hfl = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
    osg::HeightField* hf = hfl ? hfl->getHeightField() : 0;
    hfDrawable->setHeightField(hf);
    hfDrawable->setGeometry(geometry.get());


    // create a transform to place the geometry in the appropriate place
    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

//    transform->addChild(geometry.get());
    transform->addChild(hfDrawable.get());

    const osgTerrain::Locator* locator = computeMasterLocator(tile);
    if (locator)
    {
        osg::Matrixd matrix = locator->getTransform();

        osg::Vec3d center = osg::Vec3d(0.5, 0.5, 0.0) * matrix;

        // shift to center.x() to x=0 and carry all the corners with it.
        const osg::EllipsoidModel* em = locator->getEllipsoidModel();
        if (em && locator->getCoordinateSystemType()==osgTerrain::Locator::GEOCENTRIC)
        {
            osg::Matrixd localToWorldTransform;
            // note y axis maps to latitude, x axis to longitude
            em->computeLocalToWorldTransformFromLatLongHeight(center.y(), center.x(), center.z(), localToWorldTransform);
 //           OSG_NOTICE<<"We have a EllipsoidModel to take account of "<<localToWorldTransform<<std::endl;

            transform->setMatrix(localToWorldTransform);

            //osgDB::writeNodeFile(*transform, "subgraph.osgt");
        }
        else
        {
            transform->setMatrix(locator->getTransform());
        }
    }

    osg::Vec3Array* shared_vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    osg::Vec3Array* shared_normals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
    osg::FloatArray* heights = hf ? hf->getFloatArray() : 0;
    const SharedGeometry::VertexToHeightFieldMapping& vthfm = geometry->getVertexToHeightFieldMapping();

    if (hf && shared_vertices && shared_normals && (shared_vertices->size()==shared_normals->size()))
    {
        if (vthfm.size()==shared_vertices->size())
        {
            // Using cache VertexArray
            unsigned int numVertices = shared_vertices->size();
            osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
            vertices->resize(numVertices);

            for(unsigned int i=0; i<numVertices; ++i)
            {
                unsigned int hi = vthfm[i];
                (*vertices)[i] = (*shared_vertices)[i] + (*shared_normals)[i] * (*heights)[hi];
            }

            hfDrawable->setVertices(vertices.get());
        }
        else
        {
            // Setting local bounding
            unsigned int nr = hf->getNumRows();
            unsigned int nc = hf->getNumColumns();

            osg::BoundingBox bb;

            for(unsigned int r=0; r<nr; ++r)
            {
                for(unsigned int c=0; c<nc; ++c)
                {
                    unsigned int i = r*nc+c;
                    float h = (*heights)[i];
                    const osg::Vec3& v = (*shared_vertices)[i];
                    const osg::Vec3& n = (*shared_normals)[i];

                    const osg::Vec3 vt(v+n*h);
                    bb.expandBy(vt);
                }
            }
            hfDrawable->setInitialBound(bb);
            // OSG_NOTICE<<"Assigning initial bound ("<<bb.xMin()<<", "<<bb.xMax()<<") ,  ("<<bb.yMin()<<", "<<bb.yMax()<<") ("<<bb.zMin()<<", "<<bb.zMax()<<")"<< std::endl;
            // bb = hfDrawable->getBoundingBox();
            //OSG_NOTICE<<"         getBoundingBox ("<<bb.xMin()<<", "<<bb.xMax()<<") ,  ("<<bb.yMin()<<", "<<bb.yMax()<<") ("<<bb.zMin()<<", "<<bb.zMax()<<")"<< std::endl;

        }
    }

    osg::ref_ptr<osg::StateSet> stateset = transform->getOrCreateStateSet();

    // apply colour layers
    applyLayers(tile, stateset.get());

    return transform;
}

osg::ref_ptr<osg::Program> GeometryPool::getOrCreateProgram(LayerTypes& layerTypes)
{
    //OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_programMapMutex);
    ProgramMap::iterator p_itr = _programMap.find(layerTypes);
    if (p_itr!=_programMap.end())
    {
        // OSG_NOTICE<<") returning existing Program "<<p_itr->second.get()<<std::endl;
        return p_itr->second.get();
    }

#if 1
    unsigned int num_HeightField = 0;
    unsigned int num_Color = 0;
    unsigned int num_Contour = 0;
    for(LayerTypes::iterator itr = layerTypes.begin();
        itr != layerTypes.end();
        ++itr)
    {
        switch(*itr)
        {
            case(HEIGHTFIELD_LAYER): ++num_HeightField; break;
            case(COLOR_LAYER): ++num_Color; break;
            case(CONTOUR_LAYER): ++num_Contour; break;
        }
    }
    OSG_NOTICE<<"getOrCreateProgram()"<<std::endl;

    OSG_NOTICE<<"    HeightField "<<num_HeightField<<std::endl;
    OSG_NOTICE<<"    Color "<<num_Color<<std::endl;
    OSG_NOTICE<<"    Contour "<<num_Contour<<std::endl;

#endif

    osg::ref_ptr<osg::Program> program = new osg::Program;
    _programMap[layerTypes] = program;

    // add shader that provides the lighting functions
    {
        #include "shaders/lighting_vert.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::VERTEX, "shaders/lighting.vert", lighting_vert));
    }

    // OSG_NOTICE<<") creating new Program "<<program.get()<<std::endl;
    {
        #include "shaders/terrain_displacement_mapping_vert.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::VERTEX, "shaders/terrain_displacement_mapping.vert", terrain_displacement_mapping_vert));
    }


    {
        #include "shaders/terrain_displacement_mapping_geom.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::GEOMETRY, "shaders/terrain_displacement_mapping.geom", terrain_displacement_mapping_geom));

        program->setParameter( GL_GEOMETRY_VERTICES_OUT, 4 );
        program->setParameter( GL_GEOMETRY_INPUT_TYPE, GL_LINES_ADJACENCY );
        program->setParameter( GL_GEOMETRY_OUTPUT_TYPE, GL_TRIANGLE_STRIP);
    }

    if (num_Contour>0)
    {
        OSG_NOTICE<<"No support for Contour yet."<<std::endl;
    }

    {
        #include "shaders/terrain_displacement_mapping_frag.cpp"
        program->addShader(osgDB::readRefShaderFileWithFallback(osg::Shader::FRAGMENT, "shaders/terrain_displacement_mapping.frag", terrain_displacement_mapping_frag));
    }

    return program;
}

void GeometryPool::applyLayers(osgTerrain::TerrainTile* tile, osg::StateSet* stateset)
{
    typedef std::map<osgTerrain::Layer*, osg::Texture*> LayerToTextureMap;
    LayerToTextureMap layerToTextureMap;

 //   OSG_NOTICE<<"tile->getNumColorLayers() = "<<tile->getNumColorLayers()<<std::endl;

    LayerTypes layerTypes;

    osgTerrain::HeightFieldLayer* hfl = dynamic_cast<osgTerrain::HeightFieldLayer*>(tile->getElevationLayer());
    if (hfl)
    {
        osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(layerToTextureMap[hfl]);
        if (!texture2D)
        {
            texture2D = new osg::Texture2D;

            osg::ref_ptr<osg::Image> image = new osg::Image;

            const void* dataPtr = hfl->getHeightField()->getFloatArray()->getDataPointer();

            image->setImage(hfl->getNumRows(), hfl->getNumColumns(), 1,
                      GL_LUMINANCE32F_ARB,
                      GL_LUMINANCE, GL_FLOAT,
                      reinterpret_cast<unsigned char*>(const_cast<void*>(dataPtr)),
                      osg::Image::NO_DELETE);

            texture2D->setImage(image.get());
            texture2D->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
            texture2D->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
            texture2D->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
            texture2D->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
            texture2D->setBorderColor(osg::Vec4d(0.0,0.0,0.0,0.0));
            texture2D->setResizeNonPowerOfTwoHint(false);

            layerToTextureMap[hfl] = texture2D;
        }

        int textureUnit = layerTypes.size();
        stateset->setTextureAttributeAndModes(textureUnit, texture2D, osg::StateAttribute::ON);
        stateset->addUniform(new osg::Uniform("terrainTexture",textureUnit));

        layerTypes.push_back(HEIGHTFIELD_LAYER);
    }
#if 1
    int colorLayerNum = 0;
    for(unsigned int layerNum=0; layerNum<tile->getNumColorLayers(); ++layerNum)
    {
        osgTerrain::Layer* colorLayer = tile->getColorLayer(layerNum);
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

            int textureUnit = layerTypes.size();
            stateset->setTextureAttributeAndModes(textureUnit, texture2D, osg::StateAttribute::ON);

            std::stringstream str;
            str<<"colorTexture"<<colorLayerNum;
            stateset->addUniform(new osg::Uniform(str.str().c_str(),textureUnit));

            layerTypes.push_back(COLOR_LAYER);

            ++colorLayerNum;

        }
        else if (contourLayer)
        {
            OSG_NOTICE<<"Warning : GeometryPool does not presently support ContourLayers."<<std::endl;
        }
    }
#endif


    // If we have a root StateSet assigned for Terrain?
    //   If tile_LayerTypes a subset of root_LayerTypes then toggle on/off locally
    //   If tile_LayerTypes a superset of root_LayerTypes then create local Program
    //   else no need to set anything
    // else no root StateSet
    //    create new StateSet for Program required and assigned with layers required enabled
    //



    //stateset->setDefine("GL_LIGHTING", osg::StateAttribute::ON);
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex>  lock(_programMapMutex);
        if (!_rootStateSetAssigned)
        {
            _rootStateSetAssigned = true;

            _rootStateSet->setDefine("LIGHTING");

            int num_Color = 0;
            for(LayerTypes::iterator itr = layerTypes.begin();
                itr != layerTypes.end();
                ++itr)
            {
                switch(*itr)
                {
                    case(HEIGHTFIELD_LAYER): _rootStateSet->setDefine("HEIGHTFIELD_LAYER"); break;
                    case(COLOR_LAYER): ++num_Color; break;
                    case(CONTOUR_LAYER): break; // not supported right now
                }
            }

            if (num_Color>=1)
            {
                _rootStateSet->setDefine("TEXTURE_2D");
                _rootStateSet->setDefine("COLOR_LAYER0");
            }

            if (num_Color>=2) _rootStateSet->setDefine("COLOR_LAYER1");
            if (num_Color>=3) _rootStateSet->setDefine("COLOR_LAYER2");

            osg::ref_ptr<osg::Program> program = getOrCreateProgram(layerTypes);
            if (program.valid())
            {
                _rootStateSet->setAttribute(program.get());
            }
        }
    }
}

osg::StateSet* GeometryPool::getRootStateSetForTerrain(Terrain* /*terrain*/)
{
    //OSG_NOTICE<<"getRootStateSetForTerrain("<<terrain<<")"<<std::endl;
    return _rootStateSet.get();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  SharedGeometry
//
SharedGeometry::SharedGeometry()
{
    setSupportsDisplayList(false);
    _supportsVertexBufferObjects = true;
}

SharedGeometry::SharedGeometry(const SharedGeometry& rhs,const osg::CopyOp& copyop):
    osg::Drawable(rhs, copyop),
    _vertexArray(rhs._vertexArray),
    _normalArray(rhs._normalArray),
    _colorArray(rhs._colorArray),
    _texcoordArray(rhs._texcoordArray),
    _drawElements(rhs._drawElements),
    _vertexToHeightFieldMapping(rhs._vertexToHeightFieldMapping)
{
//    setSupportsDisplayList(false);
}

SharedGeometry::~SharedGeometry()
{
}

osg::VertexArrayState* SharedGeometry::createVertexArrayStateImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

    osg::VertexArrayState* vas = new osg::VertexArrayState(&state);

    // OSG_NOTICE<<"Creating new osg::VertexArrayState "<< vas<<std::endl;

    if (_vertexArray.valid()) vas->assignVertexArrayDispatcher();
    if (_colorArray.valid()) vas->assignColorArrayDispatcher();
    if (_normalArray.valid()) vas->assignNormalArrayDispatcher();
    if (_texcoordArray.valid()) vas->assignTexCoordArrayDispatcher(1);

    if (state.useVertexArrayObject(_useVertexArrayObject))
    {
        // OSG_NOTICE<<"  Setup VertexArrayState to use VAO "<<vas<<std::endl;

        vas->generateVertexArrayObject();
    }
    else
    {
        // OSG_NOTICE<<"  Setup VertexArrayState to without using VAO "<<vas<<std::endl;
    }

    return vas;
}

void SharedGeometry::compileGLObjects(osg::RenderInfo& renderInfo) const
{
    // OSG_NOTICE<<"SharedGeometry::compileGLObjects() "<<this<<std::endl;

    if (!_vertexArray) return;
    if (_vertexArray->getVertexBufferObject())
    {
        osg::State& state = *renderInfo.getState();
        unsigned int contextID = state.getContextID();
        osg::GLExtensions* extensions = state.get<osg::GLExtensions>();
        if (!extensions) return;

        osg::BufferObject* vbo = _vertexArray->getVertexBufferObject();
        osg::GLBufferObject* vbo_glBufferObject = vbo->getOrCreateGLBufferObject(contextID);
        if (vbo_glBufferObject && vbo_glBufferObject->isDirty())
        {
            // OSG_NOTICE<<"Compile buffer "<<glBufferObject<<std::endl;
            vbo_glBufferObject->compileBuffer();
            extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        }

        osg::BufferObject* ebo = _drawElements->getElementBufferObject();
        osg::GLBufferObject* ebo_glBufferObject = ebo->getOrCreateGLBufferObject(contextID);
        if (ebo_glBufferObject && ebo_glBufferObject->isDirty())
        {
            // OSG_NOTICE<<"Compile buffer "<<glBufferObject<<std::endl;
            ebo_glBufferObject->compileBuffer();
            extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
        }

        if (state.useVertexArrayObject(_useVertexArrayObject))
        {

            osg::VertexArrayState* vas = 0;

            _vertexArrayStateList[contextID] = vas = createVertexArrayState(renderInfo);

            // OSG_NOTICE<<"  setting up VertexArrayObject "<<vas<<std::endl;

            osg::State::SetCurrentVertexArrayStateProxy setVASProxy(state, vas);

            state.bindVertexArrayObject(vas);

            if (vbo_glBufferObject) vas->bindVertexBufferObject(vbo_glBufferObject);
            if (ebo_glBufferObject) vas->bindElementBufferObject(ebo_glBufferObject);

            state.unbindVertexArrayObject();
        }

    }
    else
    {
        Drawable::compileGLObjects(renderInfo);
    }
}

void SharedGeometry::resizeGLObjectBuffers(unsigned int maxSize)
{
    Drawable::resizeGLObjectBuffers(maxSize);

    osg::BufferObject* vbo = _vertexArray->getVertexBufferObject();
    if (vbo) vbo->resizeGLObjectBuffers(maxSize);

    osg::BufferObject* ebo = _drawElements->getElementBufferObject();
    if (ebo) ebo->resizeGLObjectBuffers(maxSize);
}

void SharedGeometry::releaseGLObjects(osg::State* state) const
{
    Drawable::releaseGLObjects(state);

    osg::BufferObject* vbo = _vertexArray->getVertexBufferObject();
    if (vbo) vbo->releaseGLObjects(state);

    osg::BufferObject* ebo = _drawElements->getElementBufferObject();
    if (ebo) ebo->releaseGLObjects(state);
}

void SharedGeometry::drawImplementation(osg::RenderInfo& renderInfo) const
{
    bool computeDiagonals = renderInfo.getState()->supportsShaderRequirement("COMPUTE_DIAGONALS");
    // OSG_NOTICE<<"SharedGeometry::drawImplementation "<<computeDiagonals<<std::endl;

    osg::State& state = *renderInfo.getState();
    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();


    // state.checkGLErrors("Before SharedGeometry::drawImplementation.");


    bool checkForGLErrors = state.getCheckForGLErrors()==osg::State::ONCE_PER_ATTRIBUTE;
    if (checkForGLErrors) state.checkGLErrors("start of SharedGeometry::drawImplementation()");

    osg::AttributeDispatchers& attributeDispatchers = state.getAttributeDispatchers();

    attributeDispatchers.reset();
    attributeDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());

    attributeDispatchers.activateNormalArray(_normalArray.get());
    attributeDispatchers.activateColorArray(_colorArray.get());

    if (!state.useVertexArrayObject(_useVertexArrayObject) || vas->getRequiresSetArrays())
    {
        // OSG_NOTICE<<"   sending vertex arrays vas->getRequiresSetArrays()="<<vas->getRequiresSetArrays()<<std::endl;

        vas->lazyDisablingOfVertexAttributes();

        // set up arrays
        if( _vertexArray.valid() )
            vas->setVertexArray(state, _vertexArray.get());

        if (_normalArray.valid() && _normalArray->getBinding()==osg::Array::BIND_PER_VERTEX)
            vas->setNormalArray(state, _normalArray.get());

        if (_colorArray.valid() && _colorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
            vas->setColorArray(state, _colorArray.get());

        if (_texcoordArray.valid() && _texcoordArray->getBinding()==osg::Array::BIND_PER_VERTEX)
            vas->setTexCoordArray(state, 0, _texcoordArray.get());

        vas->applyDisablingOfVertexAttributes(state);
    }

    if (checkForGLErrors) state.checkGLErrors("Geometry::drawImplementation() after vertex arrays setup.");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    GLenum primitiveType = computeDiagonals ? GL_LINES_ADJACENCY : GL_QUADS;

    bool request_bind_unbind = !state.useVertexArrayObject(_useVertexArrayObject) || state.getCurrentVertexArrayState()->getRequiresSetArrays();

    osg::GLBufferObject* ebo = _drawElements->getOrCreateGLBufferObject(state.getContextID());

    if (ebo)
    {
        /*if (request_bind_unbind)*/ state.bindElementBufferObject(ebo);

        glDrawElements(primitiveType, _drawElements->getNumIndices(), _drawElements->getDataType(), (const GLvoid *)(ebo->getOffset(_drawElements->getBufferIndex())));

        /*if (request_bind_unbind)*/ state.unbindElementBufferObject();
    }
    else
    {
        glDrawElements(primitiveType, _drawElements->getNumIndices(), _drawElements->getDataType(), _drawElements->getDataPointer());
    }

    // unbind the VBO's if any are used.
    if (request_bind_unbind) state.unbindVertexBufferObject();

    if (checkForGLErrors) state.checkGLErrors("end of SharedGeometry::drawImplementation().");
}

void SharedGeometry::accept(osg::Drawable::AttributeFunctor& af)
{
    osg::AttributeFunctorArrayVisitor afav(af);

    afav.applyArray(VERTICES,_vertexArray.get());
    afav.applyArray(NORMALS, _normalArray.get());
    afav.applyArray(COLORS, _colorArray.get());
    afav.applyArray(TEXTURE_COORDS_0,_texcoordArray.get());
}

void SharedGeometry::accept(osg::Drawable::ConstAttributeFunctor& af) const
{
    osg::ConstAttributeFunctorArrayVisitor afav(af);

    afav.applyArray(VERTICES,_vertexArray.get());
    afav.applyArray(NORMALS, _normalArray.get());
    afav.applyArray(COLORS, _colorArray.get());
    afav.applyArray(TEXTURE_COORDS_0,_texcoordArray.get());
}

void SharedGeometry::accept(osg::PrimitiveFunctor& pf) const
{
    pf.setVertexArray(_vertexArray->getNumElements(),static_cast<const osg::Vec3*>(_vertexArray->getDataPointer()));

    _drawElements->accept(pf);
}

void SharedGeometry::accept(osg::PrimitiveIndexFunctor& pif) const
{
    pif.setVertexArray(_vertexArray->getNumElements(),static_cast<const osg::Vec3*>(_vertexArray->getDataPointer()));

    _drawElements->accept(pif);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  HeightFieldDrawable
//
HeightFieldDrawable::HeightFieldDrawable()
{
    setSupportsDisplayList(false);
}

HeightFieldDrawable::HeightFieldDrawable(const HeightFieldDrawable& rhs,const osg::CopyOp& copyop):
    osg::Drawable(rhs, copyop),
    _heightField(rhs._heightField),
    _geometry(rhs._geometry),
    _vertices(rhs._vertices)
{
    setSupportsDisplayList(false);
}

HeightFieldDrawable::~HeightFieldDrawable()
{
}

void HeightFieldDrawable::drawImplementation(osg::RenderInfo& renderInfo) const
{
    if (_geometry.valid()) _geometry->draw(renderInfo);
}

void HeightFieldDrawable::compileGLObjects(osg::RenderInfo& renderInfo) const
{
    if (_geometry.valid()) _geometry->compileGLObjects(renderInfo);
}

void HeightFieldDrawable::resizeGLObjectBuffers(unsigned int maxSize)
{
    if (_geometry.valid()) _geometry->resizeGLObjectBuffers(maxSize);
}

void HeightFieldDrawable::releaseGLObjects(osg::State* state) const
{
    if (_geometry.valid()) _geometry->releaseGLObjects(state);
}

void HeightFieldDrawable::accept(osg::Drawable::AttributeFunctor& af)
{
    if (_geometry) _geometry->accept(af);
}

void HeightFieldDrawable::accept(osg::Drawable::ConstAttributeFunctor& caf) const
{
    if (_geometry) _geometry->accept(caf);
}

void HeightFieldDrawable::accept(osg::PrimitiveFunctor& pf) const
{
    // use the cached vertex positions for PrimitiveFunctor operations
    if (!_geometry) return;

    if (_vertices.valid())
    {
        pf.setVertexArray(_vertices->size(), &((*_vertices)[0]));

        const osg::DrawElementsUShort* deus = dynamic_cast<const osg::DrawElementsUShort*>(_geometry->getDrawElements());
        if (deus)
        {
            pf.drawElements(GL_QUADS, deus->size(), &((*deus)[0]));
        }
        else
        {
            const osg::DrawElementsUInt* deui = dynamic_cast<const osg::DrawElementsUInt*>(_geometry->getDrawElements());
            if (deui)
            {
                pf.drawElements(GL_QUADS, deui->size(), &((*deui)[0]));
            }
        }
    }
    else
    {
        _geometry->accept(pf);
    }
}

void HeightFieldDrawable::accept(osg::PrimitiveIndexFunctor& pif) const
{
    if (_vertices.valid())
    {
        pif.setVertexArray(_vertices->size(), &((*_vertices)[0]));

        const osg::DrawElementsUShort* deus = dynamic_cast<const osg::DrawElementsUShort*>(_geometry->getDrawElements());
        if (deus)
        {
            pif.drawElements(GL_QUADS, deus->size(), &((*deus)[0]));
        }
        else
        {
            const osg::DrawElementsUInt* deui = dynamic_cast<const osg::DrawElementsUInt*>(_geometry->getDrawElements());
            if (deui)
            {
                pif.drawElements(GL_QUADS, deui->size(), &((*deui)[0]));
            }
        }
    }
    else
    {
        _geometry->accept(pif);
    }
}
