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

#ifndef _trpage_compat_h_
#define _trpage_compat_h_

/* trpage_compat.h
    This file and the accompanying trpage_compat.cpp contain objects and procedures
    used to maintain compatibility between versions of TerraPage.  In particular, the
    ability to read older versions of TerraPage and newer applications.
 */

/* Material Table 1.0.
    This class is used to read old 1.0 material tables and convert them
    into the 2.0 material table we're inheriting from.  Users should
    never directly interact with this class.
    {secret}
 */
class trpgMatTable1_0 : public trpgMatTable {
public:
    trpgMatTable1_0() { };
    trpgMatTable1_0(const trpgMatTable &);

    /* This read method overrides the one from trpgMatTable and knows
    how to read the old school material tables.
     */
    bool    Read(trpgReadBuffer &);
    /* This write method can write a 2.0 material table as 1.0
    style for backward compatibility.
     */
    bool    Write(trpgWriteBuffer &);
protected:
};

/* Texture Table 1.0.
    This class is used to read old 1.0 texture tables and convert them
    into 2.0 texture tables.  Users should never directly interact with
    this class.
    {secret}
 */
class trpgTexTable1_0 : public trpgTexTable {
public:
    trpgTexTable1_0() { };
    trpgTexTable1_0(const trpgTexTable &);

    /* This read method overrides the one from trpgTexTable and
    knows how to read the old style texture table.
     */
    bool    Read(trpgReadBuffer &);
    /* The write method can write a 2.0 texture table as 1.0
    style for backward compatibility.
     */
    bool    Write(trpgWriteBuffer &);
protected:
};

/* Texture 1.0.
    Knows how to read an old style texture.
    {secret}
 */
class trpgTexture1_0 : public trpgTexture {
public:
    // Assignment operator from a regular trpgTexture
    trpgTexture1_0 operator = (const trpgTexture &);

    // Knows how to read old style textures
    bool    Read(trpgReadBuffer &);
    // Can write old style textures
    bool    Write(trpgWriteBuffer &);
protected:
};

/* Tile Table 1.0
    Knows how to write an old style tile table.
    {secret}
 */
class trpgTileTable1_0 : public trpgTileTable {
public:
    trpgTileTable1_0(const trpgTileTable &);
    // Can write old style tile table
    bool    Write(trpgWriteBuffer &);
};

#endif
