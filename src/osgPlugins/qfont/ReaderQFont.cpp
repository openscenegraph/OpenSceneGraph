/* -*-c++-*- OpenSceneGraph - Copyright (C) 2009-2010 Mathias Froehlich
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
#include <osgDB/Registry>
#include <osgText/Font>

#include <QtCore/QThread>
#include <QtGui/QApplication>
#include <QtGui/QFont>
#include <QtGui/QFontDatabase>

#include <osgQt/QFontImplementation>

namespace osgQFont {

class ReaderQFont : public osgDB::ReaderWriter
{
    public:
        ReaderQFont()
        {
            supportsExtension("qfont", "Qt font meta loader");
        }

        virtual const char* className() const { return "QFont Font Reader"; }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            if (!acceptsExtension(osgDB::getLowerCaseFileExtension(file)))
                return ReadResult::FILE_NOT_HANDLED;

            if (!QApplication::instance())
            {
                OSG_WARN << "Trying to load qfont \"" << file << "\" from within a non qt application!" << std::endl;
                return ReadResult::FILE_NOT_FOUND;
            }

            if (!QFontDatabase::supportsThreadedFontRendering() && QApplication::instance()->thread() != QThread::currentThread())
            {
                OSG_WARN << "Trying to load qfont \"" << file << "\" from a non gui thread "
                    "within qt application without threaded font rendering!" << std::endl;
                return ReadResult::FILE_NOT_FOUND;
            }

            QFont font;
            if (!font.fromString(QString::fromStdString(osgDB::getNameLessExtension(file))))
                return ReadResult::FILE_NOT_FOUND;

            return new osgText::Font(new osgQt::QFontImplementation(font));
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(qfont, ReaderQFont)

}
