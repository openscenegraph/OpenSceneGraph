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

#include <trpage_print.h>
#include <trpage_scene.h>
#include <trpage_managers.h>

namespace
{
   // This will recursivelly call itself up until
   // all the tiule are done
   void printBuf(int lod, int x, int y, trpgr_Archive *archive, trpgPrintGraphParser& parser, trpgMemReadBuffer &buf, trpgPrintBuffer &pBuf)
   {
      char ls[1024];
       sprintf(ls,"Tile (lod) (x,y) = (%d) (%d,%d)", lod, x, y);
       pBuf.prnLine(ls);
      pBuf.IncreaseIndent();
      parser.Reset();
      parser.Parse(buf);
      pBuf.DecreaseIndent();

      // Save the list
//      std::vector<const trpgChildRef> childRefList;
// The const in the template parameter was removed because it causes GCC to
// freak out.  I am of the opinion that const doesn't make sense in a template
// parameter for std::vector anyway... const prevents you from changing the
// value, so what exactly is the point?  How does one add entries to the vector
// without giving them a value?  -ADS
      std::vector<trpgChildRef> childRefList;
      for(unsigned int idx =0; idx < parser.GetNbChildrenRef(); idx++)
         childRefList.push_back(*parser.GetChildRef(idx));


      for(unsigned int idx =0; idx < childRefList.size(); idx++)
      {
         const trpgChildRef& childRef = childRefList[idx];
         trpgMemReadBuffer childBuf(archive->GetEndian());
         trpgwAppAddress tileAddr;
         int glod, gx, gy;

         childRef.GetTileAddress(tileAddr);
         childRef.GetTileLoc(gx,gy,glod);

         trpgTileTable::TileMode mode;
         archive->GetTileTable()->GetMode(mode);
         bool status;
         if(mode == trpgTileTable::Local)
            status = archive->ReadTile(tileAddr, childBuf);
         else
            status = archive->ReadExternalTile(gx, gy, glod, childBuf);

         if(status)
            printBuf(glod, gx, gy, archive, parser, childBuf, pBuf);

      }
   }
} // end namespace









/* Set up the callbacks for the scene graph parser.
    In our case this is just one read helper with
    a switch statement.
 */
trpgPrintGraphParser::trpgPrintGraphParser(trpgr_Archive *inArch,trpgrImageHelper *inImg,trpgPrintBuffer *inBuf):printBuf(inBuf), archive(inArch), imageHelp(inImg), childRefCB(0)
{
    // Register the readers
    AddCallback(TRPG_GEOMETRY,new ReadHelper(this,printBuf));
    AddCallback(TRPG_GROUP,new ReadHelper(this,printBuf));
    AddCallback(TRPG_ATTACH,new ReadHelper(this,printBuf));
   AddCallback(TRPG_CHILDREF,new ReadHelper(this,printBuf));
    AddCallback(TRPG_BILLBOARD,new ReadHelper(this,printBuf));
    AddCallback(TRPG_LOD,new ReadHelper(this,printBuf));
    AddCallback(TRPG_TRANSFORM,new ReadHelper(this,printBuf));
    AddCallback(TRPG_MODELREF,new ReadHelper(this,printBuf));
    AddCallback(TRPG_LAYER,new ReadHelper(this,printBuf));
    AddCallback(TRPG_LIGHT,new ReadHelper(this,printBuf));
    AddCallback(TRPG_LABEL,new ReadHelper(this,printBuf));
    AddCallback(TRPGTILEHEADER,new ReadHelper(this,printBuf));

   childRefCB = dynamic_cast<ReadHelper *>(GetCallback(TRPG_CHILDREF));
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

unsigned int trpgPrintGraphParser::GetNbChildrenRef() const
{
   if(childRefCB)
      return childRefCB->GetNbChildrenRef();
   else
      return 0;
}

const trpgChildRef* trpgPrintGraphParser::GetChildRef(unsigned int idx) const
{
   if(childRefCB)
      return childRefCB->GetChildRef(idx);
   else
      return 0;

}



void trpgPrintGraphParser::Reset()
{
   if(childRefCB)
      childRefCB->Reset();
}

void trpgPrintGraphParser::ReadHelper::Reset()
{
   childRefList.clear();
}

unsigned int trpgPrintGraphParser::ReadHelper::GetNbChildrenRef() const
{
   return childRefList.size();
}
const trpgChildRef* trpgPrintGraphParser::ReadHelper::GetChildRef(unsigned int idx) const
{
   if(idx >= childRefList.size())
      return 0;
   else
      return &childRefList[idx];
}

/* Read Helper parse method sets up the correct class depending
    on the token and asks it to read and print itself. It will save
   any child ref node encountered that a user can access to continue
   traversal.
 */
void *trpgPrintGraphParser::ReadHelper::Parse(trpgToken tok,trpgReadBuffer &buf)
{
   // This will celar any child ref list from a previous parse.
    trpgReadWriteable *obj = NULL;
    trpgTileHeader *tileHead = NULL;

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
   case TRPG_CHILDREF:
      childRefList.push_back(trpgChildRef());
      obj = &childRefList.back();
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
    case TRPG_LABEL:
        obj = new trpgLabel();
        break;

    case TRPGTILEHEADER:
        obj = tileHead = new trpgTileHeader();
        break;
    };

    if (obj) {
        if (obj->Read(buf))
            obj->Print(*pBuf);
        // For the tile header, do a little more work
        if (tok == TRPGTILEHEADER) {
            int numMat;
            tileHead->GetNumLocalMaterial(numMat);
            for (int i=0;i<numMat;i++) {
                trpgLocalMaterial locMat;
                tileHead->GetLocalMaterial(i,locMat);
                const trpgMaterial *baseMat;
                const trpgTexture *baseTex;
                int totSize;
                trpgrImageHelper *imageHelp = parse->GetImageHelp();
                int numImages=1;
                locMat.GetNumLocals(numImages);
                for (int imgN=0;imgN<numImages;imgN++) {
                    // read all the images for each local material
                    imageHelp->GetNthImageInfoForLocalMat(&locMat,imgN,&baseMat,&baseTex,totSize);

                    // Fetch the whole image
                    {
                        char *pixels = new char[totSize];
                        bool failed = false;
                        try {
                            failed = !imageHelp->GetNthImageForLocalMat(&locMat,imgN,pixels,totSize);
                        }
                        catch (...) {
                            failed = true;
                        }
                        if (failed) {
                            fprintf(stderr,"Failed to read local image %d from local material %d.\n",imgN,i);
                        } else
                            fprintf(stderr,"Read local image %d from local material %d successfully.\n",imgN,i);
                        delete [] pixels;
                    }

                    // Fetch the individual mipmap levels
                    {

                        bool hasMipmap = false;
                        baseTex->GetIsMipmap(hasMipmap);
                        int numMipmap = hasMipmap ? baseTex->CalcNumMipmaps() : 0;
                        for (int j=1;j<numMipmap;j++) {
                            //int mipOffset = (const_cast<trpgTexture *>(baseTex))->MipLevelOffset(j);
                            int mipSize = (const_cast<trpgTexture *>(baseTex))->MipLevelSize(j);
                            if (mipSize) {
                                char *pixels = new char[mipSize];
                                bool failed = false;
                                try {
                                    failed = !imageHelp->GetNthImageMipLevelForLocalMat(j,&locMat,imgN,pixels,mipSize);
                                }
                                catch (...) {
                                    failed = true;
                                }
                                if (failed)
                                    fprintf(stderr,"Failed to read mipmap level %d for local image %d from local material %d.\n",j,imgN,i);
                                else
                                    fprintf(stderr,"Read mipmap level %d for local image %d from local material %d.\n",j,imgN,i);
                                delete [] pixels;
                            }
                        }
                    }
                }
            }
        }

      // We delete all object except the child ref node
      if(tok != TRPG_CHILDREF)
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
    archive->GetTextStyleTable()->Print(pBuf);
    archive->GetSupportStyleTable()->Print(pBuf);
    archive->GetLabelPropertyTable()->Print(pBuf);
    pBuf.prnLine();

    // Read the local images and do the math for the templates

    // Now do the tiles
    if (!archive->isValid())  return false;

   int majorVersion, minorVersion;
   archive->GetHeader()->GetVersion(majorVersion, minorVersion);

    // Parser that prints out a tile scene graph
    trpgrImageHelper* imageHelp=archive->GetNewRImageHelper(archive->GetEndian(),archive->getDir(),
            *archive->GetMaterialTable(),*archive->GetTexTable());

    trpgPrintGraphParser parser(archive,imageHelp,&pBuf);

    pBuf.prnLine("====Tile Data====");
    int nl,x,y;
    trpgMemReadBuffer buf(archive->GetEndian());
    // Iterate over the terrain lods
    int numLod;
    archive->GetHeader()->GetNumLods(numLod);
    trpg2iPoint tileSize;
   if(majorVersion == 2 && minorVersion >= 1)
   {
      // Version 2.1
      // Because of variable lod support in version 2.1 and over, we can
      // no longer suppose that all lod level are all populated with tiles
      // in all of the gaming area. We have to parse the parent to know that.
      // Also the tile table only contains lod 0 tiles so we can no longer access
      // the tile directly from its grid location. So we have to traverse.
      trpg2iPoint blockTileSize;
      if(archive->GetHeader()->GetLodSize(0,blockTileSize)) {
        for(x = 0; x < blockTileSize.x; x++)
            for( y = 0; y < blockTileSize.y; y++)
                if (archive->ReadTile(x,y,0,buf))
                printBuf(0, x, y, archive, parser, buf, pBuf);

      }

   }
   else
   {
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
                          if (!parser.Parse(buf))
                          {
                              char errString[80];
                              sprintf(errString, "**** Warning: tile anomaly detected: (%d) (%d,%d) ****",nl,x,y);
                              // send it both ways so it's easier to spot
                              pBuf.prnLine(errString);
                              fprintf(stderr,"%s\n",errString);
                          }
                          pBuf.DecreaseIndent();
                       }
                   } else
                       pBuf.prnLine("  Couldn't read tile.");
               }
       }
   }

    return true;
}
