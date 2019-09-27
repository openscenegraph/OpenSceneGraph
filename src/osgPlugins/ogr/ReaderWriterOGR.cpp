/* -*- mode: c++; c-default-style: k&r; tab-width: 4; c-basic-offset: 4; -*-
 * Copyright (C) 2007 Cedric Pinson - mornifle@plopbyte.net
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

#include <osg/Image>
#include <osg/Notify>
#include <osg/Geode>
#include <osg/GL>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Material>
#include <osg/TriangleFunctor>
#include <osgUtil/Tessellator>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ImageOptions>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/ReentrantMutex>
#include <gdal_priv.h>
#include <ogr_feature.h>
#include <cpl_error.h>
#include <ogr_core.h>
#include <ogr_feature.h>
#include <ogrsf_frmts.h>

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)

void CPL_STDCALL CPLOSGErrorHandler( CPLErr eErrClass, int nError,
                             const char * pszErrorMsg )
{
    if( eErrClass == CE_Debug )
    {
        OSG_DEBUG << pszErrorMsg << std::endl;
    }
    else if( eErrClass == CE_Warning )
    {
        OSG_WARN << nError << " " << pszErrorMsg << std::endl;
    }
    else
    {
        OSG_FATAL << nError << " " << pszErrorMsg << std::endl;
    }
}

static osg::Material* createDefaultMaterial()
{
    osg::Vec4 color;
    for (int i = 0; i < 3; i++)
        color[i] = (1.0 * (rand() / (1.0*RAND_MAX)));
    color[3] = 1;
    osg::Material* mat = new osg::Material;
    mat->setDiffuse(osg::Material::FRONT_AND_BACK, color);
    return mat;
}

struct TriangulizeFunctor
{
    osg::Vec3Array* _vertexes;

    // do nothing
    void operator ()(const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3)
    {
        _vertexes->push_back(v1);
        _vertexes->push_back(v2);
        _vertexes->push_back(v3);
    }
};

static osg::Vec3Array* triangulizeGeometry(osg::Geometry* src)
{
    if (src->getNumPrimitiveSets() == 1 &&
        src->getPrimitiveSet(0)->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType &&
        src->getVertexArray() &&
        src->getVertexArray()->getType() == osg::Array::Vec3ArrayType)
        return static_cast<osg::Vec3Array*>(src->getVertexArray());

    osg::TriangleFunctor<TriangulizeFunctor> functor;
    osg::Vec3Array* array = new osg::Vec3Array;
    functor._vertexes = array;
    src->accept(functor);
    return array;
}


class ReaderWriterOGR : public osgDB::ReaderWriter
{

public:
    ReaderWriterOGR()
    {
        supportsExtension("ogr","OGR file reader");
        supportsOption("useRandomColorByFeature", "Assign a random color to each feature.");
        supportsOption("addGroupPerFeature", "Places each feature in a separate group.");
        oldHandler = CPLSetErrorHandler(CPLOSGErrorHandler);
    }

    virtual ~ReaderWriterOGR()
    {
        CPLSetErrorHandler(oldHandler);
    }

    virtual const char* className() const { return "OGR file reader"; }


    virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(fileName, options);
    }

    virtual ReadResult readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
    {
        OSG_INFO<<"OGR::readNode("<<file<<")"<<std::endl;

        if (file.empty()) return ReadResult::FILE_NOT_FOUND;

        if (osgDB::equalCaseInsensitive(osgDB::getFileExtension(file),"ogr"))
        {
            OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);
            return readFile(osgDB::getNameLessExtension(file), options);
        }

        OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex);
        std::string fileName = osgDB::findDataFile( file, options );
        if (fileName.empty()) return readFile(file, options); // ReadResult::FILE_NOT_FOUND;

        return readFile(fileName, options);
    }

    virtual ReadResult readFile(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
#if GDAL_VERSION_MAJOR<2
        if (OGRSFDriverRegistrar::GetRegistrar()->GetDriverCount() == 0)
            OGRRegisterAll();

        // Try to open data source
        OGRDataSource* file = OGRSFDriverRegistrar::Open(fileName.c_str());
#else
        if (GDALGetDriverCount() == 0)
            GDALAllRegister();

        // Try to open data source
        GDALDataset* file  = (GDALDataset*) GDALOpenEx( fileName.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
#endif

        if (!file)
            return 0;

        bool useRandomColorByFeature = false;
        bool addGroupPerFeature = false;
        if (options)
        {
            if (options->getOptionString().find("UseRandomColorByFeature") != std::string::npos)
                useRandomColorByFeature = true;
            if (options->getOptionString().find("useRandomColorByFeature") != std::string::npos)
                useRandomColorByFeature = true;
            if (options->getOptionString().find("addGroupPerFeature") != std::string::npos)
                addGroupPerFeature = true;
        }

        osg::Group* group = new osg::Group;

#if GDAL_VERSION_MAJOR<2
        for (int i = 0; i < file->GetLayerCount(); i++)
        {
            osg::Group* node = readLayer(file->GetLayer(i), file->GetName(), useRandomColorByFeature, addGroupPerFeature);
            if (node)
                group->addChild( node );
        }
        OGRDataSource::DestroyDataSource( file );
#else
        for (int i = 0; i < GDALDatasetGetLayerCount(file); i++)
        {
            OGRLayer* layer = (OGRLayer *)GDALDatasetGetLayer(file, i);
            osg::Group* node = readLayer(layer, layer->GetName(), useRandomColorByFeature, addGroupPerFeature);
            if (node)
                group->addChild( node );
        }
        GDALClose( file );
#endif

        return group;
    }

    osg::Group* readLayer(OGRLayer* ogrLayer, const std::string& /*name*/, bool useRandomColorByFeature, bool addGroupPerFeature) const
    {
        if (!ogrLayer)
            return 0;

        osg::Group* layer = new osg::Group;
        layer->setName(ogrLayer->GetLayerDefn()->GetName());
        ogrLayer->ResetReading();

        OGRFeature* ogrFeature = NULL;
        while ((ogrFeature = ogrLayer->GetNextFeature()) != NULL)
        {
            osg::Geode* feature = readFeature(ogrFeature, useRandomColorByFeature);
            if (feature)
            {
                if (addGroupPerFeature)
                {
                    osg::Group* featureGroup = new osg::Group;
                    featureGroup->addChild(feature);
                    layer->addChild(featureGroup);
                }
                else
                {
                    layer->addChild(feature);
                }
            }
            OGRFeature::DestroyFeature( ogrFeature );
        }
        return layer;
    }

    osg::Geometry* pointsToDrawable(const OGRPoint* points) const
    {
        osg::Geometry* pointGeom = new osg::Geometry();
        osg::Vec3Array* vertices = new osg::Vec3Array();
        vertices->push_back(osg::Vec3(points->getX(), points->getY(), points->getZ()));
        pointGeom->setVertexArray(vertices);
        pointGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
        return pointGeom;
    }

    osg::Geometry* linearRingToDrawable(OGRLinearRing* ring) const
    {
        osg::Geometry* contourGeom = new osg::Geometry();
        osg::Vec3Array* vertices = new osg::Vec3Array();
        OGRPoint point;
        for(int j = 0; j < ring->getNumPoints(); j++)
        {
            ring->getPoint(j, &point);
            vertices->push_back(osg::Vec3(point.getX(), point.getY(),point.getZ()));
        }
        contourGeom->setVertexArray(vertices);
        contourGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertices->size()));
        return contourGeom;
    }

    osg::Geometry* lineStringToDrawable(OGRLineString* lineString) const
    {
        osg::Geometry* contourGeom = new osg::Geometry();
        osg::Vec3Array* vertices = new osg::Vec3Array();
        OGRPoint point;
        for(int j = 0; j < lineString->getNumPoints(); j++)
        {
            lineString->getPoint(j, &point);
            vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
        }

        contourGeom->setVertexArray(vertices);
        contourGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertices->size()));
        return contourGeom;
    }

    osg::Geometry* multiPointToDrawable(OGRMultiPoint* mpoint) const
    {
        osg::Geometry* pointGeom = new osg::Geometry();
        osg::Vec3Array* vertices = new osg::Vec3Array();

        vertices->reserve(mpoint->getNumGeometries());
        for (int i = 0; i < mpoint->getNumGeometries(); i++ )
        {
            OGRGeometry* ogrGeom = mpoint->getGeometryRef(i);
            OGRwkbGeometryType ogrGeomType = ogrGeom->getGeometryType();

            if (wkbPoint != ogrGeomType && wkbPoint25D != ogrGeomType)
                continue; // skip

            OGRPoint* points = static_cast<OGRPoint*>(ogrGeom);

            vertices->push_back(osg::Vec3(points->getX(), points->getY(), points->getZ()));
        }

        pointGeom->setVertexArray(vertices);
        pointGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, vertices->size()));

        if (pointGeom->getVertexArray())
        {
            OSG_INFO << "osgOgrFeature::multiPointToDrawable " << pointGeom->getVertexArray()->getNumElements() << " vertices"<< std::endl;
        }

        return pointGeom;
    }

    osg::Geometry* multiPolygonToDrawable(OGRMultiPolygon* mpolygon) const
    {
        osg::Geometry* geom = new osg::Geometry;

        for (int i = 0; i < mpolygon->getNumGeometries(); i++ )
        {
            OGRGeometry* ogrGeom = mpolygon->getGeometryRef(i);
            OGRwkbGeometryType ogrGeomType = ogrGeom->getGeometryType();

            if (wkbPolygon != ogrGeomType && wkbPolygon25D != ogrGeomType)
                continue; // skip

            OGRPolygon* polygon = static_cast<OGRPolygon*>(ogrGeom);
            osg::ref_ptr<osg::Drawable> drw = polygonToDrawable(polygon);
            osg::ref_ptr<osg::Geometry> geometry = drw->asGeometry();
            if (geometry.valid() && geometry->getVertexArray() &&
                geometry->getVertexArray()->getNumElements() &&
                geometry->getNumPrimitiveSets() &&
                geometry->getVertexArray()->getType() == osg::Array::Vec3ArrayType )
            {

                if (!geom->getVertexArray())
                { // no yet data we put the first in
                    geom->setVertexArray(geometry->getVertexArray());
                    geom->setPrimitiveSetList(geometry->getPrimitiveSetList());

                }
                else
                { // already a polygon then append
                    int size = geom->getVertexArray()->getNumElements();
                    osg::Vec3Array* arrayDst = static_cast<osg::Vec3Array*>(geom->getVertexArray());
                    osg::ref_ptr<osg::Vec3Array> triangulized = triangulizeGeometry(geometry.get());
                    if (triangulized.valid())
                    {
                        arrayDst->insert(arrayDst->end(), triangulized->begin(), triangulized->end());
                        // shift index
                        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, size, triangulized->size()));
                    }
                }
            }
            else
            {
                OSG_WARN << "Warning something wrong with a polygon in a multi polygon" << std::endl;
            }
        }

        if (geom->getVertexArray())
        {
            OSG_INFO << "osgOgrFeature::multiPolygonToDrawable " << geom->getVertexArray()->getNumElements() << " vertices"<< std::endl;
        }

        return geom;
    }

    osg::Geometry* polygonToDrawable(OGRPolygon* polygon) const
    {
        osg::Geometry* geom = new osg::Geometry();
        osg::Vec3Array* vertices = new osg::Vec3Array();
        geom->setVertexArray(vertices);
        {
            OGRLinearRing *ring = polygon->getExteriorRing();
            OGRPoint point;
            for(int i = 0; i < ring->getNumPoints(); i++)
            {
                ring->getPoint(i, &point);
                vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
            }
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertices->size()));
        }

        if (polygon->getNumInteriorRings())
        {
            for (int i = 0; i < polygon->getNumInteriorRings(); i++)
            {
                OGRLinearRing *ring = polygon->getInteriorRing(i);
                OGRPoint point;
                for (int j = 0; j < ring->getNumPoints(); j++)
                {
                    ring->getPoint(j, &point);
                    vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
                }
                geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, vertices->size()-ring->getNumPoints() , ring->getNumPoints()));
            }
        }
        osgUtil::Tessellator tsl;
        tsl.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        tsl.setBoundaryOnly(false);
        tsl.retessellatePolygons(*geom);

        osg::Vec3Array* array = triangulizeGeometry(geom);
        geom->setVertexArray(array);
        geom->removePrimitiveSet(0,geom->getNumPrimitiveSets());
        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, array->size()));

        return geom;
    }
    osg::Geometry* multiLineStringToDrawable(OGRMultiLineString* mlineString) const
    {
        osg::Geometry* geom = new osg::Geometry;

        for (int i = 0; i < mlineString->getNumGeometries(); i++ )
        {
            OGRGeometry* ogrGeom = mlineString->getGeometryRef(i);
            OGRwkbGeometryType ogrGeomType = ogrGeom->getGeometryType();

            if (wkbLineString != ogrGeomType && wkbLineString25D != ogrGeomType)
                continue; // skip

            OGRLineString* lineString = static_cast<OGRLineString*>(ogrGeom);
            osg::ref_ptr<osg::Geometry> geometry = lineStringToDrawable(lineString);
            if (geometry.valid() &&
                geometry->getVertexArray() &&
                geometry->getNumPrimitiveSets() &&
                geometry->getVertexArray()->getType() == osg::Array::Vec3ArrayType)
            {

                if (!geom->getVertexArray())
                {
                    geom->setVertexArray(geometry->getVertexArray());
                    geom->setPrimitiveSetList(geometry->getPrimitiveSetList());

                }
                else
                {
                    int size = geom->getVertexArray()->getNumElements();

                    osg::Vec3Array* arraySrc = static_cast<osg::Vec3Array*>(geometry->getVertexArray());
                    osg::Vec3Array* arrayDst = static_cast<osg::Vec3Array*>(geom->getVertexArray());
                    arrayDst->insert(arrayDst->end(), arraySrc->begin(), arraySrc->end());
                    // shift index
                    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, size, arraySrc->size()));
                }
            }
        }
        return geom;
    }

    osg::Geode* readFeature(OGRFeature* ogrFeature, bool useRandomColorByFeature) const
    {

        if (!ogrFeature || !ogrFeature->GetGeometryRef())
            return 0;

        osg::Geometry* drawable = 0;
        bool disableCulling = false;

        // Read the geometry
        switch(ogrFeature->GetGeometryRef()->getGeometryType()) {
        case wkbPoint:
        case wkbPoint25D:
            // point to drawable
            drawable = pointsToDrawable(static_cast<OGRPoint *>(ogrFeature->GetGeometryRef()));
            disableCulling = true;
            break;

        case wkbLinearRing:
            drawable = linearRingToDrawable(static_cast<OGRLinearRing *>(ogrFeature->GetGeometryRef()));
            break;

        case wkbLineString:
        case wkbLineString25D:
            drawable = lineStringToDrawable(static_cast<OGRLineString*>(ogrFeature->GetGeometryRef()));
            break;

        case wkbPolygon:
        case wkbPolygon25D:
            drawable = polygonToDrawable(static_cast<OGRPolygon*>(ogrFeature->GetGeometryRef()));
            break;

        case wkbMultiPoint:
        case wkbMultiPoint25D:
            drawable = multiPointToDrawable(static_cast<OGRMultiPoint*>(ogrFeature->GetGeometryRef()));
            disableCulling = true;
            break;

        case wkbMultiLineString:
        case wkbMultiLineString25D:
            drawable = multiLineStringToDrawable(static_cast<OGRMultiLineString*>(ogrFeature->GetGeometryRef()));
            break;

        case wkbMultiPolygon:
        case wkbMultiPolygon25D:
            drawable = multiPolygonToDrawable(static_cast<OGRMultiPolygon*>(ogrFeature->GetGeometryRef()));
            break;

        case wkbGeometryCollection:
        case wkbGeometryCollection25D:
            OSG_WARN << "This geometry is not yet implemented " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
            break;

        case wkbNone:
            OSG_WARN << "No WKB Geometry " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
            break;

        case wkbUnknown:
        default:
            OSG_WARN << "Unknown WKB Geometry " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
            break;
        }

        if (!drawable)
            return 0;

        osg::Geode* geode = new osg::Geode();
        if (disableCulling)
            geode->setCullingActive(false); // because culling on one points geode is always true, so i disable it
        geode->addDrawable(drawable);
        if (useRandomColorByFeature)
            geode->getOrCreateStateSet()->setAttributeAndModes(createDefaultMaterial(),true);
        for(int i = 0; i < ogrFeature->GetFieldCount(); i++) {
            geode->addDescription(std::string(ogrFeature->GetFieldDefnRef(i)->GetNameRef()) + " : " + std::string(ogrFeature->GetFieldAsString(i)));
        }
        return geode;
    }

    mutable OpenThreads::ReentrantMutex _serializerMutex;
    CPLErrorHandler oldHandler;
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ogr, ReaderWriterOGR)
