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

#include "mpcdiCreateProfile.h"

using namespace mpcdi;

/* ====================================================================== */

CreateProfile::CreateProfile()
{
  m_DeleteProfile = true;
  m_Profile = new mpcdi::Profile();

  // just to fill in something.
  m_Profile->SetProfileType(mpcdi::ProfileType2d);


  m_HaveBetaMap = false;
  m_HaveDistortionMap = false;
  m_HavePerPixelResolution = false;

  // Everyone starts with level 1
  m_Profile->SetLevel(1);
}

/* ====================================================================== */

CreateProfile::~CreateProfile()
{
  if (m_DeleteProfile)
    {
      mpcdiSafeDeleteMacro(m_Profile);
    }
}

/* ====================================================================== */

void CreateProfile::CheckPerPixelResolution(Region *r,
                                            const int &xres,
                                            const int &yres)
{
  if (m_HavePerPixelResolution) return;

  // Does not do the 32 pixel by 32 pixel version well (2D Media).
  if (xres > r->GetXresolution()/10) m_HavePerPixelResolution = true;
  if (yres > r->GetYresolution()/10) m_HavePerPixelResolution = true;
  return;
}

/* ====================================================================== */

unsigned int CreateProfile::UpdateLevel()
{
  // Can't decrease the level. So, if at max, just return.
  if (m_Profile->GetLevel() == 4) return m_Profile->GetLevel();

  // BetaMap guarantees level 2 or 4.
  if (m_HaveBetaMap && m_Profile->GetLevel() == 1)
    m_Profile->SetLevel(2);

  if(m_Profile->GetProfileType() != mpcdi::ProfileTypesl)
    {
      // BetaMap + PerPixel means level 4.
      if (m_HaveBetaMap && m_HavePerPixelResolution)
        {
          m_Profile->SetLevel(4);
          return m_Profile->GetLevel();
        }
      
      // PerPixel Guarantees level 3 or 4.
      if (m_HavePerPixelResolution && m_Profile->GetLevel() == 1)
        m_Profile->SetLevel(3);
    }
  else
    {
      // BetaMap + PerPixel means level 4.
      if (m_HaveBetaMap && m_HaveDistortionMap)
        {
          m_Profile->SetLevel(4);
          return m_Profile->GetLevel();
        }

      // PerPixel Guarantees level 3 or 4.
      if (m_HaveDistortionMap && m_Profile->GetLevel() == 1)
        m_Profile->SetLevel(3);
    }

  return m_Profile->GetLevel();
}

/* ====================================================================== */
