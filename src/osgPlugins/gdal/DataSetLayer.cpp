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

using namespace GDALPlugin;

DataSetLayer::DataSetLayer()
{
    _dataset = 0;
}

DataSetLayer::DataSetLayer(const std::string& fileName)
{
    setFileName(fileName);
    _dataset = (GDALDataset*)GDALOpen(fileName.c_str(),GA_ReadOnly);
}

DataSetLayer::DataSetLayer(const DataSetLayer& dataSetLayer,const osg::CopyOp& copyop):
    ProxyLayer(dataSetLayer)
{
    _dataset = (GDALDataset*)GDALOpen(getFileName().c_str(),GA_ReadOnly);
}

DataSetLayer::~DataSetLayer()
{
    if (_dataset) delete _dataset;
}

unsigned int DataSetLayer::getNumColumns() const
{
    return _dataset!=0 ? _dataset->GetRasterXSize() : 0;
}

unsigned int DataSetLayer::getNumRows() const
{
    return _dataset!=0 ? _dataset->GetRasterYSize() : 0;
}

