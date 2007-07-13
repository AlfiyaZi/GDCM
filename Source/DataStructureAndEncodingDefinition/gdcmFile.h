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

#ifndef __gdcmFile_h
#define __gdcmFile_h

#include "gdcmDataSet.h"
#include "gdcmPreamble.h"
#include "gdcmFileMetaInformation.h"

namespace gdcm
{
/**
*/
class FileInternal;
class GDCM_EXPORT File
{
public:
  File() {};
  ~File() {};

  friend std::ostream &operator<<(std::ostream &_os, const File &_val);

  // Read
  IStream &Read(IStream &is);

  // Write
  OStream const &Write(OStream &os) const;

  const Preamble &GetPreamble() const { return P; }
  void SetPreamble(const Preamble &p) { P = p; }
  const FileMetaInformation &GetHeader() const { return Header; }
  void SetHeader( const FileMetaInformation &fmi ) { Header = fmi; }
  const DataSet &GetDataSet() const { return DS; }
  void SetDataSet( const DataSet &ds) { DS = ds; }

private:
  Preamble P;
  FileMetaInformation Header;
  DataSet DS;
};
//-----------------------------------------------------------------------------
inline std::ostream& operator<<(std::ostream &os, const File &val)
{
  os << val.GetPreamble() << std::endl;
  os << val.GetHeader() << std::endl;
  os << val.GetDataSet() << std::endl;
  return os;
}

} // end namespace gdcm

#endif //__gdcmFile_h

