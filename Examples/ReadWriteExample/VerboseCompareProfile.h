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
// .NAME VerboseCompareProfile - Compare a Profile Verbosely
// .SECTION Description
//
// .AUTHOR Samson Timoner, Scalable Display Technologies
//

#include "mpcdiProfile.h"

mpcdi::MPCDI_Error
VerboseCompareProfile(std::ostream &os,
                      mpcdi::Profile *profileIn,
                      mpcdi::Profile *profileOut);


mpcdi::MPCDI_Error 
VerboseCompareFrustum(std::ostream &os,
                      mpcdi::Frustum *frustumIn,
                      mpcdi::Frustum *frustum);

mpcdi::MPCDI_Error 
VerboseCompareDistortionMap(std::ostream &os,
                            mpcdi::DistortionMap *distortionMapIn,
                            mpcdi::DistortionMap *distortionMap);

mpcdi::MPCDI_Error 
VerboseCompareGeometryWarpFile(std::ostream &os,
                               mpcdi::GeometryWarpFile *geometryWarpFileIn,
                               mpcdi::GeometryWarpFile *geometryWarpFile);

mpcdi::MPCDI_Error 
VerboseCompareBetaMap(std::ostream &os,
                      mpcdi::BetaMap *betaMapIn,
                      mpcdi::BetaMap *betaMap);

mpcdi::MPCDI_Error 
VerboseCompareAlphaMap(std::ostream &os,
                      mpcdi::AlphaMap *alphaMapIn,
                      mpcdi::AlphaMap *alphaMap);

