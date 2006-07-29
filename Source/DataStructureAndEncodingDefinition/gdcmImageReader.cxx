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
#include "gdcmImageReader.h"
#include "gdcmExplicitDataElement.h"
#include "gdcmImplicitDataElement.h"
#include "gdcmValue.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmStringStream.h"
#include "gdcmElement.h"
#include "gdcmPhotometricInterpretation.h"
#include "gdcmTS.h"

namespace gdcm
{
ImageReader::ImageReader()
{
}

ImageReader::~ImageReader()
{
}

const Image& ImageReader::GetImage() const
{
  return PixelData;
}

//void ImageReader::SetImage(Image const &img)
//{
//  PixelData = img;
//}

const ByteValue* ImageReader::GetPointerFromElement(Tag const &tag) const
{
  const DataSet &ds = GetDataSet();
  TS::NegociatedType type = ds.GetNegociatedType();

  const DataElement& rde = ds.GetDataElement( tag );
  if( type == TS::Explicit )
    {
    const ExplicitDataElement &xde =
      dynamic_cast<const ExplicitDataElement&>(rde);
    const Value &v = xde.GetValue();
    const Value *pv = &v;
    const ByteValue *bv = dynamic_cast<const ByteValue*>(pv);
    return bv;
    }
  else if( type == TS::Implicit )
    {
    const ImplicitDataElement &ide =
      dynamic_cast<const ImplicitDataElement&>(rde);
    const Value &v = ide.GetValue();
    //const ByteValue &bv = dynamic_cast<const ByteValue&>(v);
    // C++ would throw an exception when dynamic_cast to a reference
    // since you could not construct a ref from NULL
    // Using good ol' pointer instead
    const Value *pv = &v;
    const ByteValue *bv = dynamic_cast<const ByteValue*>(pv);
    return bv;
    }
  // ooops ?
    gdcmErrorMacro( "Not sure how you are supposed to reach here" );
  return 0;
}

bool ImageReader::Read()
{
  if( !Reader::Read() )
    {
    return false;
    }

  const FileMetaInformation &header = GetHeader();
  TS::TSType ts = header.GetTransferSyntaxType();

  bool res = false;
  /* Does it really make sense to check for Media Storage SOP Class UID?
   * I need then to check consistency with 0008 0016 Instance SOP Class UID
   * ... I don't think there is an end. 
   * I'd rather go the old way check a bunch of tags (From Image Plane
   * Module).
   */
  TS::MSType ms = header.GetMediaStorageType();
  bool isImage = TS::IsImage( ms );
  if( isImage )
    {
    assert( ts != TS::TS_END && ms != TS::MS_END );
    // Good it's the easy case. It's declared as an Image:
    gdcmDebugMacro( "Sweet ! Finally a good DICOM file !" );
    PixelData.SetCompressionFromTransferSyntax( ts );
    res = ReadImage();
    }
  else
    {
    if( ms != TS::MS_END )
      {
      gdcmWarningMacro( "DICOM file is not an Image file but a : " <<
        TS::GetMSString(ms) << " SOP Class UID" );
      res = false;
      }
    else
      {
      // Too bad the media storage type was not recognized...
      // what should we do ?
      // Let's check 0008,0016:
      // D 0008|0016 [UI] [SOP Class UID] [1.2.840.10008.5.1.4.1.1.7 ]
      // ==> [Secondary Capture Image Storage]
      const Tag tsopclassuid(0x0008, 0x0016);
      if( GetDataSet().FindDataElement( tsopclassuid) )
        {
        const ByteValue *sopclassuid
          = GetPointerFromElement( tsopclassuid );
        std::string sopclassuid_str(
          sopclassuid->GetPointer(),
          sopclassuid->GetLength() );
        TS::MSType ms2 = TS::GetMSType(sopclassuid_str.c_str());
        bool isImage2 = TS::IsImage( ms2 );
        if( isImage2 )
          {
          gdcmWarningMacro( "After all it might be a DICOM file "
            "(Mallinckrodt-like)" );
          PixelData.SetCompressionType( Compression::RAW );
          res = ReadImage();
          }
        else
          {
          gdcmWarningMacro( "DICOM file is not an Image file but a : " <<
            TS::GetMSString(ms2) << " SOP Class UID" );
          res = false;
          }
        }
      else if( ts == TS::ImplicitVRBigEndianACRNEMA || header.IsEmpty() )
        {
        // Those transfer syntax have a high probability of being ACR NEMA
        gdcmWarningMacro( "Looks like an ACR-NEMA file" );
        // Hopefully all ACR-NEMA are RAW:
        PixelData.SetCompressionType( Compression::RAW );
        res = ReadACRNEMAImage();
        }
      else // there is a Unknown TransferSyntax
        {
        // god damit I don't know what to do...
        gdcmWarningMacro( "Attempting to read this file as a DICOM file"
          "\nDesperate attempt" );
        const Tag tpixeldata(0x7fe0, 0x0010);
        if( GetDataSet().FindDataElement( tpixeldata) )
          {
          // gdcm-CR-DCMTK-16-NonSamplePerPix.dcm
          // Someone defined the Transfer Syntax but I have no clue what
          // it is. Since there is Pixel Data element, let's try to read
          // that as a buggy DICOM Image file...
          PixelData.SetCompressionType( Compression::RAW );
          res = ReadImage();
          }
        else
          {
          // Giving up
          abort();
          res = false;
          }
        }
      }
    }

  //if(res) PixelData.Print( std::cout );
  return res;
}

signed short ImageReader::ReadSSFromTag( Tag const &t, StringStream &ss,
  std::string &conversion )
{
  const ByteValue *bv = GetPointerFromElement(t);
  Element<VR::SS,VM::VM1> el;
  assert( bv->GetLength() == 2 );
  conversion = std::string(bv->GetPointer(), 2); 
  ss.Str( conversion );
  el.Read( ss );
  return el.GetValue();
}

unsigned short ImageReader::ReadUSFromTag( Tag const &t, StringStream &ss,
  std::string &conversion )
{
  const ByteValue *bv = GetPointerFromElement(t);
  Element<VR::US,VM::VM1> el;
  assert( bv->GetLength() == 2 );
  conversion = std::string(bv->GetPointer(), 2); 
  ss.Str( conversion );
  el.Read( ss );
  return el.GetValue();
}

int ImageReader::ReadISFromTag( Tag const &t, StringStream &ss,
  std::string &conversion )
{
  const ByteValue *bv = GetPointerFromElement(t);
  Element<VR::IS,VM::VM1> el;
  conversion = std::string(bv->GetPointer(), bv->GetLength());
  assert( conversion.size() == bv->GetLength() );
  const char *debug = conversion.c_str();
  assert( debug[bv->GetLength()] == '\0' ); 
  ss.Str( conversion );
  el.Read( ss );
  int r = el.GetValue();
  return r;
}

bool ImageReader::ReadImage()
{
  const DataSet &ds = GetDataSet();
  TS::NegociatedType type = ds.GetNegociatedType();
  StringStream ss;
  std::string conversion;
  // Construct a stringstream to mimic the reading from the file
  ss.SetSwapCode( Stream.GetSwapCode() );
  PixelData.SetSwapCode( Stream.GetSwapCode() );

  const Tag trecognitioncode(0x0008,0x0010);
  if( ds.FindDataElement( trecognitioncode ) )
    {
    // PHILIPS_Gyroscan-12-MONO2-Jpeg_Lossless.dcm
    // PHILIPS_Gyroscan-12-Jpeg_Extended_Process_2_4.dcm
    gdcmWarningMacro( "Mixture of ACR NEMA and DICOM file" );
    }

  // Ok we have the dataset let's feed the Image (PixelData)
  // 1. First find how many dimensions there is:
  // D 0028|0008 [IS] [Number of Frames] [8 ]
  const Tag tnumberofframes = Tag(0x0028, 0x0008);
  if( ds.FindDataElement( tnumberofframes ) )
    {
    int numberofframes = ReadISFromTag( tnumberofframes, ss, conversion );
    assert( numberofframes != 0 );
    if( numberofframes > 1 )
      {
      PixelData.SetNumberOfDimensions(3);
      PixelData.SetDimensions(2, numberofframes );
      }
    else
      {
      gdcmDebugMacro( "NumberOfFrames was specified with a value of: "
        << numberofframes );
      PixelData.SetNumberOfDimensions(2);
      }
    }
  else
    {
    gdcmWarningMacro( "Attempting a guess for the number of dimensions" );
    PixelData.SetNumberOfDimensions(2);
    }

 
  // 2. What are the col & rows:
  // D 0028|0011 [US] [Columns] [512]
  const Tag tcolumns(0x0028, 0x0011);
  if( ds.FindDataElement( tcolumns ) )
    {
    PixelData.SetDimensions(0,
      ReadUSFromTag( tcolumns, ss, conversion ) );
    }
  else
    {
    // Pretty bad we really need this information. Should not 
    // happen in theory. Maybe papyrus files ??
    gdcmErrorMacro( "This should not happen !" );
    return false;
    }

  // D 0028|0010 [US] [Rows] [512]
  PixelData.SetDimensions(1,
    ReadUSFromTag( Tag(0x0028, 0x0010), ss, conversion ) );

  // 3. Pixel Type ?
  PixelType pt;
  // D 0028|0002 [US] [Samples per Pixel] [1]
  const Tag samplesperpixel = Tag(0x0028, 0x0002);
  if( ds.FindDataElement( samplesperpixel ) )
    {
    pt.SetSamplesPerPixel(
      ReadUSFromTag( samplesperpixel, ss, conversion ) );
    }

  // D 0028|0100 [US] [Bits Allocated] [16]
  pt.SetBitsAllocated(
    ReadUSFromTag( Tag(0x0028, 0x0100), ss, conversion ) );

  // D 0028|0101 [US] [Bits Stored] [12]
  pt.SetBitsStored(
    ReadUSFromTag( Tag(0x0028, 0x0101), ss, conversion ) );

  // D 0028|0102 [US] [High Bit] [11]
  pt.SetHighBit(
    ReadUSFromTag( Tag(0x0028, 0x0102), ss, conversion ) );

  // D 0028|0103 [US] [Pixel Representation] [0]
  pt.SetPixelRepresentation(
    ReadUSFromTag( Tag(0x0028, 0x0103), ss, conversion ) );

  // Very important to set the PixelType here before PlanarConfiguration
  PixelData.SetPixelType( pt );

  // 4. Planar Configuration
  // D 0028|0006 [US] [Planar Configuration] [1]
  const Tag planarconfiguration = Tag(0x0028, 0x0006);
  if( ds.FindDataElement( planarconfiguration ) )
    {
    PixelData.SetPlanarConfiguration(
      ReadUSFromTag( planarconfiguration, ss, conversion ) );
    }

  // 5. Photometric Interpretation
  // D 0028|0004 [CS] [Photometric Interpretation] [MONOCHROME2 ]
  const Tag tphotometricinterpretation(0x0028, 0x0004);
  const ByteValue *photometricinterpretation
    = GetPointerFromElement( tphotometricinterpretation );
  std::string photometricinterpretation_str(
    photometricinterpretation->GetPointer(),
    photometricinterpretation->GetLength() );
  PhotometricInterpretation pi(
    PhotometricInterpretation::GetPIType(
      photometricinterpretation_str.c_str()));
  assert( pi != PhotometricInterpretation::UNKNOW);
  PixelData.SetPhotometricInterpretation( pi );

  // TODO
  assert( pi.GetSamplesPerPixel() == pt.GetSamplesPerPixel() );

  // 6. Do the PixelData
  const Tag pixeldata = Tag(0x7fe0, 0x0010);
  if( !ds.FindDataElement( pixeldata ) )
    {
    gdcmWarningMacro( "No Pixel Data Found" );
    return false;
    }
  const DataElement& pdde = ds.GetDataElement( pixeldata );
  if( type == TS::Explicit )
    {
    const ExplicitDataElement &xde =
      dynamic_cast<const ExplicitDataElement&>(pdde);
    if( xde.GetVR() == VR::OW )
      {
      // Need to byte swap
      PixelData.SetNeedByteSwap(true);
      }
    PixelData.SetValue( xde.GetValue() );
    }
  else if( type == TS::Implicit )
    {
    const ImplicitDataElement &ide =
      dynamic_cast<const ImplicitDataElement&>(pdde);
    PixelData.SetValue( ide.GetValue() );
    }
  else
    {
    gdcmErrorMacro( "Not sure how you are supposed to reach here" );
    return false;
    }

  return true;
}

bool ImageReader::ReadACRNEMAImage()
{
  const DataSet &ds = GetDataSet();
  TS::NegociatedType type = ds.GetNegociatedType();
  StringStream ss;
  std::string conversion;
  // Construct a stringstream to mimic the reading from the file
  ss.SetSwapCode( Stream.GetSwapCode() );
  PixelData.SetSwapCode( Stream.GetSwapCode() );

  // Ok we have the dataset let's feed the Image (PixelData)
  // 1. First find how many dimensions there is:
  // D 0028|0005 [SS] [Image Dimensions (RET)] [2]
  const Tag timagedimensions = Tag(0x0028, 0x0005);
  if( ds.FindDataElement( timagedimensions ) )
    {
    int imagedimensions = ReadSSFromTag( timagedimensions, ss, conversion );
    if ( imagedimensions == 3 )
      {
      PixelData.SetNumberOfDimensions(3);
      // D 0028|0012 [US] [Planes] [262]
      PixelData.SetDimensions(2,
        ReadUSFromTag( Tag(0x0028, 0x0012), ss, conversion ) );
      }
    else if ( imagedimensions == 2 )
      {
      PixelData.SetNumberOfDimensions(2);
      }
    else
      {
      abort();
      }
    }
  else
    {
    gdcmWarningMacro( "Attempting a guess for the number of dimensions" );
    PixelData.SetNumberOfDimensions( 2 );
    }

  // 2. What are the col & rows:
  // D 0028|0011 [US] [Columns] [512]
  PixelData.SetDimensions(0,
    ReadUSFromTag( Tag(0x0028, 0x0011), ss, conversion ) );

  // D 0028|0010 [US] [Rows] [512]
  PixelData.SetDimensions(1,
    ReadUSFromTag( Tag(0x0028, 0x0010), ss, conversion ) );

  // This is the definition of an ACR NEMA image:
  // D 0008|0010 [LO] [Recognition Code (RET)] [ACR-NEMA 2.0]
  // LIBIDO compatible code:
  // D 0008|0010 [LO] [Recognition Code (RET)] [ACRNEMA_LIBIDO_1.1]
  const Tag trecognitioncode(0x0008,0x0010);
  if( ds.FindDataElement( trecognitioncode ) )
    {
    const ByteValue *libido = GetPointerFromElement( trecognitioncode );
    std::string libido_str( libido->GetPointer(), libido->GetLength() );
    assert( libido_str != "CANRME_AILIBOD1_1." );
    if( libido_str == "ACRNEMA_LIBIDO_1.1" )
      {
      // Swap Columns & Rows
      // assert( PixelData.GetNumberOfDimensions() == 2 );
      const unsigned int *dims = PixelData.GetDimensions();
      unsigned int tmp = dims[0];
      PixelData.SetDimensions(0, dims[1] );
      PixelData.SetDimensions(1, tmp );
      }
    else
      {
      assert( libido_str == "ACR-NEMA 2.0"
           || libido_str == "ACR-NEMA 1.0" );
      }
    }
  else
    {
    gdcmWarningMacro(
      "Reading as ACR NEMA an image which does not look likes ACR NEMA" );
    // File: acc-max.dcm is it ACR or DICOM ?
    // abort();
    }

  // 3. Pixel Type ?
  PixelType pt;
  // D 0028|0100 [US] [Bits Allocated] [16]
  pt.SetBitsAllocated(
    ReadUSFromTag( Tag(0x0028, 0x0100), ss, conversion ) );

  // D 0028|0101 [US] [Bits Stored] [12]
  pt.SetBitsStored(
    ReadUSFromTag( Tag(0x0028, 0x0101), ss, conversion ) );

  // D 0028|0102 [US] [High Bit] [11]
  pt.SetHighBit(
    ReadUSFromTag( Tag(0x0028, 0x0102), ss, conversion ) );

  // D 0028|0103 [US] [Pixel Representation] [0]
  pt.SetPixelRepresentation(
    ReadUSFromTag( Tag(0x0028, 0x0103), ss, conversion ) );

  PixelData.SetPixelType( pt );

  // 4. Do the PixelData
  const Tag pixeldata = Tag(0x7fe0, 0x0010);
  if( !ds.FindDataElement( pixeldata ) )
    {
    gdcmWarningMacro( "No Pixel Data Found" );
    return false;
    }
  const DataElement& pdde = ds.GetDataElement( pixeldata );
  if( type == TS::Explicit )
    {
    const ExplicitDataElement &xde =
      dynamic_cast<const ExplicitDataElement&>(pdde);
    if ( xde.GetVR() == VR::OW )
      {
      abort();
      PixelData.SetNeedByteSwap(true);
      }
    PixelData.SetValue( xde.GetValue() );
    }
  else if( type == TS::Implicit )
    {
    const ImplicitDataElement &ide =
      dynamic_cast<const ImplicitDataElement&>(pdde);
    PixelData.SetValue( ide.GetValue() );
    }
  else
    {
    gdcmErrorMacro( "Not sure how you are supposed to reach here" );
    return false;
    }

  return true;
}


} // end namespace gdcm
