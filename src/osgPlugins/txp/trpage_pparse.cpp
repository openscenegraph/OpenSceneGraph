/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the President of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   4400 East Broadway #314
   Tucson, AZ  85711
   info@terrex.com
   Tel: (520) 323-7990
   ************************
   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_pparse.cpp
    This file contains classes that can parse a TerraPage
    archive for the purpose of printing it out.
 */

#include "trpage_print.h"
#include "trpage_scene.h"

/* Set up the callbacks for the scene graph parser.
    In our case this is just one read helper with
    a switch statement.
 */
trpgPrintGraphParser::trpgPrintGraphParser(trpgPrintBuffer *inBuf)
{
    printBuf = inBuf;

    // Register the readers
    AddCallback(TRPG_GEOMETRY,new ReadHelper(printBuf));
    AddCallback(TRPG_GROUP,new ReadHelper(printBuf));
    AddCallback(TRPG_ATTACH,new ReadHelper(printBuf));
    AddCallback(TRPG_BILLBOARD,new ReadHelper(printBuf));
    AddCallback(TRPG_LOD,new ReadHelper(printBuf));
    AddCallback(TRPG_TRANSFORM,new ReadHelper(printBuf));
    AddCallback(TRPG_MODELREF,new ReadHelper(printBuf));
    AddCallback(TRPG_LAYER,new ReadHelper(printBuf));
    AddCallback(TRPG_LIGHT,new ReadHelper(printBuf));
    AddCallback(TRPGTILEHEADER,new ReadHelper(printBuf));
}

/* Start Children is called when the parser hits a Push
    in the read buffer.  We just want to indent further when
    that happens.
 */
bool trpgPrintGraphParser::StartChildren(void *)
{
    printBuf->IncreaseIndent();

    return true;
}


/* End Children is called when the parser hits a Pop
    in the read buffer.  We just want to reduce the indent
    when that happens.
 */
bool trpgPrintGraphParser::EndChildren(void *)
{
    printBuf->DecreaseIndent();

    return true;
}

/* Read Helper parse method sets up the correct class depending
    on the token and asks it to read and print itself.
 */
void *trpgPrintGraphParser::ReadHelper::Parse(trpgToken tok,trpgReadBuffer &buf)
{
    trpgReadWriteable *obj = NULL;

    switch (tok) {
    case TRPG_GEOMETRY:
        obj = new trpgGeometry();
        break;
    case TRPG_GROUP:
        obj = new trpgGroup();
        break;
    case TRPG_ATTACH:
        obj = new trpgAttach();
        break;
    case TRPG_BILLBOARD:
        obj = new trpgBillboard();
        break;
    case TRPG_LOD:
        obj = new trpgLod();
        break;
    case TRPG_TRANSFORM:
        obj = new trpgTransform();
        break;
    case TRPG_MODELREF:
        obj = new trpgModelRef();
        break;
    case TRPG_LAYER:
        obj = new trpgLayer();
        break;
    case TRPG_LIGHT:
        obj = new trpgLight();
        break;
    case TRPGTILEHEADER:
        obj = new trpgTileHeader();
        break;
    };

    if (obj) {
        if (obj->Read(buf))
            obj->Print(*pBuf);
        delete obj;
    }

    // Need to return non-zero.  Otherwise it's interpreted as an error
    return (void *)1;
}

// The following routine is not compiled if there's no _splitpath
#ifdef _splitpath
/* This is a convenience function to print out the contents
    of an entire TerraPage archive.

   There are two versions of this function.  The first takes
   a file name and the second an opened archive where the header
   has already been read.
 */
bool trpgPrintArchive(char *filename,trpgPrintBuffer &pBuf,int flags)
{
    trpgr_Archive archive;

    // Break path apart so we can find the directory
    char drive[100],dir[1024],fname[1024],ext[1024];
    _splitpath(filename,drive,dir,fname,ext);

    char rname[1024],baseDir[1024];
    sprintf(baseDir,"%s%s",drive,dir);
    sprintf(rname,"%s%s",fname,ext);

    if (!*baseDir) strcpy(baseDir,".");
    archive.SetDirectory(baseDir);
    if (!archive.OpenFile(rname)) {
        fprintf(stdout,"Failed to open archive.\n");
        return false;
    }
    if (!archive.ReadHeader()) {
        fprintf(stdout,"Failed to read header.\n");
        return false;
    }

    bool status = trpgPrintArchive(&archive,pBuff,flags);
    return status;
}
#endif

bool trpgPrintArchive(trpgr_Archive *archive,trpgPrintBuffer &pBuf,int flags)
{
    char ls[1024];

    if (!archive->isValid())  return false;    
    
    pBuf.prnLine("====Header Structures====");

    // Print out the header portion
    archive->GetHeader()->Print(pBuf);
    archive->GetMaterialTable()->Print(pBuf);
    archive->GetTexTable()->Print(pBuf);
    archive->GetModelTable()->Print(pBuf);
    archive->GetTileTable()->Print(pBuf);
    archive->GetLightTable()->Print(pBuf);
    archive->GetRangeTable()->Print(pBuf);
    pBuf.prnLine();

    // Now do the tiles
    if (!archive->isValid())  return false;    

    // Parser that prints out a tile scene graph
    trpgPrintGraphParser parser(&pBuf);

    pBuf.prnLine("====Tile Data====");
    int nl,x,y;
    trpgMemReadBuffer buf(archive->GetEndian());
    // Iterate over the terrain lods
    int numLod;
    archive->GetHeader()->GetNumLods(numLod);
    trpg2iPoint tileSize;
    for (nl=0;nl<numLod;nl++) {
        archive->GetHeader()->GetLodSize(nl,tileSize);
        // Iterate over the tiles
        for (x=tileSize.x-1;x>=0;x--)
            for (y=0;y<tileSize.y;y++) {
                sprintf(ls,"Tile (lod) (x,y) = (%d) (%d,%d)",nl,x,y);
                pBuf.prnLine(ls);
                if (archive->ReadTile(x,y,nl,buf)) {
                    if (flags & TRPGPRN_BODY) {
                        pBuf.IncreaseIndent();
                        // Parse it (also prints it
                        parser.Parse(buf);
                        pBuf.DecreaseIndent();
                    }
                } else
                    pBuf.prnLine("  Couldn't read tile.");
            }
    }

    return true;
}
