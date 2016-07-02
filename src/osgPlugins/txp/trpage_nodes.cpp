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

/* trpage_nodes.cpp
   The methods for all the hierarchy nodes (e.g. groups, transforms, etc...)
   is here.
   You should only need to modify this if you want to add something to one
   of these classes.
*/

#include <trpage_geom.h>
#include <trpage_read.h>

/* Write Group
   Basic group.
*/

// Constructor
trpgGroup::trpgGroup()
{
    name = 0;
    Reset();

}
trpgGroup::~trpgGroup()
{
    Reset();
}

// Reset
void trpgGroup::Reset()
{
    numChild = 0;
    id = -1;
    if ( name ) {
        delete [] name;
        name = 0;
    }
}

// Set functions
void trpgGroup::SetNumChild(int no)
{
    numChild = no;
}
int trpgGroup::AddChild()
{
    numChild++;
    return numChild-1;
}
void trpgGroup::SetID(int inID)
{
    id = inID;
}

void trpgGroup::SetName(const char* newname )
{
    if ( name )
    {
        delete [] name;
        name = 0;
    }
    if (newname)
    {
        if ( strlen(newname) )
        {
            name = new char[strlen(newname)+1];
            strcpy(name,newname);
        }
    }
}

// Get methods
const char* trpgGroup::GetName(void) const
{
    return name;
}

bool trpgGroup::GetNumChild(int &n) const
{
    if (!isValid())  return false;
    n = numChild;
    return true;
}
bool trpgGroup::GetID(int &inID) const
{
    if (!isValid()) return false;
    inID = id;
    return true;
}

// Validity check
bool trpgGroup::isValid() const
{
    if (numChild <= 0)  return false;
    if (id < 0)  return false;

    return true;
}

// Write group
bool trpgGroup::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_GROUP);
    buf.Add(numChild);
    buf.Add(id);

    if ( name && strlen(name) ) {
        buf.Add(name);
    }

    buf.End();

    return true;
}

// Read group
bool trpgGroup::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(numChild);
        if (numChild < 0) throw 1;
        buf.Get(id);
        if (id < 0) throw 1;
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            SetName(nm);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}

/* Write Billboard
   Represents rotational billboarded geometry.
*/

// Constructor
trpgBillboard::trpgBillboard()
{
    name = 0;
    Reset();
}
trpgBillboard::~trpgBillboard()
{
    Reset();
}

// Reset function
void trpgBillboard::Reset()
{
    id = -1;
    mode = Axial;
    type = Group;
    axis = trpg3dPoint(0,0,1);
    center = trpg3dPoint(0,0,0);
    numChild = 0;
    if ( name )
    {
        delete [] name;
        name = 0;
    }
}

// Set functions
void trpgBillboard::SetCenter(const trpg3dPoint &pt)
{
    center = pt;
    valid = true;
}
void trpgBillboard::SetMode(int m)
{
    mode = m;
}
void trpgBillboard::SetAxis(const trpg3dPoint &pt)
{
    axis = pt;
}
void trpgBillboard::SetType(int t)
{
    type = t;
}

// Get methods
bool trpgBillboard::GetCenter(trpg3dPoint &pt) const
{
    if (!isValid()) return false;
    pt = center;
    return true;
}
bool trpgBillboard::GetMode(int &m) const
{
    if (!isValid()) return false;
    m = mode;
    return true;
}
bool trpgBillboard::GetAxis(trpg3dPoint &pt) const
{
    if (!isValid()) return false;
    pt = axis;
    return true;
}
bool trpgBillboard::GetType(int &t) const
{
    if (!isValid()) return false;
    t = type;
    return true;
}

// Write billboard
bool trpgBillboard::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_BILLBOARD);
    buf.Add(numChild);
    buf.Add(id);
    buf.Add((uint8)type);
    buf.Add((uint8)mode);
    buf.Add(center);
    buf.Add(axis);

    if ( name && strlen(name) ) {
        buf.Add(name);
    }
    buf.End();

    return true;
}

// Read billboard
bool trpgBillboard::Read(trpgReadBuffer &buf)
{
    uint8 uChar;

    try {
        buf.Get(numChild);
        buf.Get(id);
        buf.Get(uChar);  type = uChar;
        buf.Get(uChar);  mode = uChar;
        buf.Get(center);
        buf.Get(axis);
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            SetName(nm);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}

/* Write Level of Detail
   Represents LOD information.
*/

// Constructor
trpgLod::trpgLod()
{
    name = 0;
    Reset();
}
trpgLod::~trpgLod()
{
    Reset();
}

// Reset function
void trpgLod::Reset()
{
    id = -1;
    numRange = 0;
    center = trpg3dPoint(0,0,0);
    switchIn = switchOut = width = 0;
    rangeIndex = -1;
    valid = true;
    if ( name ) {
        delete [] name;
        name = 0;
    }
}

// Set functions
void trpgLod::SetCenter(const trpg3dPoint &pt)
{
    center = pt;
    valid = true;
}
void trpgLod::SetNumChild(int no)
{
    if (no < 0)
        return;

    numRange = no;
}
void trpgLod::SetLOD(double in,double out,double wid)
{
    switchIn = in;
    switchOut = out;
    width = wid;
}
void trpgLod::SetID(int inID)
{
    id = inID;
}

void trpgLod::SetName(const char* newname )
{
    if ( name ) {
        delete [] name;
        name = 0;
    }
    if (newname) {
        if ( strlen(newname) ) {
            name = new char[strlen(newname)+1];
            strcpy(name,newname);
        }
    }
}

void trpgLod::SetRangeIndex(int ri)
{
    rangeIndex = ri;
}

// Get methods
const char* trpgLod::GetName(void) const
{
    return name;
}

// Get functions
bool trpgLod::GetCenter(trpg3dPoint &pt) const
{
    if (!isValid()) return false;
    pt = center;
    return true;
}
bool trpgLod::GetNumChild(int &n) const
{
    if (!isValid()) return false;
    n = numRange;
    return true;
}
bool trpgLod::GetLOD(double &in,double &out,double &wid) const
{
    if (!isValid()) return false;
    in = switchIn;
    out = switchOut;
    wid = width;
    return true;
}
bool trpgLod::GetID(int &outID) const
{
    if (!isValid()) return false;
    outID = id;
    return true;
}
bool trpgLod::GetRangeIndex(int &ri) const
{
    if (!isValid()) return false;

    ri = rangeIndex;

    return true;
}

// Write out LOD
bool trpgLod::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_LOD);
    buf.Add(id);
    buf.Add(numRange);
    buf.Add(center);
    buf.Add(switchIn);
    buf.Add(switchOut);
    buf.Add(width);

    if ( name && strlen(name) ) {
        buf.Add(name);
    } else
        buf.Add("");


    buf.End();

    return true;
}

// Read in LOD
bool trpgLod::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(id);
        buf.Get(numRange);
        if (numRange < 0) throw 1;
        buf.Get(center);
        buf.Get(switchIn);
        buf.Get(switchOut);
        buf.Get(width);
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            if (*nm)
                SetName(nm);
            // Look for a range index
            if (!buf.isEmpty())
                buf.Get(rangeIndex);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}

/* Write Layer
   A layer is just a group with a different opcode.
*/

// Constructor
trpgLayer::trpgLayer()
{
    name = 0;
}

trpgLayer::~trpgLayer()
{
    Reset();
}

// Write it
bool trpgLayer::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_LAYER);
    buf.Add(numChild);
    buf.Add(id);

    if ( name && strlen(name) ) {
        buf.Add(name);
    }

    buf.End();

    return true;
}

// Read layer
bool trpgLayer::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(numChild);
        if (numChild < 0) throw 1;
        buf.Get(id);
        if (id < 0) throw 1;
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            SetName(nm);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}

// Reset function
void trpgLayer::Reset()
{
    numChild = 0;
    if ( name ) {
        delete [] name;
        name = 0;
    }
}

/* Write Transform
   Matrix defining the transform with children.
*/

// Constructor
trpgTransform::trpgTransform()
{
    name = 0;
    Reset();
}
trpgTransform::~trpgTransform()
{
    Reset();
}

// Reset function
void trpgTransform::Reset()
{
    id = -1;
    // Note: Is this row major or column major?
    m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
    m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;

    if ( name ) {
        delete [] name;
        name = 0;
    }
}

// Set functions
void trpgTransform::SetMatrix(const float64 *im)
{
    m[0][0] = im[4*0+0]; m[0][1] = im[4*0+1]; m[0][2] = im[4*0+2]; m[0][3] = im[4*0+3];
    m[1][0] = im[4*1+0]; m[1][1] = im[4*1+1]; m[1][2] = im[4*1+2]; m[1][3] = im[4*1+3];
    m[2][0] = im[4*2+0]; m[2][1] = im[4*2+1]; m[2][2] = im[4*2+2]; m[2][3] = im[4*2+3];
    m[3][0] = im[4*3+0]; m[3][1] = im[4*3+1]; m[3][2] = im[4*3+2]; m[3][3] = im[4*3+3];
}

// Get methods
bool trpgTransform::GetMatrix(float64 *rm) const
{
    if (!isValid()) return false;
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            // Note: is this right?
            rm[i*4+j] = m[i][j];
    return true;
}

// Write transform
bool trpgTransform::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_TRANSFORM);
    buf.Add(numChild);
    buf.Add(id);
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            buf.Add(m[i][j]);

    if ( name && strlen(name) ) {
        buf.Add(name);
    }
    buf.End();

    return true;
}

// Read transform
bool trpgTransform::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(numChild);
        buf.Get(id);
        if (numChild < 0) throw 1;
        for (int i=0;i<4;i++)
            for (int j=0;j<4;j++)
            buf.Get(m[i][j]);
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            SetName(nm);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}

/* Model Reference
   This is just a matrix transform and a model ID.
*/

// Constructor
trpgModelRef::trpgModelRef()
{
    Reset();
}

trpgModelRef::~trpgModelRef()
{
}

// Reset function
void trpgModelRef::Reset()
{
    m[0][0] = 1; m[0][1] = 0; m[0][2] = 0; m[0][3] = 0;
    m[1][0] = 0; m[1][1] = 1; m[1][2] = 0; m[1][3] = 0;
    m[2][0] = 0; m[2][1] = 0; m[2][2] = 1; m[2][3] = 0;
    m[3][0] = 0; m[3][1] = 0; m[3][2] = 0; m[3][3] = 1;
    modelRef = -1;
}

// Set functions
void trpgModelRef::SetModel(int id)
{
    modelRef = id;
    valid = true;
}
void trpgModelRef::SetMatrix(const float64 *im)
{
    m[0][0] = im[4*0+0]; m[0][1] = im[4*0+1]; m[0][2] = im[4*0+2]; m[0][3] = im[4*0+3];
    m[1][0] = im[4*1+0]; m[1][1] = im[4*1+1]; m[1][2] = im[4*1+2]; m[1][3] = im[4*1+3];
    m[2][0] = im[4*2+0]; m[2][1] = im[4*2+1]; m[2][2] = im[4*2+2]; m[2][3] = im[4*2+3];
    m[3][0] = im[4*3+0]; m[3][1] = im[4*3+1]; m[3][2] = im[4*3+2]; m[3][3] = im[4*3+3];
}

// Get methods
bool trpgModelRef::GetModel(int32 &mod) const
{
    if (!isValid()) return false;
    mod = modelRef;
    return true;
}
bool trpgModelRef::GetMatrix(float64 *rm) const
{
    if (!isValid()) return false;
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            // Note: is this right?
            rm[i*4+j] = m[i][j];
    return true;
}

// Write model reference
bool trpgModelRef::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPG_MODELREF);
    buf.Add(modelRef);
    for (int i=0;i<4;i++)
        for (int j=0;j<4;j++)
            buf.Add(m[i][j]);
    buf.End();

    return true;
}

// Read model reference
bool trpgModelRef::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(modelRef);
        if (modelRef < 0)
            throw 1;
        for (int i=0;i<4;i++)
            for (int j=0;j<4;j++)
                buf.Get(m[i][j]);
    }
    catch (...) {
        return false;
    }

    valid = true;
    return isValid();
}

/* Attach Node
   You'll find one of these in each tile, except for the lowest LOD.
   It's basically a group with some extra info that tells you where to attach it.
   The ID corresponds to the one in Group and LOD.
*/

// Constructor
trpgAttach::trpgAttach()
{
    name = 0;
    Reset();
}
trpgAttach::~trpgAttach()
{
    Reset();
}

// Reset
void trpgAttach::Reset()
{
    parentID = -1;
    childPos = -1;
    if ( name ) {
        delete [] name;
        name = 0;
    }
}

// Parent ID is the node this one gets attached to
void trpgAttach::SetParentID(int pid)
{
    parentID = pid;
}
bool trpgAttach::GetParentID(int &pid) const
{
    if (!isValid()) return false;
    pid = parentID;
    return true;
}

// Child Position is a unique number of parent
// It could be used as an array index, for example
void trpgAttach::SetChildPos(int cid)
{
    childPos = cid;
}
bool trpgAttach::GetChildPos(int &cid) const
{
    if (!isValid()) return false;
    cid = childPos;
    return true;
}

// Validity check
bool trpgAttach::isValid() const
{
    if (parentID < 0 || childPos < 0) return false;
    return true;
}

// Write Attach node
bool trpgAttach::Write(trpgWriteBuffer &buf)
{
    if (!isValid()) return false;

    buf.Begin(TRPG_ATTACH);
    buf.Add(numChild);
    buf.Add(id);
    buf.Add(parentID);
    buf.Add(childPos);

    if ( name && strlen(name) ) {
        buf.Add(name);
    }

    buf.End();

    return true;
}

// Read Attach node
bool trpgAttach::Read(trpgReadBuffer &buf)
{
    try {
        buf.Get(numChild);
        buf.Get(id);
        if (id < 0)  throw 1;
        buf.Get(parentID);
        if (parentID < 0) throw 1;
        buf.Get(childPos);
        if (childPos < 0) throw 1;
        if ( !buf.isEmpty() ) {
            char nm[1024] = {0};
            buf.Get(nm,1024);
            SetName(nm);
        }
    }
    catch (...) {
        return false;
    }

    return true;
}

/* ChildRef Node
   You'll find in the parent tile one of these for each tile children.
   It gives the children grid location and file address.
*/

// Constructor
trpgChildRef::trpgChildRef()
{
    Reset();
}
trpgChildRef::~trpgChildRef()
{
    Reset();
}

// Reset
void trpgChildRef::Reset()
{
    zmin = 0.0f;
    zmax = 0.0f;
    x = -1;
    y = -1;
    lod = -1;
    addr.file = -1;
    addr.offset = -1;
}

void trpgChildRef::SetTileLoc(int gx,int gy,int glod)
{
    x = gx;
    y = gy;
    lod = glod;
}

bool trpgChildRef::GetTileLoc(int &gx,int &gy,int &glod) const
{
    if (!isValid()) return false;

    gx = x;
    gy = y;
    glod = lod;

    return true;
}

void trpgChildRef::SetTileAddress(const trpgwAppAddress& gAddr)
{
    addr = gAddr;
}
void trpgChildRef::SetTileAddress(int32 file, int32 offset)
{
    addr.file = file;
    addr.offset = offset;
}
bool trpgChildRef::GetTileAddress(int32& file, int32& offset) const
{
    if (!isValid()) return false;

    file = addr.file;
    offset = addr.offset;

    return true;

}

bool trpgChildRef::GetTileAddress(trpgwAppAddress& gAddr) const
{
    if (!isValid()) return false;

    gAddr = addr;

    return true;
}

void trpgChildRef::SetTileZValue( float gZmin, float gZmax)
{
    zmin = gZmin;
    zmax = gZmax;
}

bool trpgChildRef::GetTileZValue( float& gZmin, float& gZmax) const
{
    if (!isValid()) return false;

    gZmin = zmin;
    gZmax = zmax;

    return true;
}


// Validity check
bool trpgChildRef::isValid() const
{
    if (lod < 0)
    return false;
    return true;
}


// Write Attach node
bool trpgChildRef::Write(trpgWriteBuffer &buf)
{
    if (!isValid()) return false;

    buf.Begin(TRPG_CHILDREF);

    buf.Add(lod);
    buf.Add(x);
    buf.Add(y);
    buf.Add(addr.file);
    buf.Add(addr.offset);
    buf.Add(zmin);
    buf.Add(zmax);

    buf.End();

    return true;
}

// Read Attach node
bool trpgChildRef::Read(trpgReadBuffer &buf)
{
    try
    {

        buf.Get(lod);
        if (lod <  0) throw 1;
        buf.Get(x);
        buf.Get(y);
        buf.Get(addr.file);
        buf.Get(addr.offset);
        buf.Get(zmin);
        buf.Get(zmax);

    }
    catch (...)
    {
        return false;
    }

    return true;
}

