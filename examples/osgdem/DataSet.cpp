/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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

#include "DataSet.h"

#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Group>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#include <gdal_priv.h>
#include "cpl_string.h"
#include <gdalwarper.h>

#include <sstream>

DataSet::SourceData* DataSet::SourceData::readData(Source* source)
{
    if (!source) return 0;


    switch(source->getType())
    {
    case(Source::IMAGE):
    case(Source::HEIGHT_FIELD):
        {
            GDALDataset* gdalDataSet = (GDALDataset*)GDALOpen(source->getFileName().c_str(),GA_ReadOnly);
            if (gdalDataSet)
            {
                SourceData* data = new SourceData(source);
                data->_gdalDataSet = gdalDataSet;
                
                data->_numValuesX = gdalDataSet->GetRasterXSize();
                data->_numValuesY = gdalDataSet->GetRasterYSize();
                data->_numValuesZ = gdalDataSet->GetRasterCount();
                data->_hasGCPs = gdalDataSet->GetGCPCount()!=0;

                const char* pszSourceSRS = gdalDataSet->GetProjectionRef();
                if (!pszSourceSRS || strlen(pszSourceSRS)==0) pszSourceSRS = gdalDataSet->GetGCPProjection();
                
                data->_cs = new osgTerrain::CoordinateSystem(pszSourceSRS);

                double geoTransform[6];
                if (gdalDataSet->GetGeoTransform(geoTransform)==CE_None)
                {
                    data->_geoTransform.set( geoTransform[1],    geoTransform[4],    0.0,    0.0,
                                             geoTransform[2],    geoTransform[5],    0.0,    0.0,
                                             0.0,                0.0,                1.0,    0.0,
                                             geoTransform[0],    geoTransform[3],    0.0,    1.0);
                                            
                    data->_extents.init();
                    data->_extents.expandBy( osg::Vec3(0.0,0.0,0.0)*data->_geoTransform);
                    data->_extents.expandBy( osg::Vec3(data->_numValuesX,data->_numValuesY,0.0)*data->_geoTransform);

                }
                else if (gdalDataSet->GetGCPCount()>0 && gdalDataSet->GetGCPProjection())
                {
                    std::cout << "    Using GCP's"<< std::endl;


                    /* -------------------------------------------------------------------- */
                    /*      Create a transformation object from the source to               */
                    /*      destination coordinate system.                                  */
                    /* -------------------------------------------------------------------- */
                    void *hTransformArg = 
                        GDALCreateGenImgProjTransformer( gdalDataSet, pszSourceSRS, 
                                                         NULL, pszSourceSRS, 
                                                         TRUE, 0.0, 1 );

                    if ( hTransformArg == NULL )
                    {
                        std::cout<<" failed to create transformer"<<std::endl;
                        return NULL;
                    }

                    /* -------------------------------------------------------------------- */
                    /*      Get approximate output definition.                              */
                    /* -------------------------------------------------------------------- */
                    double adfDstGeoTransform[6];
                    int nPixels=0, nLines=0;
                    if( GDALSuggestedWarpOutput( gdalDataSet, 
                                                 GDALGenImgProjTransform, hTransformArg, 
                                                 adfDstGeoTransform, &nPixels, &nLines )
                        != CE_None )
                    {
                        std::cout<<" failed to create warp"<<std::endl;
                        return NULL;
                    }

                    GDALDestroyGenImgProjTransformer( hTransformArg );


                    data->_geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],    0.0,    0.0,
                                             adfDstGeoTransform[2],    adfDstGeoTransform[5],    0.0,    0.0,
                                             0.0,                0.0,                1.0,    0.0,
                                             adfDstGeoTransform[0],    adfDstGeoTransform[3],    0.0,    1.0);

                    data->_extents.init();
                    data->_extents.expandBy( osg::Vec3(0.0,0.0,0.0)*data->_geoTransform);
                    data->_extents.expandBy( osg::Vec3(nPixels,nLines,0.0)*data->_geoTransform);
                    
                }
                else
                {
                    std::cout << "    No GeoTransform or GCP's - unable to compute position in space"<< std::endl;
                    
                }
                return data;
            }                
        }
    case(Source::MODEL):
        {
            osg::Node* model = osgDB::readNodeFile(source->getFileName().c_str());
            if (model)
            {
                SourceData* data = new SourceData(source);
                data->_model = model;
                data->_extents.expandBy(model->getBound());
            }
            
        }
        break;
    }
    
    return 0;
}

osg::BoundingBox DataSet::SourceData::getExtents(const osgTerrain::CoordinateSystem* cs) const
{
    if (_cs==cs) return _extents;
    if (_cs.valid() && cs)
    {
        if (*_cs == *cs) return _extents;
        
        if (_gdalDataSet)
        {

            //std::cout<<"Projecting bounding volume for "<<_source->getFileName()<<std::endl;

            osg::BoundingBox bb;
            
            /* -------------------------------------------------------------------- */
            /*      Create a transformation object from the source to               */
            /*      destination coordinate system.                                  */
            /* -------------------------------------------------------------------- */
            void *hTransformArg = 
                GDALCreateGenImgProjTransformer( _gdalDataSet,_cs->getProjectionRef().c_str(),
                                                 NULL, cs->getProjectionRef().c_str(),
                                                 TRUE, 0.0, 1 );

            if (!hTransformArg)
            {
                std::cout<<" failed to create transformer"<<std::endl;
                return bb;
            }
        
            double adfDstGeoTransform[6];
            int nPixels=0, nLines=0;
            if( GDALSuggestedWarpOutput( _gdalDataSet, 
                                         GDALGenImgProjTransform, hTransformArg, 
                                         adfDstGeoTransform, &nPixels, &nLines )
                != CE_None )
            {
                std::cout<<" failed to create warp"<<std::endl;
                return bb;
            }

            osg::Matrixd geoTransform( adfDstGeoTransform[1],    adfDstGeoTransform[4],    0.0,    0.0,
                                       adfDstGeoTransform[2],    adfDstGeoTransform[5],    0.0,    0.0,
                                       0.0,                0.0,                1.0,    0.0,
                                       adfDstGeoTransform[0],    adfDstGeoTransform[3],    0.0,    1.0);

            GDALDestroyGenImgProjTransformer( hTransformArg );

            bb.expandBy( osg::Vec3(0.0,0.0,0.0)*geoTransform);
            bb.expandBy( osg::Vec3(nPixels,nLines,0.0)*geoTransform);
            
            return bb;
        }

    }
    std::cout<<"DataSet::DataSource::assuming compatible coordinates."<<std::endl;
    return _extents;
}

const DataSet::SpatialProperties& DataSet::SourceData::computeSpatialProperties(osgTerrain::CoordinateSystem* cs) const
{
    if (_cs==cs) return *this;
    if (_cs.valid() && cs)
    {
        if (*_cs == *cs) return *this;
        
        // check to see it exists in the _spatialPropertiesMap first.
        SpatialPropertiesMap::const_iterator itr = _spatialPropertiesMap.find(cs);
        if (itr!=_spatialPropertiesMap.end()) return itr->second;
        
        if (_gdalDataSet)
        {

            //std::cout<<"Projecting bounding volume for "<<_source->getFileName()<<std::endl;

            
            // insert into the _spatialPropertiesMap for future reuse.
            _spatialPropertiesMap[cs] = *this;
            DataSet::SpatialProperties& sp = _spatialPropertiesMap[cs];
            
            /* -------------------------------------------------------------------- */
            /*      Create a transformation object from the source to               */
            /*      destination coordinate system.                                  */
            /* -------------------------------------------------------------------- */
            void *hTransformArg = 
                GDALCreateGenImgProjTransformer( _gdalDataSet,_cs->getProjectionRef().c_str(),
                                                 NULL, cs->getProjectionRef().c_str(),
                                                 TRUE, 0.0, 1 );

            if (!hTransformArg)
            {
                std::cout<<" failed to create transformer"<<std::endl;
                return sp;
            }
        
            double adfDstGeoTransform[6];
            int nPixels=0, nLines=0;
            if( GDALSuggestedWarpOutput( _gdalDataSet, 
                                         GDALGenImgProjTransform, hTransformArg, 
                                         adfDstGeoTransform, &nPixels, &nLines )
                != CE_None )
            {
                std::cout<<" failed to create warp"<<std::endl;
                return sp;
            }

            sp._numValuesX = nPixels;
            sp._numValuesY = nLines;
            sp._cs = cs;
            sp._geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],  0.0,    0.0,
                                  adfDstGeoTransform[2],    adfDstGeoTransform[5],  0.0,    0.0,
                                  0.0,                      0.0,                    1.0,    0.0,
                                  adfDstGeoTransform[0],    adfDstGeoTransform[3],  0.0,    1.0);

            GDALDestroyGenImgProjTransformer( hTransformArg );

            sp._extents.expandBy( osg::Vec3(0.0,0.0,0.0)*sp._geoTransform);
            sp._extents.expandBy( osg::Vec3(nPixels,nLines,0.0)*sp._geoTransform);

            return sp;
        }

    }
    std::cout<<"DataSet::DataSource::assuming compatible coordinates."<<std::endl;
    return *this;
}

bool DataSet::SourceData::intersects(const SpatialProperties& sp) const
{
    return sp._extents.intersects(getExtents(sp._cs.get()));
}

void DataSet::SourceData::read(DestinationData& destination)
{
    if (!_source) return;
    
    switch (_source->getType())
    {
    case(Source::IMAGE):
        readImage(destination);
        break;
    case(Source::HEIGHT_FIELD):
        readHeightField(destination);
        break;
    case(Source::MODEL):
        readModels(destination);
        break;
    }
}

void DataSet::SourceData::readImage(DestinationData& destination)
{
    if (destination._image.valid())
    {
        osg::BoundingBox s_bb = getExtents(destination._cs.get());
        
        osg::BoundingBox d_bb = destination._extents;
        
        osg::BoundingBox intersect_bb(s_bb.intersect(d_bb));
        if (!intersect_bb.valid())
        {
            std::cout<<"Reading image but it does not intesection destination - ignoring"<<std::endl;
            return;
        }
        
        
       int windowX = osg::maximum((int)floorf((float)_numValuesX*(intersect_bb.xMin()-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),0);
       int windowY = osg::maximum((int)floorf((float)_numValuesY*(intersect_bb.yMin()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),0);
       int windowWidth = osg::minimum((int)ceilf((float)_numValuesX*(intersect_bb.xMax()-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),(int)_numValuesX)-windowX;
       int windowHeight = osg::minimum((int)ceilf((float)_numValuesY*(intersect_bb.yMax()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),(int)_numValuesY)-windowY;

       int destX = osg::maximum((int)floorf((float)destination._image->s()*(intersect_bb.xMin()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),0);
       int destY = osg::maximum((int)floorf((float)destination._image->t()*(intersect_bb.yMin()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),0);
       int destWidth = osg::minimum((int)ceilf((float)destination._image->s()*(intersect_bb.xMax()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),(int)destination._image->s())-destX;
       int destHeight = osg::minimum((int)ceilf((float)destination._image->t()*(intersect_bb.yMax()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),(int)destination._image->t())-destY;

        std::cout<<"   copying from "<<windowX<<"\t"<<windowY<<"\t"<<windowWidth<<"\t"<<windowHeight<<std::endl;
        std::cout<<"             to "<<destX<<"\t"<<destY<<"\t"<<destWidth<<"\t"<<destHeight<<std::endl;
        
        // which band do we want to read from...        
        int numBands = _gdalDataSet->GetRasterCount();
        GDALRasterBand* bandGray = 0;
        GDALRasterBand* bandRed = 0;
        GDALRasterBand* bandGreen = 0;
        GDALRasterBand* bandBlue = 0;
        GDALRasterBand* bandAlpha = 0;

        for(int b=1;b<=numBands;++b)
        {
            GDALRasterBand* band = _gdalDataSet->GetRasterBand(b);
            if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
            else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
            else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
            else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
            else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
            else bandGray = band;
        }


	GDALRasterBand* bandSelected = 0;
	if (!bandSelected && bandGray) bandSelected = bandGray;
	else if (!bandSelected && bandAlpha) bandSelected = bandAlpha;
	else if (!bandSelected && bandRed) bandSelected = bandRed;
	else if (!bandSelected && bandGreen) bandSelected = bandGreen;
	else if (!bandSelected && bandBlue) bandSelected = bandBlue;
        
        if (bandRed && bandGreen && bandBlue)
        {
            // RGB

            unsigned int numBytesPerPixel = 1;
            GDALDataType targetGDALType = GDT_Byte;

            int pixelSpace=3*numBytesPerPixel;
            int lineSpace=-destination._image->getRowSizeInBytes();

            unsigned char* imageData = destination._image->data(destX,destY+destHeight-1);
            std::cout << "reading RGB"<<std::endl;

            bandRed->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,(void*)(imageData+0),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
            bandGreen->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,(void*)(imageData+1),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);
            bandBlue->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,(void*)(imageData+2),destWidth,destHeight,targetGDALType,pixelSpace,lineSpace);

        }


    }
}

void DataSet::SourceData::readHeightField(DestinationData& destination)
{
    if (destination._heightField.valid())
    {
        std::cout<<"Reading height field"<<std::endl;

        osg::BoundingBox s_bb = getExtents(destination._cs.get());
        
        osg::BoundingBox d_bb = destination._extents;
        
        osg::BoundingBox intersect_bb(s_bb.intersect(d_bb));

        if (!intersect_bb.valid())
        {
            std::cout<<"Reading image but it does not intesection destination - ignoring"<<std::endl;
            return;
        }
        
       // compute dimensions to read from.        
       int windowX = osg::maximum((int)floorf((float)_numValuesX*(intersect_bb.xMin()-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),0);
       int windowY = osg::maximum((int)floorf((float)_numValuesY*(intersect_bb.yMin()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),0);
       int windowWidth = osg::minimum((int)ceilf((float)_numValuesX*(intersect_bb.xMax()-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),(int)_numValuesX)-windowX;
       int windowHeight = osg::minimum((int)ceilf((float)_numValuesY*(intersect_bb.yMax()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),(int)_numValuesY)-windowY;

       int destX = osg::maximum((int)floorf((float)destination._heightField->getNumColumns()*(intersect_bb.xMin()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),0);
       int destY = osg::maximum((int)floorf((float)destination._heightField->getNumRows()*(intersect_bb.yMin()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),0);
       int destWidth = osg::minimum((int)ceilf((float)destination._heightField->getNumColumns()*(intersect_bb.xMax()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),(int)destination._heightField->getNumColumns())-destX;
       int destHeight = osg::minimum((int)ceilf((float)destination._heightField->getNumRows()*(intersect_bb.yMax()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),(int)destination._heightField->getNumRows())-destY;

        std::cout<<"   copying from "<<windowX<<"\t"<<windowY<<"\t"<<windowWidth<<"\t"<<windowHeight<<std::endl;
        std::cout<<"             to "<<destX<<"\t"<<destY<<"\t"<<destWidth<<"\t"<<destHeight<<std::endl;


        // which band do we want to read from...        
        int numBands = _gdalDataSet->GetRasterCount();
        GDALRasterBand* bandGray = 0;
        GDALRasterBand* bandRed = 0;
        GDALRasterBand* bandGreen = 0;
        GDALRasterBand* bandBlue = 0;
        GDALRasterBand* bandAlpha = 0;

        for(int b=1;b<=numBands;++b)
        {
            GDALRasterBand* band = _gdalDataSet->GetRasterBand(b);
            if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
            else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
            else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
            else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
            else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
            else bandGray = band;
        }


	GDALRasterBand* bandSelected = 0;
	if (!bandSelected && bandGray) bandSelected = bandGray;
	else if (!bandSelected && bandAlpha) bandSelected = bandAlpha;
	else if (!bandSelected && bandRed) bandSelected = bandRed;
	else if (!bandSelected && bandGreen) bandSelected = bandGreen;
	else if (!bandSelected && bandBlue) bandSelected = bandBlue;
        
        float heightRatio = 1.0/65536;

        if (bandSelected)
        {
            // raad the data.
            osg::HeightField* hf = destination._heightField.get();
            void* floatdata = (void*)(&(hf->getHeight(destX,destY+destHeight-1))); // start at the top

            unsigned int numBytesPerZvalue = 4;
            int lineSpace=-(hf->getNumColumns()*numBytesPerZvalue); //and work down (note - sign)

            bandSelected->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,floatdata,destWidth,destHeight,GDT_Float32,numBytesPerZvalue,lineSpace);

            bool doFlip = false;
            if (doFlip)
            {
	        // now need to flip since the OSG's origin is in lower left corner.
                std::cout<<"flipping"<<std::endl;
                unsigned int copy_r = hf->getNumRows()-1;
	        for(unsigned int r=0;r<copy_r;++r,--copy_r)
	        {
		    for(unsigned int c=0;c<hf->getNumColumns();++c)
		    {
                        float temp = hf->getHeight(c,r);
                        hf->setHeight(c,r,hf->getHeight(c,copy_r)*heightRatio);
                        hf->setHeight(c,copy_r,temp*heightRatio);
		    }
	        }
            }
            else
            {
	        for(int r=destY;r<destY+destHeight;++r)
	        {
		    for(int c=destX;c<destX+destWidth;++c)
		    {
                        hf->setHeight(c,r,hf->getHeight(c,r)*heightRatio);
		    }
	        }
            }
        }


    }
}

void DataSet::SourceData::readModels(DestinationData& destination)
{
    if (_model.valid())
    {
        std::cout<<"Raading model"<<std::endl;
        destination._models.push_back(_model);
    }
}

void DataSet::Source::setSortValueFromSourceDataResolution()
{
    if (_sourceData.valid())
    {
        double dx = (_sourceData->_extents.xMax()-_sourceData->_extents.xMin())/(double)(_sourceData->_numValuesX-1);
        double dy = (_sourceData->_extents.yMax()-_sourceData->_extents.yMin())/(double)(_sourceData->_numValuesY-1);
        
        setSortValue( sqrt( dx*dx + dy*dy ) );
    }
}

void DataSet::Source::loadSourceData()
{
    std::cout<<"DataSet::Source::loadSourceData() "<<_filename<<std::endl;
    
    _sourceData = SourceData::readData(this);
}

bool DataSet::Source::needReproject(const osgTerrain::CoordinateSystem* cs) const
{
    return needReproject(cs,0.0,0.0);
}

bool DataSet::Source::needReproject(const osgTerrain::CoordinateSystem* cs, double minResolution, double maxResolution) const
{
    if (!_sourceData) return false;
    
    // handle modles by using a matrix transform only.
    if (_type==MODEL) return false;
    
    // always need to reproject imagery with GCP's.
    if (_sourceData->_hasGCPs) return true;

    bool csTheSame = (_sourceData->_cs == cs) ||
                     (_sourceData->_cs.valid() && cs && (*(_sourceData->_cs) != *cs));
                     
                     
    if (!csTheSame) return true;

    if (minResolution==0.0 && maxResolution==0.0) return false;

    // now check resolutions.
    const osg::Matrixd& m = _sourceData->_geoTransform;
    double currentResolution = sqrt(osg::square(m(0,0))+osg::square(m(1,0))+
                                    osg::square(m(0,1))+osg::square(m(1,1)));
                                   
    if (currentResolution<minResolution) return true;
    if (currentResolution>maxResolution) return true;

    return false;
}

DataSet::Source* DataSet::Source::doReproject(const std::string& filename, osgTerrain::CoordinateSystem* cs, double targetResolution) const
{
    // return nothing when repoject is inappropriate.
    if (!_sourceData) return 0;
    if (_type==MODEL) return 0;
    
    std::cout<<"reprojecting to file "<<filename<<std::endl;

    GDALDriverH hDriver = GDALGetDriverByName( osgDB::getFileExtension(filename).c_str() );
    if( hDriver == NULL) 
    {
        std::cout<<"Unable to load driver for "<<filename<<std::endl;
        
        hDriver = GDALGetDriverByName( "GTiff" );
        
        if (hDriver == NULL)
        {       
        std::cout<<"Unable to load driver for "<<"GTiff"<<std::endl;
            return 0;
        }
    }
    
    if (GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
    {
        std::cout<<"GDAL driver does not support create for "<<osgDB::getFileExtension(filename)<<std::endl;
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
    void *hTransformArg = 
         GDALCreateGenImgProjTransformer( _sourceData->_gdalDataSet,_sourceData->_cs->getProjectionRef().c_str(),
                                          NULL, cs->getProjectionRef().c_str(),
                                          TRUE, 0.0, 1 );

    if (!hTransformArg)
    {
        std::cout<<" failed to create transformer"<<std::endl;
        return 0;
    }

    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    if( GDALSuggestedWarpOutput( _sourceData->_gdalDataSet, 
                                 GDALGenImgProjTransform, hTransformArg, 
                                 adfDstGeoTransform, &nPixels, &nLines )
        != CE_None )
    {
        std::cout<<" failed to create warp"<<std::endl;
        return 0;
    }
    
    if (targetResolution>0.0f)
    {
        std::cout<<"recomputing the target transform size"<<std::endl;
        
        double currentResolution = sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2])+
                                        osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5]));

        std::cout<<"        default computed resolution "<<currentResolution<<" nPixels="<<nPixels<<" nLines="<<nLines<<std::endl;

        double extentsPixels = sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2]))*(double)(nPixels-1);
        double extentsLines = sqrt(osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5]))*(double)(nLines-1);
                                        
        double ratio = targetResolution/currentResolution;
        adfDstGeoTransform[1] *= ratio;
        adfDstGeoTransform[2] *= ratio;
        adfDstGeoTransform[4] *= ratio;
        adfDstGeoTransform[5] *= ratio;
        
        std::cout<<"    extentsPixels="<<extentsPixels<<std::endl;
        std::cout<<"    extentsLines="<<extentsLines<<std::endl;
        std::cout<<"    targetResolution="<<targetResolution<<std::endl;
        
        nPixels = (int)ceil(extentsPixels/sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2])))+1;
        nLines = (int)ceil(extentsLines/sqrt(osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5])))+1;

        std::cout<<"        target computed resolution "<<targetResolution<<" nPixels="<<nPixels<<" nLines="<<nLines<<std::endl;
        
    }

    
    GDALDestroyGenImgProjTransformer( hTransformArg );

    GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(_sourceData->_gdalDataSet,1));
    

/* --------------------------------------------------------------------- */
/*    Create the file                                                    */
/* --------------------------------------------------------------------- */

    GDALDatasetH hDstDS = GDALCreate( hDriver, filename.c_str(), nPixels, nLines, 
                         GDALGetRasterCount(_sourceData->_gdalDataSet), eDT,
                         0 );
    
    if( hDstDS == NULL )
        return NULL;

/* -------------------------------------------------------------------- */
/*      Write out the projection definition.                            */
/* -------------------------------------------------------------------- */
    GDALSetProjection( hDstDS, cs->getProjectionRef().c_str() );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );


// Set up the transformer along with the new datasets.

    hTransformArg = 
         GDALCreateGenImgProjTransformer( _sourceData->_gdalDataSet,_sourceData->_cs->getProjectionRef().c_str(),
                                          hDstDS, cs->getProjectionRef().c_str(),
                                          TRUE, 0.0, 1 );

    GDALTransformerFunc pfnTransformer = GDALGenImgProjTransform;

    
    std::cout<<"Setting projection "<<cs->getProjectionRef()<<std::endl;

/* -------------------------------------------------------------------- */
/*      Copy the color table, if required.                              */
/* -------------------------------------------------------------------- */
    GDALColorTableH hCT;

    hCT = GDALGetRasterColorTable( GDALGetRasterBand(_sourceData->_gdalDataSet,1) );
    if( hCT != NULL )
        GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );

/* -------------------------------------------------------------------- */
/*      Setup warp options.                                             */
/* -------------------------------------------------------------------- */
    GDALWarpOptions *psWO = GDALCreateWarpOptions();

    psWO->hSrcDS = _sourceData->_gdalDataSet;
    psWO->hDstDS = hDstDS;

    psWO->pfnTransformer = pfnTransformer;
    psWO->pTransformerArg = hTransformArg;

    psWO->pfnProgress = GDALTermProgress;

/* -------------------------------------------------------------------- */
/*      Setup band mapping.                                             */
/* -------------------------------------------------------------------- */
    psWO->nBandCount = GDALGetRasterCount(_sourceData->_gdalDataSet);
    psWO->panSrcBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));
    psWO->panDstBands = (int *) CPLMalloc(psWO->nBandCount*sizeof(int));

    for(int i = 0; i < psWO->nBandCount; i++ )
    {
        psWO->panSrcBands[i] = i+1;
        psWO->panDstBands[i] = i+1;
    }

/* -------------------------------------------------------------------- */
/*      Initialize and execute the warp.                                */
/* -------------------------------------------------------------------- */
    GDALWarpOperation oWO;

    if( oWO.Initialize( psWO ) == CE_None )
    {
        oWO.ChunkAndWarpImage( 0, 0, 
                               GDALGetRasterXSize( hDstDS ),
                               GDALGetRasterYSize( hDstDS ) );
    }

    std::cout<<"new projection is "<<GDALGetProjectionRef(hDstDS)<<std::endl;

/* -------------------------------------------------------------------- */
/*      Cleanup.                                                        */
/* -------------------------------------------------------------------- */
    GDALDestroyGenImgProjTransformer( hTransformArg );
    

    int anOverviewList[4] = { 2, 4, 8, 16 };
    GDALBuildOverviews( hDstDS, "AVERAGE", 4, anOverviewList, 0, NULL, 
                            GDALTermProgress/*GDALDummyProgress*/, NULL );

    GDALClose( hDstDS );
    
    Source* newSource = new Source;
    newSource->_filename = filename;
    newSource->_temporaryFile = true;
    newSource->_cs = cs;
    newSource->_geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],    0.0,    0.0,
                                  adfDstGeoTransform[2],    adfDstGeoTransform[5],    0.0,    0.0,
                                  0.0,                0.0,                1.0,    0.0,
                                  adfDstGeoTransform[0],    adfDstGeoTransform[3],    0.0,    1.0);

    newSource->_extents.init();
    newSource->_extents.expandBy( osg::Vec3(0.0,0.0,0.0)*newSource->_geoTransform);
    newSource->_extents.expandBy( osg::Vec3(nPixels,nLines,0.0)*newSource->_geoTransform);

    // reload the newly created file.
    newSource->loadSourceData();

    SourceData* newData = newSource->_sourceData.get();
    
    // override the values in the new source data.
    newData->_cs = cs;
    newData->_extents = newSource->_extents;
    newData->_geoTransform = newSource->_geoTransform;

                              
    return newSource;
}

void DataSet::Source::consolodateRequiredResolutions()
{
    if (_requiredResolutions.size()<=1) return;

    ResolutionList consolodated;
    
    ResolutionList::iterator itr = _requiredResolutions.begin();
    
    double minResX = itr->_resX;
    double minResY = itr->_resY;
    double maxResX = itr->_resX;
    double maxResY = itr->_resY;
    ++itr;
    for(;itr!=_requiredResolutions.end();++itr)
    {
        minResX = osg::minimum(minResX,itr->_resX);
        minResY = osg::minimum(minResY,itr->_resY);
        maxResX = osg::maximum(maxResX,itr->_resX);
        maxResY = osg::maximum(maxResY,itr->_resY);
    }
    
    double currResX = minResX;
    double currResY = minResY;
    while (currResX<=maxResX && currResY<=maxResY)
    {
        consolodated.push_back(ResolutionPair(currResX,currResY));
        currResX *= 2.0f;
        currResY *= 2.0f;
    }
    

    _requiredResolutions.swap(consolodated);
}

void DataSet::Source::buildOverviews()
{
    return;

    if (_sourceData.valid() && _sourceData->_gdalDataSet )
    {

        int anOverviewList[4] = { 2, 4, 8, 16 };
        GDALBuildOverviews( _sourceData->_gdalDataSet, "AVERAGE", 4, anOverviewList, 0, NULL, 
                                GDALTermProgress/*GDALDummyProgress*/, NULL );

    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


DataSet::DestinationTile::DestinationTile():
    _imagery_maxNumColumns(4096),
    _imagery_maxNumRows(4096),
    _imagery_maxSourceResolutionX(0.0f),
    _imagery_maxSourceResolutionY(0.0f),
    _terrain_maxNumColumns(1024),
    _terrain_maxNumRows(1024),
    _terrain_maxSourceResolutionX(0.0f),
    _terrain_maxSourceResolutionY(0.0f)
{
    for(int i=0;i<NUMBER_OF_POSITIONS;++i)
    {
        _neighbour[i]=0;
        _equalized[i]=false;
    }
}


void DataSet::DestinationTile::computeMaximumSourceResolution(CompositeSource* sourceGraph)
{
    for(CompositeSource::source_iterator itr(sourceGraph);itr.valid();++itr)
    {

        SourceData* data = (*itr)->getSourceData();
        if (data && (*itr)->getType()!=Source::MODEL)
        {

            SpatialProperties sp = data->computeSpatialProperties(_cs.get());

            if (!sp._extents.intersects(_extents))
            {
//                std::cout<<"DataSet::DestinationTile::computeMaximumSourceResolution:: source does not overlap ignoring for this tile."<<std::endl;
                continue;
            }

            
            if (sp._numValuesX!=0 && sp._numValuesY!=0)
            {
                float sourceResolutionX = (sp._extents.xMax()-sp._extents.xMin())/(float)sp._numValuesX;
                float sourceResolutionY = (sp._extents.yMax()-sp._extents.yMin())/(float)sp._numValuesY;

                switch((*itr)->getType())
                {
                case(Source::IMAGE):
                    if (_imagery_maxSourceResolutionX==0.0f) _imagery_maxSourceResolutionX=sourceResolutionX;
                    else _imagery_maxSourceResolutionX=osg::minimum(_imagery_maxSourceResolutionX,sourceResolutionX);
                    if (_imagery_maxSourceResolutionY==0.0f) _imagery_maxSourceResolutionY=sourceResolutionY;
                    else _imagery_maxSourceResolutionY=osg::minimum(_imagery_maxSourceResolutionY,sourceResolutionY);
                    break;
                case(Source::HEIGHT_FIELD):
                    if (_terrain_maxSourceResolutionX==0.0f) _terrain_maxSourceResolutionX=sourceResolutionX;
                    else _terrain_maxSourceResolutionX=osg::minimum(_terrain_maxSourceResolutionX,sourceResolutionX);
                    if (_terrain_maxSourceResolutionY==0.0f) _terrain_maxSourceResolutionY=sourceResolutionY;
                    else _terrain_maxSourceResolutionY=osg::minimum(_terrain_maxSourceResolutionY,sourceResolutionY);
                    break;
                default:
                    break;
                }
            }
        }
    }
}

bool DataSet::DestinationTile::computeImageResolution(unsigned int& numColumns, unsigned int& numRows, double& resX, double& resY)
{
    if (_imagery_maxSourceResolutionX!=0.0f && _imagery_maxSourceResolutionY!=0.0f &&
        _imagery_maxNumColumns!=0 && _imagery_maxNumRows!=0)
    {

        unsigned int numColumnsAtFullRes = 1+(unsigned int)ceilf((_extents.xMax()-_extents.xMin())/_imagery_maxSourceResolutionX);
        unsigned int numRowsAtFullRes = 1+(unsigned int)ceilf((_extents.yMax()-_extents.yMin())/_imagery_maxSourceResolutionY);
        numColumns = osg::minimum(_imagery_maxNumColumns,numColumnsAtFullRes);
        numRows    = osg::minimum(_imagery_maxNumRows,numRowsAtFullRes);
        
        resX = (_extents.xMax()-_extents.xMin())/(double)(numColumns-1);
        resY = (_extents.yMax()-_extents.yMin())/(double)(numRows-1);
        
        return true;
    }
    return false;
}

bool DataSet::DestinationTile::computeTerrainResolution(unsigned int& numColumns, unsigned int& numRows, double& resX, double& resY)
{
    if (_terrain_maxSourceResolutionX!=0.0f && _terrain_maxSourceResolutionY!=0.0f &&
        _terrain_maxNumColumns!=0 && _terrain_maxNumRows!=0)
    {
        unsigned int numColumnsAtFullRes = 1+(unsigned int)ceilf((_extents.xMax()-_extents.xMin())/_terrain_maxSourceResolutionX);
        unsigned int numRowsAtFullRes = 1+(unsigned int)ceilf((_extents.yMax()-_extents.yMin())/_terrain_maxSourceResolutionY);
        numColumns = osg::minimum(_terrain_maxNumColumns,numColumnsAtFullRes);
        numRows    = osg::minimum(_terrain_maxNumRows,numRowsAtFullRes);

        resX = (_extents.xMax()-_extents.xMin())/(double)(numColumns-1);
        resY = (_extents.yMax()-_extents.yMin())/(double)(numRows-1);
        
        return true;
    }
    return false;
}


void DataSet::DestinationTile::allocate()
{
    unsigned int texture_numColumns, texture_numRows;
    double texture_dx, texture_dy;
    if (computeImageResolution(texture_numColumns,texture_numRows,texture_dx,texture_dy))
    {

        _imagery = new DestinationData;
        _imagery->_cs = _cs;
        _imagery->_extents = _extents;
        _imagery->_geoTransform.set(texture_dx,      0.0,               0.0,0.0,
                                    0.0,             -texture_dy,       0.0,0.0,
                                    0.0,             0.0,               1.0,1.0,
                                    _extents.xMin(), _extents.yMax(),   0.0,1.0);
        _imagery->_image = new osg::Image;
        _imagery->_image->allocateImage(texture_numColumns,texture_numRows,1,GL_RGB,GL_UNSIGNED_BYTE);
    }


    unsigned int dem_numColumns, dem_numRows;
    double dem_dx, dem_dy;
    if (computeTerrainResolution(dem_numColumns,dem_numRows,dem_dx,dem_dy))
    {
        _terrain = new DestinationData;
        _terrain->_cs = _cs;
        _terrain->_extents = _extents;
        _terrain->_geoTransform.set(dem_dx,          0.0,               0.0,0.0,
                                    0.0,             -dem_dy,           0.0,0.0,
                                    0.0,             0.0,               1.0,1.0,
                                    _extents.xMin(), _extents.yMax(),   0.0,1.0);
        _terrain->_heightField = new osg::HeightField;
        _terrain->_heightField->allocate(dem_numColumns,dem_numRows);
        _terrain->_heightField->setOrigin(osg::Vec3(_extents.xMin(),_extents.yMin(),0.0f));
        _terrain->_heightField->setXInterval(dem_dx);
        _terrain->_heightField->setYInterval(dem_dy);

        //float xMax = _terrain->_heightField->getOrigin().x()+_terrain->_heightField->getXInterval()*(float)(dem_numColumns-1);
        //std::cout<<"ErrorX = "<<xMax-_extents.xMax()<<std::endl;

        //float yMax = _terrain->_heightField->getOrigin().y()+_terrain->_heightField->getYInterval()*(float)(dem_numRows-1);
        //std::cout<<"ErrorY = "<<yMax-_extents.yMax()<<std::endl;

    }

}

void DataSet::DestinationTile::setNeighbours(DestinationTile* left, DestinationTile* left_below, 
                                             DestinationTile* below, DestinationTile* below_right,
                                             DestinationTile* right, DestinationTile* right_above,
                                             DestinationTile* above, DestinationTile* above_left)
{
    _neighbour[LEFT] = left;
    _neighbour[LEFT_BELOW] = left_below;
    _neighbour[BELOW] = below;
    _neighbour[BELOW_RIGHT] = below_right;
    _neighbour[RIGHT] = right;
    _neighbour[RIGHT_ABOVE] = right_above;
    _neighbour[ABOVE] = above;
    _neighbour[ABOVE_LEFT] = above_left;
    
    for(int i=0;i<NUMBER_OF_POSITIONS;++i)
    {
        _equalized[i]=false;
    }
}

void DataSet::DestinationTile::checkNeighbouringTiles()
{
    for(int i=0;i<NUMBER_OF_POSITIONS;++i)
    {
        if (_neighbour[i] && _neighbour[i]->_neighbour[(i+4)%NUMBER_OF_POSITIONS]!=this)
        {
            std::cout<<"Error:: Tile "<<this<<"'s _neighbour["<<i<<"] does not point back to it."<<std::endl;
        }
    }
}

void DataSet::DestinationTile::equalizeCorner(Position position)
{
    // don't need to equalize if already done.
    if (_equalized[position]) return;

    typedef std::pair<DestinationTile*,Position> TileCornerPair;
    typedef std::vector<TileCornerPair> TileCornerList;

    TileCornerList cornersToProcess;
    DestinationTile* tile=0;

    cornersToProcess.push_back(TileCornerPair(this,position));
    
    tile = _neighbour[(position-1)%NUMBER_OF_POSITIONS];
    if (tile) cornersToProcess.push_back(TileCornerPair(tile,(Position)((position+2)%NUMBER_OF_POSITIONS)));
    
    tile = _neighbour[(position)%NUMBER_OF_POSITIONS];
    if (tile) cornersToProcess.push_back(TileCornerPair(tile,(Position)((position+4)%NUMBER_OF_POSITIONS)));

    tile = _neighbour[(position+1)%NUMBER_OF_POSITIONS];
    if (tile) cornersToProcess.push_back(TileCornerPair(tile,(Position)((position+6)%NUMBER_OF_POSITIONS)));

    // make all these tiles as equalised upfront before we return.
    TileCornerList::iterator itr;
    for(itr=cornersToProcess.begin();
        itr!=cornersToProcess.end();
        ++itr)
    {
        TileCornerPair& tcp = *itr;
        tcp.first->_equalized[tcp.second] = true;
    }

    // if there is only one valid corner to process then there is nothing to equalize against so return.
    if (cornersToProcess.size()==1) return;
    
    typedef std::pair<osg::Image*,Position> ImageCornerPair;
    typedef std::vector<ImageCornerPair> ImageCornerList;
    
    typedef std::pair<osg::HeightField*,Position> HeightFieldCornerPair;
    typedef std::vector<HeightFieldCornerPair> HeightFieldCornerList;

    ImageCornerList imagesToProcess;
    HeightFieldCornerList heightFieldsToProcess;
    
    for(itr=cornersToProcess.begin();
        itr!=cornersToProcess.end();
        ++itr)
    {
        TileCornerPair& tcp = *itr;
        if (tcp.first->_imagery.valid() && tcp.first->_imagery->_image.valid())
        {
            imagesToProcess.push_back(ImageCornerPair(tcp.first->_imagery->_image.get(),tcp.second));
        }
        
        if (tcp.first->_terrain.valid() && tcp.first->_terrain->_heightField.valid())
        {
            heightFieldsToProcess.push_back(HeightFieldCornerPair(tcp.first->_terrain->_heightField.get(),tcp.second));
        }
    }

    if (imagesToProcess.size()>1)
    {
        int red = 0;
        int green = 0;
        int blue = 0;
        // accumulate colours.
        ImageCornerList::iterator iitr;
        for(iitr=imagesToProcess.begin();
            iitr!=imagesToProcess.end();
            ++iitr)
        {
            ImageCornerPair& icp = *iitr;
            unsigned char* data=0;
            switch(icp.second)
            {
            case LEFT_BELOW:
                data = icp.first->data(0,0);
                break;
            case BELOW_RIGHT:
                data = icp.first->data(icp.first->s()-1,0);
                break;
            case RIGHT_ABOVE:
                data = icp.first->data(icp.first->s()-1,icp.first->t()-1);
                break;
            case ABOVE_LEFT:
                data = icp.first->data(0,icp.first->t()-1);
                break;
            default :
                break;
            }
            red += *(data++);
            green += *(data++);
            blue += *(data);
        }

        // divide them.
        red /= imagesToProcess.size();
        green /= imagesToProcess.size();
        blue /= imagesToProcess.size();
        // apply colour to corners.
        for(iitr=imagesToProcess.begin();
            iitr!=imagesToProcess.end();
            ++iitr)
        {
            ImageCornerPair& icp = *iitr;
            unsigned char* data=0;
            switch(icp.second)
            {
            case LEFT_BELOW:
                data = icp.first->data(0,0);
                break;
            case BELOW_RIGHT:
                data = icp.first->data(icp.first->s()-1,0);
                break;
            case RIGHT_ABOVE:
                data = icp.first->data(icp.first->s()-1,icp.first->t()-1);
                break;
            case ABOVE_LEFT:
                data = icp.first->data(0,icp.first->t()-1);
                break;
            default :
                break;
            }
            *(data++) = red;
            *(data++) = green;
            *(data) = blue;
        }
        
    }
    
    if (heightFieldsToProcess.size()>1)
    {
        float height = 0;
        // accumulate heights.
        HeightFieldCornerList::iterator hitr;
        for(hitr=heightFieldsToProcess.begin();
            hitr!=heightFieldsToProcess.end();
            ++hitr)
        {
            HeightFieldCornerPair& hfcp = *hitr;
            switch(hfcp.second)
            {
            case LEFT_BELOW:
                height += hfcp.first->getHeight(0,0);
                break;
            case BELOW_RIGHT:
                height += hfcp.first->getHeight(hfcp.first->getNumColumns()-1,0);
                break;
            case RIGHT_ABOVE:
                height += hfcp.first->getHeight(hfcp.first->getNumColumns()-1,hfcp.first->getNumRows()-1);
                break;
            case ABOVE_LEFT:
                height += hfcp.first->getHeight(0,hfcp.first->getNumRows()-1);
                break;
            default :
                break;
            }
        }
        
        // divide them.
        height /= heightFieldsToProcess.size();

        // apply height to corners.
        for(hitr=heightFieldsToProcess.begin();
            hitr!=heightFieldsToProcess.end();
            ++hitr)
        {
            HeightFieldCornerPair& hfcp = *hitr;
            switch(hfcp.second)
            {
            case LEFT_BELOW:
                hfcp.first->setHeight(0,0,height);
                break;
            case BELOW_RIGHT:
                hfcp.first->setHeight(hfcp.first->getNumColumns()-1,0,height);
                break;
            case RIGHT_ABOVE:
                hfcp.first->setHeight(hfcp.first->getNumColumns()-1,hfcp.first->getNumRows()-1,height);
                break;
            case ABOVE_LEFT:
                hfcp.first->setHeight(0,hfcp.first->getNumRows()-1,height);
                break;
            default :
                break;
            }
        }
    }

}

void DataSet::DestinationTile::equalizeEdge(Position position)
{
    // don't need to equalize if already done.
    if (_equalized[position]) return;

    DestinationTile* tile2 = _neighbour[position];
    Position position2 = (Position)((position+4)%NUMBER_OF_POSITIONS);

    _equalized[position] = true;
    
    // no neighbour of this edge so nothing to equalize.
    if (!tile2) return;
    
    tile2->_equalized[position2]=true;
    
    osg::Image* image1 = _imagery.valid()?_imagery->_image.get():0;
    osg::Image* image2 = tile2->_imagery.valid()?tile2->_imagery->_image.get():0;

    if (image1 && image2 && 
        image1->getPixelFormat()==image2->getPixelFormat() &&
        image1->getDataType()==image2->getDataType() &&
        image1->getPixelFormat()==GL_RGB &&
        image1->getDataType()==GL_UNSIGNED_BYTE)
    {
        unsigned char* data1 = 0;
        unsigned char* data2 = 0;
        unsigned int delta1 = 0;
        unsigned int delta2 = 0;
        int num = 0;
        
        switch(position)
        {
        case LEFT:
            data1 = image1->data(0,1); // LEFT hand side
            delta1 = image1->getRowSizeInBytes();
            data2 = image2->data(image2->s()-1,1); // RIGHT hand side
            delta2 = image2->getRowSizeInBytes();
            num = (image1->t()==image2->t())?image2->t()-2:0; // note miss out corners.
            break;
        case BELOW:
            data1 = image1->data(1,0); // BELOW hand side
            delta1 = 3;
            data2 = image2->data(1,image2->t()-1); // ABOVE hand side
            delta2 = 3;
            num = (image1->s()==image2->s())?image2->s()-2:0; // note miss out corners.
            break;
        case RIGHT:
            data1 = image1->data(image2->s()-1,1); // LEFT hand side
            delta1 = image1->getRowSizeInBytes();
            data2 = image2->data(0,1); // RIGHT hand side
            delta2 = image2->getRowSizeInBytes();
            num = (image1->t()==image2->t())?image2->t()-2:0; // note miss out corners.
            break;
        case ABOVE:
            data1 = image1->data(1,image2->t()-1); // ABOVE hand side
            delta1 = 3;
            data2 = image2->data(1,0); // BELOW hand side
            delta2 = 3;
            num = (image1->s()==image2->s())?image2->s()-2:0; // note miss out corners.
            break;
        default :
            break;
        }

        for(int i=0;i<num;++i)
        {
            unsigned char red =   (unsigned char)((((int)*data1+ (int)*data2)/2));
            unsigned char green = (unsigned char)((((int)*(data1+1))+ (int)(*(data2+1)))/2);
            unsigned char blue =  (unsigned char)((((int)*(data1+2))+ (int)(*(data2+2)))/2);

            *data1 = red;
            *(data1+1) = green;
            *(data1+2) = blue;

            *data2 = red;
            *(data2+1) = green;
            *(data2+2) = blue;

            data1 += delta1;
            data2 += delta2;
        }

    }

    osg::HeightField* heightField1 = _terrain.valid()?_terrain->_heightField.get():0;
    osg::HeightField* heightField2 = tile2->_terrain.valid()?tile2->_terrain->_heightField.get():0;

    if (heightField1 && heightField2)
    {
        float* data1 = 0;
        float* data2 = 0;
        unsigned int delta1 = 0;
        unsigned int delta2 = 0;
        int num = 0;
        
        switch(position)
        {
        case LEFT:
            data1 = &(heightField1->getHeight(0,1)); // LEFT hand side
            delta1 = heightField1->getNumColumns();
            data2 = &(heightField2->getHeight(heightField2->getNumColumns()-1,1)); // LEFT hand side
            delta2 = heightField2->getNumColumns();
            num = (heightField1->getNumRows()==heightField2->getNumRows())?heightField1->getNumRows()-2:0; // note miss out corners.
            break;
        case BELOW:
            data1 = &(heightField1->getHeight(1,0)); // BELOW hand side
            delta1 = 1;
            data2 = &(heightField2->getHeight(1,heightField2->getNumRows()-1)); // ABOVE hand side
            delta2 = 1;
            num = (heightField1->getNumColumns()==heightField2->getNumColumns())?heightField1->getNumColumns()-2:0; // note miss out corners.
            break;
        case RIGHT:
            data1 = &(heightField1->getHeight(heightField1->getNumColumns()-1,1)); // LEFT hand side
            delta1 = heightField1->getNumColumns();
            data2 = &(heightField2->getHeight(0,1)); // LEFT hand side
            delta2 = heightField2->getNumColumns();
            num = (heightField1->getNumRows()==heightField2->getNumRows())?heightField1->getNumRows()-2:0; // note miss out corners.
            break;
        case ABOVE:
            data1 = &(heightField1->getHeight(1,heightField1->getNumRows()-1)); // ABOVE hand side
            delta1 = 1;
            data2 = &(heightField2->getHeight(1,0)); // BELOW hand side
            delta2 = 1;
            num = (heightField1->getNumColumns()==heightField2->getNumColumns())?heightField1->getNumColumns()-2:0; // note miss out corners.
            break;
        default :
            break;
        }

        for(int i=0;i<num;++i)
        {
            float z = (*data1 + *data2)/2.0f;

            *data1 = z;
            *data2 = z;

            data1 += delta1;
            data2 += delta2;
        }

    }

}

void DataSet::DestinationTile::equalizeBoundaries()
{
    std::cout<<"DataSet::DestinationTile::equalizeBoundaries()"<<std::endl;

    equalizeCorner(LEFT_BELOW);
    equalizeCorner(BELOW_RIGHT);
    equalizeCorner(RIGHT_ABOVE);
    equalizeCorner(ABOVE_LEFT);

    equalizeEdge(LEFT);
    equalizeEdge(BELOW);
    equalizeEdge(RIGHT);
    equalizeEdge(ABOVE);
}

osg::Node* DataSet::DestinationTile::createScene()
{
    if (_terrain.valid() && _terrain->_heightField.valid())
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(_terrain->_heightField.get()));
        
        if (_imagery.valid() && _imagery->_image.valid())
        {
            std::string imageName(_name+".rgb");
            std::cout<<"Writing out imagery to "<<imageName<<std::endl;
            _imagery->_image->setFileName(imageName.c_str());
            osgDB::writeImageFile(*_imagery->_image,_imagery->_image->getFileName().c_str());

            osg::StateSet* stateset = geode->getOrCreateStateSet();
            osg::Texture2D* texture = new osg::Texture2D(_imagery->_image.get());
            texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP);
            texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP);
            stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        }
        
        return geode;
    }
    else 
    {
        std::cout<<"**** No terrain to build tile from, will need to create some fallback ****"<<std::endl;
        return 0;
    }
}

void DataSet::DestinationTile::readFrom(CompositeSource* sourceGraph)
{
    std::cout<<"DestinationTile::readFrom() "<<std::endl;
    for(CompositeSource::source_iterator itr(sourceGraph);itr.valid();++itr)
    {
        SourceData* data = (*itr)->getSourceData();
        if (data)
        {
            std::cout<<"SourceData::read() "<<std::endl;
            if (_imagery.valid()) data->read(*_imagery);
            if (_terrain.valid()) data->read(*_terrain);
        }
    }
}

void DataSet::DestinationTile::addRequiredResolutions(CompositeSource* sourceGraph)
{
    for(CompositeSource::source_iterator itr(sourceGraph);itr.valid();++itr)
    {
        Source* source = itr->get();

        if (source->intersects(*this))
        {
            if (source->getType()==Source::IMAGE)
            {
                unsigned int numCols,numRows;
                double resX, resY;
                if (computeImageResolution(numCols,numRows,resX,resY))
                {
                    source->addRequiredResolution(resX,resY);
                }
            }

            if (source->getType()==Source::HEIGHT_FIELD)
            {
                unsigned int numCols,numRows;
                double resX, resY;
                if (computeTerrainResolution(numCols,numRows,resX,resY))
                {
                    source->addRequiredResolution(resX,resY);
                }
            }
        }

    }
}


void DataSet::CompositeDestination::addRequiredResolutions(CompositeSource* sourceGraph)
{
    // handle leaves
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        (*titr)->addRequiredResolutions(sourceGraph);
    }
    
    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        (*citr)->addRequiredResolutions(sourceGraph);
    }
}

void DataSet::CompositeDestination::readFrom(CompositeSource* sourceGraph)
{
    std::cout<<"CompositeDestination::readFrom() "<<std::endl;

    // handle leaves
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        (*titr)->readFrom(sourceGraph);
    }
    
    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        (*citr)->readFrom(sourceGraph);
    }
}

void DataSet::CompositeDestination::equalizeBoundaries()
{   
    // handle leaves
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        (*titr)->equalizeBoundaries();
    }
    
    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        (*citr)->equalizeBoundaries();
    }

}


osg::Node* DataSet::CompositeDestination::createScene()
{
    if (_children.empty() && _tiles.empty()) return 0;
    
    if (_children.empty() && _tiles.size()==1) return _tiles.front()->createScene();
    
    if (_tiles.empty() && _children.size()==1) return _children.front()->createScene();
    

    if (_type==GROUP)
    {
        osg::Group* group = new osg::Group;
        for(TileList::iterator titr=_tiles.begin();
            titr!=_tiles.end();
            ++titr)
        {
            osg::Node* node = (*titr)->createScene();
            if (node) group->addChild(node);
        }

        // handle chilren
        for(ChildList::iterator citr=_children.begin();
            citr!=_children.end();
            ++citr)
        {
            osg::Node* node = (*citr)->createScene();
            if (node) group->addChild(node);
        }
        return group;            
    }

    // must be either a LOD or a PagedLOD

    typedef std::vector<osg::Node*>  NodeList;
    typedef std::map<float,NodeList> RangeNodeListMap;
    RangeNodeListMap rangeNodeListMap;

    // insert local tiles into range map
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        osg::Node* node = (*titr)->createScene();
        if (node) rangeNodeListMap[_maxVisibleDistance].push_back(node);
    }

    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        osg::Node* node = (*citr)->createScene();
        if (node) rangeNodeListMap[(*citr)->_maxVisibleDistance].push_back(node);
    }

    osg::PagedLOD* pagedLOD = (_type==PAGED_LOD) ? new osg::PagedLOD : 0;
    osg::LOD* lod = (pagedLOD) ? pagedLOD : new osg::LOD;
    
    unsigned int childNum = 0;
    for(RangeNodeListMap::reverse_iterator rnitr=rangeNodeListMap.rbegin();
        rnitr!=rangeNodeListMap.rend();
        ++rnitr)
    {
        float maxVisibleDistance = rnitr->first;
        NodeList& nodeList = rnitr->second;
        
        if (childNum==0)
        {
            // by deafult make the first child have a very visible distance so its always seen
            maxVisibleDistance = 1e8;
        }
        else
        {
            // set the previous child's minimum visible distance range
           lod->setRange(childNum-1,maxVisibleDistance,lod->getMaxRange(childNum-1));
        }
        
        osg::Node* child = 0;
        if (nodeList.size()==1)
        {
            child = nodeList.front();
        }
        else if (nodeList.size()>1)
        {
            osg::Group* group = new osg::Group;
            for(NodeList::iterator itr=nodeList.begin();
                itr!=nodeList.end();
                ++itr)
            {
                group->addChild(*itr);
            }
            child = group;
        }
    
        if (child)
        {
            
            if (pagedLOD)
            {
                std::cout<<"Need to set name of PagedLOD child"<<std::endl;
                pagedLOD->addChild(child,0,maxVisibleDistance);
            }
            else
            {
                lod->addChild(child,0,maxVisibleDistance);
            }
            
            ++childNum;
        }
    }
    return lod;
}

///////////////////////////////////////////////////////////////////////////////

DataSet::DataSet()
{
    init();
    
    _defaultColour.set(0.5f,0.5f,1.0f,1.0f);
}

void DataSet::init()
{
    static bool s_initialized = false;
    if (!s_initialized)
    {
        s_initialized = true;
        GDALAllRegister();
    }
}

void DataSet::addSource(Source* source)
{
    if (!source) return;
    
    if (!_sourceGraph.valid()) _sourceGraph = new CompositeSource;
    
    _sourceGraph->_sourceList.push_back(source);
}

void DataSet::addSource(CompositeSource* composite)
{
    if (!composite) return;

    if (!_sourceGraph.valid()) _sourceGraph = new CompositeSource;
    
    _sourceGraph->_children.push_back(composite);
}

void DataSet::loadSources()
{
    for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
    {
        (*itr)->loadSourceData();
    }
}

DataSet::CompositeDestination* DataSet::createDestinationGraph(osgTerrain::CoordinateSystem* cs,
                                                           const osg::BoundingBox& extents,
                                                           unsigned int maxImageSize,
                                                           unsigned int maxTerrainSize,
                                                           unsigned int currentLevel,
                                                           unsigned int currentX,
                                                           unsigned int currentY,
                                                           unsigned int maxNumLevels)
{

    DataSet::CompositeDestination* destinationGraph = new DataSet::CompositeDestination(cs,extents);

    destinationGraph->_maxVisibleDistance = extents.radius()*5.0f;

    // first create the topmost tile

    // create the name
    std::ostringstream os;
    os << _tileBasename << "_L"<<currentLevel<<"_X"<<currentX<<"_Y"<<currentY;

    DestinationTile* tile = new DestinationTile;
    tile->_name = os.str();
    tile->_cs = cs;
    tile->_extents = extents;
    tile->_level = currentLevel;
    tile->_X = currentX;
    tile->_Y = currentY;
    tile->setMaximumImagerySize(maxImageSize,maxImageSize);
    tile->setMaximumTerrainSize(maxTerrainSize,maxTerrainSize);
    tile->computeMaximumSourceResolution(_sourceGraph.get());
    tile->allocate();

    if (currentLevel>=maxNumLevels-1)
    {    
        // bottom level can't divide any further.
        destinationGraph->_tiles.push_back(tile);
    }
    else
    {
        destinationGraph->_type = LOD;
        destinationGraph->_tiles.push_back(tile);

        bool needToDivideX = false;
        bool needToDivideY = false;

        unsigned int texture_numColumns;
        unsigned int texture_numRows;
        double texture_dx;
        double texture_dy;
        if (tile->computeImageResolution(texture_numColumns,texture_numRows,texture_dx,texture_dy))
        {
            if (texture_dx>tile->_imagery_maxSourceResolutionX) needToDivideX = true;
            if (texture_dy>tile->_imagery_maxSourceResolutionY) needToDivideY = true;
        }

        unsigned int dem_numColumns;
        unsigned int dem_numRows;
        double dem_dx;
        double dem_dy;
        if (tile->computeTerrainResolution(dem_numColumns,dem_numRows,dem_dx,dem_dy))
        {
            if (dem_dx>tile->_terrain_maxSourceResolutionX) needToDivideX = true;
            if (dem_dy>tile->_terrain_maxSourceResolutionY) needToDivideY = true;
        }
        
        float xCenter = (extents.xMin()+extents.xMax())*0.5f;
        float yCenter = (extents.yMin()+extents.yMax())*0.5f;
        
        unsigned int newLevel = currentLevel+1;
        unsigned int newX = currentX*2;
        unsigned int newY = currentY*2;

        if (needToDivideX && needToDivideY)
        {
            std::cout<<"Need to Divide X + Y for level "<<currentLevel<<std::endl;
            // create four tiles.
            osg::BoundingBox bottom_left(extents.xMin(),extents.yMin(),extents.zMin(),xCenter,yCenter,extents.zMax());
            osg::BoundingBox bottom_right(xCenter,extents.yMin(),extents.zMin(),extents.xMax(),yCenter,extents.zMax());
            osg::BoundingBox top_left(extents.xMin(),yCenter,extents.zMin(),xCenter,extents.yMax(),extents.zMax());
            osg::BoundingBox top_right(xCenter,yCenter,extents.zMin(),extents.xMax(),extents.yMax(),extents.zMax());

            destinationGraph->_children.push_back(createDestinationGraph(cs,
                                                                         bottom_left,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(cs,
                                                                         bottom_right,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX+1,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(cs,
                                                                         top_left,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY+1,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(cs,
                                                                         top_right,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX+1,
                                                                         newY+1,
                                                                         maxNumLevels));
                                                                         
            // Set all there max distance to the same value to ensure the same LOD bining.
            float cutOffDistance = destinationGraph->_maxVisibleDistance*0.5f;
            
            for(CompositeDestination::ChildList::iterator citr=destinationGraph->_children.begin();
                citr!=destinationGraph->_children.end();
                ++citr)
            {
                (*citr)->_maxVisibleDistance = cutOffDistance;
            }
           

        }
        else if (needToDivideX)
        {
            std::cout<<"Need to Divide X only"<<std::endl;
        }
        else if (needToDivideY)
        {
            std::cout<<"Need to Divide Y only"<<std::endl;
        }
        else
        {
            std::cout<<"No Need to Divide"<<std::endl;
        }
    }
    
    return destinationGraph;
    
#if 0    
        // now get each tile to point to each other.
        top_left_tile->setNeighbours(0,0,bottom_left_tile,bottom_right_tile,top_right_tile,0,0,0);
        top_right_tile->setNeighbours(top_left_tile,bottom_left_tile,bottom_right_tile,0,0,0,0,0);
        bottom_left_tile->setNeighbours(0,0,0,0,bottom_right_tile,top_right_tile,top_left_tile,0);
        bottom_right_tile->setNeighbours(bottom_left_tile,0,0,0,0,0,top_right_tile,top_left_tile);
        
        top_left_tile->checkNeighbouringTiles();
        top_right_tile->checkNeighbouringTiles();
        bottom_left_tile->checkNeighbouringTiles();
        bottom_right_tile->checkNeighbouringTiles();
#endif    
}

void DataSet::computeDestinationGraphFromSources(unsigned int numLevels)
{

    // ensure we have a valid coordinate system
    if (!_coordinateSystem)
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            SourceData* sd = (*itr)->getSourceData();
            if (sd)
            {
                if (sd->_cs.valid())
                {
                    _coordinateSystem = sd->_cs;
                    break;
                }
            }
        }
    }


    // get the extents of the sources and
    osg::BoundingBox extents(_extents);
    if (!extents.valid()) 
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            SourceData* sd = (*itr)->getSourceData();
            if (sd) extents.expandBy(sd->getExtents(_coordinateSystem.get()));
        }
    }

    // then create the destinate graph accordingly.

    unsigned int imageSize = 256;
    unsigned int terrainSize = 64;

     _destinationGraph = createDestinationGraph(_coordinateSystem.get(),
                                                extents,
                                                imageSize,
                                                terrainSize,
                                                0,
                                                0,
                                                0,
                                                numLevels);
                                                           
        



}

template<class T>
struct DerefLessFunctor
{
    bool operator () (const T& lhs, const T& rhs)
    {
        return lhs->getSortValue() > rhs->getSortValue();
    }
};

void DataSet::CompositeSource::setSortValueFromSourceDataResolution()
{
    for(SourceList::iterator sitr=_sourceList.begin();sitr!=_sourceList.end();++sitr)
    {
        (*sitr)->setSortValueFromSourceDataResolution();
    }
        

    for(ChildList::iterator citr=_children.begin();citr!=_children.end();++citr)
    {
        (*citr)->setSortValueFromSourceDataResolution();
    }
}

void DataSet::CompositeSource::sort()
{
    // sort the sources.
    std::sort(_sourceList.begin(),_sourceList.end(),DerefLessFunctor< osg::ref_ptr<Source> >());
    
    // sort the composite sources internal data
    for(ChildList::iterator itr=_children.begin();itr!=_children.end();++itr)
    {
        (*itr)->sort();
    }
}

void DataSet::updateSourcesForDestinationGraphNeeds()
{

    std::string temporyFilePrefix("temporaryfile_");

    // compute the resolutions of the source that are required.
    {
        _destinationGraph->addRequiredResolutions(_sourceGraph.get());


        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            std::cout<<"Source File "<<source->getFileName()<<std::endl;
            

            const Source::ResolutionList& resolutions = source->getRequiredResolutions();
            std::cout<<"    resolutions.size() "<<resolutions.size()<<std::endl;
            std::cout<<"    { "<<std::endl;
            for(Source::ResolutionList::const_iterator itr=resolutions.begin();
                itr!=resolutions.end();
                ++itr)
            {
                std::cout<<"        resX="<<itr->_resX<<" resY="<<itr->_resY<<std::endl;
            }
            std::cout<<"    } "<<std::endl;

            source->consolodateRequiredResolutions();
            
            std::cout<<"    consolodated resolutions.size() "<<resolutions.size()<<std::endl;
            std::cout<<"    consolodated { "<<std::endl;
            for(Source::ResolutionList::const_iterator itr=resolutions.begin();
                itr!=resolutions.end();
                ++itr)
            {
                std::cout<<"        resX="<<itr->_resX<<" resY="<<itr->_resY<<std::endl;
            }
            std::cout<<"    } "<<std::endl;


        }


    }

#if 1
    // do standardisation of coordinates systems.
    // do any reprojection if required.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            if (source->needReproject(_coordinateSystem.get()))
            {
                // do the reprojection to a tempory file.
                std::string newFileName = temporyFilePrefix + osgDB::getStrippedName(source->getFileName()) + ".tif";
                
                Source* newSource = source->doReproject(newFileName,_coordinateSystem.get());
                
                // replace old source by new one.
                if (newSource) *itr = newSource;
            }
        }
    }
#endif
    
    // do sampling of data to required values.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            source->buildOverviews();
        }
    }

    // sort the sources so that the lowest res tiles are drawn first.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            source->setSortValueFromSourceDataResolution();
            std::cout<<"sort "<<source->getFileName()<<" value "<<source->getSortValue()<<std::endl;
            
        }
        
        // sort them so highest sortValue is first.

        _sourceGraph->sort();
    }
    
    std::cout<<"Using source_lod_iterator itr"<<std::endl;
    for(CompositeSource::source_lod_iterator csitr(_sourceGraph.get(),CompositeSource::LODSourceAdvancer(0.0));csitr.valid();++csitr)
    {
        std::cout<<"  LOD "<<(*csitr)->getFileName()<<std::endl;
    }
    std::cout<<"End of Using Source Iterator itr"<<std::endl;
    
    
    
    
    
}

void DataSet::populateDestinationGraphFromSources()
{
    // for each DestinationTile populate it.
    _destinationGraph->readFrom(_sourceGraph.get());
    
    // for each DestinationTile equalize the boundaries so they all fit each other without gaps.
    _destinationGraph->equalizeBoundaries();
}


void DataSet::createDestination(unsigned int numLevels)
{

    std::cout<<"new DataSet::createDestination()"<<std::endl;
    
    computeDestinationGraphFromSources(numLevels);
    
    updateSourcesForDestinationGraphNeeds();
    
    populateDestinationGraphFromSources();

    if (_destinationGraph.valid()) _rootNode = _destinationGraph->createScene();
}

void DataSet::writeDestination(const std::string& filename)
{
    std::cout<<"DataSet::writeDestination("<<filename<<")"<<std::endl;
    if (_rootNode.valid())
    {
        osgDB::writeNodeFile(*_rootNode,filename);
    }
}

