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
#include "gdcmReader.h"
#include "gdcmTrace.h"
#include "gdcmVR.h"
#include "gdcmFileMetaInformation.h"

namespace gdcm
{

Reader::~Reader()
{
  delete DS;
  delete Header;
  if( Preamble )
    {
    delete[] Preamble;
    }
}

/// \brief tells us if "DICM" is found as position 128
///        (i.e. the file is a 'true dicom' one)
/// If not found then seek back at beginning of file (could be Mallinckrodt
/// or old ACRNEMA with no preamble)
/// \precondition we are at the beginning of file
/// \postcondition we are at the beginning of the DataSet or
/// Meta Information Header
bool Reader::ReadPreamble()
{
  bool r = false;
  Preamble = new char[128+4];
  Stream.Read(Preamble, 128+4);
  if( Preamble[128+0] == 'D'
   && Preamble[128+1] == 'I'
   && Preamble[128+2] == 'C'
   && Preamble[128+3] == 'M')
    {
    r = true;
    }
  if(!r)
    {
    gdcmDebugMacro( "Not a DICOM V3 file" );
    delete[] Preamble;
    Preamble = 0;
    // Seek back
    Stream.Seekg(0, std::ios::beg);
    }

  return r;
}

/// \brief read the DICOM Meta Information Header
/// Find out the TransferSyntax used (default: Little Endian Explicit)
/// \precondition we are at the start of group 0x0002 (well after preamble)
/// \postcondition we are at the beginning of the DataSet
bool Reader::ReadMetaInformation()
{
  if( !Header )
    {
    Header = new FileMetaInformation;
    }
  return Header->Read(Stream);
}

bool Reader::ReadDataSet()
{
  if( !DS )
    {
    TS ts = Header->GetTransferSyntaxType();
    //std::cerr << ts << std::endl;
    if( ts.GetNegociatedType() == TS::Explicit )
      {
      DS = new DataSet(TS::Explicit);
      }
    else // default to instanciating an implicit one (old ACRNEMA...)
      {
      assert( ts.GetNegociatedType() == TS::Implicit
           || ts.GetNegociatedType() == TS::Unknown );
      DS = new DataSet(TS::Implicit);
      }
    }
  assert( Stream.GetSwapCode() != SwapCode::Unknown );
  return DS->Read(Stream);
}

TS Reader::GuessTransferSyntax()
{
  // Don't call this function if you have a meta file info
  assert( Header->GetTransferSyntaxType() == TS::TS_END );
  assert( Stream.GetSwapCode() == SwapCode::Unknown );
  std::streampos start = Stream.Tellg();
  SwapCode sc = SwapCode::Unknown;
  TS::NegociatedType nts = TS::Unknown;
  TS ts (TS::TS_END);
  Tag t;
  t.Read(Stream);
  if( ! (t.GetGroup() % 2) )
    {
    switch( t.GetGroup() )
      {
    case 0x0008:
      sc = SwapCode::LittleEndian;
      break;
    case 0x0800:
      sc = SwapCode::BigEndian;
      break;
    default:
      abort();
      }
    // Purposely not Re-use ReadVR since we can read VR_END
    char vr_str[3];
    Stream.Read(vr_str, 2);
    vr_str[2] = '\0';
    // Cannot use GetVRTypeFromFile since is assert ...
    VR::VRType vr = VR::GetVRType(vr_str);
    if( vr != VR::VR_END )
      {
      nts = TS::Explicit;
      }
    else
      {
      assert( !(VR::IsSwap(vr_str)));
      Stream.Seekg(-2, std::ios::cur); // Seek back
      if( t.GetElement() == 0x0000 )
        {
        VL gl; // group length
        gl.Read(Stream);
        switch(gl)
          {
        case 0x00000004 :
          sc = SwapCode::LittleEndian;    // 1234
          break;
        case 0x04000000 :
          sc = SwapCode::BigEndian;       // 4321
          break;
        case 0x00040000 :
          sc = SwapCode::BadLittleEndian; // 3412
          gdcmWarningMacro( "Bad Little Endian" );
          break;
        case 0x00000400 :
          sc = SwapCode::BadBigEndian;    // 2143
          gdcmWarningMacro( "Bad Big Endian" );
          break;
        default:
          abort();
          }
        }
      nts = TS::Implicit;
      }
    }
  else
    {
    gdcmWarningMacro( "Start with a private tag creator" );
    assert( t.GetGroup() > 0x0002 );
    switch( t.GetElement() )
      {
    case 0x0010:
      sc = SwapCode::LittleEndian;
      break;
    default:
      abort();
      }
    // Purposely not Re-use ReadVR since we can read VR_END
    char vr_str[3];
    Stream.Read(vr_str, 2);
    vr_str[2] = '\0';
    // Cannot use GetVRTypeFromFile since is assert ...
    VR::VRType vr = VR::GetVRType(vr_str);
    if( vr != VR::VR_END )
      {
      nts = TS::Explicit;
      }
    else
      {
      nts = TS::Implicit;
      // We are reading a private creator (0x0010) so it's LO, it's
      // difficult to come up with someting to check, maybe that
      // VL < 256 ...
      gdcmWarningMacro( "Very dangerous assertion needs some work" );
      }
    }
  assert( nts != TS::Unknown );
  assert( sc != SwapCode::Unknown );
  if( nts == TS::Implicit )
    {
    if( sc == SwapCode::BigEndian )
      {
      ts = TS::ImplicitVRBigEndianACRNEMA;
      }
    else if ( sc == SwapCode::LittleEndian )
      {
      ts = TS::ImplicitVRLittleEndian;
      }
    else
      {
      abort();
      }
    }
  else
    {
    abort();
    }
  Stream.Seekg( start, std::ios::beg );
  assert( ts != TS::TS_END );
  return ts;
}

bool Reader::Read()
{
  if( !Stream.IsOpen() )
    {
    gdcmErrorMacro( "No File" );
    return false;
    }
  if( !ReadPreamble() )
    {
    assert( Preamble == 0 );
    gdcmDebugMacro( "No Preamble" );
    }
  ReadMetaInformation();
  //std::cerr << *Header << std::endl;
  TS ts = Header->GetTransferSyntaxType();
  if( ts == TS::TS_END )
    {
    ts = GuessTransferSyntax();
    // Then save it in the 'pseudo' header
    Header->SetTransferSyntaxType( ts );
    }
  assert( ts != TS::TS_END );
  // From ts set properly the Stream for reading the dataset:
  Stream.SetSwapCode( ts.GetSwapCode() );
  // Read !
  ReadDataSet();

  assert( Stream.Eof() );
  Stream.Close();

  return true;
}

} // end namespace gdcm
