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


#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>
#include <osg/Group>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/ClusterCullingCallback>
#include <osg/Notify>
#include <osg/io_utils>

#include <osg/GLU>

#include <osgUtil/SmoothingVisitor>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Simplifier>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#include <osgFX/MultiTextureControl>

#include <osgTerrain/DataSet>

// GDAL includes
#include <gdal_priv.h>
#include <cpl_string.h>
#include <gdalwarper.h>
#include <ogr_spatialref.h>

// standard library includes
#include <sstream>
#include <iostream>
#include <algorithm>


using namespace osgTerrain;


static int s_notifyOffset = 0;
void DataSet::setNotifyOffset(int level) { s_notifyOffset = level; }
int DataSet::getNotifyOffset() { return s_notifyOffset; }

inline std::ostream& my_notify(osg::NotifySeverity level) { return osg::notify(osg::NotifySeverity(s_notifyOffset+level)); }


std::string osgTerrain::DataSet::coordinateSystemStringToWTK(const std::string& coordinateSystem)
{
    std::string wtkString;

    CPLErrorReset();
    
    OGRSpatialReferenceH hSRS = OSRNewSpatialReference( NULL );
    if( OSRSetFromUserInput( hSRS, coordinateSystem.c_str() ) == OGRERR_NONE )
    {
        char *pszResult = NULL;
        OSRExportToWkt( hSRS, &pszResult );
        
        if (pszResult) wtkString = pszResult;

        CPLFree(pszResult);

    }
    else
    {
        my_notify(osg::WARN)<<"Warning: coordinateSystem string not recognised."<<std::endl;
        
    }
    
    OSRDestroySpatialReference( hSRS );

    return wtkString;
}

enum CoordinateSystemType
{
    GEOCENTRIC,
    GEOGRAPHIC,
    PROJECTED,
    LOCAL
};

CoordinateSystemType getCoordinateSystemType(const osg::CoordinateSystemNode* lhs)
{
    if (!lhs) return PROJECTED;

    // set up LHS SpatialReference
    char* projection_string = strdup(lhs->getCoordinateSystem().c_str());
    char* importString = projection_string;
    
    OGRSpatialReference lhsSR;
    lhsSR.importFromWkt(&importString);
    
     
    
    my_notify(osg::INFO)<<"getCoordinateSystemType("<<projection_string<<")"<<std::endl;
    my_notify(osg::INFO)<<"    lhsSR.IsGeographic()="<<lhsSR.IsGeographic()<<std::endl;
    my_notify(osg::INFO)<<"    lhsSR.IsProjected()="<<lhsSR.IsProjected()<<std::endl;
    my_notify(osg::INFO)<<"    lhsSR.IsLocal()="<<lhsSR.IsLocal()<<std::endl;

    free(projection_string);

    if (strcmp(lhsSR.GetRoot()->GetValue(),"GEOCCS")==0) my_notify(osg::INFO)<<"    lhsSR. is GEOCENTRIC "<<std::endl;
    

    if (strcmp(lhsSR.GetRoot()->GetValue(),"GEOCCS")==0) return GEOCENTRIC;    
    if (lhsSR.IsGeographic()) return GEOGRAPHIC;
    if (lhsSR.IsProjected()) return PROJECTED;
    if (lhsSR.IsLocal()) return LOCAL;
    return PROJECTED;
}

double getAngularUnits(const osg::CoordinateSystemNode* lhs)
{
    // set up LHS SpatialReference
    char* projection_string = strdup(lhs->getCoordinateSystem().c_str());
    char* importString = projection_string;
    
    OGRSpatialReference lhsSR;
    lhsSR.importFromWkt(&importString);
    
    free(projection_string);

    char* str;
    double result = lhsSR.GetAngularUnits(&str);
    my_notify(osg::INFO)<<"lhsSR.GetAngularUnits("<<str<<") "<<result<<std::endl;

    return result;
}

double getLinearUnits(const osg::CoordinateSystemNode* lhs)
{
    // set up LHS SpatialReference
    char* projection_string = strdup(lhs->getCoordinateSystem().c_str());
    char* importString = projection_string;
    
    OGRSpatialReference lhsSR;
    lhsSR.importFromWkt(&importString);
    
    free(projection_string);

    char* str;
    double result = lhsSR.GetLinearUnits(&str);
    my_notify(osg::INFO)<<"lhsSR.GetLinearUnits("<<str<<") "<<result<<std::endl;

    my_notify(osg::INFO)<<"lhsSR.IsGeographic() "<<lhsSR.IsGeographic()<<std::endl;
    my_notify(osg::INFO)<<"lhsSR.IsProjected() "<<lhsSR.IsProjected()<<std::endl;
    my_notify(osg::INFO)<<"lhsSR.IsLocal() "<<lhsSR.IsLocal()<<std::endl;
    
    return result;
}

bool areCoordinateSystemEquivalent(const osg::CoordinateSystemNode* lhs,const osg::CoordinateSystemNode* rhs)
{
    // if ptr's equal the return true
    if (lhs == rhs) return true;
    
    // if one CS is NULL then true false
    if (!lhs || !rhs)
    {
        my_notify(osg::INFO)<<"areCoordinateSystemEquivalent lhs="<<lhs<<"  rhs="<<rhs<<" return true"<<std::endl;
        return false;
    }
    
    my_notify(osg::INFO)<<"areCoordinateSystemEquivalent lhs="<<lhs->getCoordinateSystem()<<"  rhs="<<rhs->getCoordinateSystem()<<std::endl;

    // use compare on ProjectionRef strings.
    if (lhs->getCoordinateSystem() == rhs->getCoordinateSystem()) return true;
    
    // set up LHS SpatialReference
    char* projection_string = strdup(lhs->getCoordinateSystem().c_str());
    char* importString = projection_string;
    
    OGRSpatialReference lhsSR;
    lhsSR.importFromWkt(&importString);
    
    free(projection_string);

    // set up RHS SpatialReference
    projection_string = strdup(rhs->getCoordinateSystem().c_str());
    importString = projection_string;

    OGRSpatialReference rhsSR;
    rhsSR.importFromWkt(&importString);

    free(projection_string);
    
    int result = lhsSR.IsSame(&rhsSR);

#if 0
    int result2 = lhsSR.IsSameGeogCS(&rhsSR);

     my_notify(osg::INFO)<<"areCoordinateSystemEquivalent "<<std::endl
              <<"LHS = "<<lhs->getCoordinateSystem()<<std::endl
              <<"RHS = "<<rhs->getCoordinateSystem()<<std::endl
              <<"result = "<<result<<"  result2 = "<<result2<<std::endl;
#endif
         return result ? true : false;
}

void DataSet::SpatialProperties::computeExtents()
{
    _extents.init();
    _extents.expandBy( osg::Vec3(0.0,0.0,0.0)*_geoTransform);
    _extents.expandBy( osg::Vec3(_numValuesX,_numValuesY,0.0)*_geoTransform);
    _extents._isGeographic = getCoordinateSystemType(_cs.get())==GEOGRAPHIC;

    my_notify(osg::INFO)<<"DataSet::SpatialProperties::computeExtents() is geographic "<<_extents._isGeographic<<std::endl;
}

DataSet::SourceData::~SourceData()
{
    if (_gdalDataset) GDALClose(_gdalDataset);
}

float DataSet::SourceData::getInterpolatedValue(GDALRasterBand *band, double x, double y)
{
    double geoTransform[6];
    geoTransform[0] = _geoTransform(3,0);
    geoTransform[1] = _geoTransform(0,0);
    geoTransform[2] = _geoTransform(1,0);
    geoTransform[3] = _geoTransform(3,1);
    geoTransform[4] = _geoTransform(0,1);
    geoTransform[5] = _geoTransform(1,1);

    double invTransform[6];
    GDALInvGeoTransform(geoTransform, invTransform);
    double r, c;
    GDALApplyGeoTransform(invTransform, x, y, &c, &r);
   
    int rowMin = osg::maximum((int)floor(r), 0);
    int rowMax = osg::maximum(osg::minimum((int)ceil(r), (int)(_numValuesY-1)), 0);
    int colMin = osg::maximum((int)floor(c), 0);
    int colMax = osg::maximum(osg::minimum((int)ceil(c), (int)(_numValuesX-1)), 0);

    if (rowMin > rowMax) rowMin = rowMax;
    if (colMin > colMax) colMin = colMax;

    float urHeight, llHeight, ulHeight, lrHeight;

    band->RasterIO(GF_Read, colMin, rowMin, 1, 1, &llHeight, 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, colMin, rowMax, 1, 1, &ulHeight, 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, colMax, rowMin, 1, 1, &lrHeight, 1, 1, GDT_Float32, 0, 0);
    band->RasterIO(GF_Read, colMax, rowMax, 1, 1, &urHeight, 1, 1, GDT_Float32, 0, 0);

    int success = 0;
    float noDataValue = band->GetNoDataValue(&success);
    if (success)
    {
      if (llHeight == noDataValue) llHeight = 0.0f;
      if (ulHeight == noDataValue) ulHeight = 0.0f;
      if (lrHeight == noDataValue) lrHeight = 0.0f;
      if (urHeight == noDataValue) urHeight = 0.0f;
    }

    double x_rem = c - (int)c;
    double y_rem = r - (int)r;

    double w00 = (1.0 - y_rem) * (1.0 - x_rem) * (double)llHeight;
    double w01 = (1.0 - y_rem) * x_rem * (double)lrHeight;
    double w10 = y_rem * (1.0 - x_rem) * (double)ulHeight;
    double w11 = y_rem * x_rem * (double)urHeight;

    float result = (float)(w00 + w01 + w10 + w11);

    return result;
}

DataSet::SourceData* DataSet::SourceData::readData(Source* source)
{
    if (!source) return 0;


    switch(source->getType())
    {
    case(Source::IMAGE):
    case(Source::HEIGHT_FIELD):
        {
            GDALDataset* gdalDataSet = (GDALDataset*)source->getGdalDataset();
            if(!gdalDataSet)
                gdalDataSet = (GDALDataset*)GDALOpen(source->getFileName().c_str(),GA_ReadOnly);
            if (gdalDataSet)
            {
                SourceData* data = new SourceData(source);
                data->_gdalDataset = gdalDataSet;
                
                data->_numValuesX = gdalDataSet->GetRasterXSize();
                data->_numValuesY = gdalDataSet->GetRasterYSize();
                data->_numValuesZ = gdalDataSet->GetRasterCount();
                data->_hasGCPs = gdalDataSet->GetGCPCount()!=0;

                const char* pszSourceSRS = gdalDataSet->GetProjectionRef();
                if (!pszSourceSRS || strlen(pszSourceSRS)==0) pszSourceSRS = gdalDataSet->GetGCPProjection();
                
                data->_cs = new osg::CoordinateSystemNode("WKT",pszSourceSRS);

                double geoTransform[6];
                if (gdalDataSet->GetGeoTransform(geoTransform)==CE_None)
                {
                    data->_geoTransform.set( geoTransform[1],    geoTransform[4],    0.0,    0.0,
                                             geoTransform[2],    geoTransform[5],    0.0,    0.0,
                                             0.0,                0.0,                1.0,    0.0,
                                             geoTransform[0],    geoTransform[3],    0.0,    1.0);
                                            
                    data->computeExtents();

                }
                else if (gdalDataSet->GetGCPCount()>0 && gdalDataSet->GetGCPProjection())
                {
                    my_notify(osg::INFO) << "    Using GCP's"<< std::endl;


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
                        my_notify(osg::INFO)<<" failed to create transformer"<<std::endl;
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
                        my_notify(osg::INFO)<<" failed to create warp"<<std::endl;
                        return NULL;
                    }

                    GDALDestroyGenImgProjTransformer( hTransformArg );


                    data->_geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],    0.0,    0.0,
                                             adfDstGeoTransform[2],    adfDstGeoTransform[5],    0.0,    0.0,
                                             0.0,                0.0,                1.0,    0.0,
                                             adfDstGeoTransform[0],    adfDstGeoTransform[3],    0.0,    1.0);

                    data->computeExtents();
                    
                }
                else
                {
                    my_notify(osg::INFO) << "    No GeoTransform or GCP's - unable to compute position in space"<< std::endl;
                    
                    data->_geoTransform.set( 1.0,    0.0,    0.0,    0.0,
                                             0.0,    1.0,    0.0,    0.0,
                                             0.0,    0.0,    1.0,    0.0,
                                             0.0,    0.0,    0.0,    1.0);
                                            
                    data->computeExtents();

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

GeospatialExtents DataSet::SourceData::getExtents(const osg::CoordinateSystemNode* cs) const
{
    return computeSpatialProperties(cs)._extents;
}

const DataSet::SpatialProperties& DataSet::SourceData::computeSpatialProperties(const osg::CoordinateSystemNode* cs) const
{
    // check to see it exists in the _spatialPropertiesMap first.
    SpatialPropertiesMap::const_iterator itr = _spatialPropertiesMap.find(cs);
    if (itr!=_spatialPropertiesMap.end()) 
    {
        return itr->second;
    }

    if (areCoordinateSystemEquivalent(_cs.get(),cs))
    {
        return *this;
    }

    if (_cs.valid() && cs)
    {
        
        if (_gdalDataset)
        {

            //my_notify(osg::INFO)<<"Projecting bounding volume for "<<_source->getFileName()<<std::endl;

            
            // insert into the _spatialPropertiesMap for future reuse.
            _spatialPropertiesMap[cs] = *this;
            DataSet::SpatialProperties& sp = _spatialPropertiesMap[cs];
            
            /* -------------------------------------------------------------------- */
            /*      Create a transformation object from the source to               */
            /*      destination coordinate system.                                  */
            /* -------------------------------------------------------------------- */
            void *hTransformArg = 
                GDALCreateGenImgProjTransformer( _gdalDataset,_cs->getCoordinateSystem().c_str(),
                                                 NULL, cs->getCoordinateSystem().c_str(),
                                                 TRUE, 0.0, 1 );

            if (!hTransformArg)
            {
                my_notify(osg::INFO)<<" failed to create transformer"<<std::endl;
                return sp;
            }
        
            double adfDstGeoTransform[6];
            int nPixels=0, nLines=0;
            if( GDALSuggestedWarpOutput( _gdalDataset, 
                                         GDALGenImgProjTransform, hTransformArg, 
                                         adfDstGeoTransform, &nPixels, &nLines )
                != CE_None )
            {
                my_notify(osg::INFO)<<" failed to create warp"<<std::endl;
                return sp;
            }

            sp._numValuesX = nPixels;
            sp._numValuesY = nLines;
            sp._cs = const_cast<osg::CoordinateSystemNode*>(cs);
            sp._geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],  0.0,    0.0,
                                  adfDstGeoTransform[2],    adfDstGeoTransform[5],  0.0,    0.0,
                                  0.0,                      0.0,                    1.0,    0.0,
                                  adfDstGeoTransform[0],    adfDstGeoTransform[3],  0.0,    1.0);

            GDALDestroyGenImgProjTransformer( hTransformArg );

            sp.computeExtents();

            return sp;
        }

    }
    my_notify(osg::INFO)<<"DataSet::DataSource::assuming compatible coordinates."<<std::endl;
    return *this;
}

bool DataSet::SourceData::intersects(const SpatialProperties& sp) const
{
    return sp._extents.intersects(getExtents(sp._cs.get()));
}

void DataSet::SourceData::read(DestinationData& destination)
{
    my_notify(osg::INFO)<<"A"<<std::endl;

    if (!_source) return;
    
    my_notify(osg::INFO)<<"B"<<std::endl;

    switch (_source->getType())
    {
    case(Source::IMAGE):
        my_notify(osg::INFO)<<"B.1"<<std::endl;
        readImage(destination);
        break;
    case(Source::HEIGHT_FIELD):
        my_notify(osg::INFO)<<"B.2"<<std::endl;
        readHeightField(destination);
        break;
    case(Source::MODEL):
        my_notify(osg::INFO)<<"B.3"<<std::endl;
        readModels(destination);
        break;
    }
    my_notify(osg::INFO)<<"C"<<std::endl;
}

void DataSet::SourceData::readImage(DestinationData& destination)
{
    my_notify(osg::INFO)<<std::endl<<"readImage "<<std::endl;

    if (destination._image.valid())
    {
        
        GeospatialExtents s_bb = getExtents(destination._cs.get());
        GeospatialExtents d_bb = destination._extents;

        // note, we have to handle the possibility of goegraphic datasets wrapping over on themselves when they pass over the dateline
        // to do this we have to test geographic datasets via two passes, each with a 360 degree shift of the source cata.
        double xoffset = d_bb.xMin() < s_bb.xMin() ? -360.0 : 0.0;
        unsigned int numXChecks = d_bb._isGeographic ? 2 : 1;
        for(unsigned int ic = 0; ic < numXChecks; ++ic, xoffset += 360.0)
        {
        
            my_notify(osg::INFO)<<"Testing "<<xoffset<<std::endl;
            my_notify(osg::INFO)<<"  s_bb "<<s_bb.xMin()+xoffset<<" "<<s_bb.xMax()+xoffset<<std::endl;
            my_notify(osg::INFO)<<"  d_bb "<<d_bb.xMin()<<" "<<d_bb.xMax()<<std::endl;
        
            GeospatialExtents intersect_bb(d_bb.intersection(s_bb, xoffset));
            if (!intersect_bb.valid())
            {   
                my_notify(osg::INFO)<<"Reading image but it does not intesection destination - ignoring"<<std::endl;
                continue;
            }

            my_notify(osg::INFO)<<"readImage s_bb is geographic "<<s_bb._isGeographic<<std::endl;
            my_notify(osg::INFO)<<"readImage d_bb is geographic "<<d_bb._isGeographic<<std::endl;
            my_notify(osg::INFO)<<"readImage intersect_bb is geographic "<<intersect_bb._isGeographic<<std::endl;



           int windowX = osg::maximum((int)floorf((float)_numValuesX*(intersect_bb.xMin()-xoffset-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),0);
           int windowY = osg::maximum((int)floorf((float)_numValuesY*(intersect_bb.yMin()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),0);
           int windowWidth = osg::minimum((int)ceilf((float)_numValuesX*(intersect_bb.xMax()-xoffset-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),(int)_numValuesX)-windowX;
           int windowHeight = osg::minimum((int)ceilf((float)_numValuesY*(intersect_bb.yMax()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),(int)_numValuesY)-windowY;

           int destX = osg::maximum((int)floorf((float)destination._image->s()*(intersect_bb.xMin()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),0);
           int destY = osg::maximum((int)floorf((float)destination._image->t()*(intersect_bb.yMin()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),0);
           int destWidth = osg::minimum((int)ceilf((float)destination._image->s()*(intersect_bb.xMax()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),(int)destination._image->s())-destX;
           int destHeight = osg::minimum((int)ceilf((float)destination._image->t()*(intersect_bb.yMax()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),(int)destination._image->t())-destY;

            my_notify(osg::INFO)<<"   copying from "<<windowX<<"\t"<<windowY<<"\t"<<windowWidth<<"\t"<<windowHeight<<std::endl;
            my_notify(osg::INFO)<<"             to "<<destX<<"\t"<<destY<<"\t"<<destWidth<<"\t"<<destHeight<<std::endl;

            int readWidth = destWidth;
            int readHeight = destHeight;
            bool doResample = false;

            float destWindowWidthRatio = (float)destWidth/(float)windowWidth;
            float destWindowHeightRatio = (float)destHeight/(float)windowHeight;
            const float resizeTolerance = 1.1;

            bool interpolateSourceImagery = true;
            if (interpolateSourceImagery && 
                (destWindowWidthRatio>resizeTolerance || destWindowHeightRatio>resizeTolerance) &&
                windowWidth>=2 && windowHeight>=2)
            {
                readWidth = windowWidth;
                readHeight = windowHeight;
                doResample = true;
            }

            bool hasRGB = _gdalDataset->GetRasterCount() >= 3;
            bool hasAlpha = _gdalDataset->GetRasterCount() >= 4;
            bool hasColorTable = _gdalDataset->GetRasterCount() >= 1 && _gdalDataset->GetRasterBand(1)->GetColorTable();
            bool hasGreyScale = _gdalDataset->GetRasterCount() == 1;
            unsigned int numSourceComponents = hasAlpha?4:3;

            if (hasRGB || hasColorTable || hasGreyScale)
            {
                // RGB

                unsigned int numBytesPerPixel = 1;
                GDALDataType targetGDALType = GDT_Byte;

                int pixelSpace=numSourceComponents*numBytesPerPixel;

                my_notify(osg::INFO) << "reading RGB"<<std::endl;

                unsigned char* tempImage = new unsigned char[readWidth*readHeight*pixelSpace];


                /* New code courtesy of Frank Warmerdam of the GDAL group */

                // RGB images ... or at least we assume 3+ band images can be treated 
                // as RGB. 
                if( hasRGB ) 
                { 
                    GDALRasterBand* bandRed = _gdalDataset->GetRasterBand(1); 
                    GDALRasterBand* bandGreen = _gdalDataset->GetRasterBand(2); 
                    GDALRasterBand* bandBlue = _gdalDataset->GetRasterBand(3); 
                    GDALRasterBand* bandAlpha = hasAlpha ? _gdalDataset->GetRasterBand(4) : 0; 

                    bandRed->RasterIO(GF_Read, 
                                      windowX,_numValuesY-(windowY+windowHeight), 
                                      windowWidth,windowHeight, 
                                      (void*)(tempImage+0),readWidth,readHeight, 
                                      targetGDALType,pixelSpace,pixelSpace*readWidth); 
                    bandGreen->RasterIO(GF_Read, 
                                        windowX,_numValuesY-(windowY+windowHeight), 
                                        windowWidth,windowHeight, 
                                        (void*)(tempImage+1),readWidth,readHeight, 
                                        targetGDALType,pixelSpace,pixelSpace*readWidth); 
                    bandBlue->RasterIO(GF_Read, 
                                       windowX,_numValuesY-(windowY+windowHeight), 
                                       windowWidth,windowHeight, 
                                       (void*)(tempImage+2),readWidth,readHeight, 
                                       targetGDALType,pixelSpace,pixelSpace*readWidth); 

                    if (bandAlpha)
                    {
                        bandAlpha->RasterIO(GF_Read, 
                                           windowX,_numValuesY-(windowY+windowHeight), 
                                           windowWidth,windowHeight, 
                                           (void*)(tempImage+3),readWidth,readHeight, 
                                           targetGDALType,pixelSpace,pixelSpace*readWidth); 
                    }
                } 

                else if( hasColorTable ) 
                { 
                    // Pseudocolored image.  Convert 1 band + color table to 24bit RGB. 

                    GDALRasterBand *band; 
                    GDALColorTable *ct; 
                    int i; 


                    band = _gdalDataset->GetRasterBand(1); 


                    band->RasterIO(GF_Read, 
                                   windowX,_numValuesY-(windowY+windowHeight), 
                                   windowWidth,windowHeight, 
                                   (void*)(tempImage+0),readWidth,readHeight, 
                                   targetGDALType,pixelSpace,pixelSpace*readWidth); 


                    ct = band->GetColorTable(); 


                    for( i = 0; i < readWidth * readHeight; i++ ) 
                    { 
                        GDALColorEntry sEntry; 


                        // default to greyscale equilvelent. 
                        sEntry.c1 = tempImage[i*3]; 
                        sEntry.c2 = tempImage[i*3]; 
                        sEntry.c3 = tempImage[i*3]; 


                        ct->GetColorEntryAsRGB( tempImage[i*3], &sEntry ); 


                        // Apply RGB back over destination image. 
                        tempImage[i*3 + 0] = sEntry.c1; 
                        tempImage[i*3 + 1] = sEntry.c2; 
                        tempImage[i*3 + 2] = sEntry.c3; 
                    } 
                } 


                else if (hasGreyScale)
                { 
                    // Greyscale image.  Convert 1 band to 24bit RGB. 
                    GDALRasterBand *band; 


                    band = _gdalDataset->GetRasterBand(1); 


                    band->RasterIO(GF_Read, 
                                   windowX,_numValuesY-(windowY+windowHeight), 
                                   windowWidth,windowHeight, 
                                   (void*)(tempImage+0),readWidth,readHeight, 
                                   targetGDALType,pixelSpace,pixelSpace*readWidth); 
                    band->RasterIO(GF_Read, 
                                   windowX,_numValuesY-(windowY+windowHeight), 
                                   windowWidth,windowHeight, 
                                   (void*)(tempImage+1),readWidth,readHeight, 
                                   targetGDALType,pixelSpace,pixelSpace*readWidth); 
                    band->RasterIO(GF_Read, 
                                   windowX,_numValuesY-(windowY+windowHeight), 
                                   windowWidth,windowHeight, 
                                   (void*)(tempImage+2),readWidth,readHeight, 
                                   targetGDALType,pixelSpace,pixelSpace*readWidth); 
                } 

                if (doResample || readWidth!=destWidth || readHeight!=destHeight)
                {
                    unsigned char* destImage = new unsigned char[destWidth*destHeight*pixelSpace];

                    // rescale image by hand as glu seem buggy....
                    for(int j=0;j<destHeight;++j)
                    {
                        float  t_d = (float)j/((float)destHeight-1);
                        for(int i=0;i<destWidth;++i)
                        {
                            float s_d = (float)i/((float)destWidth-1);

                            float flt_read_i = s_d * ((float)readWidth-1);
                            float flt_read_j = t_d * ((float)readHeight-1);

                            int read_i = (int)flt_read_i;
                            if (read_i>=readWidth) read_i=readWidth-1;

                            float flt_read_ir = flt_read_i-read_i;
                            if (read_i==readWidth-1) flt_read_ir=0.0f;

                            int read_j = (int)flt_read_j;
                            if (read_j>=readHeight) read_j=readHeight-1;

                            float flt_read_jr = flt_read_j-read_j;
                            if (read_j==readHeight-1) flt_read_jr=0.0f;

                            unsigned char* dest = destImage + (j*destWidth + i) * pixelSpace;
                            if (flt_read_ir==0.0f)  // no need to interpolate i axis.
                            {
                                if (flt_read_jr==0.0f)  // no need to interpolate j axis.
                                {
                                    // copy pixels
                                    unsigned char* src = tempImage + (read_j*readWidth + read_i) * pixelSpace;
                                    dest[0] = src[0];
                                    dest[1] = src[1];
                                    dest[2] = src[2];
                                    if (numSourceComponents==4) dest[3] = src[3];
                                    //std::cout<<"copy"<<std::endl;
                                }
                                else  // need to interpolate j axis.
                                {
                                    // copy pixels
                                    unsigned char* src_0 = tempImage + (read_j*readWidth + read_i) * pixelSpace;
                                    unsigned char* src_1 = src_0 + readWidth*pixelSpace;
                                    float r_0 = 1.0f-flt_read_jr;
                                    float r_1 = flt_read_jr;
                                    dest[0] = (unsigned char)((float)src_0[0]*r_0 + (float)src_1[0]*r_1);
                                    dest[1] = (unsigned char)((float)src_0[1]*r_0 + (float)src_1[1]*r_1);
                                    dest[2] = (unsigned char)((float)src_0[2]*r_0 + (float)src_1[2]*r_1);
                                    if (numSourceComponents==4) dest[3] = (unsigned char)((float)src_0[3]*r_0 + (float)src_1[3]*r_1);
                                    //std::cout<<"interpolate j axis"<<std::endl;
                                }
                            }
                            else // need to interpolate i axis.
                            {
                                if (flt_read_jr==0.0f) // no need to interpolate j axis.
                                {
                                    // copy pixels
                                    unsigned char* src_0 = tempImage + (read_j*readWidth + read_i) * pixelSpace;
                                    unsigned char* src_1 = src_0 + pixelSpace;
                                    float r_0 = 1.0f-flt_read_ir;
                                    float r_1 = flt_read_ir;
                                    dest[0] = (unsigned char)((float)src_0[0]*r_0 + (float)src_1[0]*r_1);
                                    dest[1] = (unsigned char)((float)src_0[1]*r_0 + (float)src_1[1]*r_1);
                                    dest[2] = (unsigned char)((float)src_0[2]*r_0 + (float)src_1[2]*r_1);
                                    if (numSourceComponents==4) dest[3] = (unsigned char)((float)src_0[3]*r_0 + (float)src_1[3]*r_1);
                                    //std::cout<<"interpolate i axis"<<std::endl;
                                }
                                else  // need to interpolate i and j axis.
                                {
                                    unsigned char* src_0 = tempImage + (read_j*readWidth + read_i) * pixelSpace;
                                    unsigned char* src_1 = src_0 + readWidth*pixelSpace;
                                    unsigned char* src_2 = src_0 + pixelSpace;
                                    unsigned char* src_3 = src_1 + pixelSpace;
                                    float r_0 = (1.0f-flt_read_ir)*(1.0f-flt_read_jr);
                                    float r_1 = (1.0f-flt_read_ir)*flt_read_jr;
                                    float r_2 = (flt_read_ir)*(1.0f-flt_read_jr);
                                    float r_3 = (flt_read_ir)*flt_read_jr;
                                    dest[0] = (unsigned char)(((float)src_0[0])*r_0 + ((float)src_1[0])*r_1 + ((float)src_2[0])*r_2 + ((float)src_3[0])*r_3);
                                    dest[1] = (unsigned char)(((float)src_0[1])*r_0 + ((float)src_1[1])*r_1 + ((float)src_2[1])*r_2 + ((float)src_3[1])*r_3);
                                    dest[2] = (unsigned char)(((float)src_0[2])*r_0 + ((float)src_1[2])*r_1 + ((float)src_2[2])*r_2 + ((float)src_3[2])*r_3);
                                    if (numSourceComponents==4) dest[3] = (unsigned char)(((float)src_0[3])*r_0 + ((float)src_1[3])*r_1 + ((float)src_2[3])*r_2 + ((float)src_3[3])*r_3);
                                    //std::cout<<"interpolate i & j axis"<<std::endl;
                                }
                            }

                        }
                    }

                    delete [] tempImage;  
                    tempImage = destImage;
                }

                // now copy into destination image
                unsigned char* sourceRowPtr = tempImage;
                int sourceRowDelta = pixelSpace*destWidth;
                unsigned char* destinationRowPtr = destination._image->data(destX,destY+destHeight-1);
                int destinationRowDelta = -(int)(destination._image->getRowSizeInBytes());
                int destination_pixelSpace = destination._image->getPixelSizeInBits()/8;
                bool destination_hasAlpha = osg::Image::computeNumComponents(destination._image->getPixelFormat())==4;

                // copy image to destination image
                for(int row=0;
                    row<destHeight;
                    ++row, sourceRowPtr+=sourceRowDelta, destinationRowPtr+=destinationRowDelta)
                {
                    unsigned char* sourceColumnPtr = sourceRowPtr;
                    unsigned char* destinationColumnPtr = destinationRowPtr;

                    for(int col=0;
                        col<destWidth;
                        ++col, sourceColumnPtr+=pixelSpace, destinationColumnPtr+=destination_pixelSpace)
                    {
                        if (hasAlpha)
                        {
                            // only copy over source pixel if its alpha value is not 0
                            if (sourceColumnPtr[3]!=0)
                            {
                                if (sourceColumnPtr[3]!=255)
                                {
                                    // source alpha is full on so directly copy over.
                                    destinationColumnPtr[0] = sourceColumnPtr[0];
                                    destinationColumnPtr[1] = sourceColumnPtr[1];
                                    destinationColumnPtr[2] = sourceColumnPtr[2];

                                    if (destination_hasAlpha)
                                        destinationColumnPtr[3] = sourceColumnPtr[3];
                                }
                                else
                                {
                                    // source value isn't full on so blend it with destination 
                                    float rs = (float)sourceColumnPtr[3]/255.0f; 
                                    float rd = 1.0f-rs;

                                    destinationColumnPtr[0] = (int)(rd * (float)destinationColumnPtr[0] + rs * (float)sourceColumnPtr[0]);
                                    destinationColumnPtr[1] = (int)(rd * (float)destinationColumnPtr[1] + rs * (float)sourceColumnPtr[1]);
                                    destinationColumnPtr[2] = (int)(rd * (float)destinationColumnPtr[2] + rs * (float)sourceColumnPtr[2]);

                                    if (destination_hasAlpha)
                                        destinationColumnPtr[3] = osg::maximum(destinationColumnPtr[3],sourceColumnPtr[3]);

                                }
                            }
                        }
                        else if (sourceColumnPtr[0]!=0 || sourceColumnPtr[1]!=0 || sourceColumnPtr[2]!=0)
                        {
                            destinationColumnPtr[0] = sourceColumnPtr[0];
                            destinationColumnPtr[1] = sourceColumnPtr[1];
                            destinationColumnPtr[2] = sourceColumnPtr[2];
                        }
                    }
                }

                delete [] tempImage;

            }
            else
            {
                my_notify(osg::INFO)<<"Warning image not read as Red, Blue and Green bands not present"<<std::endl;
            }
        }
    }
}

void DataSet::SourceData::readHeightField(DestinationData& destination)
{
    my_notify(osg::INFO)<<"In DataSet::SourceData::readHeightField"<<std::endl;

    if (destination._heightField.valid())
    {
        my_notify(osg::INFO)<<"Reading height field"<<std::endl;

        GeospatialExtents s_bb = getExtents(destination._cs.get());
        GeospatialExtents d_bb = destination._extents;

        // note, we have to handle the possibility of goegraphic datasets wrapping over on themselves when they pass over the dateline
        // to do this we have to test geographic datasets via two passes, each with a 360 degree shift of the source cata.
        double xoffset = d_bb.xMin() < s_bb.xMin() ? -360.0 : 0.0;
        unsigned int numXChecks = d_bb._isGeographic ? 2 : 1;
        for(unsigned int ic = 0; ic < numXChecks; ++ic, xoffset += 360.0)
        {
        
            osg::notify(osg::INFO)<<"Testing "<<xoffset<<std::endl;
            osg::notify(osg::INFO)<<"  s_bb "<<s_bb.xMin()+xoffset<<" "<<s_bb.xMax()+xoffset<<std::endl;
            osg::notify(osg::INFO)<<"  d_bb "<<d_bb.xMin()<<" "<<d_bb.xMax()<<std::endl;
        
            GeospatialExtents intersect_bb(d_bb.intersection(s_bb, xoffset));

            if (!intersect_bb.valid())
            {
                my_notify(osg::INFO)<<"Reading height field but it does not intesection destination - ignoring"<<std::endl;
                continue;
            }

           int destX = osg::maximum((int)floorf((float)destination._heightField->getNumColumns()*(intersect_bb.xMin()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),0);
           int destY = osg::maximum((int)floorf((float)destination._heightField->getNumRows()*(intersect_bb.yMin()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),0);
           int destWidth = osg::minimum((int)ceilf((float)destination._heightField->getNumColumns()*(intersect_bb.xMax()-d_bb.xMin())/(d_bb.xMax()-d_bb.xMin())),(int)destination._heightField->getNumColumns())-destX;
           int destHeight = osg::minimum((int)ceilf((float)destination._heightField->getNumRows()*(intersect_bb.yMax()-d_bb.yMin())/(d_bb.yMax()-d_bb.yMin())),(int)destination._heightField->getNumRows())-destY;


            // which band do we want to read from...        
            int numBands = _gdalDataset->GetRasterCount();
            GDALRasterBand* bandGray = 0;
            GDALRasterBand* bandRed = 0;
            GDALRasterBand* bandGreen = 0;
            GDALRasterBand* bandBlue = 0;
            GDALRasterBand* bandAlpha = 0;

            for(int b=1;b<=numBands;++b)
            {
                GDALRasterBand* band = _gdalDataset->GetRasterBand(b);
                if (band->GetColorInterpretation()==GCI_GrayIndex) bandGray = band;
                else if (band->GetColorInterpretation()==GCI_RedBand) bandRed = band;
                else if (band->GetColorInterpretation()==GCI_GreenBand) bandGreen = band;
                else if (band->GetColorInterpretation()==GCI_BlueBand) bandBlue = band;
                else if (band->GetColorInterpretation()==GCI_AlphaBand) bandAlpha = band;
                else if (bandGray == 0) bandGray = band;
            }


            GDALRasterBand* bandSelected = 0;
            if (!bandSelected && bandGray) bandSelected = bandGray;
            else if (!bandSelected && bandAlpha) bandSelected = bandAlpha;
            else if (!bandSelected && bandRed) bandSelected = bandRed;
            else if (!bandSelected && bandGreen) bandSelected = bandGreen;
            else if (!bandSelected && bandBlue) bandSelected = bandBlue;

            if (bandSelected)
            {

                if (bandSelected->GetUnitType()) my_notify(osg::INFO) << "bandSelected->GetUnitType()=" << bandSelected->GetUnitType()<<std::endl;
                else my_notify(osg::INFO) << "bandSelected->GetUnitType()= null" <<std::endl;


                int success = 0;
                float noDataValue = bandSelected->GetNoDataValue(&success);
                if (success)
                {
                    my_notify(osg::INFO)<<"We have NoDataValue = "<<noDataValue<<std::endl;
                }
                else
                {
                    my_notify(osg::INFO)<<"We have no NoDataValue"<<std::endl;
                    noDataValue = 0.0f;
                }

                float offset = bandSelected->GetOffset(&success);
                if (success)
                {
                    my_notify(osg::INFO)<<"We have Offset = "<<offset<<std::endl;
                }
                else
                {
                    my_notify(osg::INFO)<<"We have no Offset"<<std::endl;
                    offset = 0.0f;
                }

                float scale = bandSelected->GetScale(&success);
                if (success)
                {
                    my_notify(osg::INFO)<<"We have Scale = "<<scale<<std::endl;
                }
                else
                {
                    scale = destination._dataSet->getVerticalScale();
                    my_notify(osg::INFO)<<"We have no Scale from file so use DataSet vertical scale of "<<scale<<std::endl;

                }

                my_notify(osg::INFO)<<"********* getLinearUnits = "<<getLinearUnits(_cs.get())<<std::endl;

                // raad the data.
                osg::HeightField* hf = destination._heightField.get();

                float noDataValueFill = 0.0f;
                bool ignoreNoDataValue = true;

                bool interpolateTerrain = true;

                if (interpolateTerrain)
                {
                    //Sample terrain at each vert to increase accuracy of the terrain.
                    int endX = destX + destWidth;
                    int endY = destY + destHeight;

                    double orig_X = hf->getOrigin().x();
                    double orig_Y = hf->getOrigin().y();
                    double delta_X = hf->getXInterval();
                    double delta_Y = hf->getYInterval();

                    for (int c = destX; c < endX; ++c)
                    {
                        double geoX = orig_X + (delta_X * (double)c);
                        for (int r = destY; r < endY; ++r)
                        {
                            double geoY = orig_Y + (delta_Y * (double)r);
                            float h = getInterpolatedValue(bandSelected, geoX-xoffset, geoY);
                            if (h!=noDataValue) hf->setHeight(c,r,offset + h*scale);
                            else if (!ignoreNoDataValue) hf->setHeight(c,r,noDataValueFill);
                        }
                    }
                }
                else
                {
                    // compute dimensions to read from.        
                   int windowX = osg::maximum((int)floorf((float)_numValuesX*(intersect_bb.xMin()-xoffset-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),0);
                   int windowY = osg::maximum((int)floorf((float)_numValuesY*(intersect_bb.yMin()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),0);
                   int windowWidth = osg::minimum((int)ceilf((float)_numValuesX*(intersect_bb.xMax()-xoffset-s_bb.xMin())/(s_bb.xMax()-s_bb.xMin())),(int)_numValuesX)-windowX;
                   int windowHeight = osg::minimum((int)ceilf((float)_numValuesY*(intersect_bb.yMax()-s_bb.yMin())/(s_bb.yMax()-s_bb.yMin())),(int)_numValuesY)-windowY;

                    my_notify(osg::INFO)<<"   copying from "<<windowX<<"\t"<<windowY<<"\t"<<windowWidth<<"\t"<<windowHeight<<std::endl;
                    my_notify(osg::INFO)<<"             to "<<destX<<"\t"<<destY<<"\t"<<destWidth<<"\t"<<destHeight<<std::endl;

                    // read data into temporary array
                    float* heightData = new float [ destWidth*destHeight ];

                    //bandSelected->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,floatdata,destWidth,destHeight,GDT_Float32,numBytesPerZvalue,lineSpace);
                    bandSelected->RasterIO(GF_Read,windowX,_numValuesY-(windowY+windowHeight),windowWidth,windowHeight,heightData,destWidth,destHeight,GDT_Float32,0,0);

                    float* heightPtr = heightData;

                    for(int r=destY+destHeight-1;r>=destY;--r)
                    {
                        for(int c=destX;c<destX+destWidth;++c)
                        {
                            float h = *heightPtr++;
                            if (h!=noDataValue) hf->setHeight(c,r,offset + h*scale);
                            else if (!ignoreNoDataValue) hf->setHeight(c,r,noDataValueFill);

                            h = hf->getHeight(c,r);
                        }
                    }

                    delete [] heightData;
                }          
            }
        }
    }
}

void DataSet::SourceData::readModels(DestinationData& destination)
{
    if (_model.valid())
    {
        my_notify(osg::INFO)<<"Raading model"<<std::endl;
        destination._models.push_back(_model);
    }
}


void DataSet::Source::setGdalDataset(void* gdalDataSet)
{
    _gdalDataset = (GDALDataset*)gdalDataSet;
}

void* DataSet::Source::getGdalDataset()
{
    return _gdalDataset;
}

const void* DataSet::Source::getGdalDataset() const
{
    return _gdalDataset;
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
    my_notify(osg::INFO)<<"DataSet::Source::loadSourceData() "<<_filename<<std::endl;
    
    _sourceData = SourceData::readData(this);
    
    assignCoordinateSystemAndGeoTransformAccordingToParameterPolicy();
}    

void DataSet::Source::assignCoordinateSystemAndGeoTransformAccordingToParameterPolicy()
{
    if (getCoordinateSystemPolicy()==PREFER_CONFIG_SETTINGS)
    {
        _sourceData->_cs = _cs;
        
        my_notify(osg::INFO)<<"assigning CS from Source to Data."<<std::endl;
        
    }
    else
    {
        _cs = _sourceData->_cs;
        my_notify(osg::INFO)<<"assigning CS from Data to Source."<<std::endl;
    }
    
    if (getGeoTransformPolicy()==PREFER_CONFIG_SETTINGS)
    {
        _sourceData->_geoTransform = _geoTransform;

        my_notify(osg::INFO)<<"assigning GeoTransform from Source to Data."<<_geoTransform<<std::endl;

    }
    else if (getGeoTransformPolicy()==PREFER_CONFIG_SETTINGS_BUT_SCALE_BY_FILE_RESOLUTION)
    {
    
        // scale the x and y axis.
        double div_x = 1.0/(double)(_sourceData->_numValuesX - 1);
        double div_y = 1.0/(double)(_sourceData->_numValuesY - 1);
    
        _geoTransform(0,0) *= div_x;
        _geoTransform(1,0) *= div_x;
        _geoTransform(2,0) *= div_x;
    
        _geoTransform(0,1) *= div_y;
        _geoTransform(1,1) *= div_y;
        _geoTransform(2,1) *= div_y;

        _sourceData->_geoTransform = _geoTransform;

        my_notify(osg::INFO)<<"assigning GeoTransform from Source to Data."<<_geoTransform<<std::endl;

    }
    else
    {
        _geoTransform = _sourceData->_geoTransform;
        my_notify(osg::INFO)<<"assigning GeoTransform from Data to Source."<<_geoTransform<<std::endl;
    }
    
    _sourceData->computeExtents();
    
    _extents = _sourceData->_extents;
}

bool DataSet::Source::needReproject(const osg::CoordinateSystemNode* cs) const
{
    return needReproject(cs,0.0,0.0);
}

bool DataSet::Source::needReproject(const osg::CoordinateSystemNode* cs, double minResolution, double maxResolution) const
{
    if (!_sourceData) return false;
    
    // handle modles by using a matrix transform only.
    if (_type==MODEL) return false;
    
    // always need to reproject imagery with GCP's.
    if (_sourceData->_hasGCPs)
    {
        my_notify(osg::INFO)<<"Need to to reproject due to presence of GCP's"<<std::endl;
        return true;
    }

    if (!areCoordinateSystemEquivalent(_cs.get(),cs))
    {
        my_notify(osg::INFO)<<"Need to do reproject !areCoordinateSystemEquivalent(_cs.get(),cs)"<<std::endl;

        return true;
    }
     
    if (minResolution==0.0 && maxResolution==0.0) return false;

    // now check resolutions.
    const osg::Matrixd& m = _sourceData->_geoTransform;
    double currentResolution = sqrt(osg::square(m(0,0))+osg::square(m(1,0))+
                                    osg::square(m(0,1))+osg::square(m(1,1)));
                                   
    if (currentResolution<minResolution) return true;
    if (currentResolution>maxResolution) return true;

    return false;
}

DataSet::Source* DataSet::Source::doReproject(const std::string& filename, osg::CoordinateSystemNode* cs, double targetResolution) const
{
    // return nothing when repoject is inappropriate.
    if (!_sourceData) return 0;
    if (_type==MODEL) return 0;
    
    my_notify(osg::INFO)<<"reprojecting to file "<<filename<<std::endl;

    GDALDriverH hDriver = GDALGetDriverByName( "GTiff" );
        
    if (hDriver == NULL)
    {       
    my_notify(osg::INFO)<<"Unable to load driver for "<<"GTiff"<<std::endl;
        return 0;
    }
    
    if (GDALGetMetadataItem( hDriver, GDAL_DCAP_CREATE, NULL ) == NULL )
    {
        my_notify(osg::INFO)<<"GDAL driver does not support create for "<<osgDB::getFileExtension(filename)<<std::endl;
        return 0;
    }

/* -------------------------------------------------------------------- */
/*      Create a transformation object from the source to               */
/*      destination coordinate system.                                  */
/* -------------------------------------------------------------------- */
    void *hTransformArg = 
         GDALCreateGenImgProjTransformer( _sourceData->_gdalDataset,_sourceData->_cs->getCoordinateSystem().c_str(),
                                          NULL, cs->getCoordinateSystem().c_str(),
                                          TRUE, 0.0, 1 );

    if (!hTransformArg)
    {
        my_notify(osg::INFO)<<" failed to create transformer"<<std::endl;
        return 0;
    }

    double adfDstGeoTransform[6];
    int nPixels=0, nLines=0;
    if( GDALSuggestedWarpOutput( _sourceData->_gdalDataset, 
                                 GDALGenImgProjTransform, hTransformArg, 
                                 adfDstGeoTransform, &nPixels, &nLines )
        != CE_None )
    {
        my_notify(osg::INFO)<<" failed to create warp"<<std::endl;
        return 0;
    }
    
    if (targetResolution>0.0f)
    {
        my_notify(osg::INFO)<<"recomputing the target transform size"<<std::endl;
        
        double currentResolution = sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2])+
                                        osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5]));

        my_notify(osg::INFO)<<"        default computed resolution "<<currentResolution<<" nPixels="<<nPixels<<" nLines="<<nLines<<std::endl;

        double extentsPixels = sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2]))*(double)(nPixels-1);
        double extentsLines = sqrt(osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5]))*(double)(nLines-1);
                                        
        double ratio = targetResolution/currentResolution;
        adfDstGeoTransform[1] *= ratio;
        adfDstGeoTransform[2] *= ratio;
        adfDstGeoTransform[4] *= ratio;
        adfDstGeoTransform[5] *= ratio;
        
        my_notify(osg::INFO)<<"    extentsPixels="<<extentsPixels<<std::endl;
        my_notify(osg::INFO)<<"    extentsLines="<<extentsLines<<std::endl;
        my_notify(osg::INFO)<<"    targetResolution="<<targetResolution<<std::endl;
        
        nPixels = (int)ceil(extentsPixels/sqrt(osg::square(adfDstGeoTransform[1])+osg::square(adfDstGeoTransform[2])))+1;
        nLines = (int)ceil(extentsLines/sqrt(osg::square(adfDstGeoTransform[4])+osg::square(adfDstGeoTransform[5])))+1;

        my_notify(osg::INFO)<<"        target computed resolution "<<targetResolution<<" nPixels="<<nPixels<<" nLines="<<nLines<<std::endl;
        
    }

    
    GDALDestroyGenImgProjTransformer( hTransformArg );

    GDALDataType eDT = GDALGetRasterDataType(GDALGetRasterBand(_sourceData->_gdalDataset,1));
    

/* --------------------------------------------------------------------- */
/*    Create the file                                                    */
/* --------------------------------------------------------------------- */

    int numSourceBands = GDALGetRasterCount(_sourceData->_gdalDataset);
    int numDestinationBands = (numSourceBands >= 3) ? 4 : numSourceBands; // expand RGB to RGBA, but leave other formats unchanged

    GDALDatasetH hDstDS = GDALCreate( hDriver, filename.c_str(), nPixels, nLines, 
                         numDestinationBands , eDT,
                         0 );
    
    if( hDstDS == NULL )
        return NULL;
        
        

/* -------------------------------------------------------------------- */
/*      Write out the projection definition.                            */
/* -------------------------------------------------------------------- */
    GDALSetProjection( hDstDS, cs->getCoordinateSystem().c_str() );
    GDALSetGeoTransform( hDstDS, adfDstGeoTransform );


// Set up the transformer along with the new datasets.

    hTransformArg = 
         GDALCreateGenImgProjTransformer( _sourceData->_gdalDataset,_sourceData->_cs->getCoordinateSystem().c_str(),
                                          hDstDS, cs->getCoordinateSystem().c_str(),
                                          TRUE, 0.0, 1 );

    GDALTransformerFunc pfnTransformer = GDALGenImgProjTransform;

    
    my_notify(osg::INFO)<<"Setting projection "<<cs->getCoordinateSystem()<<std::endl;

/* -------------------------------------------------------------------- */
/*      Copy the color table, if required.                              */
/* -------------------------------------------------------------------- */
    GDALColorTableH hCT;

    hCT = GDALGetRasterColorTable( GDALGetRasterBand(_sourceData->_gdalDataset,1) );
    if( hCT != NULL )
        GDALSetRasterColorTable( GDALGetRasterBand(hDstDS,1), hCT );

/* -------------------------------------------------------------------- */
/*      Setup warp options.                                             */
/* -------------------------------------------------------------------- */
    GDALWarpOptions *psWO = GDALCreateWarpOptions();

    psWO->hSrcDS = _sourceData->_gdalDataset;
    psWO->hDstDS = hDstDS;

    psWO->pfnTransformer = pfnTransformer;
    psWO->pTransformerArg = hTransformArg;

    if(osg::isNotifyEnabled(osg::NotifySeverity(osg::INFO + getNotifyOffset())))
      psWO->pfnProgress = GDALTermProgress;
    else 
      psWO->pfnProgress = GDALDummyProgress;
      
/* -------------------------------------------------------------------- */
/*      Setup band mapping.                                             */
/* -------------------------------------------------------------------- */
    psWO->nBandCount = numSourceBands;//numDestinationBands;
    psWO->panSrcBands = (int *) CPLMalloc(numDestinationBands*sizeof(int));
    psWO->panDstBands = (int *) CPLMalloc(numDestinationBands*sizeof(int));

    int i;
    for(i = 0; i < psWO->nBandCount; i++ )
    {
        psWO->panSrcBands[i] = i+1;
        psWO->panDstBands[i] = i+1;
    }


/* -------------------------------------------------------------------- */
/*      Setup no datavalue                                              */
/* -----------------------------------------------------`--------------- */

    psWO->padfSrcNoDataReal = (double*) CPLMalloc(psWO->nBandCount*sizeof(double));
    psWO->padfSrcNoDataImag = (double*) CPLMalloc(psWO->nBandCount*sizeof(double));

    psWO->padfDstNoDataReal = (double*) CPLMalloc(psWO->nBandCount*sizeof(double));
    psWO->padfDstNoDataImag = (double*) CPLMalloc(psWO->nBandCount*sizeof(double));

    for(i = 0; i < psWO->nBandCount; i++ )
    {
        int success = 0;
        GDALRasterBand* band = (i<numSourceBands) ? _sourceData->_gdalDataset->GetRasterBand(i+1) : 0;
        double noDataValue = band ? band->GetNoDataValue(&success) : 0.0;
        double new_noDataValue = 0;
        if (success)
        {
            my_notify(osg::INFO)<<"\tassinging no data value "<<noDataValue<<" to band "<<i+1<<std::endl;

            psWO->padfSrcNoDataReal[i] = noDataValue;
            psWO->padfSrcNoDataImag[i] = 0.0;
            psWO->padfDstNoDataReal[i] = new_noDataValue;
            psWO->padfDstNoDataImag[i] = 0.0;

            GDALRasterBandH dest_band = GDALGetRasterBand(hDstDS,i+1);
            GDALSetRasterNoDataValue( dest_band, new_noDataValue);
        }
        else
        {
            psWO->padfSrcNoDataReal[i] = 0.0;
            psWO->padfSrcNoDataImag[i] = 0.0;
            psWO->padfDstNoDataReal[i] = new_noDataValue;
            psWO->padfDstNoDataImag[i] = 0.0;

            GDALRasterBandH dest_band = GDALGetRasterBand(hDstDS,i+1);
            GDALSetRasterNoDataValue( dest_band, new_noDataValue);
        }
    }    

    psWO->papszWarpOptions = (char**)CPLMalloc(2*sizeof(char*));
    psWO->papszWarpOptions[0] = strdup("INIT_DEST=NO_DATA");
    psWO->papszWarpOptions[1] = 0;
    
    if (numDestinationBands==4)
    {
/*    
        GDALSetRasterColorInterpretation( 
            GDALGetRasterBand( hDstDS, numDestinationBands ), 
            GCI_AlphaBand );
*/            
        psWO->nDstAlphaBand = numDestinationBands;
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

    my_notify(osg::INFO)<<"new projection is "<<GDALGetProjectionRef(hDstDS)<<std::endl;

/* -------------------------------------------------------------------- */
/*      Cleanup.                                                        */
/* -------------------------------------------------------------------- */
    GDALDestroyGenImgProjTransformer( hTransformArg );
    
#if 0
    int anOverviewList[4] = { 2, 4, 8, 16 };
    GDALBuildOverviews( hDstDS, "AVERAGE", 4, anOverviewList, 0, NULL, 
                            GDALTermProgress/*GDALDummyProgress*/, NULL );
#endif

    GDALClose( hDstDS );
    
    Source* newSource = new Source;
    newSource->_type = _type;
    newSource->_filename = filename;
    newSource->_temporaryFile = true;
    newSource->_cs = cs;

    newSource->_coordinateSystemPolicy = _coordinateSystemPolicy;
    newSource->_geoTransformPolicy = _geoTransformPolicy;

    newSource->_minLevel = _minLevel;
    newSource->_maxLevel = _maxLevel;
    newSource->_layer = _layer;

    newSource->_requiredResolutions = _requiredResolutions;

    newSource->_numValuesX = nPixels;
    newSource->_numValuesY = nLines;
    newSource->_geoTransform.set( adfDstGeoTransform[1],    adfDstGeoTransform[4],      0.0,    0.0,
                                  adfDstGeoTransform[2],    adfDstGeoTransform[5],      0.0,    0.0,
                                  0.0,                      0.0,                        1.0,    0.0,
                                  adfDstGeoTransform[0],    adfDstGeoTransform[3],      0.0,    1.0);

    newSource->computeExtents();

    // reload the newly created file.
    newSource->loadSourceData();
                              
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

    if (_sourceData.valid() && _sourceData->_gdalDataset )
    {

        int anOverviewList[4] = { 2, 4, 8, 16 };
        GDALBuildOverviews( _sourceData->_gdalDataset, "AVERAGE", 4, anOverviewList, 0, NULL, 
                                GDALTermProgress/*GDALDummyProgress*/, NULL );

    }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////


DataSet::DestinationTile::DestinationTile():
    _dataSet(0),
    _level(0),
    _tileX(0),
    _tileY(0),
    _pixelFormat(GL_RGB),
    _maxSourceLevel(0),
    _imagery_maxNumColumns(4096),
    _imagery_maxNumRows(4096),
    _terrain_maxNumColumns(1024),
    _terrain_maxNumRows(1024),
    _terrain_maxSourceResolutionX(0.0f),
    _terrain_maxSourceResolutionY(0.0f),
    _complete(false)
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
        Source* source = itr->get();
        if (!source || source->getMaxLevel()<_level)
        {
            // skip the contribution of this source since this destination tile exceeds its contribution level.
            continue;
        }

        SourceData* data = source->getSourceData();
        if (data && source->getType()!=Source::MODEL)
        {

            SpatialProperties sp = data->computeSpatialProperties(_cs.get());

            if (!sp._extents.intersects(_extents))
            {
                // skip this source since it doesn't overlap this tile.
                continue;
            }

            
            if (sp._numValuesX!=0 && sp._numValuesY!=0)
            {
                _maxSourceLevel = osg::maximum((*itr)->getMaxLevel(),_maxSourceLevel);

                float sourceResolutionX = (sp._extents.xMax()-sp._extents.xMin())/(float)sp._numValuesX;
                float sourceResolutionY = (sp._extents.yMax()-sp._extents.yMin())/(float)sp._numValuesY;

                switch((*itr)->getType())
                {
                    case(Source::IMAGE):
                    {
                        ImageData& imageData = getImageData(source->getLayer());
                        if (imageData._imagery_maxSourceResolutionX==0.0f) imageData._imagery_maxSourceResolutionX=sourceResolutionX;
                        else imageData._imagery_maxSourceResolutionX=osg::minimum(imageData._imagery_maxSourceResolutionX,sourceResolutionX);
                        if (imageData._imagery_maxSourceResolutionY==0.0f) imageData._imagery_maxSourceResolutionY=sourceResolutionY;
                        else imageData._imagery_maxSourceResolutionY=osg::minimum(imageData._imagery_maxSourceResolutionY,sourceResolutionY);
                        break;
                    }
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


bool DataSet::DestinationTile::computeImageResolution(unsigned int layer, unsigned int& numColumns, unsigned int& numRows, double& resX, double& resY)
{
    ImageData& imageData = getImageData(layer);
    if (imageData._imagery_maxSourceResolutionX!=0.0f && imageData._imagery_maxSourceResolutionY!=0.0f &&
        _imagery_maxNumColumns!=0 && _imagery_maxNumRows!=0)
    {
        unsigned int numColumnsAtFullRes = 1+(unsigned int)ceilf((_extents.xMax()-_extents.xMin())/imageData._imagery_maxSourceResolutionX);
        unsigned int numRowsAtFullRes = 1+(unsigned int)ceilf((_extents.yMax()-_extents.yMin())/imageData._imagery_maxSourceResolutionY);
        unsigned int numColumnsRequired = osg::minimum(_imagery_maxNumColumns,numColumnsAtFullRes);
        unsigned int numRowsRequired    = osg::minimum(_imagery_maxNumRows,numRowsAtFullRes);

        // use a minimum image size of 4x4 to avoid mipmap generation problems in OpenGL at sizes at 2x2. 
        numColumns = 4;
        numRows = 4;
        
        // round to nearest power of two above or equal to the required resolution
        while (numColumns<numColumnsRequired) numColumns *= 2;
        while (numRows<numRowsRequired) numRows *= 2;
        
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
    for(unsigned int layerNum=0;
        layerNum<_imagery.size();
        ++layerNum)
    {
        if (computeImageResolution(layerNum,texture_numColumns,texture_numRows,texture_dx,texture_dy))
        {

            ImageData& imageData = getImageData(layerNum);

            imageData._imagery = new DestinationData(_dataSet);
            imageData._imagery->_cs = _cs;
            imageData._imagery->_extents = _extents;
            imageData._imagery->_geoTransform.set(texture_dx,      0.0,               0.0,0.0,
                                        0.0,             -texture_dy,       0.0,0.0,
                                        0.0,             0.0,               1.0,1.0,
                                        _extents.xMin(), _extents.yMax(),   0.0,1.0);


            imageData._imagery->_image = new osg::Image;

            std::string imageName(_name+_dataSet->getDestinationImageExtension());
            imageData._imagery->_image->setFileName(imageName.c_str());

            imageData._imagery->_image->allocateImage(texture_numColumns,texture_numRows,1,_pixelFormat,GL_UNSIGNED_BYTE);
            unsigned char* data = imageData._imagery->_image->data();
            unsigned int totalSize = imageData._imagery->_image->getTotalSizeInBytesIncludingMipmaps();
            for(unsigned int i=0;i<totalSize;++i)
            {
                *(data++) = 0;
            }
        }
    }

    unsigned int dem_numColumns, dem_numRows;
    double dem_dx, dem_dy;
    if (computeTerrainResolution(dem_numColumns,dem_numRows,dem_dx,dem_dy))
    {
        _terrain = new DestinationData(_dataSet);
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
        //my_notify(osg::INFO)<<"ErrorX = "<<xMax-_extents.xMax()<<std::endl;

        //float yMax = _terrain->_heightField->getOrigin().y()+_terrain->_heightField->getYInterval()*(float)(dem_numRows-1);
        //my_notify(osg::INFO)<<"ErrorY = "<<yMax-_extents.yMax()<<std::endl;

    }

}

void DataSet::DestinationTile::computeNeighboursFromQuadMap()
{
    if (_dataSet)
    {
        setNeighbours(_dataSet->getTile(_level,_tileX-1,_tileY),_dataSet->getTile(_level,_tileX-1,_tileY-1),
                      _dataSet->getTile(_level,_tileX,_tileY-1),_dataSet->getTile(_level,_tileX+1,_tileY-1),
                      _dataSet->getTile(_level,_tileX+1,_tileY),_dataSet->getTile(_level,_tileX+1,_tileY+1),
                      _dataSet->getTile(_level,_tileX,_tileY+1),_dataSet->getTile(_level,_tileX-1,_tileY+1));
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
    
    
//     my_notify(osg::INFO)<<"LEFT="<<_neighbour[LEFT]<<std::endl;
//     my_notify(osg::INFO)<<"LEFT_BELOW="<<_neighbour[LEFT_BELOW]<<std::endl;
//     my_notify(osg::INFO)<<"BELOW="<<_neighbour[BELOW]<<std::endl;
//     my_notify(osg::INFO)<<"BELOW_RIGHT="<<_neighbour[BELOW_RIGHT]<<std::endl;
//     my_notify(osg::INFO)<<"RIGHT="<<_neighbour[RIGHT]<<std::endl;
//     my_notify(osg::INFO)<<"RIGHT_ABOVE="<<_neighbour[RIGHT_ABOVE]<<std::endl;
//     my_notify(osg::INFO)<<"ABOVE="<<_neighbour[ABOVE]<<std::endl;
//     my_notify(osg::INFO)<<"ABOVE_LEFT="<<_neighbour[ABOVE_LEFT]<<std::endl;
    
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
            my_notify(osg::INFO)<<"Error:: Tile "<<this<<"'s _neighbour["<<i<<"] does not point back to it."<<std::endl;
        }
    }
}

void DataSet::DestinationTile::allocateEdgeNormals()
{
    osg::HeightField* hf = _terrain->_heightField.get();
    if (!hf) return;
    
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
    

    for(unsigned int layerNum=0;
        layerNum<_imagery.size();
        ++layerNum)
    {
    
        typedef std::pair<osg::Image*,Position> ImageCornerPair;
        typedef std::vector<ImageCornerPair> ImageCornerList;

        ImageCornerList imagesToProcess;

        for(itr=cornersToProcess.begin();
            itr!=cornersToProcess.end();
            ++itr)
        {
            TileCornerPair& tcp = *itr;
            if (layerNum<tcp.first->_imagery.size())
            {
                ImageData& imageData = tcp.first->_imagery[layerNum];
                if (imageData._imagery.valid() && imageData._imagery->_image.valid())
                {
                    imagesToProcess.push_back(ImageCornerPair(imageData._imagery->_image.get(),tcp.second));
                }
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
    }    
    
    typedef std::pair<osg::HeightField*,TileCornerPair> HeightFieldCornerPair;
    typedef std::vector<HeightFieldCornerPair> HeightFieldCornerList;
    HeightFieldCornerList heightFieldsToProcess;
    
    for(itr=cornersToProcess.begin();
        itr!=cornersToProcess.end();
        ++itr)
    {
        TileCornerPair& tcp = *itr;
        if (tcp.first->_terrain.valid() && tcp.first->_terrain->_heightField.valid())
        {
            heightFieldsToProcess.push_back(HeightFieldCornerPair(tcp.first->_terrain->_heightField.get(),tcp));
        }
    }


    if (heightFieldsToProcess.size()>1)
    {
        float height = 0;
        osg::Vec2 heightDelta;
        // accumulate heights & normals
        HeightFieldCornerList::iterator hitr;
        for(hitr=heightFieldsToProcess.begin();
            hitr!=heightFieldsToProcess.end();
            ++hitr)
        {
            HeightFieldCornerPair& hfcp = *hitr;
            switch(hfcp.second.second)
            {
            case LEFT_BELOW:
                height += hfcp.first->getHeight(0,0);
                heightDelta += hfcp.first->getHeightDelta(0,0);
                break;
            case BELOW_RIGHT:
                height += hfcp.first->getHeight(hfcp.first->getNumColumns()-1,0);
                heightDelta += hfcp.first->getHeightDelta(hfcp.first->getNumColumns()-1,0);
                break;
            case RIGHT_ABOVE:
                height += hfcp.first->getHeight(hfcp.first->getNumColumns()-1,hfcp.first->getNumRows()-1);
                heightDelta += hfcp.first->getHeightDelta(hfcp.first->getNumColumns()-1,hfcp.first->getNumRows()-1);
                break;
            case ABOVE_LEFT:
                height += hfcp.first->getHeight(0,hfcp.first->getNumRows()-1);
                heightDelta += hfcp.first->getHeightDelta(0,hfcp.first->getNumRows()-1);
                break;
            default :
                break;
            }
        }
        
        // divide them.
        height /= heightFieldsToProcess.size();
        heightDelta /= heightFieldsToProcess.size();


        // apply height and normals to corners.
        for(hitr=heightFieldsToProcess.begin();
            hitr!=heightFieldsToProcess.end();
            ++hitr)
        {
            HeightFieldCornerPair& hfcp = *hitr;
            TileCornerPair& tcp = hfcp.second;
            switch(tcp.second)
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
            tcp.first->_heightDeltas[tcp.second].clear();
            tcp.first->_heightDeltas[tcp.second].push_back(heightDelta);
        }
    }

}

const char* edgeString(DataSet::DestinationTile::Position position)
{
    switch(position)
    {
        case DataSet::DestinationTile::LEFT: return "left";
        case DataSet::DestinationTile::BELOW: return "below";
        case DataSet::DestinationTile::RIGHT: return "right";
        case DataSet::DestinationTile::ABOVE: return "above";
        default : return "<not an edge>";
    }    
}

void DataSet::DestinationTile::setTileComplete(bool complete)
{
    _complete = complete;
    my_notify(osg::INFO)<<"setTileComplete("<<complete<<") for "<<_level<<"\t"<<_tileX<<"\t"<<_tileY<<std::endl;
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
    
    for(unsigned int layerNum=0;
        layerNum<_imagery.size();
        ++layerNum)
    {
        // do we have a image to equalize?
        if (!_imagery[layerNum]._imagery.valid()) continue;
        
        // does the neighbouring tile have an image to equalize?
        if (layerNum>=tile2->_imagery.size()) continue;
        if (!(tile2->_imagery[layerNum]._imagery.valid())) continue;
    

        osg::Image* image1 = _imagery[layerNum]._imagery->_image.get();
        osg::Image* image2 = tile2->_imagery[layerNum]._imagery->_image.get();

        //my_notify(osg::INFO)<<"Equalizing edge "<<edgeString(position)<<" of \t"<<_level<<"\t"<<_tileX<<"\t"<<_tileY
        //         <<"  neighbour "<<tile2->_level<<"\t"<<tile2->_tileX<<"\t"<<tile2->_tileY<<std::endl;


    //   if (_tileY==0) return;

        if (image1 && image2 && 
            image1->getPixelFormat()==image2->getPixelFormat() &&
            image1->getDataType()==image2->getDataType() &&
            image1->getPixelFormat()==GL_RGB &&
            image1->getDataType()==GL_UNSIGNED_BYTE)
        {

            //my_notify(osg::INFO)<<"   Equalizing image1= "<<image1<<                         " with image2 = "<<image2<<std::endl;
            //my_notify(osg::INFO)<<"              data1 = 0x"<<std::hex<<(int)image1->data()<<" with data2  = 0x"<<(int)image2->data()<<std::endl;

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
                //my_notify(osg::INFO)<<"       left "<<num<<std::endl;
                break;
            case BELOW:
                data1 = image1->data(1,0); // BELOW hand side
                delta1 = 3;
                data2 = image2->data(1,image2->t()-1); // ABOVE hand side
                delta2 = 3;
                num = (image1->s()==image2->s())?image2->s()-2:0; // note miss out corners.
                //my_notify(osg::INFO)<<"       below "<<num<<std::endl;
                break;
            case RIGHT:
                data1 = image1->data(image1->s()-1,1); // LEFT hand side
                delta1 = image1->getRowSizeInBytes();
                data2 = image2->data(0,1); // RIGHT hand side
                delta2 = image2->getRowSizeInBytes();
                num = (image1->t()==image2->t())?image2->t()-2:0; // note miss out corners.
                //my_notify(osg::INFO)<<"       right "<<num<<std::endl;
                break;
            case ABOVE:
                data1 = image1->data(1,image1->t()-1); // ABOVE hand side
                delta1 = 3;
                data2 = image2->data(1,0); // BELOW hand side
                delta2 = 3;
                num = (image1->s()==image2->s())?image2->s()-2:0; // note miss out corners.
                //my_notify(osg::INFO)<<"       above "<<num<<std::endl;
                break;
            default :
                //my_notify(osg::INFO)<<"       default "<<num<<std::endl;
                break;
            }

            for(int i=0;i<num;++i)
            {
                unsigned char red =   (unsigned char)((((int)*data1+ (int)*data2)/2));
                unsigned char green = (unsigned char)((((int)*(data1+1))+ (int)(*(data2+1)))/2);
                unsigned char blue =  (unsigned char)((((int)*(data1+2))+ (int)(*(data2+2)))/2);
    #if 1
                *data1 = red;
                *(data1+1) = green;
                *(data1+2) = blue;

                *data2 = red;
                *(data2+1) = green;
                *(data2+2) = blue;
    #endif

    #if 0
                *data1 = 255;
                *(data1+1) = 0;
                *(data1+2) = 0;

                *data2 = 0;
                *(data2+1) = 0;
                *(data2+2) = 0;
    #endif
                data1 += delta1;
                data2 += delta2;

                //my_notify(osg::INFO)<<"    equalizing colour "<<(int)data1<<"  "<<(int)data2<<std::endl;

            }

        }
    }
    
    osg::HeightField* heightField1 = _terrain.valid()?_terrain->_heightField.get():0;
    osg::HeightField* heightField2 = tile2->_terrain.valid()?tile2->_terrain->_heightField.get():0;

    if (heightField1 && heightField2)
    {
        //my_notify(osg::INFO)<<"   Equalizing heightfield"<<std::endl;

        float* data1 = 0;
        float* data2 = 0;
        unsigned int delta1 = 0;
        unsigned int delta2 = 0;
        int num = 0;
        
        unsigned int i1 = 0;
        unsigned int j1 = 0;
        unsigned int i2 = 0;
        unsigned int j2 = 0;
        unsigned int deltai = 0;
        unsigned int deltaj = 0;

        switch(position)
        {
        case LEFT:
            i1 = 0;
            j1 = 1;
            i2 = heightField2->getNumColumns()-1;
            j2 = 1;
            deltai = 0;
            deltaj = 1;

            data1 = &(heightField1->getHeight(0,1)); // LEFT hand side
            delta1 = heightField1->getNumColumns();
            data2 = &(heightField2->getHeight(heightField2->getNumColumns()-1,1)); // RIGHT hand side
            delta2 = heightField2->getNumColumns();
            num = (heightField1->getNumRows()==heightField2->getNumRows())?heightField1->getNumRows()-2:0; // note miss out corners.
            break;

        case BELOW:
            i1 = 1;
            j1 = 0;
            i2 = 1;
            j2 = heightField2->getNumRows()-1;
            deltai = 1;
            deltaj = 0;

            data1 = &(heightField1->getHeight(1,0)); // BELOW hand side
            delta1 = 1;
            data2 = &(heightField2->getHeight(1,heightField2->getNumRows()-1)); // ABOVE hand side
            delta2 = 1;
            num = (heightField1->getNumColumns()==heightField2->getNumColumns())?heightField1->getNumColumns()-2:0; // note miss out corners.
            break;

        case RIGHT:
            i1 = heightField1->getNumColumns()-1;
            j1 = 1;
            i2 = 0;
            j2 = 1;
            deltai = 0;
            deltaj = 1;

            data1 = &(heightField1->getHeight(heightField1->getNumColumns()-1,1)); // LEFT hand side
            delta1 = heightField1->getNumColumns();
            data2 = &(heightField2->getHeight(0,1)); // LEFT hand side
            delta2 = heightField2->getNumColumns();
            num = (heightField1->getNumRows()==heightField2->getNumRows())?heightField1->getNumRows()-2:0; // note miss out corners.
            break;

        case ABOVE:
            i1 = 1;
            j1 = heightField1->getNumRows()-1;
            i2 = 1;
            j2 = 0;
            deltai = 1;
            deltaj = 0;

            data1 = &(heightField1->getHeight(1,heightField1->getNumRows()-1)); // ABOVE hand side
            delta1 = 1;
            data2 = &(heightField2->getHeight(1,0)); // BELOW hand side
            delta2 = 1;
            num = (heightField1->getNumColumns()==heightField2->getNumColumns())?heightField1->getNumColumns()-2:0; // note miss out corners.
            break;
        default :
            break;
        }
        
        _heightDeltas[position].clear();
        _heightDeltas[position].reserve(num);
        tile2->_heightDeltas[(position+4)%NUMBER_OF_POSITIONS].clear();
        tile2->_heightDeltas[(position+4)%NUMBER_OF_POSITIONS].reserve(num);

        for(int i=0;i<num;++i)
        {
            // equalize height
            float z = (*data1 + *data2)/2.0f;

            *data1 = z;
            *data2 = z;

            data1 += delta1;
            data2 += delta2;
            
            // equailize normals
            osg::Vec2 heightDelta = (heightField1->getHeightDelta(i1,j1) + 
                                    heightField2->getHeightDelta(i2,j2))*0.5f;
                               
            // pass the normals on to the tiles.
            _heightDeltas[position].push_back(heightDelta);
            tile2->_heightDeltas[(position+4)%NUMBER_OF_POSITIONS].push_back(heightDelta);

            i1 += deltai;
            i2 += deltai;
            j1 += deltaj;
            j2 += deltaj;
            
            

        }


    }

}

void DataSet::DestinationTile::equalizeBoundaries()
{
    my_notify(osg::INFO)<<"DataSet::DestinationTile::equalizeBoundaries()"<<std::endl;

    equalizeCorner(LEFT_BELOW);
    equalizeCorner(BELOW_RIGHT);
    equalizeCorner(RIGHT_ABOVE);
    equalizeCorner(ABOVE_LEFT);

    equalizeEdge(LEFT);
    equalizeEdge(BELOW);
    equalizeEdge(RIGHT);
    equalizeEdge(ABOVE);
}


void DataSet::DestinationTile::optimizeResolution()
{
    if (_terrain.valid() && _terrain->_heightField.valid())
    {
        osg::HeightField* hf = _terrain->_heightField.get();
    
        // compute min max of height field
        float minHeight = hf->getHeight(0,0);
        float maxHeight = minHeight;
        for(unsigned int r=0;r<hf->getNumRows();++r)
        {
            for(unsigned int c=0;c<hf->getNumColumns();++c)
            {
                float h = hf->getHeight(c,r);
                if (h<minHeight) minHeight = h;
                if (h>maxHeight) maxHeight = h;
            }
        }

        if (minHeight==maxHeight)
        {
            my_notify(osg::INFO)<<"******* We have a flat tile ******* "<<std::endl;

            unsigned int minimumSize = 8;

            unsigned int numColumns = minimumSize;
            unsigned int numRows = minimumSize;
            
            float ratio_y_over_x = (_extents.yMax()-_extents.yMin())/(_extents.xMax()-_extents.xMin());
            if (ratio_y_over_x > 1.2) numRows = (unsigned int)ceilf((float)numRows*ratio_y_over_x);
            else if (ratio_y_over_x < 0.8) numColumns = (unsigned int)ceilf((float)numColumns/ratio_y_over_x);
            
            
            hf->allocate(numColumns,numRows);
            hf->setOrigin(osg::Vec3(_extents.xMin(),_extents.yMin(),0.0f));
            hf->setXInterval((_extents.xMax()-_extents.xMin())/(float)(numColumns-1));
            hf->setYInterval((_extents.yMax()-_extents.yMin())/(float)(numRows-1));

            for(unsigned int r=0;r<numRows;++r)
            {
                for(unsigned int c=0;c<numColumns;++c)
                {
                    hf->setHeight(c,r,minHeight);
                } 
            } 
        }
    }
}

osg::Node* DataSet::DestinationTile::createScene()
{
    if (_dataSet->getGeometryType()==HEIGHT_FIELD)
    {
        return createHeightField();
    }
    else
    {
        return createPolygonal();
    }
}

osg::StateSet* DataSet::DestinationTile::createStateSet()
{
    if (_imagery.empty()) return 0;

    unsigned int numValidImagerLayers = 0;
    unsigned int layerNum;
    for(layerNum=0;
        layerNum<_imagery.size();
        ++layerNum)
    {
        if (_imagery[layerNum]._imagery.valid() && 
            _imagery[layerNum]._imagery->_image.valid())
        {
            ++numValidImagerLayers;
        }
    }
    
    if (numValidImagerLayers==0) return 0;

    osg::StateSet* stateset = new osg::StateSet;

    osg::Texture* baseTexture = 0;
    for(layerNum=0;
        layerNum<_imagery.size();
        ++layerNum)
    {
        ImageData& imageData = _imagery[layerNum];
        if (!imageData._imagery.valid() || !imageData._imagery->_image.valid()) continue;
        
        osg::Image* image = imageData._imagery->_image.get();

        std::string imageExension(".dds"); // ".rgb"
        //std::string imageExension(".jp2"); // ".rgb"
        std::string imageName = _name;
        if (layerNum>0)
        {
            imageName += "_l";
            imageName += char('0'+layerNum);
        }
        imageName += imageExension;
        image->setFileName(imageName.c_str());

        osg::Texture2D* texture = new osg::Texture2D;
        
        if (baseTexture==0) baseTexture=texture;
        
        texture->setImage(image);
        texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
        texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
        switch (_dataSet->getMipMappingMode())
        {
          case(DataSet::NO_MIP_MAPPING):
            {
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
                texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            }
            break;
          case(DataSet::MIP_MAPPING_HARDWARE):
            {
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            }
            break;
          case(DataSet::MIP_MAPPING_IMAGERY):
            {
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);
                texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
            }
            break;
        }        

        texture->setMaxAnisotropy(_dataSet->getMaxAnisotropy());
        stateset->setTextureAttributeAndModes(layerNum,texture,osg::StateAttribute::ON);

        bool inlineImageFile = _dataSet->getDestinationTileExtension()==".ive";
        bool compressedImageSupported = inlineImageFile || imageExension==".dds";
        bool mipmapImageSupported = compressedImageSupported; // inlineImageFile;
        
        int minumCompressedTextureSize = 64;
        int minumDXT3CompressedTextureSize = 256;
        
        if (compressedImageSupported && 
            image->s()>=minumCompressedTextureSize && image->t()>=minumCompressedTextureSize &&
            (_dataSet->getTextureType()==COMPRESSED_TEXTURE || _dataSet->getTextureType()==COMPRESSED_RGBA_TEXTURE) &&
            (image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA))
        {
        
            if (image->s()>=minumDXT3CompressedTextureSize && image->t()>=minumDXT3CompressedTextureSize)
                texture->setInternalFormatMode(osg::Texture::USE_S3TC_DXT3_COMPRESSION);
            else
                texture->setInternalFormatMode(osg::Texture::USE_S3TC_DXT5_COMPRESSION);

            // force the mip mapping off temporay if we intend the graphics hardware to do the mipmapping.
            if (_dataSet->getMipMappingMode()==DataSet::MIP_MAPPING_HARDWARE)
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);

            // get OpenGL driver to create texture from image.
            texture->apply(*(_dataSet->getState()));

            image->readImageFromCurrentTexture(0,true);

            // restore the mip mapping mode.
            if (_dataSet->getMipMappingMode()==DataSet::MIP_MAPPING_HARDWARE)
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR_MIPMAP_LINEAR);

            texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);


            texture->dirtyTextureObject();

            my_notify(osg::INFO)<<">>>>>>>>>>>>>>>compressed image.<<<<<<<<<<<<<<"<<std::endl;

        }
        else
        {
            if (_dataSet->getTextureType()==RGB_16 && image->getPixelFormat()==GL_RGB)
            {
                image->scaleImage(image->s(),image->t(),image->r(),GL_UNSIGNED_SHORT_5_6_5);
            }
            else if (_dataSet->getTextureType()==RGBA_16 && image->getPixelFormat()==GL_RGBA)
            {
                image->scaleImage(image->s(),image->t(),image->r(),GL_UNSIGNED_SHORT_5_5_5_1);
            }

            if (mipmapImageSupported && _dataSet->getMipMappingMode()==DataSet::MIP_MAPPING_IMAGERY)
            {

                osg::ref_ptr<osg::State> state = new osg::State;

                // get OpenGL driver to create texture from image.
                texture->apply(*(_dataSet->getState()));

                image->readImageFromCurrentTexture(0,true);

                texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);

                texture->dirtyTextureObject();

                my_notify(osg::INFO)<<">>>>>>>>>>>>>>>mip mapped image.<<<<<<<<<<<<<<"<<std::endl;

            }
        }
    }
    
    // now fill in any blank texture units.
    bool fillInAllTextureUnits = true;
    if (fillInAllTextureUnits && baseTexture)
    {
        for(layerNum=0;
            layerNum<_dataSet->getNumOfTextureLevels();
            ++layerNum)
        {
            bool applyBaseTexture = false;
            if (layerNum>=_imagery.size()) applyBaseTexture=true;
            else 
            {
                ImageData& imageData = _imagery[layerNum];
                if (!imageData._imagery.valid() || 
                    !imageData._imagery->_image.valid()) applyBaseTexture=true;
            }
            if (applyBaseTexture)        
                stateset->setTextureAttributeAndModes(layerNum,baseTexture,osg::StateAttribute::ON);
        }
    }
        
    return stateset;
}

osg::Node* DataSet::DestinationTile::createHeightField()
{
    osg::ShapeDrawable* shapeDrawable = 0;

    if (_terrain.valid() && _terrain->_heightField.valid())
    {
        my_notify(osg::INFO)<<"--- Have terrain build tile ----"<<std::endl;

        osg::HeightField* hf = _terrain->_heightField.get();
        
        shapeDrawable = new osg::ShapeDrawable(hf);

        hf->setSkirtHeight(shapeDrawable->getBound().radius()*0.01f);
    }
    else 
    {
        my_notify(osg::INFO)<<"**** No terrain to build tile from use flat terrain fallback ****"<<std::endl;
        // create a dummy height field to file in the gap
        osg::HeightField* hf = new osg::HeightField;
        hf->allocate(2,2);
        hf->setOrigin(osg::Vec3(_extents.xMin(),_extents.yMin(),0.0f));
        hf->setXInterval(_extents.xMax()-_extents.xMin());
        hf->setYInterval(_extents.yMax()-_extents.yMin());

        shapeDrawable = new osg::ShapeDrawable(hf);

        hf->setSkirtHeight(shapeDrawable->getBound().radius()*0.01f);
    }

    if (!shapeDrawable) return 0;

    osg::StateSet* stateset = createStateSet();
    if (stateset)
    {
        shapeDrawable->setStateSet(stateset);
    }
    else
    {
        shapeDrawable->setColor(_dataSet->getDefaultColor());
    }
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(shapeDrawable);
    
    return geode;

}


static osg::Vec3 computeLocalPosition(const osg::Matrixd& worldToLocal, double X, double Y, double Z)
{
    return osg::Vec3(X*worldToLocal(0,0) + Y*worldToLocal(1,0) + Z*worldToLocal(2,0) + worldToLocal(3,0),
                     X*worldToLocal(0,1) + Y*worldToLocal(1,1) + Z*worldToLocal(2,1) + worldToLocal(3,1),
                     X*worldToLocal(0,2) + Y*worldToLocal(1,2) + Z*worldToLocal(2,2) + worldToLocal(3,2));
}

static inline osg::Vec3 computeLocalSkirtVector(const osg::EllipsoidModel* et, const osg::HeightField* grid, unsigned int i, unsigned int j, float length, bool useLocalToTileTransform, const osg::Matrixd& localToWorld)
{ 
    // no local to tile transform + mapping from lat+longs to XYZ so we need to use
    // a rotatated skirt vector - use the gravity vector.
    double longitude = grid->getOrigin().x()+grid->getXInterval()*((double)(i));
    double latitude = grid->getOrigin().y()+grid->getYInterval()*((double)(j));
    double midZ = grid->getOrigin().z();
    double X,Y,Z;
    et->convertLatLongHeightToXYZ(osg::DegreesToRadians(latitude),osg::DegreesToRadians(longitude),midZ,X,Y,Z);
    osg::Vec3 gravitationVector = et->computeLocalUpVector(X,Y,Z);
    gravitationVector.normalize();

    if (useLocalToTileTransform) gravitationVector = osg::Matrixd::transform3x3(localToWorld,gravitationVector);

    return gravitationVector * -length;
}

osg::Node* DataSet::DestinationTile::createPolygonal()
{
    my_notify(osg::INFO)<<"--------- DataSet::DestinationTile::createDrawableGeometry() ------------- "<<std::endl;

    const osg::EllipsoidModel* et = _dataSet->getEllipsoidModel();
    bool mapLatLongsToXYZ = _dataSet->mapLatLongsToXYZ();
    bool useLocalToTileTransform = _dataSet->getUseLocalTileTransform();

    osg::ref_ptr<osg::HeightField> grid = 0;
    
    if (_terrain.valid() && _terrain->_heightField.valid())
    {
        my_notify(osg::INFO)<<"--- Have terrain build tile ----"<<std::endl;
        grid = _terrain->_heightField.get();
    }
    else
    {
        unsigned int minimumSize = 8;
        unsigned int numColumns = minimumSize;
        unsigned int numRows = minimumSize;
        
        if (mapLatLongsToXYZ)
        {
            float longitude_range = (_extents.xMax()-_extents.xMin());
            float latitude_range = (_extents.yMax()-_extents.yMin());
            
            if (longitude_range>45.0) numColumns = (unsigned int)ceilf((float)numColumns*sqrtf(longitude_range/45.0));
            if (latitude_range>45.0) numRows = (unsigned int)ceilf((float)numRows*sqrtf(latitude_range/45.0));
            
            my_notify(osg::INFO)<<"numColumns = "<<numColumns<<"  numRows="<<numRows<<std::endl;
        }
        else
        {
            float ratio_y_over_x = (_extents.yMax()-_extents.yMin())/(_extents.xMax()-_extents.xMin());
            if (ratio_y_over_x > 1.2) numRows = (unsigned int)ceilf((float)numRows*ratio_y_over_x);
            else if (ratio_y_over_x < 0.8) numColumns = (unsigned int)ceilf((float)numColumns/ratio_y_over_x);
        }

        grid = new osg::HeightField;
        grid->allocate(numColumns,numRows);
        grid->setOrigin(osg::Vec3(_extents.xMin(),_extents.yMin(),0.0f));
        grid->setXInterval((_extents.xMax()-_extents.xMin())/(float)(numColumns-1));
        grid->setYInterval((_extents.yMax()-_extents.yMin())/(float)(numRows-1));
    }

    if (!grid)
    {
        my_notify(osg::INFO)<<"**** No terrain to build tile from use flat terrain fallback ****"<<std::endl;
        
        return 0;
    }

    bool createSkirt = true;

    // compute sizes.
    unsigned int numColumns = grid->getNumColumns();
    unsigned int numRows = grid->getNumRows();
    unsigned int numVerticesInBody = numColumns*numRows;
    unsigned int numVerticesInSkirt = createSkirt ? numColumns*2 + numRows*2 - 4 : 0;
    unsigned int numVertices = numVerticesInBody+numVerticesInSkirt;


    // create the geometry.
    osg::Geometry* geometry = new osg::Geometry;
    
    osg::Vec3Array& v = *(new osg::Vec3Array(numVertices));
    osg::Vec2Array& t = *(new osg::Vec2Array(numVertices));
    osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

    color[0].set(255,255,255,255);

    osg::ref_ptr<osg::Vec3Array> n = new osg::Vec3Array(numVertices); // must use ref_ptr so the array isn't removed when smooothvisitor is used    
    
    float skirtRatio = _dataSet->getSkirtRatio();
    osg::Matrixd localToWorld;
    osg::Matrixd worldToLocal;
    osg::Vec3 skirtVector(0.0f,0.0f,0.0f);

    
    osg::Vec3 center_position(0.0f,0.0f,0.0f);
    osg::Vec3 center_normal(0.0f,0.0f,1.0f);
    osg::Vec3 transformed_center_normal(0.0f,0.0f,1.0f);
    double globe_radius = et ? et->getRadiusPolar() : 1.0;
    float skirtLength = _extents.radius()*skirtRatio;

    bool useClusterCullingCallback = mapLatLongsToXYZ;

    if (useLocalToTileTransform)
    {
        if (mapLatLongsToXYZ)
        {
            double midLong = grid->getOrigin().x()+grid->getXInterval()*((double)(numColumns-1))*0.5;
            double midLat = grid->getOrigin().y()+grid->getYInterval()*((double)(numRows-1))*0.5;
            double midZ = grid->getOrigin().z();
            et->computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(midLat),osg::DegreesToRadians(midLong),midZ,localToWorld);
            
            double minLong = grid->getOrigin().x();
            double minLat = grid->getOrigin().y();

            double minX,minY,minZ;
            et->convertLatLongHeightToXYZ(osg::DegreesToRadians(minLat),osg::DegreesToRadians(minLong),midZ, minX,minY,minZ);
            
            double midX,midY;
            et->convertLatLongHeightToXYZ(osg::DegreesToRadians(midLat),osg::DegreesToRadians(midLong),midZ, midX,midY,midZ);
            
            double length = sqrt((midX-minX)*(midX-minX) + (midY-minY)*(midY-minY)); 
            
            skirtLength = length*skirtRatio;
            skirtVector.set(0.0f,0.0f,-skirtLength);
            
            center_normal.set(midX,midY,midZ);
            center_normal.normalize();
            
            worldToLocal.invert(localToWorld);
            
            center_position = computeLocalPosition(worldToLocal,midX,midY,midZ);
            transformed_center_normal = osg::Matrixd::transform3x3(localToWorld,center_normal);
            
        }
        else
        {
            double midX = grid->getOrigin().x()+grid->getXInterval()*((double)(numColumns-1))*0.5;
            double midY = grid->getOrigin().y()+grid->getYInterval()*((double)(numRows-1))*0.5;
            double midZ = grid->getOrigin().z();
            localToWorld.makeTranslate(midX,midY,midZ);
            worldToLocal.invert(localToWorld);
            
            skirtVector.set(0.0f,0.0f,-skirtLength);
        }
        
    }
    else if (mapLatLongsToXYZ) 
    {
        // no local to tile transform + mapping from lat+longs to XYZ so we need to use
        // a rotatated skirt vector - use the gravity vector.
        double midLong = grid->getOrigin().x()+grid->getXInterval()*((double)(numColumns-1))*0.5;
        double midLat = grid->getOrigin().y()+grid->getYInterval()*((double)(numRows-1))*0.5;
        double midZ = grid->getOrigin().z();
        double X,Y,Z;
        et->convertLatLongHeightToXYZ(osg::DegreesToRadians(midLat),osg::DegreesToRadians(midLong),midZ,X,Y,Z);
        osg::Vec3 gravitationVector = et->computeLocalUpVector(X,Y,Z);
        gravitationVector.normalize();
        skirtVector = gravitationVector * skirtLength;
    }
    else
    {
        skirtVector.set(0.0f,0.0f,-skirtLength);
    }
    
    unsigned int vi=0;
    unsigned int r,c;
    
    // populate the vertex/normal/texcoord arrays from the grid.
    double orig_X = grid->getOrigin().x();
    double delta_X = grid->getXInterval();
    double orig_Y = grid->getOrigin().y();
    double delta_Y = grid->getYInterval();
    double orig_Z = grid->getOrigin().z();


    float min_dot_product = 1.0f;
    float max_cluster_culling_height = 0.0f;
    float max_cluster_culling_radius = 0.0f;

    for(r=0;r<numRows;++r)
    {
        for(c=0;c<numColumns;++c)
        {
            double X = orig_X + delta_X*(double)c;
            double Y = orig_Y + delta_Y*(double)r;
            double Z = orig_Z + grid->getHeight(c,r);
            double height = Z;

            if (mapLatLongsToXYZ)
            {
                et->convertLatLongHeightToXYZ(osg::DegreesToRadians(Y),osg::DegreesToRadians(X),Z,
                                             X,Y,Z);
            }

            if (useLocalToTileTransform)
            {
                v[vi] = computeLocalPosition(worldToLocal,X,Y,Z);
            }
            else
            {
                v[vi].set(X,Y,Z);
            }


            if (useClusterCullingCallback)
            {
                osg::Vec3 dv = v[vi] - center_position;
                double d = sqrt(dv.x()*dv.x() + dv.y()*dv.y() + dv.z()*dv.z());
                double theta = acos( globe_radius/ (globe_radius + fabs(height)) );
                double phi = 2.0 * asin (d*0.5/globe_radius); // d/globe_radius;
                double beta = theta+phi;
                double cutoff = osg::PI_2 - 0.1;
                //my_notify(osg::INFO)<<"theta="<<theta<<"\tphi="<<phi<<" beta "<<beta<<std::endl;
                if (phi<cutoff && beta<cutoff)
                {

                    float local_dot_product = -sin(theta + phi);
                    float local_m = globe_radius*( 1.0/ cos(theta+phi) - 1.0);
                    float local_radius = static_cast<float>(globe_radius * tan(beta)); // beta*globe_radius;
                    min_dot_product = osg::minimum(min_dot_product, local_dot_product);
                    max_cluster_culling_height = osg::maximum(max_cluster_culling_height,local_m);      
                    max_cluster_culling_radius = osg::maximum(max_cluster_culling_radius,local_radius);
                }
                else
                {
                    //my_notify(osg::INFO)<<"Turning off cluster culling for wrap around tile."<<std::endl;
                    useClusterCullingCallback = false;
                }
            }

            // note normal will need rotating.
            if (n.valid())
            {
                (*n)[vi] = grid->getNormal(c,r);
                
            }

            t[vi].x() = (c==numColumns-1)? 1.0f : (float)(c)/(float)(numColumns-1);
            t[vi].y() = (r==numRows-1)? 1.0f : (float)(r)/(float)(numRows-1);

            ++vi;
            
        }
    }
    


    //geometry->setUseDisplayList(false);
    geometry->setVertexArray(&v);

    if (n.valid())
    {
        geometry->setNormalArray(n.get());
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }

    geometry->setColorArray(&color);
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    bool fillInAllTextureUnits = true;
    if (fillInAllTextureUnits)
    {
        for(unsigned int layerNum=0;
            layerNum<_dataSet->getNumOfTextureLevels();
            ++layerNum)
        {
            geometry->setTexCoordArray(layerNum,&t);
        }
    }
    else
    {
        for(unsigned int layerNum=0;
            layerNum<_imagery.size();
            ++layerNum)
        {
            ImageData& imageData = _imagery[layerNum];
            if (imageData._imagery.valid() && imageData._imagery->_image.valid()) 
            {
                geometry->setTexCoordArray(layerNum,&t);
            }
        }
    }
    
    osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_TRIANGLES,2*3*(numColumns-1)*(numRows-1)));
    geometry->addPrimitiveSet(&drawElements);
    int ei=0;
    for(r=0;r<numRows-1;++r)
    {
        for(c=0;c<numColumns-1;++c)
        {
            unsigned short i00 = (r)*numColumns+c;
            unsigned short i10 = (r)*numColumns+c+1;
            unsigned short i01 = (r+1)*numColumns+c;
            unsigned short i11 = (r+1)*numColumns+c+1;

            float diff_00_11 = fabsf(v[i00].z()-v[i11].z());
            float diff_01_10 = fabsf(v[i01].z()-v[i10].z());
            if (diff_00_11<diff_01_10)
            {
                // diagonal between 00 and 11
                drawElements[ei++] = i00;
                drawElements[ei++] = i10;
                drawElements[ei++] = i11;

                drawElements[ei++] = i00;
                drawElements[ei++] = i11;
                drawElements[ei++] = i01;
            }
            else
            {
                // diagonal between 01 and 10
                drawElements[ei++] = i01;
                drawElements[ei++] = i00;
                drawElements[ei++] = i10;

                drawElements[ei++] = i01;
                drawElements[ei++] = i10;
                drawElements[ei++] = i11;
            }
            }
    }

#if 1
    osgUtil::SmoothingVisitor sv;
    sv.smooth(*geometry);  // this will replace the normal vector with a new one

    // now we have to reassign the normals back to the orignal pointer.
    n = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
    if (n.valid() && n->size()!=numVertices) n->resize(numVertices);
#endif
    // now apply the normals computed through equalization
    for(unsigned int position=0; position<NUMBER_OF_POSITIONS; ++position)
    {
        if (!_heightDeltas[position].empty())
        {
            // we have normal to apply
            unsigned int i=0;
            unsigned int j=0;
            unsigned int deltai=0;
            unsigned int deltaj=0;
            switch(position)
            {
                case LEFT:
                    i = 0;
                    j = 1;
                    deltai = 0;
                    deltaj = 1;
                    break;
                case LEFT_BELOW:
                    i = 0;
                    j = 0;
                    deltai = 0;
                    deltaj = 0;
                    break;
                case BELOW:
                    i = 1;
                    j = 0;
                    deltai = 1;
                    deltaj = 0;
                    break;
                case BELOW_RIGHT:
                    i = numColumns-1;
                    j = 0;
                    deltai = 0;
                    deltaj = 0;
                    break;
                case RIGHT:
                    i = numColumns-1;
                    j = 1;
                    deltai = 0;
                    deltaj = 1;
                    break;
                case RIGHT_ABOVE:
                    i = numColumns-1;
                    j = numRows-1;
                    deltai = 0;
                    deltaj = 0;
                    break;
                case ABOVE:
                    i = 1;
                    j = numRows-1;
                    deltai = 1;
                    deltaj = 0;
                    break;
                case ABOVE_LEFT:
                    i = 0;
                    j = numRows-1;
                    deltai = 0;
                    deltaj = 0;
                    break;
            }
            

            // need to reproject normals.
            for(HeightDeltaList::iterator itr = _heightDeltas[position].begin();
                itr != _heightDeltas[position].end();
                ++itr, i += deltai, j += deltaj)
            {
                osg::Vec3& normal = (*n)[i + j*numColumns];
                osg::Vec2 heightDelta = *itr;

                if (mapLatLongsToXYZ)
                {
                
                    double X = orig_X + delta_X*(double)i;
                    double Y = orig_Y + delta_Y*(double)j;
                    double Z = orig_Z + grid->getHeight(i,j);
                    osg::Matrixd normalLocalToWorld;
                    et->computeLocalToWorldTransformFromLatLongHeight(osg::DegreesToRadians(Y),osg::DegreesToRadians(X),Z,normalLocalToWorld);
                    osg::Matrixd normalToLocalReferenceFrame(normalLocalToWorld*worldToLocal);

                    // need to compute the x and y delta for this point in space.
                    double X0, Y0, Z0;
                    double X1, Y1, Z1;
                    double X2, Y2, Z2;

                    et->convertLatLongHeightToXYZ(osg::DegreesToRadians(Y),osg::DegreesToRadians(X),Z,
                                                 X0,Y0,Z0);

                    et->convertLatLongHeightToXYZ(osg::DegreesToRadians(Y),osg::DegreesToRadians(X+delta_X),Z,
                                                 X1,Y1,Z1);

                    et->convertLatLongHeightToXYZ(osg::DegreesToRadians(Y+delta_Y),osg::DegreesToRadians(X),Z,
                                                 X2,Y2,Z2);
                                               
                    X1 -= X0;
                    Y1 -= Y0;
                    Z1 -= Z0;                          
                                               
                    X2 -= X0;
                    Y2 -= Y0;
                    Z2 -= Z0;                          

                    float xInterval = sqrt(X1*X1 + Y1*Y1 + Z1*Z1);
                    float yInterval = sqrt(X2*X2 + Y2*Y2 + Z2*Z2);

                    // need to set up the normal from the scaled heightDelta.
                    normal.x() = -heightDelta.x() / xInterval;
                    normal.y() = -heightDelta.y() / yInterval;
                    normal.z() = 1.0f;

                    normal = osg::Matrixd::transform3x3(normal,normalToLocalReferenceFrame);
                    normal.normalize();
                    
                }
                else
                {
                    normal.x() = -heightDelta.x() / grid->getXInterval();
                    normal.y() = -heightDelta.y() / grid->getYInterval();
                    normal.z() = 1.0f;
                    normal.normalize();
               }
            }

        }

    }

#if 0
    std::cout<<"Normals"<<std::endl;
    for(osg::Vec3Array::iterator nitr = n->begin();
        nitr != n->end();
        ++nitr)
    {
        osg::Vec3& normal = *nitr;
        std::cout<<"   Local normal = "<<normal<< " vs "<<transformed_center_normal<<std::endl;
    }
#endif

    if (useClusterCullingCallback)
    {
        // set up cluster cullling, 
        osg::ClusterCullingCallback* ccc = new osg::ClusterCullingCallback;

        ccc->set(center_position + transformed_center_normal*max_cluster_culling_height ,
                 transformed_center_normal, 
                 min_dot_product,
                 max_cluster_culling_radius);
        geometry->setCullCallback(ccc);
    }
    
    osgUtil::Simplifier::IndexList pointsToProtectDuringSimplification;

    if (numVerticesInSkirt>0)
    {
        osg::DrawElementsUShort& skirtDrawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numVerticesInSkirt+2));
        geometry->addPrimitiveSet(&skirtDrawElements);
        int ei=0;
        int firstSkirtVertexIndex = vi;
        // create bottom skirt vertices
        r=0;
        for(c=0;c<numColumns-1;++c)
        {
            // assign indices to primitive set
            skirtDrawElements[ei++] = (r)*numColumns+c;
            skirtDrawElements[ei++] = vi;
            
            // mark these points as protected to prevent them from being removed during simplification
            pointsToProtectDuringSimplification.push_back((r)*numColumns+c);
            pointsToProtectDuringSimplification.push_back(vi);
               
            osg::Vec3 localSkirtVector = !mapLatLongsToXYZ ? 
                                            skirtVector :
                                            computeLocalSkirtVector(et, grid.get(), c, r, skirtLength, useLocalToTileTransform, localToWorld);
            
            // add in the new point on the bottom of the skirt
            v[vi] = v[(r)*numColumns+c]+localSkirtVector;
            if (n.valid()) (*n)[vi] = (*n)[r*numColumns+c];
            t[vi++] = t[(r)*numColumns+c];
        }
        // create right skirt vertices
        c=numColumns-1;
        for(r=0;r<numRows-1;++r)
        {
            // assign indices to primitive set
            skirtDrawElements[ei++] = (r)*numColumns+c;
            skirtDrawElements[ei++] = vi;
            
            // mark these points as protected to prevent them from being removed during simplification
            pointsToProtectDuringSimplification.push_back((r)*numColumns+c);
            pointsToProtectDuringSimplification.push_back(vi);

            osg::Vec3 localSkirtVector = !mapLatLongsToXYZ ? 
                                            skirtVector :
                                            computeLocalSkirtVector(et, grid.get(), c, r, skirtLength, useLocalToTileTransform, localToWorld);
            
            // add in the new point on the bottom of the skirt
            v[vi] = v[(r)*numColumns+c]+localSkirtVector;
            if (n.valid()) (*n)[vi] = (*n)[(r)*numColumns+c];
            t[vi++] = t[(r)*numColumns+c];
        }
        // create top skirt vertices
        r=numRows-1;
        for(c=numColumns-1;c>0;--c)
        {
            // assign indices to primitive set
            skirtDrawElements[ei++] = (r)*numColumns+c;
            skirtDrawElements[ei++] = vi;
            
            // mark these points as protected to prevent them from being removed during simplification
            pointsToProtectDuringSimplification.push_back((r)*numColumns+c);
            pointsToProtectDuringSimplification.push_back(vi);

            osg::Vec3 localSkirtVector = !mapLatLongsToXYZ ? 
                                            skirtVector :
                                            computeLocalSkirtVector(et, grid.get(), c, r, skirtLength, useLocalToTileTransform, localToWorld);
            
            // add in the new point on the bottom of the skirt
            v[vi] = v[(r)*numColumns+c]+localSkirtVector;
            if (n.valid()) (*n)[vi] = (*n)[(r)*numColumns+c];
            t[vi++] = t[(r)*numColumns+c];
        }
        // create left skirt vertices
        c=0;
        for(r=numRows-1;r>0;--r)
        {
            // assign indices to primitive set
            skirtDrawElements[ei++] = (r)*numColumns+c;
            skirtDrawElements[ei++] = vi;
            
            // mark these points as protected to prevent them from being removed during simplification
            pointsToProtectDuringSimplification.push_back((r)*numColumns+c);
            pointsToProtectDuringSimplification.push_back(vi);

            osg::Vec3 localSkirtVector = !mapLatLongsToXYZ ? 
                                            skirtVector :
                                            computeLocalSkirtVector(et, grid.get(), c, r, skirtLength, useLocalToTileTransform, localToWorld);
            
            // add in the new point on the bottom of the skirt
            v[vi] = v[(r)*numColumns+c]+localSkirtVector;
            if (n.valid()) (*n)[vi] = (*n)[(r)*numColumns+c];
            t[vi++] = t[(r)*numColumns+c];
        }
        skirtDrawElements[ei++] = 0;
        skirtDrawElements[ei++] = firstSkirtVertexIndex;
    }

    if (n.valid())
    {
        geometry->setNormalArray(n.get());
        geometry->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    }


    osg::StateSet* stateset = createStateSet();
    if (stateset)
    {
        geometry->setStateSet(stateset);
    }
    else
    {
        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0] = _dataSet->getDefaultColor();

        geometry->setColorArray(colours);
        geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
    }
    
    
    osg::Geode* geode = new osg::Geode;
    geode->addDrawable(geometry);

    if (_dataSet->getWriteNodeBeforeSimplification())
    {
        osgDB::writeNodeFile(*geode,"NodeBeforeSimplification.osg");
    }

    if (_dataSet->getSimplifyTerrain())
    {
        unsigned int targetMaxNumVertices = 2048;
        float sample_ratio = (numVertices <= targetMaxNumVertices) ? 1.0f : (float)targetMaxNumVertices/(float)numVertices; 
    
        osgUtil::Simplifier simplifier(sample_ratio,geometry->getBound().radius()/2000.0f);

        simplifier.setDoTriStrip(false);
        simplifier.setSmoothing(false);
    
        simplifier.simplify(*geometry, pointsToProtectDuringSimplification);  // this will replace the normal vector with a new one
    }

    osgUtil::TriStripVisitor tsv;
    tsv.setMinStripSize(3);
    tsv.stripify(*geometry);



    if (useLocalToTileTransform)
    {
        osg::MatrixTransform* mt = new osg::MatrixTransform;
        mt->setMatrix(localToWorld);
        mt->addChild(geode);
        
        bool addLocalAxes = false;
        if (addLocalAxes)
        {
            float s = geode->getBound().radius()*0.5f;
            osg::MatrixTransform* scaleAxis = new osg::MatrixTransform;
            scaleAxis->setMatrix(osg::Matrix::scale(s,s,s));
            scaleAxis->addChild(osgDB::readNodeFile("axes.osg"));
            mt->addChild(scaleAxis);
        }
                
        return mt;
    }
    else
    {
        return geode;
    }
}

void DataSet::DestinationTile::readFrom(CompositeSource* sourceGraph)
{
    allocate();

    my_notify(osg::INFO)<<"DestinationTile::readFrom() "<<std::endl;
    for(CompositeSource::source_iterator itr(sourceGraph);itr.valid();++itr)
    {
    
        Source* source = itr->get();
        if (source && 
            _level>=source->getMinLevel() && _level<=source->getMaxLevel() && 
            (*itr)->getSourceData()) 
        {
            my_notify(osg::INFO)<<"DataSet::DestinationTile::readFrom -> SourceData::read() "<<std::endl;
            my_notify(osg::INFO)<<"    destination._level="<<_level<<"\t"<<source->getMinLevel()<<"\t"<<source->getMaxLevel()<<std::endl;

            SourceData* data = (*itr)->getSourceData();
            if (source->getType()==DataSet::Source::IMAGE)
            {
                unsigned int layerNum = source->getLayer();
                
                if (layerNum==0)
                {
                    // copy the base layer 0 into layer 0 and all subsequent layers to provide a backdrop.
                    for(unsigned int i=0;i<_imagery.size();++i)
                    {
                        if (_imagery[i]._imagery.valid())
                        {
                            data->read(*(_imagery[i]._imagery));
                        }
                    }
                }
                else
                {
                    // copy specific layer.
                    if (layerNum<_imagery.size() && _imagery[layerNum]._imagery.valid())
                    {
                        data->read(*(_imagery[layerNum]._imagery));
                    }
                }
            }
            else
            {
                if (_terrain.valid()) data->read(*_terrain);
            }
        }
    }

    optimizeResolution();

}

void DataSet::DestinationTile::unrefData()
{
    _imagery.clear();
    _terrain = 0;
    _models = 0;
}

void DataSet::DestinationTile::addRequiredResolutions(CompositeSource* sourceGraph)
{
    for(CompositeSource::source_iterator itr(sourceGraph);itr.valid();++itr)
    {
        Source* source = itr->get();
        if (source && source->intersects(*this))
        {
            if (source->getType()==Source::IMAGE)
            {
                unsigned int numCols,numRows;
                double resX, resY;
                if (computeImageResolution(source->getLayer(),numCols,numRows,resX,resY))
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

void DataSet::CompositeDestination::computeNeighboursFromQuadMap()
{
    // handle leaves
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        (*titr)->computeNeighboursFromQuadMap();
    }
    
    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        (*citr)->computeNeighboursFromQuadMap();
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
    my_notify(osg::INFO)<<"CompositeDestination::readFrom() "<<std::endl;

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


class CollectClusterCullingCallbacks : public osg::NodeVisitor
{
public:


    struct Triple
    {
        Triple():
            _drawable(0),
            _callback(0) {}
    
        Triple(osg::NodePath nodePath, osg::Drawable* drawable, osg::ClusterCullingCallback* callback):
            _nodePath(nodePath),
            _drawable(drawable),
            _callback(callback) {}

        Triple(const Triple& t):
            _nodePath(t._nodePath),
            _drawable(t._drawable),
            _callback(t._callback) {}

        Triple& operator = (const Triple& t)
        {
            _nodePath = t._nodePath;
            _drawable = t._drawable;
            _callback = t._callback;
            return *this;
        }

        osg::NodePath                   _nodePath;
        osg::Drawable*                  _drawable;
        osg::ClusterCullingCallback*    _callback;
    };

    typedef std::vector<Triple> ClusterCullingCallbackList;

    CollectClusterCullingCallbacks():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(osg::Geode& geode)
    {
        for(unsigned int i=0; i<geode.getNumDrawables();++i)
        {
            osg::ClusterCullingCallback* callback = dynamic_cast<osg::ClusterCullingCallback*>(geode.getDrawable(i)->getCullCallback());
            if (callback) 
            {
                _callbackList.push_back(Triple(getNodePath(),geode.getDrawable(i),callback));
            }
        }
    }
    
    ClusterCullingCallbackList _callbackList;
    
};

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


#if 1
    typedef std::vector<osg::Node*>  NodeList;

    // collect all the local tiles
    NodeList tileNodes;
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        osg::Node* node = (*titr)->createScene();
        if (node) tileNodes.push_back(node);
    }

    NodeList childNodes;
    ChildList::iterator citr;
    for(citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        osg::Node* node = (*citr)->createScene();
        if (node) childNodes.push_back(node);
    }


    float cutOffDistance = -FLT_MAX;
    for(citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        cutOffDistance = osg::maximum(cutOffDistance,(*citr)->_maxVisibleDistance);
    }


    osg::LOD* myLOD = new osg::LOD;
 
    float farDistance = _dataSet->getMaximumVisibleDistanceOfTopLevel();
    if (tileNodes.size()==1)
    {
        myLOD->addChild(tileNodes.front());
    }
    else if (tileNodes.size()>1)
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=tileNodes.begin();
            itr != tileNodes.end();
            ++itr)
        {
            group->addChild(*itr);
        }
        myLOD->addChild(group);
    }
    
    if (childNodes.size()==1)
    {
        myLOD->addChild(childNodes.front());
    }
    else if (childNodes.size()>1)
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=childNodes.begin();
            itr != childNodes.end();
            ++itr)
        {
            group->addChild(*itr);
        }
        myLOD->addChild(group);
    }


    // find cluster culling callbacks on drawables and move them to the myLOD level.
    {
        CollectClusterCullingCallbacks collect;
        myLOD->accept(collect);

        if (!collect._callbackList.empty())
        {
            if (collect._callbackList.size()==1)
            {
                CollectClusterCullingCallbacks::Triple& triple = collect._callbackList.front();
            
                osg::Matrixd matrix = osg::computeLocalToWorld(triple._nodePath);
                
                triple._callback->transform(matrix);
                
                osg::notify(osg::INFO)<<"cluster culling matrix "<<matrix<<std::endl;

                // moving cluster culling callback myLOD node.
                myLOD->setCullCallback(triple._callback);
                
                // remove it from the drawable.
                triple._drawable->setCullCallback(0);
            }
        }
    }
        
    cutOffDistance = osg::maximum(cutOffDistance, myLOD->getBound().radius()*_dataSet->getRadiusToMaxVisibleDistanceRatio());
    
    myLOD->setRange(0,cutOffDistance,farDistance);
    myLOD->setRange(1,0,cutOffDistance);
    
    if (myLOD->getNumChildren()>0)
        myLOD->setCenter(myLOD->getBound().center());
    
    return myLOD;
#else
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
        
        if (node) 
        {
            double maxVisibleDistance = osg::maximum(_maxVisibleDistance, node->getBound().radius()*_dataSet->getRadiusToMaxVisibleDistanceRatio());
            rangeNodeListMap[maxVisibleDistance].push_back(node);
        }
    }

    // handle chilren
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        osg::Node* node = (*citr)->createScene();
        if (node)
        {
            double maxVisibleDistance = osg::maximum((*citr)->_maxVisibleDistance, node->getBound().radius()*_dataSet->getRadiusToMaxVisibleDistanceRatio());
            rangeNodeListMap[maxVisibleDistance].push_back(node);
        }
    }

    osg::LOD* lod = new osg::LOD;
    
    float farDistance = _dataSet->getMaximumVisibleDistanceOfTopLevel();

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
            maxVisibleDistance = farDistance;
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
            
            lod->addChild(child,0,maxVisibleDistance);
            
            ++childNum;
        }
    }
    return lod;
#endif
}

bool DataSet::CompositeDestination::areSubTilesComplete()
{
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        for(TileList::iterator itr=(*citr)->_tiles.begin();
            itr!=(*citr)->_tiles.end();
            ++itr)
        {
            if (!(*itr)->getTileComplete()) 
            {
                return false;
            }
        }
    }
    return true;
}

std::string DataSet::CompositeDestination::getSubTileName()
{
    return _name+"_subtile"+_dataSet->getDestinationTileExtension();
}


osg::Node* DataSet::CompositeDestination::createPagedLODScene()
{
    if (_children.empty() && _tiles.empty()) return 0;
    
    if (_children.empty() && _tiles.size()==1) return _tiles.front()->createScene();
    
    if (_tiles.empty() && _children.size()==1) return _children.front()->createPagedLODScene();
    
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

    // collect all the local tiles
    NodeList tileNodes;
    for(TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        osg::Node* node = (*titr)->createScene();
        if (node) tileNodes.push_back(node);
    }

    float cutOffDistance = -FLT_MAX;
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        cutOffDistance = osg::maximum(cutOffDistance,(*citr)->_maxVisibleDistance);
    }


    osg::PagedLOD* pagedLOD = new osg::PagedLOD;
 
    float farDistance = _dataSet->getMaximumVisibleDistanceOfTopLevel();
    if (tileNodes.size()==1)
    {
        pagedLOD->addChild(tileNodes.front());
    }
    else if (tileNodes.size()>1)
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=tileNodes.begin();
            itr != tileNodes.end();
            ++itr)
        {
            group->addChild(*itr);
        }
        pagedLOD->addChild(group);
    }
    

    // find cluster culling callbacks on drawables and move them to the PagedLOD level.
    {
        CollectClusterCullingCallbacks collect;
        pagedLOD->accept(collect);

        if (!collect._callbackList.empty())
        {
            if (collect._callbackList.size()==1)
            {
                CollectClusterCullingCallbacks::Triple& triple = collect._callbackList.front();
            
                osg::Matrixd matrix = osg::computeLocalToWorld(triple._nodePath);
                
                triple._callback->transform(matrix);
                
                osg::notify(osg::INFO)<<"cluster culling matrix "<<matrix<<std::endl;

                // moving cluster culling callback pagedLOD node.
                pagedLOD->setCullCallback(triple._callback);
                
                // remove it from the drawable.
                triple._drawable->setCullCallback(0);
            }
        }
    }
        
    cutOffDistance = osg::maximum(cutOffDistance, pagedLOD->getBound().radius()*_dataSet->getRadiusToMaxVisibleDistanceRatio());
    
    pagedLOD->setRange(0,cutOffDistance,farDistance);
    
    pagedLOD->setFileName(1,getSubTileName());
    pagedLOD->setRange(1,0,cutOffDistance);
    
    if (pagedLOD->getNumChildren()>0)
        pagedLOD->setCenter(pagedLOD->getBound().center());
    
    return pagedLOD;
}

osg::Node* DataSet::CompositeDestination::createSubTileScene()
{
    if (_type==GROUP ||
        _children.empty() || 
        _tiles.empty()) return 0;

    // handle chilren
    typedef std::vector<osg::Node*>  NodeList;
    NodeList nodeList;
    for(ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        osg::Node* node = (*citr)->createPagedLODScene();
        if (node) nodeList.push_back(node);
    }

    if (nodeList.size()==1)
    {
        return nodeList.front();
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
        return group;
    }
    else
    {
        return 0;
    }
}

void DataSet::CompositeDestination::unrefSubTileData()
{
    for(CompositeDestination::ChildList::iterator citr=_children.begin();
        citr!=_children.end();
        ++citr)
    {
        (*citr)->unrefLocalData();
    } 
}

void DataSet::CompositeDestination::unrefLocalData()
{
    for(CompositeDestination::TileList::iterator titr=_tiles.begin();
        titr!=_tiles.end();
        ++titr)
    {
        DestinationTile* tile = titr->get();
        my_notify(osg::INFO)<<"   unref tile level="<<tile->_level<<" X="<<tile->_tileX<<" Y="<<tile->_tileY<<std::endl;
        tile->unrefData();
    }
}

///////////////////////////////////////////////////////////////////////////////

DataSet::DataSet()
{
    init();
    
    _maximumTileImageSize = 256;
    _maximumTileTerrainSize = 64;
    
    _maximumVisiableDistanceOfTopLevel = 1e10;
    
    _radiusToMaxVisibleDistanceRatio = 7.0f;
    _verticalScale = 1.0f;
    _skirtRatio = 0.02f;

    _convertFromGeographicToGeocentric = false;
    
    _tileBasename = "output";
    _tileExtension = ".ive";
    _imageExtension = ".dds";

    
    _defaultColor.set(0.5f,0.5f,1.0f,1.0f);
    _databaseType = PagedLOD_DATABASE;
    _geometryType = POLYGONAL;
    _textureType = COMPRESSED_TEXTURE;
    _maxAnisotropy = 1.0;
    _mipMappingMode = MIP_MAPPING_IMAGERY;

    _useLocalTileTransform = true;
    
    _decorateWithCoordinateSystemNode = true;
    _decorateWithMultiTextureControl = true;
    
    _numTextureLevels = 1;
    
    _writeNodeBeforeSimplification = false;

    _simplifyTerrain = true;

    setEllipsoidModel(new osg::EllipsoidModel());
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

void DataSet::setDestinationName(const std::string& filename)
{
    std::string path = osgDB::getFilePath(filename);
    std::string base = osgDB::getStrippedName(filename);
    std::string extension = '.'+osgDB::getLowerCaseFileExtension(filename);

    osg::notify(osg::INFO)<<"setDestinationName("<<filename<<")"<<std::endl;
    osg::notify(osg::INFO)<<"   path "<<path<<std::endl;
    osg::notify(osg::INFO)<<"   base "<<base<<std::endl;
    osg::notify(osg::INFO)<<"   extension "<<extension<<std::endl;

    setDirectory(path);
    setDestinationTileBaseName(base);
    setDestinationTileExtension(extension);
} 

void DataSet::setDirectory(const std::string& directory)
{
    _directory = directory;
    
    if (_directory.empty()) return;
    
#ifdef WIN32    
    // convert trailing forward slash if any to back slash.
    if (_directory[_directory.size()-1]=='/') _directory[_directory.size()-1] = '\\';

    // if no trailing back slash exists add one.
    if (_directory[_directory.size()-1]!='\\') _directory.push_back('\\');
#else
    // convert trailing back slash if any to forward slash.
    if (_directory[_directory.size()-1]=='\\') _directory[_directory.size()-1] = '/';

    // if no trailing forward slash exists add one.
    if (_directory[_directory.size()-1]!='/') _directory.push_back('/');
#endif    
    osg::notify(osg::NOTICE)<<"directory name set "<<_directory<<std::endl;
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

DataSet::CompositeDestination* DataSet::createDestinationGraph(CompositeDestination* parent,
                                                           osg::CoordinateSystemNode* cs,
                                                           const GeospatialExtents& extents,
                                                           unsigned int maxImageSize,
                                                           unsigned int maxTerrainSize,
                                                           unsigned int currentLevel,
                                                           unsigned int currentX,
                                                           unsigned int currentY,
                                                           unsigned int maxNumLevels)
{

    DataSet::CompositeDestination* destinationGraph = new DataSet::CompositeDestination(cs,extents);

    if (mapLatLongsToXYZ())
    {
        // we need to project the extents into world coords to get the appropriate size to use for control max visible distance
        float max_range = osg::maximum(extents.xMax()-extents.xMin(),extents.yMax()-extents.yMin());
        float projected_radius =  osg::DegreesToRadians(max_range) * getEllipsoidModel()->getRadiusEquator();
        float center_offset = (max_range/360.0f) * getEllipsoidModel()->getRadiusEquator();
        destinationGraph->_maxVisibleDistance = projected_radius * getRadiusToMaxVisibleDistanceRatio() + center_offset;
    }
    else
    {
        destinationGraph->_maxVisibleDistance = extents.radius()*getRadiusToMaxVisibleDistanceRatio();
    }

    // first create the topmost tile

    // create the name
    std::ostringstream os;
    os << _tileBasename << "_L"<<currentLevel<<"_X"<<currentX<<"_Y"<<currentY;

    destinationGraph->_parent = parent;
    destinationGraph->_name = os.str();
    destinationGraph->_level = currentLevel;
    destinationGraph->_tileX = currentX;
    destinationGraph->_tileY = currentY;
    destinationGraph->_dataSet = this;


    DestinationTile* tile = new DestinationTile;
    tile->_name = destinationGraph->_name;
    tile->_level = currentLevel;
    tile->_tileX = currentX;
    tile->_tileY = currentY;
    tile->_dataSet = this;
    tile->_cs = cs;
    tile->_extents = extents;
    tile->_pixelFormat = (getTextureType()==COMPRESSED_RGBA_TEXTURE||
                          getTextureType()==RGBA ||
                          getTextureType()==RGBA_16) ? GL_RGBA : GL_RGB;
    tile->setMaximumImagerySize(maxImageSize,maxImageSize);
    tile->setMaximumTerrainSize(maxTerrainSize,maxTerrainSize);
    tile->computeMaximumSourceResolution(_sourceGraph.get());

    insertTileToQuadMap(destinationGraph);

    if (currentLevel>=maxNumLevels-1 || currentLevel>=tile->_maxSourceLevel)
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

        // note, resolutionSensitivityScale should probably be customizable.. will consider this option for later inclusion.
        double resolutionSensitivityScale = 0.9;
        for(unsigned int layerNum=0;
            layerNum<tile->_imagery.size();
            ++layerNum)
        {
            unsigned int texture_numColumns;
            unsigned int texture_numRows;
            double texture_dx;
            double texture_dy;
            if (tile->computeImageResolution(layerNum,texture_numColumns,texture_numRows,texture_dx,texture_dy))
            {
                if (texture_dx*resolutionSensitivityScale>tile->_imagery[layerNum]._imagery_maxSourceResolutionX) needToDivideX = true;
                if (texture_dy*resolutionSensitivityScale>tile->_imagery[layerNum]._imagery_maxSourceResolutionY) needToDivideY = true;
            }
        }
                
        unsigned int dem_numColumns;
        unsigned int dem_numRows;
        double dem_dx;
        double dem_dy;
        if (tile->computeTerrainResolution(dem_numColumns,dem_numRows,dem_dx,dem_dy))
        {
            if (dem_dx*resolutionSensitivityScale>tile->_terrain_maxSourceResolutionX) needToDivideX = true;
            if (dem_dy*resolutionSensitivityScale>tile->_terrain_maxSourceResolutionY) needToDivideY = true;
        }
        
        float xCenter = (extents.xMin()+extents.xMax())*0.5f;
        float yCenter = (extents.yMin()+extents.yMax())*0.5f;
        
        unsigned int newLevel = currentLevel+1;
        unsigned int newX = currentX*2;
        unsigned int newY = currentY*2;

        if (needToDivideX && needToDivideY)
        {
            float aspectRatio = (extents.yMax()- extents.yMin())/(extents.xMax()- extents.xMin());
            
            if (aspectRatio>1.414) needToDivideX = false;
            else if (aspectRatio<.707) needToDivideY = false;
        }

        if (needToDivideX && needToDivideY)
        {
            my_notify(osg::INFO)<<"Need to Divide X + Y for level "<<currentLevel<<std::endl;
            // create four tiles.
            GeospatialExtents bottom_left(extents.xMin(),extents.yMin(),xCenter,yCenter, extents._isGeographic);
            GeospatialExtents bottom_right(xCenter,extents.yMin(),extents.xMax(),yCenter, extents._isGeographic);
            GeospatialExtents top_left(extents.xMin(),yCenter,xCenter,extents.yMax(), extents._isGeographic);
            GeospatialExtents top_right(xCenter,yCenter,extents.xMax(),extents.yMax(), extents._isGeographic);

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         bottom_left,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         bottom_right,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX+1,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         top_left,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY+1,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
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
            my_notify(osg::INFO)<<"Need to Divide X only"<<std::endl;

            // create two tiles.
            GeospatialExtents left(extents.xMin(),extents.yMin(),xCenter,extents.yMax(), extents._isGeographic);
            GeospatialExtents right(xCenter,extents.yMin(),extents.xMax(),extents.yMax(), extents._isGeographic);

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         left,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         right,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX+1,
                                                                         newY,
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
        else if (needToDivideY)
        {
            my_notify(osg::INFO)<<"Need to Divide Y only"<<std::endl;

            // create two tiles.
            GeospatialExtents top(extents.xMin(),yCenter,extents.xMax(),extents.yMax(), extents._isGeographic);
            GeospatialExtents bottom(extents.xMin(),extents.yMin(),extents.xMax(),yCenter, extents._isGeographic);

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         bottom,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
                                                                         newY,
                                                                         maxNumLevels));

            destinationGraph->_children.push_back(createDestinationGraph(destinationGraph,
                                                                         cs,
                                                                         top,
                                                                         maxImageSize,
                                                                         maxTerrainSize,
                                                                         newLevel,
                                                                         newX,
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
        else
        {
            my_notify(osg::INFO)<<"No Need to Divide"<<std::endl;
        }
    }
    
    return destinationGraph;
}

void DataSet::computeDestinationGraphFromSources(unsigned int numLevels)
{
    if (!_sourceGraph) return;

    // ensure we have a valid coordinate system
    if (!_destinationCoordinateSystem)
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            SourceData* sd = (*itr)->getSourceData();
            if (sd)
            {
                if (sd->_cs.valid())
                {
                    _destinationCoordinateSystem = sd->_cs;
                    my_notify(osg::INFO)<<"Setting coordinate system to "<<_destinationCoordinateSystem->getCoordinateSystem()<<std::endl;
                    break;
                }
            }
        }
    }
    
    if (!_intermediateCoordinateSystem)
    {
        CoordinateSystemType cst = getCoordinateSystemType(_destinationCoordinateSystem.get());

        my_notify(osg::INFO)<<"new DataSet::createDestination()"<<std::endl;
        if (cst!=GEOGRAPHIC && getConvertFromGeographicToGeocentric())
        {
            // need to use the geocentric coordinate system as a base for creating an geographic intermediate
            // coordinate system.
            OGRSpatialReference oSRS;
            
            char    *pszWKT = NULL;
            oSRS.SetWellKnownGeogCS( "WGS84" );
            oSRS.exportToWkt( &pszWKT );
            
            setIntermediateCoordinateSystem(pszWKT);
        }
        else
        {
            _intermediateCoordinateSystem = _destinationCoordinateSystem;
        }
    }


    CoordinateSystemType destinateCoordSytemType = getCoordinateSystemType(_destinationCoordinateSystem.get());
    if (destinateCoordSytemType==GEOGRAPHIC && !getConvertFromGeographicToGeocentric())
    {
        // convert elevation into degrees.
        setVerticalScale(1.0f/111319.0f);
    }

    // get the extents of the sources and
    GeospatialExtents extents(_extents);
    extents._isGeographic = destinateCoordSytemType==GEOGRAPHIC;

    if (!extents.valid()) 
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            SourceData* sd = (*itr)->getSourceData();
            if (sd)
            {
                GeospatialExtents local_extents(sd->getExtents(_intermediateCoordinateSystem.get()));
                my_notify(osg::INFO)<<"local_extents = xMin()"<<local_extents.xMin()<<" "<<local_extents.xMax()<<std::endl;
                my_notify(osg::INFO)<<"                yMin()"<<local_extents.yMin()<<" "<<local_extents.yMax()<<std::endl;
                
                if (destinateCoordSytemType==GEOGRAPHIC)
                {
                    // need to clamp within -180 and 180 range.
                    if (local_extents.xMin()>180.0) 
                    {
                        // shift back to -180 to 180 range
                        local_extents.xMin() -= 360.0;
                        local_extents.xMax() -= 360.0;
                    }
                    else if (local_extents.xMin()<-180.0) 
                    {
                        // shift back to -180 to 180 range
                        local_extents.xMin() += 360.0;
                        local_extents.xMax() += 360.0;
                    }
                }

                extents.expandBy(local_extents);
            }
        }
    }
    

    if (destinateCoordSytemType==GEOGRAPHIC)
    {
        double xRange = extents.xMax() - extents.xMin();
        if (xRange>360.0) 
        {
            // clamp to proper 360 range.
            extents.xMin() = -180.0;
            extents.xMax() = 180.0;
        }
    }
    

    // compute the number of texture layers required.
    unsigned int maxTextureUnit = 0;
    for(CompositeSource::source_iterator sitr(_sourceGraph.get());sitr.valid();++sitr)
    {
        Source* source = sitr->get();
        if (source) 
        {
            if (maxTextureUnit<source->getLayer()) maxTextureUnit = source->getLayer();
        }
    }
    _numTextureLevels = maxTextureUnit+1;


    my_notify(osg::INFO)<<"extents = xMin()"<<extents.xMin()<<" "<<extents.xMax()<<std::endl;
    my_notify(osg::INFO)<<"          yMin()"<<extents.yMin()<<" "<<extents.yMax()<<std::endl;

    // then create the destinate graph accordingly.
    _destinationGraph = createDestinationGraph(0,
                                               _intermediateCoordinateSystem.get(),
                                               extents,
                                               _maximumTileImageSize,
                                               _maximumTileTerrainSize,
                                               0,
                                               0,
                                               0,
                                               numLevels);
                                                           
    // now traverse the destination graph to build neighbours.        
    _destinationGraph->computeNeighboursFromQuadMap();

}

template<class T>
struct DerefLessFunctor
{
    bool operator () (const T& lhs, const T& rhs)
    {
        if (!lhs || !rhs) return lhs<rhs;
        if (lhs->getLayer() < rhs->getLayer()) return true;
        if (rhs->getLayer() < lhs->getLayer()) return false;
        return (lhs->getSortValue() > rhs->getSortValue());
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
        if (itr->valid()) (*itr)->sort();
    }
}

void DataSet::updateSourcesForDestinationGraphNeeds()
{
    if (!_destinationGraph || !_sourceGraph) return;


    std::string temporyFilePrefix("temporaryfile_");

    // compute the resolutions of the source that are required.
    {
        _destinationGraph->addRequiredResolutions(_sourceGraph.get());


        for(CompositeSource::source_iterator sitr(_sourceGraph.get());sitr.valid();++sitr)
        {
            Source* source = sitr->get();
            if (source) 
            {
                my_notify(osg::INFO)<<"Source File "<<source->getFileName()<<std::endl;


                const Source::ResolutionList& resolutions = source->getRequiredResolutions();
                my_notify(osg::INFO)<<"    resolutions.size() "<<resolutions.size()<<std::endl;
                my_notify(osg::INFO)<<"    { "<<std::endl;
                Source::ResolutionList::const_iterator itr;
                for(itr=resolutions.begin();
                    itr!=resolutions.end();
                    ++itr)
                {
                    my_notify(osg::INFO)<<"        resX="<<itr->_resX<<" resY="<<itr->_resY<<std::endl;
                }
                my_notify(osg::INFO)<<"    } "<<std::endl;

                source->consolodateRequiredResolutions();

                my_notify(osg::INFO)<<"    consolodated resolutions.size() "<<resolutions.size()<<std::endl;
                my_notify(osg::INFO)<<"    consolodated { "<<std::endl;
                for(itr=resolutions.begin();
                    itr!=resolutions.end();
                    ++itr)
                {
                    my_notify(osg::INFO)<<"        resX="<<itr->_resX<<" resY="<<itr->_resY<<std::endl;
                }
                my_notify(osg::INFO)<<"    } "<<std::endl;
            }

        }


    }

    // do standardisation of coordinates systems.
    // do any reprojection if required.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            if (source && source->needReproject(_intermediateCoordinateSystem.get()))
            {
                // do the reprojection to a tempory file.
                std::string newFileName = temporyFilePrefix + osgDB::getStrippedName(source->getFileName()) + ".tif";
                
                Source* newSource = source->doReproject(newFileName,_intermediateCoordinateSystem.get());
                
                // replace old source by new one.
                if (newSource) *itr = newSource;
                else
                {
                    my_notify(osg::WARN)<<"Failed to reproject"<<source->getFileName()<<std::endl;
                    *itr = 0;
                }
            }
        }
    }
    
    // do sampling of data to required values.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            if (source) source->buildOverviews();
        }
    }

    // sort the sources so that the lowest res tiles are drawn first.
    {
        for(CompositeSource::source_iterator itr(_sourceGraph.get());itr.valid();++itr)
        {
            Source* source = itr->get();
            if (source)
            {
                source->setSortValueFromSourceDataResolution();
                my_notify(osg::INFO)<<"sort "<<source->getFileName()<<" value "<<source->getSortValue()<<std::endl;
            }
            
        }
        
        // sort them so highest sortValue is first.

        _sourceGraph->sort();
    }
    
    my_notify(osg::INFO)<<"Using source_lod_iterator itr"<<std::endl;
        
    // buggy mips compiler requires this local variable in source_lod_iterator
    // usage below, since using _sourceGraph.get() as it should be was causing
    // a MIPSpro compiler error "The member "osgTerrain::DataSet::_sourceGraph" is inaccessible."
    CompositeSource* my_sourceGraph = _sourceGraph.get();

    for(CompositeSource::source_lod_iterator csitr(my_sourceGraph,CompositeSource::LODSourceAdvancer(0.0));csitr.valid();++csitr)
    {
        Source* source = csitr->get();
        if (source)
        {
            my_notify(osg::INFO)<<"  LOD "<<(*csitr)->getFileName()<<std::endl;
        }
    }
    my_notify(osg::INFO)<<"End of Using Source Iterator itr"<<std::endl;
    
}

void DataSet::populateDestinationGraphFromSources()
{
    if (!_destinationGraph || !_sourceGraph) return;

    my_notify(osg::NOTICE)<<"started DataSet::populateDestinationGraphFromSources)"<<std::endl;

    if (_databaseType==LOD_DATABASE)
    {

        // for each DestinationTile populate it.
        _destinationGraph->readFrom(_sourceGraph.get());

        // for each DestinationTile equalize the boundaries so they all fit each other without gaps.
        _destinationGraph->equalizeBoundaries();
        
        
    }
    else
    {
        // for each level
        //  compute x and y range
        //  from top row down to bottom row equalize boundairies a write out
    }
    my_notify(osg::NOTICE)<<"completed DataSet::populateDestinationGraphFromSources)"<<std::endl;
}


void DataSet::_readRow(Row& row)
{
    my_notify(osg::NOTICE)<<"_readRow "<<row.size()<<std::endl;
    for(Row::iterator citr=row.begin();
        citr!=row.end();
        ++citr)
    {
        CompositeDestination* cd = citr->second;
        for(CompositeDestination::TileList::iterator titr=cd->_tiles.begin();
            titr!=cd->_tiles.end();
            ++titr)
        {
            DestinationTile* tile = titr->get();
            my_notify(osg::NOTICE)<<"   reading tile level="<<tile->_level<<" X="<<tile->_tileX<<" Y="<<tile->_tileY<<std::endl;
            tile->readFrom(_sourceGraph.get());
        }
    }
}

void DataSet::_equalizeRow(Row& row)
{
    my_notify(osg::NOTICE)<<"_equalizeRow "<<row.size()<<std::endl;
    for(Row::iterator citr=row.begin();
        citr!=row.end();
        ++citr)
    {
        CompositeDestination* cd = citr->second;
        for(CompositeDestination::TileList::iterator titr=cd->_tiles.begin();
            titr!=cd->_tiles.end();
            ++titr)
        {
            DestinationTile* tile = titr->get();
            my_notify(osg::NOTICE)<<"   equalizing tile level="<<tile->_level<<" X="<<tile->_tileX<<" Y="<<tile->_tileY<<std::endl;
            tile->equalizeBoundaries();
            tile->setTileComplete(true);
        }
    }
}

void DataSet::_writeNodeFile(const osg::Node& node,const std::string& filename)
{
    if (_archive.valid()) _archive->writeNode(node,filename);
    else osgDB::writeNodeFile(node, filename);
}

void DataSet::_writeImageFile(const osg::Image& image,const std::string& filename)
{
    if (_archive.valid()) _archive->writeImage(image,filename);
    else osgDB::writeImageFile(image, filename);
}


class WriteImageFilesVisitor : public osg::NodeVisitor
{
public:

    WriteImageFilesVisitor(osgTerrain::DataSet* dataSet):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _dataSet(dataSet) {}

    osgTerrain::DataSet* _dataSet;
    
    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet()) apply(*(node.getStateSet()));

        traverse(node);
    }

    virtual void apply(osg::Geode& geode)
    {
        if (geode.getStateSet()) apply(*(geode.getStateSet()));

        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            if (geode.getDrawable(i)->getStateSet()) apply(*(geode.getDrawable(i)->getStateSet()));
        }
        
        traverse(geode);
    }

    void apply(osg::StateSet& stateset)
    {
        for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
        {
            osg::Image* image = 0;
            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture2D) image = texture2D->getImage();
            
            if (image)
            {
                _dataSet->_writeImageFile(*image,(_dataSet->getDirectory()+image->getFileName()).c_str());
            }
        }
    }
};

void DataSet::_writeRow(Row& row)
{
    my_notify(osg::NOTICE)<<"_writeRow "<<row.size()<<std::endl;
    for(Row::iterator citr=row.begin();
        citr!=row.end();
        ++citr)
    {
        CompositeDestination* cd = citr->second;
        CompositeDestination* parent = cd->_parent;
        
        if (parent)
        {
            if (!parent->getSubTilesGenerated() && parent->areSubTilesComplete())
            {
                osg::ref_ptr<osg::Node> node = parent->createSubTileScene();
                std::string filename = _directory+parent->getSubTileName();
                if (node.valid())
                {
                    my_notify(osg::NOTICE)<<"   writeSubTile filename="<<filename<<std::endl;
                    _writeNodeFile(*node,filename);

                    if (_tileExtension==".osg")
                    {
                        WriteImageFilesVisitor wifv(this);
                        node->accept(wifv);
                    }

                    parent->setSubTilesGenerated(true);
                    parent->unrefSubTileData();
                }
                else
                {
                    my_notify(osg::WARN)<<"   failed to writeSubTile node for tile, filename="<<filename<<std::endl;
                }
            }
        }
        else
        {
            osg::ref_ptr<osg::Node> node = cd->createPagedLODScene();
            
            if (_decorateWithCoordinateSystemNode)
            {
                node = decorateWithCoordinateSystemNode(node.get());
            }
            
            if (_decorateWithMultiTextureControl)
            {
                node = decorateWithMultiTextureControl(node.get());
            }

            if (!_comment.empty())
            {
                node->addDescription(_comment);
            }

            std::string filename = _directory+_tileBasename+_tileExtension;

            if (node.valid())
            {
                my_notify(osg::NOTICE)<<"   writeNodeFile = "<<cd->_level<<" X="<<cd->_tileX<<" Y="<<cd->_tileY<<" filename="<<filename<<std::endl;
                _writeNodeFile(*node,filename);
                
                if (_tileExtension==".osg")
                {
                    WriteImageFilesVisitor wifv(this);
                    node->accept(wifv);
                }
            }
            else
            {
                my_notify(osg::WARN)<<"   faild to write node for tile = "<<cd->_level<<" X="<<cd->_tileX<<" Y="<<cd->_tileY<<" filename="<<filename<<std::endl;
            }

            // record the top nodes as the rootNode of the database
            _rootNode = node;

        }
    }
}

void DataSet::createDestination(unsigned int numLevels)
{
    my_notify(osg::NOTICE)<<"started DataSet::createDestination("<<numLevels<<")"<<std::endl;

    computeDestinationGraphFromSources(numLevels);
    
    updateSourcesForDestinationGraphNeeds();

    my_notify(osg::NOTICE)<<"completed DataSet::createDestination("<<numLevels<<")"<<std::endl;

}

osg::Node* DataSet::decorateWithCoordinateSystemNode(osg::Node* subgraph)
{
    // don't decorate if no coord system is set.
    if (_destinationCoordinateSystem->getCoordinateSystem().empty()) 
        return subgraph;

    osg::CoordinateSystemNode* csn = new osg::CoordinateSystemNode(
            _destinationCoordinateSystem->getFormat(),
            _destinationCoordinateSystem->getCoordinateSystem());
    
    // set the ellipsoid model if geocentric coords are used.
    if (getConvertFromGeographicToGeocentric()) csn->setEllipsoidModel(getEllipsoidModel());
    
    // add the a subgraph.
    csn->addChild(subgraph);

    return csn;
}

osg::Node* DataSet::decorateWithMultiTextureControl(osg::Node* subgraph)
{
    // if only one layer exists don't need to decorate with MultiTextureControl
    if (_numTextureLevels<=1) return subgraph;
    
    
    // multiple layers active so use osgFX::MultiTextureControl to manage them
    osgFX::MultiTextureControl* mtc = new osgFX::MultiTextureControl;
    float r = 1.0f/(float)_numTextureLevels;
    for(unsigned int i=0;i<_numTextureLevels;++i)
    {
        mtc->setTextureWeight(i,r);
    }

    // add the a subgraph.
    mtc->addChild(subgraph);

    return mtc;
}


void DataSet::_buildDestination(bool writeToDisk)
{
    if (!_state) _state = new osg::State;

    osg::ref_ptr<osgDB::ReaderWriter::Options> previous_options = osgDB::Registry::instance()->getOptions();
    osgDB::Registry::instance()->setOptions(new osgDB::ReaderWriter::Options("precision 16"));

    if (!_archive && !_archiveName.empty())
    {
        unsigned int indexBlockSizeHint=4096;
        _archive = osgDB::openArchive(_archiveName, osgDB::Archive::CREATE, indexBlockSizeHint);
    }

    if (_destinationGraph.valid())
    {
        std::string filename = _directory+_tileBasename+_tileExtension;
        
        if (_archive.valid())
        {
            my_notify(osg::NOTICE)<<"started DataSet::writeDestination("<<_archiveName<<")"<<std::endl;
            my_notify(osg::NOTICE)<<"        archive file = "<<_archiveName<<std::endl;
            my_notify(osg::NOTICE)<<"        archive master file = "<<filename<<std::endl;
        }
        else
        {
            my_notify(osg::NOTICE)<<"started DataSet::writeDestination("<<filename<<")"<<std::endl;
        }

        if (_databaseType==LOD_DATABASE)
        {
            populateDestinationGraphFromSources();
            _rootNode = _destinationGraph->createScene();

            if (_decorateWithCoordinateSystemNode)
            {
                _rootNode = decorateWithCoordinateSystemNode(_rootNode.get());
            }

            if (_decorateWithMultiTextureControl)
            {
                _rootNode = decorateWithMultiTextureControl(_rootNode.get());
            }

            if (!_comment.empty())
            {
                _rootNode->addDescription(_comment);
            }

            if (writeToDisk)
            {
                _writeNodeFile(*_rootNode,filename);
                if (_tileExtension==".osg")
                {
                    WriteImageFilesVisitor wifv(this);
                    _rootNode->accept(wifv);
                }
            }
        }
        else  // _databaseType==PagedLOD_DATABASE
        {
            // for each level build read and write the rows.
            for(QuadMap::iterator qitr=_quadMap.begin();
                qitr!=_quadMap.end();
                ++qitr)
            {
                Level& level = qitr->second;
                
                // skip is level is empty.
                if (level.empty()) continue;
                
                my_notify(osg::INFO)<<"New level"<<std::endl;

                Level::iterator prev_itr = level.begin();
                _readRow(prev_itr->second);
                Level::iterator curr_itr = prev_itr;
                ++curr_itr;
                for(;
                    curr_itr!=level.end();
                    ++curr_itr)
                {
                    _readRow(curr_itr->second);
                    
                    _equalizeRow(prev_itr->second);
                    if (writeToDisk) _writeRow(prev_itr->second);
                    
                    prev_itr = curr_itr;
                }
                
                _equalizeRow(prev_itr->second);
                if (writeToDisk)
                {
                    _writeRow(prev_itr->second);
                }
            }
        }

        if (_archive.valid())
        {
            my_notify(osg::NOTICE)<<"completed DataSet::writeDestination("<<_archiveName<<")"<<std::endl;
            my_notify(osg::NOTICE)<<"          archive file = "<<_archiveName<<std::endl;
            my_notify(osg::NOTICE)<<"          archive master file = "<<filename<<std::endl;
        }
        else
        {
            my_notify(osg::NOTICE)<<"completed DataSet::writeDestination("<<filename<<")"<<std::endl;
        }

    }
    else
    {
        my_notify(osg::WARN)<<"Error: no scene graph to output, no file written."<<std::endl;
    }

    if (_archive.valid()) _archive->close();

    osgDB::Registry::instance()->setOptions(previous_options.get());

}

