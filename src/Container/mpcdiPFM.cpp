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

#include "mpcdiPFM.h"
#include "mpcdiErrorHelper.h"
#include <stdlib.h>
#include <string.h>

using namespace mpcdi;

PFM::PFM(unsigned int sizeX, unsigned int sizeY)
  : m_SizeX(sizeX),
    m_SizeY(sizeY)
{
  size_t size = sizeX*sizeY*sizeof(NODE);
  m_Data = (NODE*)malloc(size);
  memset(m_Data,0,size); //initialize to zero
}

/* ====================================================================== */

PFM::~PFM()
{
  mpcdiSafeDeleteMacro(m_Data);
}

/* ====================================================================== */

MPCDI_Error PFM::CopyData(const PFM &source)
{
  if ((source.GetSizeX() != m_SizeX) || (source.GetSizeY() != m_SizeY))
  {
    CREATE_ERROR_MSG(msg,"PFM copy cannot be performed on objects of different size \
                          input (" << source.GetSizeX() << "," << source.GetSizeY() << ") \
                          vs this (" << this->GetSizeX() << "," << this->GetSizeY() << std::endl);
    ReturnCustomErrorMacro(MPCDI_FAILURE, msg);
  }
  if (m_Data == NULL)
  {
    CREATE_ERROR_MSG(msg,"PFM copy cannot be performed to object that is not allocated. \
                          This is probably a memory allocation error." << std::endl);
  }

  memcpy( m_Data, source.m_Data, m_SizeX*m_SizeY*sizeof(NODE) );
  return MPCDI_SUCCESS;
}

