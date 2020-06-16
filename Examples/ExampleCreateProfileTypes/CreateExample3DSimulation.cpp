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

#include "mpcdiCreate3DSimulationProfile.h"

/* ====================================================================== */

mpcdi::MPCDI_Error FillInCreatorCommonData(
               mpcdi::CreateProfile &ProfileCreator,
               const unsigned int &xRes=300,
               const unsigned int &yRes=200);

/* ====================================================================== */

mpcdi::MPCDI_Error CreateExample3DSimulation(mpcdi::Profile *&profile)
{
  mpcdi::Create3DSimulationProfile ProfileCreator;

  // Fill In Data Common to all MPCDI Creators
  MPCDI_FAIL_RET(FillInCreatorCommonData(ProfileCreator));


  // Specific to this Creator.
  // Set the Geometric Unit and Origin of 3D Data in all
  // regions.
  mpcdi::Display *Disp = ProfileCreator.GetProfile()->GetDisplay();
  for(mpcdi::Display::BufferIterator buf = Disp->GetBufferBegin();
      buf!=Disp->GetBufferEnd();++buf)
    {
      for(mpcdi::Buffer::RegionIterator reg = buf->second->GetRegionBegin();
          reg != buf->second->GetRegionEnd();++reg)
        {
          MPCDI_FAIL_RET(ProfileCreator.CreateFrustum(reg->second));
          mpcdi::Frustum *f = ProfileCreator.GetFrustum(reg->second);
          f->SetPitch(-10);
          f->SetYaw(5);
          f->SetRoll(7);
          f->SetDownAngle(-10);
          f->SetUpAngle(10);
          f->SetLeftAngle(-20);
          f->SetRightAngle(12);
        }
    }


  // Validate
  MPCDI_FAIL_RET(ProfileCreator.ValidateProfile());

  // the results
  ProfileCreator.SetDeleteProfile(false);
  profile = ProfileCreator.GetProfile();

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */
