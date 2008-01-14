/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield 
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

#ifndef DATASETLAYER
#define DATASETLAYER 1

#include <osgTerrain/Layer>
#include <osgDB/ReaderWriter>

#include <gdal_priv.h>

namespace GDALPlugin {

class DataSetLayer : public osgTerrain::Layer
{
    public:

        DataSetLayer();

        DataSetLayer(const std::string& fileName);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        DataSetLayer(const DataSetLayer& dataSetLayer,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
        
        META_Object(GDALPlugin, DataSetLayer);
        
        virtual bool isOpen() const { return _dataset!=0; }

        virtual void open();

        virtual void close();

        virtual unsigned int getNumColumns() const;

        virtual unsigned int getNumRows() const;

        virtual osgTerrain::ImageLayer* extractImageLayer(unsigned int sourceMinX, unsigned int sourceMinY, unsigned int sourceMaxX, unsigned int sourceMaxY, unsigned int targetWidth=0, unsigned int targetHeight=0);

        void setGdalReader(const osgDB::ReaderWriter* rw);

    protected:
    
        virtual ~DataSetLayer();
    
        void setUpLocator();
    
        GDALDataset* _dataset;

        osgDB::ReaderWriter* _gdalReader;


};

}

#endif
