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

#include "mpcdiGeometryWarpFile.h"

using namespace mpcdi;

MPCDI_DEFINE_ENUM_CONV_FUNC(GeometricUnit,mpcdi,GEOMETRIC_UNIT_ENUMS);
MPCDI_DEFINE_ENUM_CONV_FUNC(OriginOf3DData,mpcdi,ORGIN_OF_3D_ENUMS);
MPCDI_DEFINE_ENUM_CONV_FUNC(Interpolation,mpcdi,INTERPOLATION_ENUMS);

GeometryWarpFile::GeometryWarpFile(unsigned int sizeX, unsigned int sizeY)
  : PFM(sizeX,sizeY),
  m_GeometricUnit(GeometricUnitunknown),
  m_Interpolation(Interpolationunknown),
  m_OriginOf3DData(OriginOf3DDataunknown)
{

}