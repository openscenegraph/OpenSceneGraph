//
// OpenFlight® loader for OpenSceneGraph
//
//  Copyright (C) 2005-2006  Brede Johansen
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

        virtual void read(RecordInputStream& /*in*/, Document& document)
        {
            document.pushLevel();
        }

    protected:

        virtual ~PushLevel() {}
};

RegisterRecordProxy<PushLevel> g_PushLevel(PUSH_LEVEL_OP);


/** PophLevel
*/
class PopLevel : public Record
{
    public:

        PopLevel() {}

        META_Record(PopLevel)

        virtual void read(RecordInputStream& /*in*/, Document& document)
        {
            document.popLevel();
        }

    protected:

        virtual ~PopLevel() {}
};

RegisterRecordProxy<PopLevel> g_PopLevel(POP_LEVEL_OP);


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

RegisterRecordProxy<PushSubface> g_PushSubface(PUSH_SUBFACE_OP);


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

RegisterRecordProxy<PopSubface> g_PopSubface(POP_SUBFACE_OP);


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

RegisterRecordProxy<PushExtension> g_PushExtension(PUSH_EXTENSION_OP);


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

RegisterRecordProxy<PopExtension> g_PopExtension(POP_EXTENSION_OP);


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
    //      in().seekg(in.getEndOfRecord(), std::ios_base::beg);
            // loop until PopAttribute

        }

    protected:

        virtual ~PushAttribute() {}
};

RegisterRecordProxy<PushAttribute> g_PushAttribute(PUSH_ATTRIBUTE_OP);


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

RegisterRecordProxy<PopAttribute> g_PopAttribute(POP_ATTRIBUTE_OP);


} // end namespace



