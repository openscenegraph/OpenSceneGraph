//////////////////////////////////////////////////////////////////////////////////////////
//  BITSET.cpp
//  functions for class for set of bits to represent many true/falses
//  You may use this code however you wish, but if you do, please credit me and
//  provide a link to my website in a readme file or similar
//  Downloaded from: www.paulsprojects.net
//  Created:  8th August 2002
//////////////////////////////////////////////////////////////////////////////////////////
#include "memory.h"
#include "BITSET.h"

#include <cstring>

bool BITSET::Init(int numberOfBits)
{
  //Delete any memory allocated to bits
  m_bits.clear();

  //Calculate size
  m_numBytes=(numberOfBits>>3)+1;

  //Create memory
  m_bits.reserve(m_numBytes);
  m_bits_aux=&m_bits[0];

  ClearAll();

  return true;
}

void BITSET::ClearAll()
{
  memset(m_bits_aux, 0, m_numBytes);
}

void BITSET::SetAll()
{
  memset(m_bits_aux, 0xFF, m_numBytes);
}

void BITSET::Clear(int bitNumber)
{
  m_bits_aux[bitNumber>>3] &= ~(1<<(bitNumber & 7));
}

void BITSET::Set(int bitNumber)
{
  m_bits_aux[bitNumber>>3] |= 1<<(bitNumber&7);
}

unsigned char BITSET::IsSet(int bitNumber) const
{
  return static_cast<unsigned char>(m_bits_aux[bitNumber>>3] & 1<<(bitNumber&7));
}
