/*=========================================================================

  Program: GDCM (Grass Root DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006 Mathieu Malaterre
  Copyright (c) 1993-2005 CREATIS
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#ifndef __gdcmVM_h
#define __gdcmVM_h

#include "gdcmTypes.h"
#include <iostream>
/* \brief Value Multiplicity
 * Looking at the DICOMV3 dict only there is very few cases:
 * 1
 * 2
 * 3
 * 4
 * 5
 * 6
 * 8
 * 16
 * 24
 * 1-2
 * 1-3
 * 1-8
 * 1-32
 * 1-99
 * 1-n
 * 2-2n
 * 2-n
 * 3-3n
 * 3-n
 */
namespace gdcm
{

class GDCM_EXPORT VM
{
public:
  typedef enum {
    VM0 = 0,
    VM1,
    VM2,
    VM3,
    VM4,
    VM5,
    VM6,
    VM8,
    VM16,
    VM24,
    VM1_2,
    VM1_3,
    VM1_8,
    VM1_32,
    VM1_99,
    VM1_n,
    VM2_2n,
    VM2_n,
    VM3_3n,
    VM3_n,
    VM_END  // Custom tag to count number of entry
  } VMType;

  //friend std::ostream& operator<<(std::ostream& _os, const VRType&_val);

  // Return the string as written in the official DICOM dict from 
  // a custom enum type
  static const char* GetVMString(const VMType &vm);
  static VMType GetVMType(const char *vm);

  // Check if vm1 is valid compare to vm2, i.e vm1 is element of vm2
  // vm1 is typically deduct from counting in a ValueField
  static bool IsValid(const int &vm1, const VMType &vm2);
};
//-----------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream& _os, const VM::VMType&_val)
{
  _os << VM::GetVMString(_val);
  return _os;
}

template<int T> struct TypeToLength;
#define TYPETOLENGTH(type,length) \
  template<> struct TypeToLength<VM::type> \
  { enum { Length = length }; };
// TODO: Could be generated from XML file
TYPETOLENGTH(VM0,1)
TYPETOLENGTH(VM1,1)
TYPETOLENGTH(VM2,2)
TYPETOLENGTH(VM3,3)
TYPETOLENGTH(VM4,4)
TYPETOLENGTH(VM5,5)
TYPETOLENGTH(VM6,6)
TYPETOLENGTH(VM8,8)
TYPETOLENGTH(VM16,16)
TYPETOLENGTH(VM24,24)
TYPETOLENGTH(VM1_2,2)
TYPETOLENGTH(VM1_3,3)
TYPETOLENGTH(VM1_8,8)
TYPETOLENGTH(VM1_32,32)
TYPETOLENGTH(VM1_99,99)
//TYPETOLENGTH(VM1_n,
//TYPETOLENGTH(VM2_2n,
//TYPETOLENGTH(VM2_n,
//TYPETOLENGTH(VM3_3n,
//TYPETOLENGTH(VM3_n,

} // end namespace gdcm

#endif //__gdcmVM_h
