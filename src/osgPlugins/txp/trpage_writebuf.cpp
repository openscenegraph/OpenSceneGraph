/* ************************
   Copyright Terrain Experts Inc.
   Terrain Experts Inc (TERREX) reserves all rights to this source code
   unless otherwise specified in writing by the Chief Operating Officer
   of TERREX.
   This copyright may be updated in the future, in which case that version
   supercedes this one.
   -------------------
   Terrex Experts Inc.
   84 West Santa Clara St., Suite 380
   San Jose, CA 95113
   info@terrex.com
   Tel: (408) 293-9977
   ************************
   */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* trpage_writebuf.cpp
	This source file contains the implementation of trpgMemWriteBuffer.
	That is a subclass of trpgWriteBuffer, which implements an interface
	 definition for an object which can accept data to be written.
	The Mem version is used to write (for example) a tile's worth of data.
	That data can then be written to a file (or sent over the network).
	You should not need to change this implementation.  Simply sublcass
	 trpgWriteBuffer and implement all of the required methods.  The
	 resulting class can then be used in all the places a trpgWriteBuffer
	 is required.
	 */

#include "trpage_io.h"
#include "trpage_swap.h"

/* **********************
   Memory Write Buffer functions
   **********************
   */
// Constructor
trpgMemWriteBuffer::trpgMemWriteBuffer(trpgEndian in_ness)
{
	ness = in_ness;
	cpuNess = trpg_cpu_byte_order();
	data = NULL;
	curLen = totLen = 0;
}

// Destructor
trpgMemWriteBuffer::~trpgMemWriteBuffer()
{
	if (data)
		delete data;
	data = NULL;
}

/* Length()
	Return the length of the given buffer.
	*/
int trpgMemWriteBuffer::length() const
{
	return curLen;
}

/* getData()
	Return a pointer to the memory buffer.
	*/
const char *trpgMemWriteBuffer::getData() const
{
	return data;
}

/* Length()
	Set the maximum buffer length.
	*/
void trpgMemWriteBuffer::setLength(unsigned int len)
{
	if ((int)len > totLen) {
		char *old_data = data;
		int oldLen = totLen;
		totLen = 2*len;
		data = new char[totLen];

		if (old_data) {
			memcpy(data,old_data,oldLen);
			delete old_data;
		}
	}
}

/* append()
	Append the given data to our buffer.
	*/
void trpgMemWriteBuffer::append(unsigned int len,const char *val)
{
	if (len == 0)  return;
	setLength(curLen+len);
	memcpy(&data[curLen],val,len);
	curLen += len;
}

/* set()
	Set a specific portion of the buffer to a given value.
	*/
void trpgMemWriteBuffer::set(unsigned int pos,unsigned int len,const char *val)
{
	if (len == 0) return;
	if (pos+len > (unsigned int)curLen) return;

	memcpy(&data[pos],val,len);
}

/* --- replacement virtual functions --- */

/* Reset()
	Drop whatever's being stored.
	*/
void trpgMemWriteBuffer::Reset()
{
	curLen = 0;
}

// Add(Int32)
void trpgMemWriteBuffer::Add(int32 val)
{
	if (ness != cpuNess)
		val = trpg_byteswap_int(val);
	append(sizeof(int32),(const char *)&val);
}

// Add(int64)
void trpgMemWriteBuffer::Add(int64 val)
{
	if (ness != cpuNess)
		val = trpg_byteswap_llong(val);
	append(sizeof(int64),(const char *)&val);
}

// Add(string)
// [len] [value...]
void trpgMemWriteBuffer::Add(const char *val)
{
	int32 len = (val ? strlen(val) : 0),vlen = len;
	if (ness != cpuNess)
		vlen = trpg_byteswap_int(vlen);
	append(sizeof(int32),(const char *)&len);
	append(len,val);
}

// Add(float32)
void trpgMemWriteBuffer::Add(float32 val)
{
	char cval[4];
	if (ness != cpuNess)
		trpg_byteswap_float_to_4bytes(val,cval);
	else
		memcpy(cval,&val,4);

	append(sizeof(float32),cval);
}

// Add(float64)
void trpgMemWriteBuffer::Add(float64 val)
{
	char cval[8];
	if (ness != cpuNess)
		trpg_byteswap_double_to_8bytes(val,cval);
	else
		memcpy(cval,&val,8);

	append(sizeof(float64),cval);
}

// Add(int8)
void trpgMemWriteBuffer::Add(uint8 val)
{
	// No byte swapping needed
	append(sizeof(uint8),(const char *)&val);
}

#if (bool != int32)
// Add(bool)
void trpgMemWriteBuffer::Add(bool val)
{
	uint8 ival;

	ival = (val ? 1 : 0);
	Add(ival);
}
#endif

#if (trpgDiskRef != int64)
// Add(trpgDiskRef)
void trpgMemWriteBuffer::Add(trpgDiskRef val)
{
	if (ness != cpuNess)
		val = trpg_byteswap_llong(val);

	append(sizeof(trpgDiskRef),(const char *)&val);
}
#endif

// Add(trpgToken)
void trpgMemWriteBuffer::Add(trpgToken val)
{
	if (ness != cpuNess)
		val = trpg_byteswap_short(val);

	append(sizeof(trpgToken),(const char *)&val);
}

// Add(tx2iPoint)
void trpgWriteBuffer::Add(const trpg2iPoint &val)
{
	Add((int32)val.x);
	Add((int32)val.y);
}

// Add(tx2dPoint)
void trpgWriteBuffer::Add(const trpg2dPoint &val)
{
	Add((float64)val.x);
	Add((float64)val.y);
}

// Add(trpg3dPoint)
void trpgWriteBuffer::Add(const trpg3dPoint &val)
{
	Add((float64)val.x);
	Add((float64)val.y);
	Add((float64)val.z);
}

// Add(trpgColor)
void trpgWriteBuffer::Add(const trpgColor &val)
{
	Add(val.red);
	Add(val.green);
	Add(val.blue);
}

/* Push()
	Starts defining a new object.
	Need to keep track of where length goes.
	*/
void trpgMemWriteBuffer::Begin(trpgToken tok)
{
	Add(tok);
	lengths.push_back(curLen);
	Add((int32)0);
}

/* Push()
	Pushes a level on the stack.  For defining children.
	*/
void trpgMemWriteBuffer::Push()
{
	Add((trpgToken)TRPG_PUSH);
}

/* Pop()
	Pops a level off the "stack".
	*/
void trpgMemWriteBuffer::Pop()
{
	Add((trpgToken)TRPG_POP);
}

/* End()
	Finished defining an object.
	Write the length out where appropriate.
	*/
void trpgMemWriteBuffer::End()
{
	if (lengths.size() == 0)
		// Note: say something clever here
		return;

	int id = lengths.size()-1;
	int32 len = curLen - lengths[id];
	int32 rlen = len-sizeof(int32);
	if (ness != cpuNess)
		rlen = trpg_byteswap_int(rlen);
	set(curLen - len,sizeof(int32),(const char *)&rlen);
	lengths.resize(id);
}
