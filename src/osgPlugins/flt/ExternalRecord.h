// ExternalRecord.h

#ifndef __FLT_EXTERNAL_RECORD_H
#define __FLT_EXTERNAL_RECORD_H


#include "opcodes.h"
#include "Record.h"
#include "RecordVisitor.h"

#include <osg/ref_ptr>

namespace flt {

struct SExternalReference
{
    SRecHeader    RecHeader;
    char    szPath[200];        // 199 char ASCII Path; 0 terminates
    // version 11, 12 & 13 stops here!
    uint8    swReserved[4];     // Reserved
    uint32   dwFlags;           // Flags (bits from left to right)
                                // 0 = Color Palette Override
                                // 1 = Material Palette Override
                                // 2 = Texture Palette Override
                                // 3 = Line Palette Override
                                // 4 = Sound Palette Override
                                // 5 = Light source Palette Override
                                // 6 = Light point Palette Override
                                // 7-31 Spare
//    int16    iReserved;       // Reserved
};


class ExternalRecord : public PrimNodeRecord
{
    public:
        ExternalRecord();

        virtual Record* clone() const { return new ExternalRecord(); }
        virtual const char* className() const { return "ExternalRecord"; }
        virtual int classOpcode() const { return EXTERNAL_REFERENCE_OP; }
        virtual void accept(RecordVisitor& rv) { rv.apply(*this); }
//      virtual void traverse(RecordVisitor& rv);

        SExternalReference* getData() const { return (SExternalReference*)_pData; }

        void setExternal(FltFile* pExternal);

        FltFile* getExternal() { return _fltfile.get(); }

        const std::string getFilename( void );
        std::string getModelName() ;

        enum Flag
        {
            COLOR_PALETTE_OVERRIDE      = BIT31,
            MATERIAL_PALETTE_OVERRIDE   = BIT30,
            TEXTURE_PALETTE_OVERRIDE    = BIT29,
            LINESTYLE_PALETTE_OVERRIDE  = BIT28,
            SOUND_PALETTE_OVERRIDE      = BIT27,
            LIGHTSOURCE_PALETTE_OVERRIDE = BIT26,
            LIGHT_POINT_PALETTE_OVERRIDE = BIT25
        };

    protected:
        virtual ~ExternalRecord();

//      virtual bool readLocalData(Input& fr);
//      virtual bool writeLocalData(Output& fw);

        virtual void endian();

        osg::ref_ptr<FltFile>    _fltfile;
};


}; // end namespace flt

#endif

