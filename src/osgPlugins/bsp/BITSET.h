//////////////////////////////////////////////////////////////////////////////////////////
//  BITSET.h
//  class declaration for set of bits to represent many true/falses
//  You may use this code however you wish, but if you do, please credit me and
//  provide a link to my website in a readme file or similar
//  Downloaded from: www.paulsprojects.net
//  Created:  8th August 2002
//////////////////////////////////////////////////////////////////////////////////////////  

#ifndef BITSET_H
#define BITSET_H

#include <vector>


class BITSET
{
public:
  BITSET() : m_numBytes(0)
  {}
  ~BITSET()
  {
  }

  bool Init(int numberOfBits);
  void ClearAll();
  void SetAll();

  void Clear(int bitNumber);
  void Set(int bitNumber);

  unsigned char IsSet(int bitNumber) const;

protected:
  int m_numBytes; //size of bits array
  unsigned char * m_bits_aux;
  std::vector<unsigned char> m_bits;
};

#endif
