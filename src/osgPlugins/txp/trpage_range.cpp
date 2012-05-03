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

/* trpage_range.cpp
    Methods for the Range Table.  Consult trpg_geom.h for details
    on what this is and how it works.
    */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <trpage_geom.h>
#include <trpage_read.h>

/* *******************
    Range Methods
   *******************
 */

trpgRange::trpgRange(void)
{
    category = NULL;
    subCategory = NULL;
    Reset();
}

trpgRange::~trpgRange(void)
{
    Reset();
}

trpgRange::trpgRange(const trpgRange &in):
    trpgReadWriteable(in)
{
    category = NULL;
    subCategory = NULL;
    *this = in;
}

void trpgRange::Reset(void)
{
    errMess[0] = '\0';
    if (category)
        delete [] category;
    category = NULL;
    if (subCategory)
        delete [] subCategory;
    subCategory = NULL;

    inLod = outLod = 0.0;
    priority = 0;
    handle = -1;
    writeHandle = false;
}

void trpgRange::SetCategory(const char *cat,const char *subCat)
{
    if (category)  delete [] category;
    category = NULL;
    if (cat) {
        category = new char[strlen(cat)+1];
        strcpy(category,cat);
    }

    if (subCategory)  delete [] subCategory;
    subCategory = NULL;
    if (subCat) {
        subCategory = new char[strlen(subCat)+1];
        strcpy(subCategory,subCat);
    }
}

void trpgRange::GetCategory(char *cat,int catLen,char *subCat,int subCatLen) const
{
    if (category && cat) {
        strncpy(cat,category,catLen);
    } else
        *cat = 0;
    if (subCategory && subCat) {
        strncpy(subCat,subCategory,subCatLen);
    } else
        *subCat = 0;
}

void trpgRange::SetLodInfo(double in,double out)
{
    inLod = in;
    outLod = out;
}

void trpgRange::GetLodInfo(double &in,double &out) const
{
    in = inLod;
    out = outLod;
}

void trpgRange::SetPriority(int prior)
{
    priority = prior;
}

void trpgRange::GetPriority(int &prior) const
{
    prior = priority;
}

trpgRange & trpgRange::operator = (const trpgRange &other)
{
    Reset();
    inLod = other.inLod;
    outLod = other.outLod;
    SetCategory(other.category,other.subCategory);
    priority = other.priority;
    handle = other.handle;
    writeHandle = other.writeHandle;
    return *this;
}

bool trpgRange::operator == (const trpgRange &in) const
{
    if (inLod != in.inLod || outLod != in.outLod)
        return false;
    if (priority != in.priority)  return false;

    if (category && in.category) {
        if (strcmp(category,in.category))
            return false;
    } else {
        if ((category && !in.category) ||
            (!category && in.category) )
            return false;
    }

    if (subCategory && in.subCategory) {
        if (strcmp(subCategory,in.subCategory))
            return false;
    } else {
        if ((subCategory && !in.subCategory) ||
            (subCategory && in.subCategory))
            return false;
    }
    if(handle != in.handle)
        return false;
    if(writeHandle != in.writeHandle)
        return false;
    return true;
}

bool trpgRange::Write(trpgWriteBuffer &buf)
{
    buf.Begin(TRPG_RANGE);
    buf.Add(inLod);
    buf.Add(outLod);
    buf.Add(priority);
    buf.Add((category ? category : ""));
    buf.Add((subCategory ? subCategory : ""));
    if(writeHandle) {
        buf.Add((int32)handle);
    }
    buf.End();

    return true;
}

bool trpgRange::Read(trpgReadBuffer &buf)
{
    char catStr[1024],subStr[1024];

    Reset();
    valid = false;

    try {
        buf.Get(inLod);
        buf.Get(outLod);
        buf.Get(priority);
        buf.Get(catStr,1024);
        buf.Get(subStr,1024);
        SetCategory(catStr,subStr);

        // Read the handle if we can..
        try {
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
        catch (...) {
            handle = -1;
        }
        valid = true;
    }

    catch (...) {
        return false;
    }

    return isValid();
}

/*  ***************
    Range Table methods
    ***************
 */

trpgRangeTable::trpgRangeTable(void)
{
    valid = true;
}

trpgRangeTable::~trpgRangeTable(void)
{
    // vector cleans up itself
}

void trpgRangeTable::Reset(void)
{
    rangeMap.clear();
    valid = true;
}

bool trpgRangeTable::GetRange(int id,trpgRange &ret) const
{
    if (!isValid())
        return false;

    if (id < 0)// || id >= rangeList.size())
        return false;

    RangeMapType::const_iterator itr = rangeMap.find(id);
    if(itr == rangeMap.end()) {
        return false;
    }
    ret = itr->second;
    return true;
}

bool trpgRangeTable::SetRange(int id,trpgRange &inRange)
{
    if (!isValid())
        return false;

    if (id < 0)// || id >= rangeList.size())
        return false;

    rangeMap[id] = inRange;

    return true;
}

int trpgRangeTable::AddRange(trpgRange &range)
{
    int handle = range.GetHandle();
    if(handle==-1) {
        handle = rangeMap.size();
    }
    rangeMap[handle] = range;
    return handle;

}

int trpgRangeTable::FindAddRange(trpgRange &range)
{
    RangeMapType::iterator itr = rangeMap.begin();
    for (  ; itr != rangeMap.end( ); itr++) {
        if(itr->second==range)
            return itr->first;
    }

#if 0
    for (int i=0;i<rangeList.size();i++) {
        if (range == rangeList[i])
            return i;
    }
#endif
    return AddRange(range);
}

bool trpgRangeTable::Write(trpgWriteBuffer &buf)
{
    if (!isValid())
        return false;

    buf.Begin(TRPGRANGETABLE);
    buf.Add((int32)rangeMap.size());

    RangeMapType::iterator itr = rangeMap.begin();
    for (  ; itr != rangeMap.end( ); itr++) {
        trpgRange &range = itr->second;
        range.Write(buf);
    }

    buf.End();

    return true;
}

bool trpgRangeTable::Read(trpgReadBuffer &buf)
{
    int32 numRange;
    trpgToken tok;
    int32 len;
    valid = false;

    try {
        buf.Get(numRange);
        if (numRange < 0) throw 1;
        for (int i=0;i<numRange;i++) {
            // Read in the individual range
            buf.GetToken(tok,len);
            if (tok != TRPG_RANGE) throw 1;
            buf.PushLimit(len);
            trpgRange range;
            bool status = range.Read(buf);
            buf.PopLimit();
            if (!status) throw 1;
            AddRange(range);
        }

        valid = true;
    }

    catch (...) {
        return false;
    }

    return isValid();
}

trpgRangeTable & trpgRangeTable::operator = (const trpgRangeTable &inTab)
{
    Reset();
    RangeMapType::const_iterator itr = inTab.rangeMap.begin();
    for (  ; itr != inTab.rangeMap.end( ); itr++)
        rangeMap[itr->first] = itr->second;

#if 0
    for (int i=0;i<inTab.rangeList.size();i++)
        rangeList.push_back(inTab.rangeList[i]);
#endif
    return *this;
}
