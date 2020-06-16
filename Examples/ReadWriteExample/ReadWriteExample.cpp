/* =========================================================================

  Program:   MPCDI Library
  Language:  C++
  Date:      $Date: 2012-02-08 11:39:41 -0500 (Wed, 08 Feb 2012) $
  Version:   $Revision: 18341 $

  Copyright (c) 2013 Scalable Display Technologies, Inc.
  All Rights Reserved.
  The MPCDI Library is distributed under the BSD license.
  Please see License.txt distributed with this package.

===================================================================auto== */
// .NAME Reader - An MPCDI Reader and write example.
// .SECTION Description
//
// .AUTHOR Scalable Display Technologies, Inc.
//


#include "mpcdiDisplay.h"
#include "mpcdiWriter.h"
#include "mpcdiReader.h"

#include "CreateSampleProfile.h"
#include "VerboseCompareProfile.h"
#include <iostream>
#include <algorithm>

#define DO_PAUSE
#ifdef DO_PAUSE
#  define PAUSE {std::cout<< "Press enter to continue....";std::cin.ignore();}
#else 
# definE PAUSE
#endif

/* ====================================================================== */
mpcdi::MPCDI_Error  WriteTest(const std::string &FileName,
                              mpcdi::Profile *profile);
mpcdi::MPCDI_Error ReadTest(const std::string &FileName, 
                           mpcdi::Profile * &profileIn);
/* ====================================================================== */

int main( int argc, const char ** argv )
{
  bool DoWriteTest = true;
  bool DoReadTest = true;
  mpcdi::MPCDI_Error mpcdi_err = mpcdi::MPCDI_SUCCESS;
  std::string profileName= "SampleMPCDI.mpcdi"; /*target profile name*/

  /* create a new profile */
  mpcdi::Profile *profileOut = NULL;
  if (MPCDI_FAILED(mpcdi_err = CreateSampleProfile(std::cout,
                                                   profileOut)))
    {
      PAUSE;
      return mpcdi_err;
    }

  if (DoWriteTest)
    {
      if(MPCDI_FAILED(WriteTest(profileName,profileOut)))
        {
          delete profileOut;
          PAUSE;
          return mpcdi::MPCDI_FAILURE;
        }
    }

  mpcdi::Profile *profileIn = NULL;
  if (DoReadTest)
    {
      if (MPCDI_FAILED(ReadTest(profileName,profileIn)))
        {
          delete profileOut;
          PAUSE;
          return mpcdi::MPCDI_FAILURE;
        }
    }

  if(DoReadTest && DoWriteTest)
    {
      if (MPCDI_FAILED(VerboseCompareProfile(std::cout,profileIn,profileOut)))
        {
          std::cout << "Error on Read/Write Test: " << std::endl;
          PAUSE;
          delete profileOut;
          delete profileIn;
          return mpcdi_err;
        }
      std::cout << "Success on Read/Write Test. " << std::endl;
    }


  /* clean up */
  if (profileOut)   delete profileOut;
  if (profileIn)    delete profileIn;
  PAUSE;
  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */

mpcdi::MPCDI_Error  WriteTest(const std::string &FileName,
                              mpcdi::Profile *profile)
{
  mpcdi::Writer *writer = mpcdi::Writer::CreateWriter();
  writer->SetOverwriteExistingFile(true);
  bool test = writer->GetOverwriteExistingFile();
  mpcdi::MPCDI_Error mpcdi_err = writer->Write(FileName, *profile);
  delete writer;
  if (MPCDI_FAILED(mpcdi_err))
    {
      std::cout << "Error writing: " 
                <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() 
                << std::endl;
    }
  return mpcdi_err;
}

/* ====================================================================== */

mpcdi::MPCDI_Error ReadTest(const std::string &FileName, 
                            mpcdi::Profile * &profileIn)
{
  profileIn = new mpcdi::Profile(); 

  /* perform read from disk */
  mpcdi::Reader *Reader = mpcdi::Reader::CreateReader();
  std::cout << Reader->GetSupportedVersions() << std::endl;
  mpcdi::MPCDI_Error mpcdi_err = Reader->Read(FileName, profileIn);
  if (MPCDI_FAILED(mpcdi_err))
  {
    std::cout << "Error encountered while reading: " 
              <<  mpcdi_err << ": " 
              << mpcdi::ErrorHelper::GetLastError() 
              << std::endl;
    delete profileIn;
    profileIn = NULL;
  }

  delete Reader;
  return mpcdi_err;
}

/* ====================================================================== */
