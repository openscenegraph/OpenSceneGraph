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

/* trpage_mode.cpp
    This source file contains the methods trpgModel and trpgModelTable.
    You should only modify this code if you want to add data to these classes.
    */

#include "trpage_geom.h"
#include "trpage_read.h"

/* Write Model class
    Represents a model reference.
    */
trpgModel::trpgModel()
{
    name = NULL;
    type = External;
    useCount = 0;
}
trpgModel::trpgModel(const trpgModel &in):trpgReadWriteable()
{
    name = NULL;
    type = External;
    *this = in;
}

// Reset function
void trpgModel::Reset()
{
    if (name)
        delete [] name;
    name = NULL;
    useCount = 0;
}

trpgModel::~trpgModel()
{
    Reset();
}

// Set functions
void trpgModel::SetName(const char *nm)
{
    if (name)
        delete [] name;

    name = new char[(nm ? strlen(nm) : 0)+1];
    strcpy(name,nm);

    type = External;
}
void trpgModel::SetReference(trpgDiskRef pos)
{
    if (name)
        delete [] name;

    diskRef = pos;

    type = Local;
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
        return false;

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

    buf.Begin(TRPGMODELREF);
    buf.Add(type);
    if (name)
        buf.Add(name);
    else
        buf.Add(diskRef);
    buf.Add(useCount);

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
    if (!isValid() || type != External) return false;
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

bool trpgModel::Read(trpgReadBuffer &buf)
{
    char tmpName[1024];

    try {
        buf.Get(type);
        if (type == Local)
            buf.Get(diskRef);
        else {
            buf.Get(tmpName,1023);
            SetName(tmpName);
        }
        buf.Get(useCount);
    }
    catch(...) {
        return false;
    }

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
    models.resize(0);
}

// Set functions
void trpgModelTable::SetNumModels(int no)
{
    models.resize(no);
}
void trpgModelTable::SetModel(int id,const trpgModel &mod)
{
    if (id < 0 || static_cast<unsigned int>(id) >= models.size())
        return;

    models[id] = mod;
}
int trpgModelTable::AddModel(const trpgModel &mod)
{
    models.push_back(mod);

    return models.size()-1;
}
int trpgModelTable::FindAddModel(const trpgModel &mod)
{
    for (unsigned int i=0;i<models.size();i++)
    if (models[i] == mod)
        return i;

    return AddModel(mod);
}

// Validity check
bool trpgModelTable::isValid() const
{
    for (unsigned int i=0;i<models.size();i++)
        if (!models[i].isValid())
            return false;

    return true;
}

// Write out the model table
bool trpgModelTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGMODELTABLE);
    buf.Add((int32)models.size());
    for (unsigned int i=0;i<models.size();i++)
        models[i].Write(buf);
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
    nm = models.size();
    return true;
}
bool trpgModelTable::GetModel(int id,trpgModel &model) const
{
    if (!isValid() || id < 0 || static_cast<unsigned int>(id) >= models.size())
        return false;
    model = models[id];
    return true;
}
trpgModel *trpgModelTable::GetModelRef(int id)
{
    if (id < 0 || static_cast<unsigned int>(id) >= models.size())
        return NULL;
    return &models[id];
}

bool trpgModelTable::Read(trpgReadBuffer &buf)
{
    int32 numModel;
    trpgModel model;
    trpgToken tok;
    int32 len;
    bool status;

    try {
        buf.Get(numModel);
        for (int i=0;i<numModel;i++) {
            buf.GetToken(tok,len);
            if (tok != TRPGMODELREF)  throw 1;
            buf.PushLimit(len);
            status = model.Read(buf);
            buf.PopLimit();
            if (!status)  throw 1;
            models.push_back(model);
        }
    }
    catch (...) {
        return false;
    }

    return isValid();
}
