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
#include "gdcmItem.h"
#include "gdcmStringStream.h"
#include "gdcmVR.h"
#include "gdcmExplicitDataElement.h"
#include "gdcmSwapper.h"

void CreateDataElement(gdcm::ExplicitDataElement &de, int offset)
{
  std::stringstream ss;

  gdcm::Tag tag(0x1234, 0x5678+offset);
  gdcm::VR vr = gdcm::VR::UN;
  const char str[] = "GDCM";
  uint32_t len = strlen(str);
  assert( sizeof(uint32_t) == 4 );
  gdcm::ByteValue val(str, len);
  tag.Write<gdcm::SwapperNoOp>(ss);
  vr.Write(ss);
  const char *slen = reinterpret_cast<char*>(&len);
  ss.write( slen, 4);
  val.Write<gdcm::SwapperNoOp>(ss);
  de.Read<gdcm::SwapperNoOp>( ss );

  std::cout << de << std::endl;
}

int TestItem(int , char *[])
{
  gdcm::Item item;
  std::cout << item << std::endl;

  gdcm::Item it1;
  gdcm::Item it2;

  gdcm::DataSet ds;
  gdcm::ExplicitDataElement xde;
  CreateDataElement(xde,0);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,1);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,2);
  ds.InsertDataElement( xde );
  CreateDataElement(xde,3);
  ds.InsertDataElement( xde );

  std::cout << ds << std::endl;

  return 0;
}
