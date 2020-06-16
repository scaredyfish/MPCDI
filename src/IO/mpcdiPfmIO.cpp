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

#include "mpcdiPfmIO.h"
#include "mpcdiErrorHelper.h"
#include <string.h>
#include <iostream>
#include <fstream>

using namespace mpcdi;

#define PNG_SIGNATURE_SIZE 8

/* ====================================================================== */

MPCDI_Error PfmIO::Read(std::string fileName, mpcdi::PFM *&pfm)
{
  std::fstream source(fileName.c_str(), std::ios::in | std::ios::binary);
  if (!source.is_open())
  {
    source.close();
    CREATE_ERROR_MSG(msg,"Error opening file " << fileName << " for reading");
    ReturnCustomErrorMacro(MPCDI_PFM_READ_ERROR,msg);
  }

  MPCDI_Error err = Read(source, pfm);

  source.close();

  return err;
}

/* ====================================================================== */

MPCDI_Error PfmIO::Read(std::istream &source, mpcdi::PFM *&pfm)
{
  // char strPF[3];
  unsigned int sizeX = 0;
  unsigned int sizeY = 0;
  float scale = 0;

  char buffer[256]={0};

  source.read(buffer,3);
  if (!(buffer[0]==80 && buffer[1]==70)) // || buffer[1]=102 if grayscale is allowed which we don;t
  {
    CREATE_ERROR_MSG(msg,"File is not a valid PFM file first two bytes are: " << buffer[0] << buffer[1] << std::endl);
    ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
  }

  source >> sizeX >> sizeY;
  source >> scale;
  source.read(buffer,1); // get end of line out of the way

  pfm = new PFM(sizeX,sizeY);

  int lSize;
  lSize = pfm->GetSizeX()*3*sizeof(float);
  for(int y=pfm->GetSizeY()-1; y>=0 && source.good(); y--)
  {
    source.read((char*)pfm->GetData()+lSize*y, lSize);
  }

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error PfmIO::Write(std::string fileName, mpcdi::PFM &pfm)
{
  std::fstream source(fileName.c_str(), std::ios::out | std::ios::binary);
  if (!source.is_open())
  {
    source.close();
    CREATE_ERROR_MSG(msg,"Error opening file " << fileName << " for writing");
    ReturnCustomErrorMacro(MPCDI_PFM_WRITE_ERROR,msg);
  }

  MPCDI_Error err = Write(source, pfm);

  source.close();

  return err;
}

/* ====================================================================== */
#pragma warning(disable : 4996)

MPCDI_Error PfmIO::Write( std::ostream &source, mpcdi::PFM &pfm)
{
 char sizes[256];
 
 source.write("PF\n",sizeof(char)*3);
 sprintf(sizes, "%d %d\n", pfm.GetSizeX(), pfm.GetSizeY());
 
 source.write(sizes,sizeof(char) * strlen(sizes));
 source.write("-1.000000\n",sizeof(char)*10);
 
 int rowSize = pfm.GetSizeX()*3*sizeof(float);
 for(int y=pfm.GetSizeY()-1; y>=0; y--)
  source.write((char*)pfm.GetData()+rowSize*y, rowSize);
 
 return MPCDI_SUCCESS;
}
#pragma warning(default : 4996)


/* ====================================================================== */
