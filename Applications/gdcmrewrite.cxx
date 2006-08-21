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
/*
 * Tools to rewrite. Goals being to 'purify' a DICOM file.
 * For now it will do the minimum:
 * - If Group Length is present, the length is garantee to be correct
 * - If Element with Group Tag 0x1, 0x3, 0x5 or 0x7 are present they are
 *   simply discarded (not written).
 * - Elements are written in alphabetical order
 * - 32bits VR have the residue bytes sets to 0x0,0x0
 * - Same goes from Item Length end delimitor, sets to 0x0,0x0
 *
 *
 * \todo in a close future:
 * - Rewrite PMS SQ into DICOM SQ
 * - Rewrite Implicit SQ with defined length as undefined length
 * - Set appropriate VR from DICOM dict
 * - PixelData with `overlay` in unused bits should be cleanup
 * - Any broken JPEG file (wrong bits) should be fixed
 * - DicomObject bug should be fixed
 * - Meta and Dataset should have a matching UID (more generally File Meta
 *   should be correct (Explicit!) and consistant with DataSet)
 * - User should be able to specify he wants Group Length (or remove them)
 *
 *
 * - Later on, it should run through a Validator
 *   which will make sure all field 1, 1C are present and those only
 * - In a perfect world I should remove private tags and transform them into
 *   public fields.
 * - DA should be correct, PN should be correct (no space!)
 * - Enumerated Value should be correct
 */
#include "gdcmReader.h"
#include "gdcmWriter.h"
#include "gdcmFileMetaInformation.h"
#include "gdcmDataSet.h"

#include <string>
#include <iostream>

#include <stdio.h>     /* for printf */
#include <stdlib.h>    /* for exit */
#include <getopt.h>
#include <string.h>

int main (int argc, char *argv[])
{
  int c;
  //int digit_optind = 0;

  std::string filename;
  std::string outfilename;
  while (1) {
    //int this_option_optind = optind ? optind : 1;
    int option_index = 0;
    static struct option long_options[] = {
        {"input", 1, 0, 0},
        {"output", 1, 0, 0},
        {0, 0, 0, 0}
    };

    c = getopt_long (argc, argv, "i:o:",
      long_options, &option_index);
    if (c == -1)
      {
      break;
      }

    switch (c)
      {
    case 0:
        {
        const char *s = long_options[option_index].name;
        printf ("option %s", s);
        if (optarg)
          {
          if( option_index == 0 ) /* input */
            {
            assert( strcmp(s, "input") == 0 );
            assert( filename.empty() );
            filename = optarg;
            }
          printf (" with arg %s", optarg);
          }
        printf ("\n");
        }
      break;

    case 'i':
      printf ("option i with value ’%s’\n", optarg);
      assert( filename.empty() );
      filename = optarg;
      break;

    case 'o':
      printf ("option o with value ’%s’\n", optarg);
      assert( outfilename.empty() );
      outfilename = optarg;
      break;

    case '?':
      break;

    default:
      printf ("?? getopt returned character code 0%o ??\n", c);
      }
  }

  if (optind < argc)
    {
    printf ("non-option ARGV-elements: ");
    while (optind < argc)
      {
      printf ("%s ", argv[optind++]);
      }
    printf ("\n");
    }

  if( filename.empty() )
    {
    std::cerr << "Need input file (-i)\n";
    return 1;
    }
  // else
  //std::cout << "Filename: " << filename << std::endl;
  gdcm::Reader reader;
  reader.SetFileName( filename.c_str() );
  if( !reader.Read() )
    {
    std::cerr << "Failed to read: " << filename << std::endl;
    return 1;
    }

  //const gdcm::FileMetaInformation &h = reader.GetHeader();
  //std::cout << h << std::endl;

  //const gdcm::DataSet &ds = reader.GetDataSet();
  //std::cout << ds << std::endl;

  if( outfilename.empty() )
    {
    std::cerr << "Need output file (-o)\n";
    return 1;
    }

  gdcm::Writer writer;
  writer.SetFileName( outfilename.c_str() );
  writer.SetPreamble( reader.GetPreamble() );
  writer.SetHeader( reader.GetHeader() );
  writer.SetDataSet( reader.GetDataSet() );
  if( !writer.Write() )
    {
    std::cerr << "Failed to write: " << outfilename << std::endl;
    return 1;
    }

  return 0;
}

