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

#define OLDMODELSTYLE

/* trpage_model.cpp
    This source file contains the methods trpgModel and trpgModelTable.
    You should only modify this code if you want to add data to these classes.
    */

#include <trpage_geom.h>
#include <trpage_read.h>

/* Write Model class
    Represents a model reference.
    */
trpgModel::trpgModel()
{
    name = NULL;
    type = External;
    useCount = 0;
    diskRef = -1;
    handle = -1;
    writeHandle = false;
}

trpgModel::trpgModel(const trpgModel &in):
    trpgReadWriteable(in)
{
    if (in.name)
    {
        name = new char[strlen(in.name)+1];
        strcpy(name,in.name);
    }
    else
        name = NULL;

    type=in.type;
    useCount=in.useCount;
    diskRef=in.diskRef;
    handle = in.handle;
    writeHandle = in.writeHandle;
}


// Reset function
void trpgModel::Reset()
{
    if (name)
        delete [] name;
    name = NULL;
    useCount = 0;
    diskRef = -1;
    handle = -1;
    writeHandle = false;
}

trpgModel::~trpgModel()
{
    Reset();
}

// Set functions
void trpgModel::SetType(int t)
{
    type = t;
}

void trpgModel::SetName(const char *nm)
{
    if (name)
        delete [] name;

    if (nm)
    {
        name = new char[(nm ? strlen(nm) : 0)+1];
        strcpy(name,nm);
    }
}
void trpgModel::SetReference(trpgDiskRef pos)
{
    diskRef = pos;
}
void trpgModel::SetNumTiles(int num)
{
    useCount = num;
}
void trpgModel::AddTile()
{
    useCount++;
}

// Validity check
bool trpgModel::isValid() const
{
    if (type == External && !name)
    {
        strcpy(errMess, "Model is external with no name");
        return false;
    }

    return true;
}

// Copy from one to another
trpgModel& trpgModel::operator = (const trpgModel &in)
{
    if (name) {
        delete [] name;
        name = NULL;
    }

    type = in.type;
    if (in.name)
        SetName(in.name);
    diskRef = in.diskRef;
    useCount = in.useCount;
    writeHandle = in.writeHandle;
    handle = in.handle;
    return *this;
}

// Compare two models
int trpgModel::operator == (const trpgModel &in) const
{
    if (type != in.type)
        return 0;

    switch (type) {
    case Local:
        if (diskRef == in.diskRef)
            return 1;
        else
            return 0;
        break;
    case External:
        if (!name && !in.name)
            return 1;
        if (!name || !in.name)
            return 0;
        if (strcmp(name,in.name))
            return 0;
        break;
    }

    return 1;
}

// Write a model reference out
bool trpgModel::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    // We will use two different tokens to track the
    // format used in terrapage 2.2, and older versions
    int tok = TRPGMODELREF;
    if(writeHandle)
        tok = TRPGMODELREF2;

// Nick messed up the model entries when checking into txv4; now we're
// a bit stuck because U3 dbs don't have models in the U2 viewer.
// This will force the old behavior.
#ifdef OLDMODELSTYLE
    buf.Begin(tok);
    buf.Add(type);
    //writeHandle is only set for terrapage 2.2, and we use the different token.
    if(writeHandle) {
        buf.Add((int)handle);
    }
    if (name)
        buf.Add(name);
    else
        buf.Add(diskRef);
    buf.Add(useCount);

#else
    buf.Begin(tok);
    if(writeHandle) {
        buf.Add((int)handle);
    }
    buf.Add(type);
    buf.Add(name);
    buf.Add(diskRef);
    buf.Add(useCount);
#endif
    buf.End();

    return true;
}

/*    *******************
    Model Read Methods
    *******************
    */
// Get methods
bool trpgModel::GetType(int &t)
{
    if (!isValid()) return false;
    t = type;
    return true;
}
bool trpgModel::GetName(char *str,int strLen) const
{
    if (!isValid()) return false;
    int len = (name ? strlen(name) : 0);
    strncpy(str,name,MIN(len,strLen)+1);
    return true;
}
bool trpgModel::GetNumTiles(int &ret) const
{
    if (!isValid()) return false;

    ret = useCount;
    return true;
}
bool trpgModel::GetReference(trpgDiskRef &ref) const
{
    if (!isValid() || type != Local) return false;
    ref = diskRef;
    return true;
}

bool trpgModel::Read(trpgReadBuffer &buf, bool hasHandle)
{
    // MD: added complexity here - written multiple ways by
    // mistake, unraveling the various cases.
    char tmpName[1024];

    try {
        buf.Get(type);
        // TerraPage 2.2 will store the unique handle after the type
        // we use a different token, so this is backwards compatible.
        if(hasHandle) {
            int32 tempHandle;
            if(buf.Get(tempHandle))
            {
                handle = tempHandle;
            }
            else
            {
                handle = -1;
            }
        }
        else
            handle = -1;

        if (type == Local) {
            // two possibilities:
            // name, diskRef, useCount
            // diskRef, useCount
            // diskRef + useCount = 12 bytes...
            if (buf.TestLimit(13))
            {
                buf.Get(tmpName,1023);
                SetName(tmpName);
            }
            buf.Get(diskRef);
            buf.Get(useCount);
        }
        else
        {
            // two possibilities:
            // name, diskRef, useCount
            // name, useCount
            buf.Get(tmpName,1023);
            SetName(tmpName);
            // useCount = 4 bytes...
            if (buf.TestLimit(5))
                buf.Get(diskRef);
            buf.Get(useCount);
        }
    }
    catch(...) {
        return false;
    }

    // going to make this fail if the buffer isn't empty.
    if (buf.TestLimit(1)) return false;

    return isValid();
}

/* Write Model Reference table
    Groups of models for the entire file.
    */

// Constructor
trpgModelTable::trpgModelTable()
{
}

trpgModelTable::~trpgModelTable()
{
}

// Reset function
void trpgModelTable::Reset()
{
    modelsMap.clear();
}

// Set functions
void trpgModelTable::SetNumModels(int /*no*/)
{
    // This method isn't needed with a map
    //models.resize(no);
}
void trpgModelTable::SetModel(int id,const trpgModel &mod)
{
    if (id < 0)
        return;

    modelsMap[id] = mod;
    //models[id] = mod;
}

int trpgModelTable::AddModel(trpgModel &mod)
{
    int hdl = modelsMap.size();
    if(mod.GetHandle()==-1) {
        modelsMap[hdl] = mod;
        return hdl;
    }
    modelsMap[mod.GetHandle()] = mod;
    return mod.GetHandle();
}

int trpgModelTable::FindAddModel(trpgModel &mod)
{

    ModelMapType::iterator itr = modelsMap.begin();
    for (  ; itr != modelsMap.end( ); itr++) {
        if(itr->second == mod) {
            return itr->first;
        }
    }
    return AddModel(mod);
}

bool trpgModelTable::FindByName(const char *name, unsigned int &mId)
{
    ModelMapType::const_iterator itr = modelsMap.begin();
    for (  ; itr != modelsMap.end( ); itr++) {
        char theName[1023];
        itr->second.GetName(theName,1023);
        if(strcmp(name,theName)==0) {
            mId = itr->first;
            return true;
        }
    }
    return false;
}

// Validity check
bool trpgModelTable::isValid() const
{
    ModelMapType::const_iterator itr = modelsMap.begin();
    for (  ; itr != modelsMap.end( ); itr++) {
        if(!itr->second.isValid()) {
            if(itr->second.getErrMess())
                strcpy(errMess, itr->second.getErrMess());
            return false;
        }
    }
    return true;
}



// Write out the model table
bool trpgModelTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGMODELTABLE);
    buf.Add((int32)modelsMap.size());
    ModelMapType::iterator itr = modelsMap.begin();
    for (  ; itr != modelsMap.end( ); itr++) {
        itr->second.Write(buf);
    }
    buf.End();

    return true;
}

/*    ***************
    Model Table Read methods
    ***************
    */

// Get methods
bool trpgModelTable::GetNumModels(int &nm) const
{
    if (!isValid())  return false;
    nm = modelsMap.size();
    return true;
}
bool trpgModelTable::GetModel(int id,trpgModel &model) const
{
    if (!isValid() || id < 0 ) //|| id >= models.size())
        return false;
    //model = models[id];
    ModelMapType::const_iterator itr = modelsMap.find(id);
    if(itr == modelsMap.end())    {
        return false;
    }
    model = itr->second;
    return true;
}
trpgModel *trpgModelTable::GetModelRef(int id)
{
    if (id < 0 ) //|| id >= models.size())
        return NULL;
    //return &models[id];
    ModelMapType::iterator itr = modelsMap.find(id);
    if(itr == modelsMap.end())    {
        return 0;
    }
    return &(itr->second);
}

bool trpgModelTable::Read(trpgReadBuffer &buf)
{
    int32 numModel;
    trpgToken tok;
    int32 len;
    bool status;

    try {
        buf.Get(numModel);
        for (int i=0;i<numModel;i++) {
            trpgModel model;
            buf.GetToken(tok,len);
            bool readHandle;
            if (tok == TRPGMODELREF)
                readHandle = false;
            else if (tok == TRPGMODELREF2)
                readHandle = true;
            else
                throw 1;
            buf.PushLimit(len);
            status = model.Read(buf,readHandle);
            buf.PopLimit();
            if (!status)  throw 1;
            AddModel(model);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}
