/*****************************************************************************
 * Reader for Terrex TerraPage File format for OpenSceneGraph Library
 *
 * Copyright (C) 2002  Boris Bralo All Rights Reserved.
 * 
 * based on code from Scene Graph Library: 
 * Copyright (C) Bryan Walsh All Rights Reserved.
 * and Terrex
 * Copyright Terrain Experts Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *****************************************************************************/

#ifndef READER_WRITER_TXP_H
#define READER_WRITER_TXP_H

#include <osgTXP/trpage_sys.h>
#include <osg/Object>
#include <osg/Node>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>

namespace txp
{
class ReaderWriterTXP : public osgDB::ReaderWriter
{
public:
    virtual const char* className() { return "TXP Reader/Writer"; }
    virtual bool acceptsExtension(const std::string& extension)
    {
        return osgDB::equalCaseInsensitive(extension,"txp");
    }
    virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options*);
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options*);
};
}; // namespace
#endif
