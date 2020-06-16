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

#include <time.h>
#include "mpcdiDisplay.h"
#include "mpcdiFileUtils.h"
#include "mpcdiErrorHelper.h"

using namespace mpcdi;

Display::~Display()
{
  for(BufferIterator i = m_Buffers.begin();i!=m_Buffers.end();i++)
  {
    mpcdiSafeDeleteMacro(i->second);
    i->second = NULL;
  }
  m_Buffers.clear();
}

/* ====================================================================== */

Buffer *Display::GetBuffer(std::string regionId)
{
  if (m_Buffers.count(regionId) == 1)
    return m_Buffers.find(regionId)->second;

  return NULL;
}

/* ====================================================================== */

std::vector<std::string> Display::GetBufferNames()
{
  std::vector<std::string> bufferNames;
  for(BufferIterator i = m_Buffers.begin();i!=m_Buffers.end();i++)
    bufferNames.push_back(i->first);
  
  return bufferNames;
}

/* ====================================================================== */

MPCDI_Error Display::NewBuffer(std::string regionId)
{
  if (m_Buffers.count(regionId) != 0)
  {
    CREATE_ERROR_MSG(regionErr, "A buffer with id: " << regionId << " already exists");
    ReturnCustomErrorMacro(MPCDI_NON_UNIQUE_ID, regionErr);
  }

  m_Buffers.insert(BufferPair(regionId,new Buffer(regionId)));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error Display::DeleteBuffer(std::string regionId)
{
  if (m_Buffers.count(regionId) != 0)
  {
    BufferIterator region=m_Buffers.find(regionId);
    mpcdiSafeDeleteMacro(region->second);
    region->second = NULL;
    m_Buffers.erase(regionId);
    return MPCDI_SUCCESS;
  }

  return MPCDI_REGION_DOES_NOT_EXISTS;
}

/* ====================================================================== */