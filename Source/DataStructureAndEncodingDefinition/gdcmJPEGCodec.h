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
#ifndef __gdcmJPEGcodec_h
#define __gdcmJPEGcodec_h

#include "gdcmCodec.h"

namespace gdcm
{
  
class JPEGInternals;
class JPEGCodec : public Codec
{
public:
  JPEGCodec();
  ~JPEGCodec();
  bool CanDecode(TS const &ts);
  bool Decode(IStream &is, OStream &os);
private:
  JPEGInternals *Internals;
};

} // end namespace gdcm

#endif //__gdcmJPEGcodec_h
