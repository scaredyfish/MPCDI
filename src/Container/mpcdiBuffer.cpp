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

#include "mpcdiBuffer.h"
#include "mpcdiErrorHelper.h"

using namespace mpcdi;

/* ====================================================================== */

Buffer::~Buffer()
{
  for(RegionIterator i = m_Regions.begin();i!=m_Regions.end();i++)
  {
    mpcdiSafeDeleteMacro(i->second);
    i->second = NULL;
  }
  m_Regions.clear();
}

/* ====================================================================== */

Region *Buffer::GetRegion(std::string regionId)
{
  if (m_Regions.count(regionId) == 1)
    return m_Regions.find(regionId)->second;

  return NULL;
}

/* ====================================================================== */

std::vector<std::string> Buffer::GetRegionNames()
{
  std::vector<std::string> regionNames;
  for(RegionIterator i = m_Regions.begin();i != m_Regions.end();i++)
    regionNames.push_back(i->first);
  
  return regionNames;
}

/* ====================================================================== */

MPCDI_Error Buffer::NewRegion(std::string regionId)
{
  if (m_Regions.count(regionId) != 0)
  {
    CREATE_ERROR_MSG(regionErr, "A region with id: " << regionId << " already exists");
    ReturnCustomErrorMacro(MPCDI_NON_UNIQUE_ID, regionErr);
  }

  m_Regions.insert(RegionPair(regionId,new Region(regionId)));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error Buffer::DeleteRegion(std::string regionId)
{
  if (m_Regions.count(regionId) != 0)
  {
    RegionIterator region=m_Regions.find(regionId);
    mpcdiSafeDeleteMacro(region->second);
    region->second = NULL;
    m_Regions.erase(regionId);
    return MPCDI_SUCCESS;
  }

  return MPCDI_REGION_DOES_NOT_EXISTS;
}

/* ====================================================================== */
