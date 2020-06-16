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

#include "mpcdiRegion.h"
#include "mpcdiErrors.h"

using namespace mpcdi;


/* ====================================================================== */
Region::Region(std::string id):
    m_Id(id),
    m_Frustum(NULL),
    m_CoordinateFrame(NULL),
    m_FileSet(new FileSet(id))
{ 
  m_Xresolution = m_Yresolution = -1; m_X = m_Y = m_Xsize = m_Ysize = -1.0f;
}

/* ====================================================================== */

Region::~Region()
{
  mpcdiSafeDeleteMacro(m_FileSet);
  mpcdiSafeDeleteMacro(m_Frustum);
  mpcdiSafeDeleteMacro(m_CoordinateFrame);
}

/* ====================================================================== */

MPCDI_Error Region::SetCoordinateFrame()
{
  mpcdiSafeDeleteMacro(m_CoordinateFrame);
  m_CoordinateFrame = new CoordinateFrame();
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error Region::SetFrustum()
{
  mpcdiSafeDeleteMacro(m_Frustum);
  m_Frustum = new Frustum();
  return MPCDI_SUCCESS;
}

/* ====================================================================== */
