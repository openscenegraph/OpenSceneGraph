// -*-c++-*-

/*
 * Copyright (C) 2001 Ulrich Hertlein <u.hertlein@web.de>
 *
 * The Open Scene Graph (OSG) is a cross platform C++/OpenGL library for 
 * real-time rendering of large 3D photo-realistic models. 
 * The OSG homepage is http://www.openscenegraph.org/
 *
 * This software is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _IMAGESTREAM_H_
#define _IMAGESTREAM_H_

#include <osg/Image>
#include <osg/TexMat>

namespace osg {

    /**
     * Image Stream class.
     */
    class SG_EXPORT ImageStream : public Image
    {
    public:
        ImageStream() {
            _texMat = new TexMat;
            _texMat->ref();
        }
      
        virtual Object* clone() const { return new ImageStream; }
        virtual bool isSameKindAs(const Object* obj) const {
            return dynamic_cast<const ImageStream*>(obj) != NULL;
        }
        virtual const char* className() const { return "ImageStream"; }

        /// Return suitable texture matrix.
        inline const TexMat* getTexMat() const { return _texMat; }

        /// Start or continue MPEG stream.
        virtual inline void start() {}

        /// Stop MPEG stream.
        virtual inline void stop() {}

        /// Rewind MPEG stream.
        virtual inline void rewind() {}

    protected:
        virtual ~ImageStream() {
            _texMat->unref();
        }

        TexMat* _texMat;
    };

} // namespace

#endif
