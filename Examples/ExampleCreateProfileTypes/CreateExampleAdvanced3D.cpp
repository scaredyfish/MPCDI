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

#include "mpcdiCreateAdvanced3DProfile.h"

/* ====================================================================== */

mpcdi::MPCDI_Error FillInCreatorCommonData(
               mpcdi::CreateProfile &ProfileCreator,
               const unsigned int &xRes=300,
               const unsigned int &yRes=200);

/* ====================================================================== */

mpcdi::MPCDI_Error CreateExampleAdvanced3D(mpcdi::Profile *&profile)
{
  mpcdi::CreateAdvanced3DProfile ProfileCreator;

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
          ProfileCreator.
            SetGeometricUnit(ProfileCreator.GetGeometryWarpFile(reg->second),
            mpcdi::GeometricUnitm);
          ProfileCreator.
            SetOriginOf3DData(ProfileCreator.GetGeometryWarpFile(reg->second),
            mpcdi::OriginOf3DDatafloorCenter);
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
