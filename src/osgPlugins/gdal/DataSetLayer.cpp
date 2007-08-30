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

#include "DataSetLayer.h"

#include <osg/Notify>

using namespace GDALPlugin;

DataSetLayer::DataSetLayer():
    _dataset(0)
{
}

DataSetLayer::DataSetLayer(const std::string& fileName):
    _dataset(0)
{
    openFile(fileName);
}

DataSetLayer::DataSetLayer(const DataSetLayer& dataSetLayer,const osg::CopyOp& copyop):
    ProxyLayer(dataSetLayer)
{
    if (dataSetLayer._dataset) open();
}

DataSetLayer::~DataSetLayer()
{
    close();
}

void DataSetLayer::open()
{
    if (_dataset) return;

    if (getFileName().empty()) return;
    
    _dataset = static_cast<GDALDataset*>(GDALOpen(getFileName().c_str(),GA_ReadOnly));
    
    setUpLocator();
}

void DataSetLayer::close()
{
    if (_dataset)
    {
        GDALClose(static_cast<GDALDatasetH>(_dataset));
        
        _dataset = 0;
    }
}

unsigned int DataSetLayer::getNumColumns() const
{
    return _dataset!=0 ? _dataset->GetRasterXSize() : 0;
}

unsigned int DataSetLayer::getNumRows() const
{
    return _dataset!=0 ? _dataset->GetRasterYSize() : 0;
}

osgTerrain::ImageLayer* DataSetLayer::extractImageLayer(unsigned int sourceMinX, unsigned int sourceMinY, unsigned int sourceMaxX, unsigned int sourceMaxY, unsigned int targetWidth, unsigned int targetHeight)
{
    if (!_dataset || sourceMaxX<sourceMinX || sourceMaxY<sourceMinY) return 0;

    osg::notify(osg::NOTICE)<<"DataSetLayer::extractImageLayer("<<sourceMinX<<", "<<sourceMinY<<", "<<sourceMaxX<<", "<<sourceMaxY<<", target:"<<targetWidth<<", "<<targetHeight<<") not yet implemented"<<std::endl;

    return 0;
}

void DataSetLayer::setUpLocator()
{
    osg::notify(osg::NOTICE)<<"DataSetLayer::setUpLocator()"<<std::endl;
}
