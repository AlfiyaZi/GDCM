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
#include "gdcmSequenceOfFragments.h"
#include "gdcmImplicitDataElement.h"

namespace gdcm
{

IStream& SequenceOfFragments::Read(IStream &is)
{
  if( SequenceLengthField.IsUndefined() )
    {
    const Tag seqDelItem(0xfffe,0xe0dd);
    // First item is the basic offset table:
    Table.Read(is);
    gdcmDebugMacro( "Table: " << Table );
    // not used for now...
    Fragment frag;
    do
      {
      frag.Read(is);
      gdcmDebugMacro( "Frag: " << frag );
      Fragments.push_back( frag );
      }
    while( frag.GetTag() != seqDelItem );
    assert( frag.GetVL() == 0 );
    }
  else
    {
    abort();
    }
  return is;
}

OStream const & SequenceOfFragments::Write(OStream &os) const
{
  if( !Table.Write(os) )
    {
    assert(0 && "Should not happen");
    return os;
    }
  FragmentVector::const_iterator it = Fragments.begin();
  for(;it != Fragments.end(); ++it)
    {
    it->Write(os);
    }
  return os;
}

unsigned int SequenceOfFragments::GetNumberOfFragments() const
{
  // Do not count the last fragment
  assert( SequenceLengthField.IsUndefined() );
  return Fragments.size() - 1;
}

VL SequenceOfFragments::ComputeLength() const
{
  VL length = 0;
  // First the table
  length += Table.GetLength();
  // Then all the fragments
  FragmentVector::const_iterator it = Fragments.begin();
  for(;it != Fragments.end(); ++it)
    {
    length += it->GetLength();
    }
  assert( SequenceLengthField.IsUndefined() );
  return length;
}

unsigned long SequenceOfFragments::ComputeByteLength() const
{
  unsigned long r = 0;
  FragmentVector::const_iterator it = Fragments.begin();
  for(;it != Fragments.end(); ++it)
    {
    r += it->GetVL();
    }
  return r;
}

bool SequenceOfFragments::GetFragBuffer(unsigned int fragNb, char *buffer, unsigned long &length) const
{
  FragmentVector::const_iterator it = Fragments.begin();
    {
    const Fragment &frag = *(it+fragNb);
    const ByteValue &bv = dynamic_cast<const ByteValue&>(frag.GetValue());
    const VL len = frag.GetVL();
    bv.GetBuffer(buffer, len);
    length = len;
    }
  return true;
}

const Fragment& SequenceOfFragments::GetFragment(unsigned int num) const
{
  assert( num < Fragments.size() );
  FragmentVector::const_iterator it = Fragments.begin();
  const Fragment &frag = *(it+num);
  return frag;
}

bool SequenceOfFragments::GetBuffer(char *buffer, unsigned long length) const
{
  FragmentVector::const_iterator it = Fragments.begin();
  unsigned long total = 0;
  for(;it != Fragments.end(); ++it)
    {
    const Fragment &frag = *it;
    const ByteValue &bv = dynamic_cast<const ByteValue&>(frag.GetValue());
    const VL len = frag.GetVL();
    bv.GetBuffer(buffer, len);
    buffer += len;
    total += len;
    }
  if( total != length )
    {
    //std::cerr << " DEBUG: " << total << " " << length << std::endl;
    abort();
    return false;
    }
  return true;
}

bool SequenceOfFragments::WriteBuffer(std::ostream &os) const
{
  FragmentVector::const_iterator it = Fragments.begin();
  unsigned long total = 0;
  for(;it != Fragments.end(); ++it)
    {
    const Fragment &frag = *it;
    const ByteValue &bv = dynamic_cast<const ByteValue&>(frag.GetValue());
    const VL len = frag.GetVL();
    bv.WriteBuffer(os);
    total += len;
    }
  //if( total != length )
  //  {
  //  //std::cerr << " DEBUG: " << total << " " << length << std::endl;
  //  abort();
  //  return false;
  //  }
  return true;
}

} // end namespace gdcm
