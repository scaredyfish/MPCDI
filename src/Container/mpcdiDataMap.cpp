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

#include "mpcdiDataMap.h"
#include "mpcdiErrorHelper.h"

using namespace mpcdi;

DataMap::DataMap(unsigned int sizeX, unsigned int sizeY,
                 ComponentDepth componentDepth)
  : m_SizeX(sizeX),
    m_SizeY(sizeY),
    m_ComponentDepth(componentDepth),
    m_Data(std::vector<unsigned char>(sizeX*sizeY*componentDepth))
{
  m_BitDepth = mpcdi::BD_EIGHT;
}

/* ====================================================================== */

DataMap::~DataMap()
{
  m_Data.clear();
}

/* ====================================================================== */

MPCDI_Error DataMap::CopyData(const DataMap &source)
{
  if ((source.GetSizeX() != m_SizeX) || (source.GetSizeY() != m_SizeY) || 
      (source.GetBitDepth() != m_BitDepth) ||
      (source.GetComponentDepth() != m_ComponentDepth))
  {
    CREATE_ERROR_MSG(msg,"DataMap copy cannot be performed on maps of different size \
                          input (" << source.GetSizeX() << "," << source.GetSizeY() << ") \
                          vs this (" << this->GetSizeX() << "," << this->GetSizeY() << std::endl);
    ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
  }

  std::copy( source.m_Data.begin(), source.m_Data.end(), m_Data.begin() );
  return MPCDI_SUCCESS;
}

