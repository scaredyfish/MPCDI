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
// .NAME CreateAll - Create one of each of the 4 MPCDI types
// .SECTION Description
//
// .AUTHOR Scalable Display Technologies, Inc.
//


#include "mpcdiWriter.h"
#include "mpcdiCreateProfile.h"
#include "CreateSampleData.h"
#include <iostream>

#define DO_PAUSE
#ifdef DO_PAUSE
#  define PAUSE {std::cout<< "Press enter to continue....";std::cin.ignore();}
#else 
# definE PAUSE
#endif

/* ====================================================================== */

mpcdi::MPCDI_Error CreateExample2DMedia(mpcdi::Profile *&profile);
mpcdi::MPCDI_Error CreateExample3DSimulation(mpcdi::Profile *&profile);
mpcdi::MPCDI_Error CreateExampleAdvanced3D(mpcdi::Profile *&profile);
mpcdi::MPCDI_Error CreateExampleShaderLamp(mpcdi::Profile *&profile);

mpcdi::MPCDI_Error Write(mpcdi::Profile *Profile,const std::string &FileName);

/* ====================================================================== */

int main( int argc, const char ** argv )
{
  mpcdi::Profile *Profile = NULL;
  if (MPCDI_FAILED(CreateExample2DMedia(Profile)))
    {
      std::cout << "Failed to Create 2D Media Profile" << std::endl;
      PAUSE;
      return mpcdi::MPCDI_FAILURE;
    }
  std::cout << "Created 2D Media Profile" << std::endl;
  MPCDI_FAIL_RET(Write(Profile,"Media2D.mpcdi"));
  if (Profile != NULL) { delete Profile; Profile = NULL; }

  if (MPCDI_FAILED(CreateExample3DSimulation(Profile)))
    {
      std::cout << "Failed to Create 3D Simulation Profile" << std::endl;
      PAUSE;
      return mpcdi::MPCDI_FAILURE;
    }
  std::cout << "Created 3D Simulation Profile" << std::endl;
  MPCDI_FAIL_RET(Write(Profile,"Sim3D.mpcdi"));
  if (Profile != NULL) { delete Profile; Profile = NULL; }


  if (MPCDI_FAILED(CreateExampleShaderLamp(Profile)))
    {
      std::cout << "Failed to Create Shader Lamp Profile" << std::endl;
      PAUSE;
      return mpcdi::MPCDI_FAILURE;
    }
  std::cout << "Created Shader Lamp Profile" << std::endl;
  MPCDI_FAIL_RET(Write(Profile,"ShaderLamp.mpcdi"));
  if (Profile != NULL) { delete Profile; Profile = NULL; }

  if (MPCDI_FAILED(CreateExampleAdvanced3D(Profile)))
    {
      std::cout << "Failed to Create Advancde 3D Profile" << std::endl;
      PAUSE;
      return mpcdi::MPCDI_FAILURE;
    }
  std::cout << "Created Advanced 3D Profile" << std::endl;
  MPCDI_FAIL_RET(Write(Profile,"Advanced3D.mpcdi"));
  if (Profile != NULL) { delete Profile; Profile = NULL; }

  std::cout << "4 Files Written to Disk." << std::endl;
  PAUSE;
  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */

mpcdi::MPCDI_Error Write(mpcdi::Profile *Profile, const std::string &FileName)
{
  mpcdi::Writer *writer = mpcdi::Writer::CreateWriter();
  mpcdi::MPCDI_Error mpcdi_err = writer->Write(FileName, *Profile);
  delete writer;
  return mpcdi_err;
}

/* ====================================================================== */

// This is for all of the different profiles.
mpcdi::MPCDI_Error FillInCreatorCommonData(
                                   mpcdi::CreateProfile &ProfileCreator,
                                   const unsigned int &xRes,
                                   const unsigned int &yRes)
{
  std::string newBufferId = "buffer1";
  std::string newRegionId = "proj1";
  mpcdi::ComponentDepth COMPONENT_DEPTH=mpcdi::CD_THREE;

  // This next line is optional, 1 is the lowest, so it does not
  // do anything really.
  ProfileCreator.SetLevel(1);
  MPCDI_FAIL_RET(ProfileCreator.CreateNewBuffer(newBufferId));
  MPCDI_FAIL_RET(ProfileCreator.CreateNewRegion(newBufferId,newRegionId));

  mpcdi::Buffer *buffer = ProfileCreator.GetBuffer(newBufferId);
  buffer->SetXresolution(xRes);
  buffer->SetYresolution(yRes);

  mpcdi::Region *region = ProfileCreator.GetRegion(buffer,newRegionId);
  region->SetResolution(xRes,yRes);
  region->SetSize(0.9f,0.92f); /* needs to be set but not relevant for this sample */
  region->SetX(0.01f);
  region->SetY(0.07f);

  MPCDI_FAIL_RET(ProfileCreator.CreateAlphaMap(region,
                                               xRes,
                                               yRes,
                                               COMPONENT_DEPTH));
  ProfileCreator.SetGammaEmbedded(region,2.2);
  MPCDI_FAIL_RET(ProfileCreator.CreateBetaMap(region,
                                               100,100,mpcdi::CD_ONE));
  MPCDI_FAIL_RET(ProfileCreator.CreateGeometryWarpFile(region,
                                                       xRes,yRes));

  // Fill in the data.
  mpcdi::AlphaMap *alphaMap = ProfileCreator.GetAlphaMap(region);
  for (unsigned int c=0;c<(unsigned int)COMPONENT_DEPTH;c++)
  {
    DrawRectangle(alphaMap,255,xRes/2,yRes/2,c,
                  xRes/4+xRes/(1+COMPONENT_DEPTH)*c,
                  yRes/4+yRes/(1+COMPONENT_DEPTH)*c); 
  }

  // Fill in the data.
  mpcdi::BetaMap *betaMap = ProfileCreator.GetBetaMap(region);

  mpcdi::GeometryWarpFile *geometryWarpFile =
    ProfileCreator.GetGeometryWarpFile(region);
  DrawGradientInPFM(geometryWarpFile,0,0,xRes,yRes);
  DrawRectangleInPFM(geometryWarpFile,0,0,0,xRes/5,yRes/5); 

  return mpcdi::MPCDI_SUCCESS;
}

/* ====================================================================== */
