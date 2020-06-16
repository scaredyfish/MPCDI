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

#include "mpcdiProfile.h"
#include "mpcdiErrorHelper.h"

using namespace mpcdi;

MPCDI_DEFINE_ENUM_CONV_FUNC(ProfileType,mpcdi,PROFILE_TYPE_ENUMS);

/* ====================================================================== */

Profile::Profile():
  m_Level(1), m_ProfileType(ProfileType2d), m_Display(new Display())
{
  m_Version.MajorVersion = 1;
  m_Version.MinorVersion = 0;
}

/* ====================================================================== */

Profile::~Profile()
{
 mpcdiSafeDeleteMacro(m_Display);
}

/* ====================================================================== */

MPCDI_Error Profile::ValidateProfile()
{
  std::vector<std::string> bufferNames=m_Display->GetBufferNames();
  if (bufferNames.size() == 0)
  {
    CREATE_ERROR_MSG(msg,"A profile should contain at least one buffer found none");
    ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
  }

  /* a3d or sl, number of buffers must be 1 */
  if  (m_ProfileType == ProfileType3d || m_ProfileType == ProfileTypesl)
    if (bufferNames.size() != 1)
    {
      CREATE_ERROR_MSG(msg,"When profile " << mpcdi::GetProfileType(m_ProfileType) << "there should only be one buffer not " << bufferNames.size() << std::endl);
      ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
    }

  bool firstRegion=true;
  OriginOf3DData globalOriginOf3DData = OriginOf3DDataunkown;
  for(mpcdi::Display::BufferIterator bufferIt=this->GetDisplay()->GetBufferBegin(); bufferIt != this->GetDisplay()->GetBufferEnd(); bufferIt++) 
  {
    mpcdi::Buffer *buffer = bufferIt->second;
    for(mpcdi::Buffer::RegionIterator regionIt=buffer->GetRegionBegin(); regionIt != buffer->GetRegionEnd(); regionIt++) 
    { 
      mpcdi::Region *region=regionIt->second;
      GeometryWarpFile *geometryWarpFile = region->GetFileSet()->GetGeometryWarpFile();

      /* x/y resolution should be set */
      if (region->GetXresolution() < 0 || region->GetYresolution() < 0 )
      {
        CREATE_ERROR_MSG(msg,"Region should have x and y resolution tags set");
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      /* x/y should be set */
      if (MPCDI_FAILED(mpcdi::Region::CheckRangeX(region->GetX())) || 
            MPCDI_FAILED(mpcdi::Region::CheckRangeY(region->GetY())))
      {
        CREATE_ERROR_MSG(msg, "Region should have x and y tags set or they are out of range");
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }
      
      /* x/y size should be set */
      if (MPCDI_FAILED(mpcdi::Region::CheckRangeXsize(region->GetXsize())) || 
            MPCDI_FAILED(mpcdi::Region::CheckRangeYsize(region->GetYsize())))
      {
        CREATE_ERROR_MSG(msg, "Region should have xsize and ysize tags set or they are out of range");
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      /* If the profile is 3D Simulation (3d) or Shader Lamps (sl), the region tag must contain a frustum tag. */
      if (m_ProfileType == ProfileType3d || m_ProfileType == ProfileTypesl)
        if (region->GetFrustum() == NULL)
        {
          CREATE_ERROR_MSG(msg, "Profile of type" << mpcdi::GetProfileType(m_ProfileType) << "should have frustum set");
          ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
        }

      /* If the profile is set to Shader Lamps (sl), the region tag must contain a coordinateFrame tag. 
      A coordinateFrame tag shall contain nine consecutive tags. They are as follows and is described 
      in further detail in section 2.3.1.2 */
      if (m_ProfileType == ProfileTypesl)
        if (region->GetCoordinateFrame() == NULL)
        {
          CREATE_ERROR_MSG(msg, "Profile of type" << mpcdi::GetProfileType(m_ProfileType) 
                                << "should have a coordinate frame set");
          ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
        }

      /* always contains geometry warp file */
      if (geometryWarpFile==NULL)                     
      {
        CREATE_ERROR_MSG(msg,"Missing geometry warp file for region " << regionIt->first);
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      if(firstRegion)
      {
        globalOriginOf3DData = geometryWarpFile->GetOriginOf3DData();
        firstRegion = false;
      }

      /* origin of 3d data is different on any geometry warp file */
      if (globalOriginOf3DData != geometryWarpFile->GetOriginOf3DData())
      {
        CREATE_ERROR_MSG(msg,"All geometry warp files should have the same 3d origin found " << globalOriginOf3DData << " and " << geometryWarpFile->GetOriginOf3DData());
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      /* A betaMap tag must exist if profiles level is set to 2 or 4. */
      if (m_Level == 2 || m_Level == 3 && region->GetFileSet()->GetBetaMap() == NULL)
      {
        CREATE_ERROR_MSG(msg,"With level " << m_Level << " a betamap must be present");
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      /* always contains alpha map */
      if (region->GetFileSet()->GetAlphaMap()==NULL)                     
      {
        CREATE_ERROR_MSG(msg,"Missing alpha mape for region " << regionIt->first);
        ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
      }

      /* If the profile is a3 or sl. Then a geometricUnit tag and an 
          originOf3DData tag must both be present inside the geometryWarpFile tag. */
      //FIXME: doubt if we need to do something with this here
      
      /* If shader lamp then we should have a distortionMap */
      if (m_ProfileType == ProfileTypesl)
      {
        /* A distortionMap tag, must exist within the fileset tag if profile is set to Shader Lamps (sl).*/
        if (region->GetFileSet()->GetDistortionMap() == NULL)
        {
          CREATE_ERROR_MSG(msg,"ProfileType is " << mpcdi::GetProfileType(m_ProfileType) << "expected distortion map");
          ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
        }
        
        /* If the profile is set to Shader Lamps (sl), then this tag cannot be set to idealEyePoint */
        if (geometryWarpFile->GetOriginOf3DData() ==  OriginOf3DDataidealEyePoint)
        {
          CREATE_ERROR_MSG(msg,"ProfileType is " << mpcdi::GetProfileType(m_ProfileType) << "origin of data should not be " << mpcdi::GetOriginOf3DData(OriginOf3DDataidealEyePoint));
          ReturnCustomErrorMacro(MPCDI_FAILURE,msg);
        }
      }
      
    }
  }

  return MPCDI_SUCCESS;
}
