/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LECTOR_ESCRITOR_H__
#define __LECTOR_ESCRITOR_H__

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/ReaderWriter>

class VrmlReaderWriter: public osgDB::ReaderWriter {
public:
    VrmlReaderWriter() {  }
    virtual const char* className() { return "VRML"; }
    virtual bool acceptsExtension(const std::string& extension) {
		return osgDB::equalCaseInsensitive(extension,"wrl")
            || osgDB::equalCaseInsensitive(extension,"iv");
    }
    virtual ReadResult readNode(const std::string& fileName,const osgDB::ReaderWriter::Options*);
};

#endif
