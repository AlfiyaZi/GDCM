/*=========================================================================

  Program: GDCM (Grassroots DICOM). A DICOM library
  Module:  $URL$

  Copyright (c) 2006-2008 Mathieu Malaterre
  All rights reserved.
  See Copyright.txt or http://gdcm.sourceforge.net/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "gdcmFileExplicitFilter.h"
#include "gdcmSequenceOfFragments.h"
#include "gdcmSequenceOfItems.h"
#include "gdcmFragment.h"
#include "gdcmGlobal.h"
#include "gdcmDict.h"
#include "gdcmDicts.h"
#include "gdcmGroupDict.h"
#include "gdcmVR.h"
#include "gdcmVM.h"
#include "gdcmDataSetHelper.h"

namespace gdcm
{

void FileExplicitFilter::SetRecomputeItemLength(bool b)
{
  RecomputeItemLength = b;
}

void FileExplicitFilter::SetRecomputeSequenceLength(bool b)
{
  RecomputeSequenceLength = b;
}

bool FileExplicitFilter::ProcessDataSet(DataSet &ds, Dicts const & dicts)
{
  if( RecomputeSequenceLength || RecomputeItemLength )
    {
    gdcmWarningMacro( "Not implemented sorry" );
    return false;
    }
  DataSet::Iterator it = ds.Begin();
  for( ; it != ds.End(); )
    {
    DataElement de = *it;
    std::string strowner;
    const char *owner = 0;
    const Tag& t = de.GetTag();
    if( t.IsPrivate() && !ChangePrivateTags )
      {
      // nothing to do ! just skip
      ++it;
      continue;
      }
    if( t.IsPrivate() && !t.IsPrivateCreator() )
      { 
      strowner = ds.GetPrivateCreator(t);
      owner = strowner.c_str();
      }
    const DictEntry &entry = dicts.GetDictEntry(t,owner);
    const VR &vr = entry.GetVR();
    const VM &vm = entry.GetVM();

    //assert( de.GetVR() == VR::INVALID );
    VR cvr = DataSetHelper::ComputeVR(*F,ds, t);
    VR oldvr = de.GetVR();
    SequenceOfItems *sqi = de.GetSequenceOfItems();
    if( de.GetByteValue() )
      {
      // all set
      assert( cvr != VR::SQ /*&& cvr != VR::UN*/ );
      assert( cvr != VR::INVALID );
      if( cvr != VR::UN )
        {
        // about to change , make some paranoid test:
        //assert( cvr.Compatible( oldvr ) ); // LT != LO but there are somewhat compatible
        if( cvr & VR::VRASCII )
          {
          //assert( oldvr & VR::VRASCII || oldvr == VR::INVALID || oldvr == VR::UN );
          // gdcm-JPEG-Extended.dcm has a couple of VR::OB private field
          // is this a good idea to change them to an ASCII when we know this might not work ?
          if( !(oldvr & VR::VRASCII || oldvr == VR::INVALID || oldvr == VR::UN) )
            {
            assert( t.IsPrivate() ); // Hopefully this only happen with private tag
            gdcmErrorMacro( "Cannot convert VR for tag: " << t );
            return false;
            }
          }
        if( cvr & VR::VRBINARY )
          {
          // PHILIPS_Gyroscan-12-MONO2-Jpeg_Lossless.dcm
          if( !( oldvr & VR::VRBINARY || oldvr == VR::INVALID || oldvr == VR::UN ) )
            {
            assert( t.IsPrivate() ); // Hopefully this only happen with private tag
            gdcmErrorMacro( "Cannot convert VR for tag: " << t );
            return false;
            }
          }

        // let's do one more check we are going to make this attribute explicit VR, there is
        // still a special case, when VL is > uint16_max then we must give up:
        if( !(cvr & VR::VL32) && de.GetVL() > UINT16_MAX )
          {
          abort(); // TODO need testing
          }
        de.SetVR( cvr );
        }
      }
    else if( sqi )
      {
      assert( cvr == VR::SQ || cvr == VR::UN );
      de.SetVR( VR::SQ );
      sqi->SetLengthToUndefined();
      de.SetVLToUndefined();
      // recursive
      SequenceOfItems::ItemVector::iterator it = sqi->Items.begin();
      for(; it != sqi->Items.end(); ++it)
        {
        //Item &item = const_cast<Item&>(*it);
        Item &item = *it;
        item.SetVLToUndefined();
        DataSet &nds = item.GetNestedDataSet();
        //const DataElement &deitem = item;
        ProcessDataSet(nds, dicts);
        }
      }
    else if( de.GetSequenceOfFragments() )
      {
      assert( cvr & VR::OB_OW );
      }
    ++it;
    ds.Replace( de );
    }
  return true;
}

bool FileExplicitFilter::Change()
{
  //if( !UseVRUN)
  //  {
  //  gdcmErrorMacro( "Not implemented" );
  //  return false;
  //  }
  const Global& g = GlobalInstance;
  const Dicts &dicts = g.GetDicts();
  const Dict &d = dicts.GetPublicDict();

  DataSet &ds = F->GetDataSet();

  bool b = ProcessDataSet(ds, dicts);

  return b;
}


} // end namespace gdcm

