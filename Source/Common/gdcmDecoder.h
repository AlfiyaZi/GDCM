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

#ifndef __gdcmDecoder_h
#define __gdcmDecoder_h

#include "gdcmTypes.h"

namespace gdcm
{

class TS;
class IStream;
class OStream;
class Decoder
{
public:
  virtual ~Decoder() {}
  virtual bool CanDecode(TS const &) { return false; }
  virtual bool Decode(IStream &is, OStream &os) { return false; }
};

} // end namespace gdcm

#endif //__gdcmDecoder_h
