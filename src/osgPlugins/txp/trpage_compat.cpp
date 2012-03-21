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

/* trpage_compat.cpp
   This file and the accompanying trpage_compat.h contain objects and procedures
   used to maintain compatibility between versions of TerraPage.  In particular, the
   ability to read older versions of TerraPage and newer applications.

*/

#include <trpage_geom.h>
#include <trpage_read.h>
#include <trpage_compat.h>

/* Old short Material definition from 1.0.
   {secret}
*/
class trpgShortMaterial {
public:
    // Full trpgMaterial definition this one is based on
    int baseMat;
    // Currently the only thing a short material overrides is texture
    std::vector<int> texids;
};

trpgMatTable1_0::trpgMatTable1_0(const trpgMatTable &inTbl)
{
    *((trpgMatTable *)this) = inTbl;
}

bool trpgMatTable1_0::Read(trpgReadBuffer &buf)
{
    trpgMaterial mat;
    trpgToken matTok;
    int32 len;
    bool status;
    unsigned int i,j,k;

    std::vector<trpgShortMaterial> shortTable;
    std::vector<trpgMaterial> baseMats;

    try {
        buf.Get(numTable);
        buf.Get(numMat);
        if (numTable <= 0 || numMat < 0) throw 1;

        // Short material tables are always full size
        shortTable.resize(numTable*numMat);

        // Look for short material table
        buf.GetToken(matTok,len);
        if (matTok == TRPGSHORTMATTABLE) {
            int32 numTex,texId;
            buf.PushLimit(len);
            for (i=0;i<(unsigned int)numTable;i++) {
                for (j=0;j<(unsigned int)numMat;j++) {
                    trpgShortMaterial &smat = shortTable[i*numMat+j];
                    buf.Get(smat.baseMat);
                    buf.Get(numTex);
                    for (k=0;k<(unsigned int)numTex;k++) {
                        buf.Get(texId);
                        smat.texids.push_back(texId);
                    }
                }
            }
            buf.PopLimit();

            // Now read the base materials
            int32 numBaseMat;
            buf.Get(numBaseMat);
            if (numBaseMat < 0) throw 1;
            baseMats.resize(numBaseMat);
            for (i=0;i<(unsigned int)numBaseMat;i++) {
                buf.GetToken(matTok,len);
                if (matTok != TRPGMATERIAL) throw 1;
                buf.PushLimit(len);
                mat.Reset();
                status = mat.Read(buf);
                buf.PopLimit();
                if (!status) throw 1;
                baseMats[i] = mat;
            }
        }
    }
    catch (...) {
        return false;
    }

    // Now convert to the new style material table
    for (i=0;i<shortTable.size();i++) {
        trpgShortMaterial &shortMat = shortTable[i];
        trpgMaterial &baseMat = baseMats[shortMat.baseMat];
        AddMaterial(baseMat,false);

        trpgMaterial newMat = baseMat;
        newMat.SetNumTexture(shortMat.texids.size());
        for (j=0;j<shortMat.texids.size();j++) {
            int texId;
            trpgTextureEnv texEnv;
            baseMat.GetTexture(j,texId,texEnv);
            newMat.SetTexture(j,shortMat.texids[j],texEnv);
        }
    }

    valid = true;
    return true;
}

bool trpgMatTable1_0::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    // Create one short material for every material
    std::vector<trpgShortMaterial> shortMats;
    shortMats.resize(numTable*numMat);
    // Iterate over the existing materials
    int i=0;

    MaterialMapType::iterator itr = materialMap.begin();
    for (  ; itr != materialMap.end( ); itr++) {
        //for (i=0;i<numTable*numMat;i++) {
        trpgMaterial &mat = itr->second; //matTables[i];
        // Fill in the short material
        trpgShortMaterial &sMat = shortMats[i];
        sMat.baseMat = 0;
        int numTex;
        mat.GetNumTexture(numTex);
        for (int j=0;j<numTex;j++) {
            int texId;
            trpgTextureEnv texEnv;
            mat.GetTexture(j,texId,texEnv);
            sMat.texids.push_back(texId);
            sMat.baseMat = i;
        }
        i++;
    }

    // Write the 1.0 material table
    buf.Begin(TRPGMATTABLE2);
    buf.Add(numTable);
    buf.Add(numMat);

    // Write the short materials
    buf.Begin(TRPGSHORTMATTABLE);
    for (i=0;i<static_cast<int>(shortMats.size());i++) {
        trpgShortMaterial &sMat = shortMats[i];
        buf.Add(sMat.baseMat);
        buf.Add((int)(sMat.texids.size()));
        unsigned int j;
        for (j=0;j<sMat.texids.size();j++)
            buf.Add(sMat.texids[j]);
    }
    buf.End();

    // Write the regular materials
    buf.Add((int)materialMap.size());//numTable*numMat);
    //for (i=0;i<numTable*numMat;i++) {
    itr = materialMap.begin();
    for (  ; itr != materialMap.end( ); itr++) {

        trpgMaterial &mat = itr->second; //matTables[i];

        // This will be bigger than the old 1.0 material, but it doesn't matter since
        //  the new stuff is on the end.
        mat.Write(buf);
    }

    // Close Mat Table
    buf.End();

    return true;
}

trpgTexture1_0 trpgTexture1_0::operator = (const trpgTexture &inTex)
{
    *((trpgTexture *)this) = inTex;

    return *this;
}

bool trpgTexture1_0::Read(trpgReadBuffer &buf)
{
    mode = External;

    try {
        char texName[1024];
        buf.Get(texName,1023);
        SetName(texName);
        buf.Get(useCount);
    }
    catch (...) {
        return false;
    }

    return true;
}

bool trpgTexture1_0::Write(trpgWriteBuffer &buf)
{
    // Can only deal with external textures in 1.0
    if (mode != External)
        return false;

    // Write the name and use count
    buf.Add(name);
    buf.Add(useCount);

    return true;
}

trpgTexTable1_0::trpgTexTable1_0(const trpgTexTable &inTbl)
{
    *((trpgTexTable *)this) = inTbl;
}

bool trpgTexTable1_0::Read(trpgReadBuffer &buf)
{
    int32 numTex;

    try {
        buf.Get(numTex);
        //texList.resize(numTex);
        for (int i=0;i<numTex;i++) {
            trpgTexture1_0 tex1_0;
            tex1_0.Read(buf);
            AddTexture(tex1_0);
            //texList[i] = tex1_0;
        }
    }
    catch (...) {
        return false;
    }

    valid = true;
    return true;
}

bool trpgTexTable1_0::Write(trpgWriteBuffer &buf)
{
    int32 numTex;

    if (!isValid())
        return false;

    buf.Begin(TRPGTEXTABLE);

    numTex = textureMap.size();
    buf.Add(numTex);
    TextureMapType::const_iterator itr = textureMap.begin();
    for (  ; itr != textureMap.end( ); itr++) {
        trpgTexture1_0 tex1_0;
        tex1_0 = itr->second;
        if (!tex1_0.Write(buf))
            return false;
    }
    buf.End();

    return true;

}

trpgTileTable1_0::trpgTileTable1_0(const trpgTileTable& /*inTable*/)
{
    // Nothing to copy for now
}

bool trpgTileTable1_0::Write(trpgWriteBuffer &buf)
{
    try {
        buf.Begin(TRPGTILETABLE);
        buf.Add("");
        buf.End();
    }
    catch (...) {
        return false;
    }

    return true;
}
