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

#ifndef __gdcmImage_h
#define __gdcmImage_h

#include "gdcmPixelType.h"
#include "gdcmSwapCode.h"
#include "gdcmPhotometricInterpretation.h"

#include <vector>

namespace gdcm
{

/**
 * \brief Image
 * \note
 * This is the container for an Image in the general sense.
 * From this container you should be able to request information like:
 * - Origin
 * - Dimension
 * - PixelType
 * ...
 * But also to retrieve the image as a raw buffer (char *)
 * Since we have to deal with both RAW data and JPEG stream (which
 * internally encode all the above information) this API might seems 
 * redundant. One way to solve that would be to subclass gdcm::Image
 * with gdcm::JPEGImage which would from the stream extract the header info
 * and fill it to please gdcm::Image...well except origin for instance
 * 
 */
class GDCM_EXPORT Image
{
public:
  Image ():NumberOfDimensions(0),PlanarConfiguration(0),Dimensions(),SC(),CompressionType(Compression::UNKNOWN) {}
  virtual ~Image() {}

  unsigned int GetNumberOfDimensions() const;
  void SetNumberOfDimensions(unsigned int dim);

  const unsigned int *GetDimensions() const;
  void SetDimensions(unsigned int *dims);
  void SetDimensions(unsigned int idx, unsigned int dim);

  // Get/Set PixelType
  const PixelType &GetPixelType() const
    {
    return PT;
    }
  void SetPixelType(PixelType const &pt)
    {
    PT = pt;
    }

  // Acces the raw data
  virtual bool GetBuffer(char *buffer) const;

  // TODO does this really belong here ?
  const double *GetSpacing() const;
  void SetSpacing(double *spacing);

  const double *GetOrigin() const;
  void SetOrigin(double *ori);

  void Print(std::ostream &os) const;

  unsigned int GetPlanarConfiguration() const;
  void SetPlanarConfiguration(unsigned int pc);

  const PhotometricInterpretation &GetPhotometricInterpretation() const;
  void SetPhotometricInterpretation(PhotometricInterpretation const &pi);

  SwapCode GetSwapCode() const
    {
    return SC;
    }
  void SetSwapCode(SwapCode sc)
    {
    SC = sc;
    }

  Compression::Types GetCompressionType() const
    {
    return CompressionType;
    }
  void SetCompressionType(Compression::Types ct)
    {
    CompressionType = ct;
    }
  bool GetNeedByteSwap() const
    {
    return NeedByteSwap;
    }
  void SetNeedByteSwap(bool b)
    {
    NeedByteSwap = b;
    }

//  Image(Image const&);
//  Image &operator= (Image const&);

  unsigned long GetBufferLength() const;

private:
  unsigned int NumberOfDimensions;
  unsigned int PlanarConfiguration;
  std::vector<unsigned int> Dimensions;
  std::vector<double> Spacing;
  std::vector<double> Origin;

  PixelType PT;
  PhotometricInterpretation PI;

  SwapCode SC;
  Compression::Types CompressionType;
  bool NeedByteSwap;
};

} // end namespace gdcm

#endif //__gdcmImage_h

