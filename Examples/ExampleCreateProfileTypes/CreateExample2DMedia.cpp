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

#include "mpcdiCreate2DMediaProfile.h"

/* ====================================================================== */

mpcdi::MPCDI_Error FillInCreatorCommonData(
               mpcdi::CreateProfile &ProfileCreator,
               const unsigned int &xRes=300,
               const unsigned int &yRes=200);

/* ====================================================================== */

mpcdi::MPCDI_Error CreateExample2DMedia(mpcdi::Profile *&profile)
{
  mpcdi::Create2DMediaProfile ProfileCreator;

  // Fill In Data Common to all MPCDI Creators
  MPCDI_FAIL_RET(FillInCreatorCommonData(ProfileCreator));

  // Stuff Specific to this profile (nothing)

  // Validate
  MPCDI_FAIL_RET(ProfileCreator.ValidateProfile());

  // the results
  ProfileCreator.SetDeleteProfile(false);
  profile = ProfileCreator.GetProfile();

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */
