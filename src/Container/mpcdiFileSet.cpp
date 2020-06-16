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
#include "mpcdiFileSet.h"
#include "mpcdiFileUtils.h"
#include "mpcdiErrorHelper.h"

using namespace mpcdi;

/* ====================================================================== */

FileSet::FileSet(std::string regionId):
  m_GeometryWarpFile(NULL),
  m_AlphaMap(NULL),
  m_BetaMap(NULL),
  m_DistortionMap(NULL), 
  m_RegionId(regionId)
{

}

/* ====================================================================== */

FileSet::~FileSet()
{
  mpcdiSafeDeleteMacro(m_AlphaMap);
  mpcdiSafeDeleteMacro(m_BetaMap);
  mpcdiSafeDeleteMacro(m_DistortionMap);
  mpcdiSafeDeleteMacro(m_GeometryWarpFile)
}

/* ====================================================================== */

MPCDI_Error FileSet::SetAlphaMap(unsigned int sizeX, unsigned int sizeY,
                                 mpcdi::ComponentDepth componentDepth)
{
  mpcdiSafeDeleteMacro(m_AlphaMap);
  m_AlphaMap = new AlphaMap(sizeX,sizeY,componentDepth);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error FileSet::SetBetaMap(unsigned int sizeX, unsigned int sizeY,
                                mpcdi::ComponentDepth componentDepth)
{
  mpcdiSafeDeleteMacro(m_BetaMap);
  m_BetaMap = new BetaMap(sizeX,sizeY,componentDepth);
  
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error FileSet::SetDistortionMap(unsigned int sizeX, 
                                      unsigned int sizeY)
{
  mpcdiSafeDeleteMacro(m_DistortionMap);
  m_DistortionMap = new DistortionMap(sizeX,sizeY);
  
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error FileSet::SetGeometryWarpFile(unsigned int sizeX,
                                         unsigned int sizeY)
{
  mpcdiSafeDeleteMacro(m_GeometryWarpFile);
  m_GeometryWarpFile = new GeometryWarpFile(sizeX,sizeY);
  
  return MPCDI_SUCCESS;
}
