/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 */
#define LIB3DS_EXPORT
#include "readwrite.h"
#include <osg/Endian>


/*!
 * \defgroup readwrite Portable Binary Input/Ouput
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */

static bool s_requiresByteSwap = false;

extern LIB3DSAPI void setByteOrder()
{
    s_requiresByteSwap = osg::getCpuByteOrder()==osg::BigEndian;
}



/*!
 * \ingroup readwrite
 *
 * Read a byte from a file stream.  
 *
 * \param f  Input file stream. 
 *
 * \return The byte read. 
 */
Lib3dsByte
lib3ds_byte_read(iostream *strm)
{
  Lib3dsByte b;

  ASSERT(strm);
  strm->read((char*)&b,1);
  return(b);
}


/**
 * Read a word from a file stream in little endian format.   
 *
 * \param f  Input file stream. 
 *
 * \return The word read. 
 */
Lib3dsWord
lib3ds_word_read(iostream *strm)
{
  Lib3dsByte b[2];
  Lib3dsWord w;

  ASSERT(strm);  
  strm->read((char*)&b,2);
  w=((Lib3dsWord)b[1] << 8) |
    ((Lib3dsWord)b[0]);
  return(w);
}


/*!
 * \ingroup readwrite
 *
 * Read a dword from file a stream in little endian format.   
 *
 * \param f  Input file stream. 
 *
 * \return The dword read. 
 */
Lib3dsDword
lib3ds_dword_read(iostream *strm)
{
  Lib3dsByte b[4];
  Lib3dsDword d;        
                         
  ASSERT(strm);  
  strm->read((char*)&b,4);
  d=((Lib3dsDword)b[3] << 24) |
    ((Lib3dsDword)b[2] << 16) |
    ((Lib3dsDword)b[1] << 8) |
    ((Lib3dsDword)b[0]);
  return(d);
}


/*!
 * \ingroup readwrite
 *
 * Read a signed byte from a file stream.   
 *
 * \param f  Input file stream. 
 *
 * \return The signed byte read. 
 */
Lib3dsIntb
lib3ds_intb_read(iostream *strm)
{
  Lib3dsIntb b;

  ASSERT(strm);  
  strm->read((char*)&b,1);
  return(b);
}


/*!
 * \ingroup readwrite
 *
 * Read a signed word from a file stream in little endian format.   
 *
 * \param f  Input file stream. 
 *
 * \return The signed word read. 
 */
Lib3dsIntw
lib3ds_intw_read(iostream *strm)
{
  Lib3dsByte b[2];

  ASSERT(strm);
  strm->read((char*)&b,2);

  if (s_requiresByteSwap)
  {
    osg::swapBytes2((char*)b);
  }

  return (*((Lib3dsIntw*)b));
}


/*!
 * \ingroup readwrite
 *
 * Read a signed dword a from file stream in little endian format.   
 *
 * \param f  Input file stream. 
 *
 * \return The signed dword read. 
 */
Lib3dsIntd
lib3ds_intd_read(iostream *strm)
{
  Lib3dsByte b[4];     
                         
  ASSERT(strm);
  strm->read((char*)&b,4);

  if (s_requiresByteSwap)
  {
    osg::swapBytes4((char*)b);
  }

  return (*((Lib3dsIntd*)b));

}


/*!
 * \ingroup readwrite
 *
 * Read a float from a file stream in little endian format.   
 *
 * \param f  Input file stream. 
 *
 * \return The float read. 
 */
Lib3dsFloat
lib3ds_float_read(iostream *strm)
{
  Lib3dsByte b[4];

  ASSERT(strm);
  b[0]=b[1]=b[2]=b[3]=0;
  strm->read((char*)&b,4);  

  if (s_requiresByteSwap)
  {
    osg::swapBytes4((char*)b);
  }

  return (*((Lib3dsFloat*)b));
}


/*!
 * \ingroup readwrite
 * \ingroup vector
 *
 * Read a vector from a file stream in little endian format.   
 *
 * \param v  The vector to store the data. 
 * \param f  Input file stream. 
 *
 * \return The float read. 
 */
Lib3dsBool
lib3ds_vector_read(Lib3dsVector v, iostream *strm)
{
  v[0]=lib3ds_float_read(strm);
  v[1]=lib3ds_float_read(strm);
  v[2]=lib3ds_float_read(strm);

  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }

  /*printf("lib3ds_vector_read %f %f %f\n",v[0],v[1],v[2]);*/

  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 */
Lib3dsBool
lib3ds_rgb_read(Lib3dsRgb rgb, iostream *strm)
{
  rgb[0]=lib3ds_float_read(strm);
  rgb[1]=lib3ds_float_read(strm);
  rgb[2]=lib3ds_float_read(strm);

  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  /*printf("lib3ds_rgb_read %f %f %f\n",rgb[0],rgb[1],rgb[2]);*/

  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Read a zero-terminated string from a file stream.
 *
 * \param s       The buffer to store the read string.
 * \param buflen  Buffer length.
 * \param f       The input file stream.
 *
 * \return        True on success, False otherwise.
 */
Lib3dsBool
lib3ds_string_read(char *s, int buflen, iostream *strm)
{
  int k=0;
  ASSERT(s);
  s--;
  do
  {
      s++;
      k++;
      strm->read(s,1);
  } while ((*s!=0) && (k<buflen));

  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}

/*!
 * \ingroup readwrite
 *
 * Writes a byte into a file stream.
 *
 * \param b  The byte to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_byte_write(Lib3dsByte b, iostream *strm)
{
  ASSERT(strm);
  strm->write((char*)&b,1);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a word into a little endian file stream.
 *
 * \param w  The word to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_word_write(Lib3dsWord w, iostream *strm)
{
  Lib3dsByte b[2];

  ASSERT(strm);
  b[1]=(Lib3dsByte)(((Lib3dsWord)w & 0xFF00) >> 8);
  b[0]=(Lib3dsByte)((Lib3dsWord)w & 0x00FF);
  strm->write((char*)b,2);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a dword into a little endian file stream.
 *
 * \param d  The dword to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_dword_write(Lib3dsDword d, iostream *strm)
{
  Lib3dsByte b[4];

  ASSERT(strm);
  b[3]=(Lib3dsByte)(((Lib3dsDword)d & 0xFF000000) >> 24);
  b[2]=(Lib3dsByte)(((Lib3dsDword)d & 0x00FF0000) >> 16);
  b[1]=(Lib3dsByte)(((Lib3dsDword)d & 0x0000FF00) >> 8);
  b[0]=(Lib3dsByte)(((Lib3dsDword)d & 0x000000FF));

  strm->write((char*)b,4);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a signed byte in a file stream.
 *
 * \param b  The signed byte to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_intb_write(Lib3dsIntb b, iostream *strm)
{
  ASSERT(strm);
  strm->write((const char*)&b,1);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a signed word into a little endian file stream.
 *
 * \param w  The signed word to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_intw_write(Lib3dsIntw w, iostream *strm)
{
  Lib3dsByte b[2];

  ASSERT(strm);
  b[1]=(Lib3dsByte)(((Lib3dsWord)w & 0xFF00) >> 8);
  b[0]=(Lib3dsByte)((Lib3dsWord)w & 0x00FF);

  strm->write((const char*)b,2);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a signed dword into a little endian file stream.
 *
 * \param d  The signed dword to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_intd_write(Lib3dsIntd d, iostream *strm)
{
  Lib3dsByte b[4];

  ASSERT(strm);
  b[3]=(Lib3dsByte)(((Lib3dsDword)d & 0xFF000000) >> 24);
  b[2]=(Lib3dsByte)(((Lib3dsDword)d & 0x00FF0000) >> 16);
  b[1]=(Lib3dsByte)(((Lib3dsDword)d & 0x0000FF00) >> 8);
  b[0]=(Lib3dsByte)(((Lib3dsDword)d & 0x000000FF));

  strm->write((const char*)b,4);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a float into a little endian file stream.
 *
 * \param f  The float to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_float_write(Lib3dsFloat l, iostream *strm)
{
  ASSERT(strm);

  Lib3dsByte b[4];
  Lib3dsByte* ptr = (Lib3dsByte*) (&l);

  if (s_requiresByteSwap)
  {
      b[3] = *ptr++;
      b[2] = *ptr++;
      b[1] = *ptr++;
      b[0] = *ptr++;
  }
  else
  {
      b[0] = *ptr++;
      b[1] = *ptr++;
      b[2] = *ptr++;
      b[3] = *ptr++;
  }

  strm->write((char*)b,4);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 * \ingroup vector
 *
 * Writes a vector into a file stream in little endian format.   
 *
 * \param v  The vector to write to the file stream. 
 * \param f  Input file stream. 
 */
Lib3dsBool
lib3ds_vector_write(Lib3dsVector v, iostream *strm)
{
  if (!lib3ds_float_write(v[0], strm)) {
    return(LIB3DS_FALSE);
  }
  if (!lib3ds_float_write(v[1], strm)) {
    return(LIB3DS_FALSE);
  }
  if (!lib3ds_float_write(v[2], strm)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 */
Lib3dsBool
lib3ds_rgb_write(Lib3dsRgb rgb, iostream *strm)
{
  if (!lib3ds_float_write(rgb[0], strm)) {
    return(LIB3DS_FALSE);
  }
  if (!lib3ds_float_write(rgb[1], strm)) {
    return(LIB3DS_FALSE);
  }
  if (!lib3ds_float_write(rgb[2], strm)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup readwrite
 *
 * Writes a zero-terminated string into a file stream.
 *
 * \param f  The float to write to the file stream.
 * \param f  The input file stream.
 *
 * \return   True on success, False otherwise.
 */
Lib3dsBool
lib3ds_string_write(const char *s, iostream *strm)
{
  ASSERT(s);
  ASSERT(strm);
  do strm->write(s,1); while (*s++);
  if (strm->fail()) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}

