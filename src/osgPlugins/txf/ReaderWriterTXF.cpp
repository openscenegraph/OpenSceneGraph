/* -*-c++-*- OpenSceneGraph - Copyright (C) 2006 Mathias Froehlich
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

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>
#include <osg/Notify>

#include "TXFFont.h"

class ReaderWriterTXF : public osgDB::ReaderWriter
{
    public:
        ReaderWriterTXF()
        {
            supportsExtension("txf","TXF Font format");
        }

        virtual const char* className() const { return "TXF Font Reader/Writer"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile(file, options);
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream stream;
            stream.open(fileName.c_str(), std::ios::in | std::ios::binary);
            if (!stream.is_open()) return ReadResult::FILE_NOT_FOUND;

            TXFFont* impl = new TXFFont(fileName);
            osg::ref_ptr<osgText::Font> font = new osgText::Font(impl);
            if (!impl->loadFont(stream)) return ReadResult::FILE_NOT_HANDLED;
            return font.release();
        }

        virtual ReadResult readObject(std::istream& stream, const osgDB::ReaderWriter::Options*) const
        {
            TXFFont* impl = new TXFFont("streamed font");
            osg::ref_ptr<osgText::Font> font = new osgText::Font(impl);
            if (!impl->loadFont(stream)) return ReadResult::FILE_NOT_HANDLED;
            return font.release();
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(txf, ReaderWriterTXF)
