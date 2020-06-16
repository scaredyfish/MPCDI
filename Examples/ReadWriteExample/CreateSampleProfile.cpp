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

#include "CreateSampleProfile.h"
#include "mpcdiDisplay.h"
#include "mpcdiWriter.h"
#include "mpcdiReader.h"
#include "mpcdiUtils.h"
#include "mpcdiCreateShaderLampProfile.h"
#include <iostream>
#include <algorithm>

/* ====================================================================== */

mpcdi::MPCDI_Error DrawRectangle(mpcdi::DataMap *data,
                                 const unsigned char &intensity,
                                 const int &cx,
                                 const int &cy,
                                 const int &cc,
                                 const int &Sidex, 
                                 const int &Sidey);
mpcdi::MPCDI_Error DrawGradientInPFM(mpcdi::PFM *data,  
                                     const int &lx,
                                     const int &ly,
                                     const int &Sidex, 
                                     const int &Sidey);
mpcdi::MPCDI_Error DrawGradientInPFM(mpcdi::PFM *data,  
                                     const int &lx,
                                     const int &ly,
                                     const int &Sidex, 
                                     const int &Sidey);
mpcdi::MPCDI_Error DrawRectangleInPFM(mpcdi::PFM *data,
                                      const float &intensity,
                                      const int &cx,
                                      const int &cy,
                                      const int &Sidex, 
                                      const int &Sidey);

/* ====================================================================== */

mpcdi::MPCDI_Error CreateSampleProfile(std::ostream &os,
                                       mpcdi::Profile *&profile)

{
  std::string newBufferId = "sample_buffer";
  unsigned int X_RESOLUTION=200;
  unsigned int Y_RESOLUTION=200;
  mpcdi::ComponentDepth COMPONENT_DEPTH=mpcdi::CD_THREE;

  mpcdi::CreateShaderLampProfile ProfileCreator;

  ProfileCreator.SetLevel(1);
  MPCDI_FAIL_RET(ProfileCreator.CreateNewBuffer(newBufferId));

  mpcdi::Buffer *buffer = ProfileCreator.GetBuffer(newBufferId);
  buffer->SetXresolution(X_RESOLUTION);
  buffer->SetYresolution(Y_RESOLUTION);

  for(int i=0;i<3;i++)
  {
    // Regions are roughly stacked on top of each other. A little noise
    // to test the read/write test.
    std::string newRegionId = std::string("Proj")+mpcdi::NumberToString(i);
    MPCDI_FAIL_RET(ProfileCreator.CreateNewRegion(newBufferId,newRegionId));
    mpcdi::Region *region = ProfileCreator.GetRegion(buffer,newRegionId);
    region->SetResolution(X_RESOLUTION,Y_RESOLUTION);
    region->SetSize(0.9f-0.01f*i,0.92f-0.01f*i); 
    region->SetX(0.01f+0.01f*i);
    region->SetY(0.07f+0.01f*i);
    
    MPCDI_FAIL_RET(ProfileCreator.CreateAlphaMap(region,
                                                 X_RESOLUTION,
                                                 Y_RESOLUTION,
                                                 COMPONENT_DEPTH));
    MPCDI_FAIL_RET(ProfileCreator.CreateBetaMap(region,
                                                100,100,mpcdi::CD_ONE));
    MPCDI_FAIL_RET(ProfileCreator.CreateGeometryWarpFile(region,
                                                         X_RESOLUTION,
                                                         Y_RESOLUTION));
    MPCDI_FAIL_RET(ProfileCreator.CreateDistortionMap(region,
                                                      X_RESOLUTION,
                                                      Y_RESOLUTION));
    MPCDI_FAIL_RET(ProfileCreator.CreateFrustum(region));
    MPCDI_FAIL_RET(ProfileCreator.CreateCoordinateFrame(region));
    
    mpcdi::Frustum *f = ProfileCreator.GetFrustum(region);
    f->SetPitch(-10+i);          f->SetYaw(5+i);  f->SetRoll(7+i);
    f->SetDownAngle(-11-i);      f->SetUpAngle(10+i);
    f->SetLeftAngle(-20-i);      f->SetRightAngle(12+i);
    
    mpcdi::CoordinateFrame *cf = ProfileCreator.GetCoordinateFrame(region);
    cf->SetPos(0,0,1);
    cf->  SetYaw(0,0,1);
    cf->SetPitch(0,1,0);
    cf-> SetRoll(1,0,0);
    
    // Fill in the data.
    mpcdi::AlphaMap *alphaMap = ProfileCreator.GetAlphaMap(region);
    for (unsigned int c=0;c<(unsigned int)COMPONENT_DEPTH;c++)
      {
        DrawRectangle(alphaMap,255,X_RESOLUTION/2,Y_RESOLUTION/2,c,
                      X_RESOLUTION/4+X_RESOLUTION/(1+COMPONENT_DEPTH)*c,
                      Y_RESOLUTION/4+Y_RESOLUTION/(1+COMPONENT_DEPTH)*c); 
      }
    
    // Fill in the data.
    mpcdi::BetaMap *betaMap = ProfileCreator.GetBetaMap(region);
    
    // Not doing anything with the betamap or DistortionMap.
    
    mpcdi::GeometryWarpFile *geometryWarpFile =
      ProfileCreator.GetGeometryWarpFile(region);
    DrawGradientInPFM(geometryWarpFile,0,0,X_RESOLUTION,Y_RESOLUTION);
    DrawRectangleInPFM(geometryWarpFile,0,0,0,X_RESOLUTION/5,Y_RESOLUTION/5); 
  }

  // Validate.
  MPCDI_FAIL_RET(ProfileCreator.ValidateProfile());

  // Get the results, make sure it does not get deleted.
  ProfileCreator.SetDeleteProfile(false);
  profile = ProfileCreator.GetProfile();

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */

mpcdi::MPCDI_Error CreateSampleProfile2(std::ostream &os,
                                       mpcdi::Profile *&profile)
{
  mpcdi::MPCDI_Error mpcdi_err = mpcdi::MPCDI_SUCCESS;

  /* profile settings */
  unsigned int X_RESOLUTION=200;
  unsigned int Y_RESOLUTION=200;
  mpcdi::ComponentDepth COMPONENT_DEPTH=mpcdi::CD_THREE;

  bool DoSetFrustum = true;
  bool DoSetBetaMap = true;
  bool DoSetDistortionmap = true;

  /* create profile */
  profile = new mpcdi::Profile();
  profile->SetProfileType(mpcdi::ProfileType2d);
  profile->SetLevel(1);

  mpcdi::Display *display= profile->GetDisplay();

  std::string newBufferId = "sample_buffer";
  if (MPCDI_FAILED(mpcdi_err = display->NewBuffer(newBufferId)))
  {
    os << "Error creating new buffer: " 
       <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }

  mpcdi::Buffer *buffer = display->GetBuffer(newBufferId);
  buffer->SetXresolution(X_RESOLUTION);
  buffer->SetYresolution(Y_RESOLUTION);

  std::string newRegionId = "sample_region";
  if (MPCDI_FAILED(mpcdi_err = buffer->NewRegion(newRegionId)))
  {
    os << "Error creating new region: " 
              <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }

  mpcdi::Region *region = buffer->GetRegion(newRegionId);
  region->SetResolution(X_RESOLUTION,Y_RESOLUTION);
  region->SetSize(0.25,0.25); /* needs to be set but not relevant for this sample */
  region->SetX(0.5);
  region->SetY(0.5);
  /*set frustum if needed*/
  if (DoSetFrustum)
  {
    region->SetFrustum(); 
    region->GetFrustum()->SetDownAngle(-10);
    region->GetFrustum()->SetUpAngle(10);
    region->GetFrustum()->SetPitch(-10);
  }

  /* set alpha map */
  if (MPCDI_FAILED(region->GetFileSet()->SetAlphaMap(X_RESOLUTION,Y_RESOLUTION,COMPONENT_DEPTH)))
  {
    os << "Error creating new alphamap: " 
       <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }
  mpcdi::AlphaMap *alphaMap = region->GetFileSet()->GetAlphaMap();
  /*for testing purposes, draw something */
  for (unsigned int c=0;c<(unsigned int)COMPONENT_DEPTH;c++)
  {
    DrawRectangle(alphaMap,255,X_RESOLUTION/2,Y_RESOLUTION/2,c,
                  X_RESOLUTION/4+X_RESOLUTION/(1+COMPONENT_DEPTH)*c,
                  Y_RESOLUTION/4+Y_RESOLUTION/(1+COMPONENT_DEPTH)*c); 
  }

  /* set beta map if desired */
  if (DoSetBetaMap && MPCDI_FAILED(region->GetFileSet()->SetBetaMap(100,100,mpcdi::CD_ONE)))
  {
    os << "Error creating new betamap: " 
       <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }

  /* set distortionmap if desired */
  if (DoSetDistortionmap && MPCDI_FAILED(region->GetFileSet()->SetDistortionMap(X_RESOLUTION,Y_RESOLUTION)))
  {
    os << "Error creating new distortionmap: " 
       <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }

  /* set geometry warp file */
  if (MPCDI_FAILED(region->GetFileSet()->SetGeometryWarpFile(X_RESOLUTION,Y_RESOLUTION)))
  {
    os << "Error creating new geometry warp file: " 
       <<  mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }
  mpcdi::GeometryWarpFile *geometryWarpFile = region->GetFileSet()->GetGeometryWarpFile();
  DrawGradientInPFM(geometryWarpFile,0,0,X_RESOLUTION,Y_RESOLUTION);
  DrawRectangleInPFM(geometryWarpFile,0,0,0,X_RESOLUTION/5,Y_RESOLUTION/5); 
  /*just for testing draw a black block in the origin*/

  /* validate the profile, checks if its build up conform the standard */
  if MPCDI_FAILED(mpcdi_err = profile->ValidateProfile())
  {
    os <<  "Error encountered while validating profile" 
       << mpcdi_err << mpcdi::ErrorHelper::GetLastError() << std::endl;
    return mpcdi::MPCDI_FAILURE;
  }

  return mpcdi_err;
}

/* ====================================================================== */

mpcdi::MPCDI_Error DrawRectangle(mpcdi::DataMap *data,
                                      const unsigned char &intensity,
                                      const int &cx,
                                      const int &cy,
                                      const int &cc,
                                      const int &Sidex, 
                                      const int &Sidey)
{
  if (cc>=data->GetComponentDepth())   return mpcdi::MPCDI_FAILURE;

  for(int j=0;j<(int)data->GetSizeY();j++)
    for(int i=0;i<(int)data->GetSizeX();i++)
      {
        if ((abs(i-cx)<=Sidex/2)&& (abs(j-cy)<=Sidey/2))
          (*data)(i,j,cc) = intensity;
      }

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */

mpcdi::MPCDI_Error DrawGradientInPFM(mpcdi::PFM *data,  
                                      const int &lx,
                                      const int &ly,
                                      const int &Sidex, 
                                      const int &Sidey)
{
  if (lx >= (int) data->GetSizeX() || ly >= (int) data->GetSizeY() || 
      lx<0 || ly<0)
    return mpcdi::MPCDI_FAILURE;

  int xBound = std::min((unsigned int)(lx+Sidex),data->GetSizeX());
  int yBound = std::min((unsigned int)(ly+Sidey),data->GetSizeY());

  for(int j=ly;j<yBound;j++)
  {
    for(int i=lx;i<xBound;i++)
    {
      (*data)(i,j).r = (i-lx)/((float)xBound);
      (*data)(i,j).g = (j-ly)/((float)yBound);
    }
  }

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */

mpcdi::MPCDI_Error DrawRectangleInPFM(mpcdi::PFM *data,
                                      const float &intensity,
                                      const int &cx,
                                      const int &cy,
                                      const int &Sidex, 
                                      const int &Sidey)
{
  for(int j=0;j<(int)data->GetSizeY();j++)
    for(int i=0;i<(int)data->GetSizeX();i++)
    {
      if ((abs(i-cx)<=Sidex/2)&& (abs(j-cy)<=Sidey/2))
      {
        (*data)(i,j).r =intensity;
        (*data)(i,j).g =intensity;
        (*data)(i,j).b =intensity;
      }
    }

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */
/* ====================================================================== */
