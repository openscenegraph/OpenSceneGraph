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

//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2007  Brede Johansen
//

#include <assert.h>
#include <osg/Geode>
#include <osg/Geometry>
#include "Registry.h"
#include "Document.h"
#include "RecordInputStream.h"

namespace flt {



/** PushLevel
*/
class PushLevel : public Record
{
    public:

        PushLevel() {}

        META_Record(PushLevel)

        virtual void readRecord(RecordInputStream& /*in*/, Document& document)
        {
            document.pushLevel();
        }

    protected:

        virtual ~PushLevel() {}
};

REGISTER_FLTRECORD(PushLevel, PUSH_LEVEL_OP)



/** PophLevel
*/
class PopLevel : public Record
{
    public:

        PopLevel() {}

        META_Record(PopLevel)

        virtual void read(RecordInputStream& /*in*/, Document& document)
        {
            PrimaryRecord* parentPrimary = document.getTopOfLevelStack();
            PrimaryRecord* currentPrimary = document.getCurrentPrimaryRecord();

            // Call dispose() for primary without push, pop level pair.
            if (currentPrimary && currentPrimary!=parentPrimary)
            {
                currentPrimary->dispose(document);
            }

            // Call dispose() for primary with push, pop level pair.
            if (parentPrimary)
            {
                parentPrimary->dispose(document);
            }

            document.popLevel();
        }

    protected:

        virtual ~PopLevel() {}
};

REGISTER_FLTRECORD(PopLevel, POP_LEVEL_OP)



/** PushSubface
*/
class PushSubface : public Record
{
    public:

        PushSubface() {}

        META_Record(PushSubface)

        virtual void read(RecordInputStream& /*in*/, Document& document)
        {
            document.pushSubface();
        }

    protected:

        virtual ~PushSubface() {}
};

REGISTER_FLTRECORD(PushSubface, PUSH_SUBFACE_OP)



/** PopSubface
*/
class PopSubface : public Record
{
    public:

        PopSubface() {}

        META_Record(PopSubface)

        virtual void read(RecordInputStream& /*in*/, Document& document)
        {
            document.popSubface();
        }

    protected:

        virtual ~PopSubface() {}
};

REGISTER_FLTRECORD(PopSubface, POP_SUBFACE_OP)



/** PushExtension
*/
class PushExtension : public Record
{
    public:

        PushExtension() {}

        META_Record(PushExtension)

        virtual void read(RecordInputStream& in, Document& document)
        {
            readRecord(in,document);
            document.pushExtension();
        }

    protected:

        virtual ~PushExtension() {}
};

REGISTER_FLTRECORD(PushExtension, PUSH_EXTENSION_OP)



/** PopExtension
*/
class PopExtension : public Record
{
    public:

        PopExtension() {}

        META_Record(PopExtension)

        virtual void read(RecordInputStream& in, Document& document)
        {
            readRecord(in,document);
            document.popExtension();
        }

    protected:

        virtual ~PopExtension() {}
};

REGISTER_FLTRECORD(PopExtension, POP_EXTENSION_OP)



/** PushAttribute - Reserved subtree
*/
class PushAttribute : public Record
{
    public:

        PushAttribute() {}

        META_Record(PushAttribute)

        virtual void read(RecordInputStream& in, Document& document)
        {
            readRecord(in,document);
        }

    protected:

        virtual ~PushAttribute() {}
};

REGISTER_FLTRECORD(PushAttribute, PUSH_ATTRIBUTE_OP)



/** PopAttribute
*/
class PopAttribute : public Record
{
    public:

        PopAttribute() {}

        META_Record(PopAttribute)

        virtual void read(RecordInputStream& in, Document& document)
        {
            readRecord(in,document);
        }

    protected:

        virtual ~PopAttribute() {}
};

REGISTER_FLTRECORD(PopAttribute, POP_ATTRIBUTE_OP)



} // end namespace



