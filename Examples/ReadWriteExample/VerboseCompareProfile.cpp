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

#include "VerboseCompareProfile.h"
#include "mpcdiUtils.h"
#include "mpcdiDisplay.h"
#include "mpcdiWriter.h"
#include "mpcdiReader.h"
#include <iostream>
#include <algorithm>

/* ====================================================================== */

template <class T>
bool CheckValues(const T &a, const T &b) { return a == b; }

template <>
bool CheckValues<float>(const float &a, const float &b) 
{ if ((a==0) || (b==0)) return a==b;  return ((a-b)/a < 0.001f ); }

template <>
bool CheckValues<double>(const double &a, const double &b) 
{ if ((a==0) || (b==0)) return a==b;  return ((a-b)/a < 0.001 ); }

// return a success/failure string. Also set the error to failure if 
// the comparison failed.
#define CHECK_VALUES(_pIn, _pOut, _func)\
  (CheckValues(_pIn->_func(),_pOut->_func()) ? "\tSUCCESS" : (err = mpcdi::MPCDI_FAILURE,"\tFAILED"))


/* ====================================================================== */

mpcdi::MPCDI_Error
VerboseCompareProfile(std::ostream &os,
                      mpcdi::Profile *profileIn,
                      mpcdi::Profile *profileOut)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  os << "Comparing Two Profiles:" << std::endl;
  os << "General info:" << std::endl;
  os << "  Date: " << profileOut->GetDate() << CHECK_VALUES(profileIn, profileOut, GetDate) << std::endl;
 err = mpcdi::MPCDI_SUCCESS; // ignore date changes from time stamp.
  os << "  Level: " << profileOut->GetLevel() << CHECK_VALUES(profileIn, profileOut, GetLevel) << std::endl;
  os << "  Version: " << mpcdi::Utils::ProfileVersionToString(profileOut->GetVersion()) << CHECK_VALUES(profileIn, profileOut, GetVersion) << std::endl;
  os << "  ProfileType: " << profileOut->GetProfileType() << CHECK_VALUES(profileIn, profileOut, GetProfileType) << std::endl;

  os << "Buffer Info:" << std::endl; 
  for(mpcdi::Display::BufferIterator bufferIt=
        profileOut->GetDisplay()->GetBufferBegin(); 
      bufferIt != profileOut->GetDisplay()->GetBufferEnd();  bufferIt++) 
  {
    mpcdi::Buffer *buffer = bufferIt->second;
    os << " id: " << buffer->GetId() << std::endl;

    mpcdi::Buffer *bufferIn=
      profileIn->GetDisplay()->GetBuffer(buffer->GetId());

    if (bufferIn == NULL)
    {
      os << "  COMPARE FAILED. Buffer not found in generated profile!" 
                << std::endl;
      continue;
    }

    os << "  Region info:" << std::endl;
    for(mpcdi::Buffer::RegionIterator regionIt=buffer->GetRegionBegin();
        regionIt != buffer->GetRegionEnd(); regionIt++) 
      { 
        mpcdi::Region *region=regionIt->second;
        os << "   id: " << region->GetId() << std::endl;
        mpcdi::Region *regionIn = bufferIn->GetRegion(region->GetId());
        if (regionIn != NULL)
          {
            os << "   Xresolution: " << region->GetXresolution() << CHECK_VALUES(region,regionIn,GetXresolution) << std::endl;
            os << "   Yresolution: " << region->GetYresolution() << CHECK_VALUES(region,regionIn,GetYresolution) << std::endl;
            os << "   Size X: " << region->GetXsize() << CHECK_VALUES(region,regionIn,GetXsize) << std::endl;
            os << "   Size Y: " << region->GetYsize() << CHECK_VALUES(region,regionIn,GetYsize) << std::endl;
            os << "   Origin X: " << region->GetX() << CHECK_VALUES(region,regionIn,GetX) << std::endl;
            os << "   Origin Y: " << region->GetY() << CHECK_VALUES(region,regionIn,GetY) << std::endl;
            
            if (MPCDI_FAILED(VerboseCompareFrustum(os,region->GetFrustum(),
                                                   regionIn->GetFrustum())))
              {
                err = mpcdi::MPCDI_FAILURE;
              }
            
            if (MPCDI_FAILED(VerboseCompareAlphaMap(os,
                                region->GetFileSet()->GetAlphaMap(),
                                regionIn->GetFileSet()->GetAlphaMap())))
              {
                err = mpcdi::MPCDI_FAILURE;
              }
            
            if (MPCDI_FAILED(VerboseCompareBetaMap(os,
                                region->GetFileSet()->GetBetaMap(),
                                regionIn->GetFileSet()->GetBetaMap())))
              {
                err = mpcdi::MPCDI_FAILURE;
              }
            
            if (MPCDI_FAILED(VerboseCompareDistortionMap(os,
                                region->GetFileSet()->GetDistortionMap(),
                                regionIn->GetFileSet()->GetDistortionMap())))
              
              {
                err = mpcdi::MPCDI_FAILURE;
              }
            
            if (MPCDI_FAILED(VerboseCompareGeometryWarpFile(os,
                               region->GetFileSet()->GetGeometryWarpFile(),
                               regionIn->GetFileSet()->GetGeometryWarpFile())))
              
              {
                err = mpcdi::MPCDI_FAILURE;
              }
          }
        else // regionIn is Null
          {
            os << "   FAILED Region not found in profile read in."<<std::endl;
          }
      }
  }

  return err;
}

/* ====================================================================== */

mpcdi::MPCDI_Error 
VerboseCompareFrustum(std::ostream &os,
                      mpcdi::Frustum *frustumIn,
                      mpcdi::Frustum *frustum)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  // If one or both is NULL...
  if ((frustum == NULL) &&(frustumIn == NULL)) return mpcdi::MPCDI_SUCCESS;
  if (((frustum == NULL) && (frustumIn != NULL))||
      ((frustum != NULL) && (frustumIn == NULL)))
    {
      os << "   COMPARE FAILED: Only one frustum is non-NULL" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  os << "   Frustum:" << std::endl;
  os << "     Yaw: "   << frustum->GetYaw() << CHECK_VALUES(frustum,frustumIn,GetYaw) << std::endl;
  os << "     Pitch: " << frustum->GetPitch() << CHECK_VALUES(frustum,frustumIn,GetPitch) << std::endl;
  os << "     Roll: "  << frustum->GetRoll() << CHECK_VALUES(frustum,frustumIn,GetRoll) << std::endl;
  os << "     LeftAngle: " << frustum->GetLeftAngle() << CHECK_VALUES(frustum,frustumIn,GetLeftAngle) << std::endl;
  os << "     RightAngle: " << frustum->GetRightAngle() << CHECK_VALUES(frustum,frustumIn,GetRightAngle) << std::endl;
  os << "     UpAngle: " << frustum->GetUpAngle() << CHECK_VALUES(frustum,frustumIn,GetUpAngle) << std::endl;
  os << "     DownAngle: " << frustum->GetDownAngle() << CHECK_VALUES(frustum,frustumIn,GetDownAngle) << std::endl;
  if ( *frustum == *frustumIn)
    os << "     in and out Frustum are the same" << std::endl;
  
  return err;
}

/* ====================================================================== */

mpcdi::MPCDI_Error 
VerboseCompareBetaMap(std::ostream &os,
                      mpcdi::BetaMap *betaMapIn,
                      mpcdi::BetaMap *betaMap)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  // If one or both is NULL...
  if ((betaMap == NULL) &&(betaMapIn == NULL))
    return mpcdi::MPCDI_SUCCESS;

  if (((betaMap == NULL) && (betaMapIn != NULL))||
      ((betaMap != NULL) && (betaMapIn == NULL)))
    {
      os << "   COMPARE FAILED: One Beta Map is non-NULL" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  os << "   BetaMap:" << std::endl;
  os << "     BitDepth: " << betaMap->GetBitDepth() << CHECK_VALUES(betaMap,betaMapIn,GetBitDepth) << std::endl;
  os << "     ComponentDepth: " << betaMap->GetComponentDepth() << CHECK_VALUES(betaMap,betaMapIn,GetComponentDepth) << std::endl;
  os << "     Size X: " << betaMap->GetSizeX() << CHECK_VALUES(betaMap,betaMapIn,GetSizeX) << std::endl;
  os << "     Size Y: " << betaMap->GetSizeY() << CHECK_VALUES(betaMap,betaMapIn,GetSizeY) << std::endl;
  if ( *betaMap->GetData() == *betaMapIn->GetData())
    os << "     in and out BetaMap are the same" << std::endl;
  else
    {
      os << "     FAILED: in and out BetaMap are different" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  return err;
}

/* ====================================================================== */

mpcdi::MPCDI_Error 
VerboseCompareAlphaMap(std::ostream &os,
                      mpcdi::AlphaMap *alphaMapIn,
                      mpcdi::AlphaMap *alphaMap)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  // If one or both is NULL...
  if ((alphaMap == NULL) &&(alphaMapIn == NULL))
    return mpcdi::MPCDI_SUCCESS;

  if (((alphaMap == NULL) && (alphaMapIn != NULL))||
      ((alphaMap != NULL) && (alphaMapIn == NULL)))
    {
      os << "   COMPARE FAILED: One Alpha Map is non-NULL" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  os << "   AlphaMap:" << std::endl;
  os << "     BitDepth: " << alphaMap->GetBitDepth() << CHECK_VALUES(alphaMap,alphaMapIn,GetBitDepth) << std::endl;
  os << "     ComponentDepth: " << alphaMap->GetComponentDepth() << CHECK_VALUES(alphaMap,alphaMapIn,GetComponentDepth) << std::endl;
  os << "     Size X: " << alphaMap->GetSizeX() << CHECK_VALUES(alphaMap,alphaMapIn,GetSizeX) << std::endl;
  os << "     Size Y: " << alphaMap->GetSizeY() << CHECK_VALUES(alphaMap,alphaMapIn,GetSizeY) << std::endl;
  if ( *alphaMap->GetData() == *alphaMapIn->GetData())
    os << "     in and out AlphaMap are the same" << std::endl;
  else
    {
      os << "     FAILED: in and out AlphaMap are different" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  return err;
}


/* ====================================================================== */

mpcdi::MPCDI_Error 
VerboseCompareDistortionMap(std::ostream &os,
                            mpcdi::DistortionMap *distortionMapIn,
                            mpcdi::DistortionMap *distortionMap)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  // If one or both is NULL...
  if ((distortionMap == NULL) &&(distortionMapIn == NULL))
    return mpcdi::MPCDI_SUCCESS;

  if (((distortionMap == NULL) && (distortionMapIn != NULL))||
      ((distortionMap != NULL) && (distortionMapIn == NULL)))
    {
      os << "   COMPARE FAILED: One distortion Map is non-NULL" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  os << "   DistortionMap:" << std::endl;
  os << "     Size X: " << distortionMap->GetSizeX() 
            << CHECK_VALUES(distortionMap,distortionMapIn,GetSizeX)
            << std::endl;
  os << "     Size Y: " << distortionMap->GetSizeY() 
            << CHECK_VALUES(distortionMap,distortionMapIn,GetSizeY) 
            << std::endl;
  if(distortionMap->GetSizeX() != distortionMapIn->GetSizeX())
    return mpcdi::MPCDI_FAILURE;
  if(distortionMap->GetSizeY() != distortionMapIn->GetSizeY())
    return mpcdi::MPCDI_FAILURE;

  for(unsigned int j=0; j<distortionMap->GetSizeX(); j++)
    for(unsigned int i=0; i<distortionMap->GetSizeY();i++)
      { 
        if ((*distortionMap)(j,i)!=(*distortionMapIn)(j,i))
          {
            os << "     FAILED DistortionMap first differs at (x,y) (" 
               << j << "," << i << ")" << std::endl;
            return mpcdi::MPCDI_FAILURE;
          }
      }

  os << "     in and out DistortionMap are the same" << std::endl;

  return err;
}

/* ====================================================================== */


mpcdi::MPCDI_Error 
VerboseCompareGeometryWarpFile(std::ostream &os,
                               mpcdi::GeometryWarpFile *geometryWarpFileIn,
                               mpcdi::GeometryWarpFile *geometryWarpFile)
{
  mpcdi::MPCDI_Error err = mpcdi::MPCDI_SUCCESS;

  // If one or both is NULL...
  if ((geometryWarpFile == NULL) &&(geometryWarpFileIn == NULL))
    return mpcdi::MPCDI_SUCCESS;

  if (((geometryWarpFile == NULL) && (geometryWarpFileIn != NULL))||
      ((geometryWarpFile != NULL) && (geometryWarpFileIn == NULL)))
    {
      os << "   COMPARE FAILED: One geometry warp is non-NULL" << std::endl;
      return mpcdi::MPCDI_FAILURE;
    }

  os << "   GeometryWarpFile:" << std::endl;
  os << "     Size X: " << geometryWarpFile->GetSizeX() 
     << CHECK_VALUES(geometryWarpFile,geometryWarpFileIn,GetSizeX) 
     << std::endl;
  os << "     Size Y: " << geometryWarpFile->GetSizeY() 
     << CHECK_VALUES(geometryWarpFile,geometryWarpFileIn,GetSizeY) 
     << std::endl;

  if(geometryWarpFile->GetSizeX() != geometryWarpFileIn->GetSizeX())
    return mpcdi::MPCDI_FAILURE;
  if(geometryWarpFile->GetSizeY() != geometryWarpFileIn->GetSizeY())
    return mpcdi::MPCDI_FAILURE;

  for(unsigned int j=0; j<geometryWarpFile->GetSizeX(); j++)
    for(unsigned int i=0; i<geometryWarpFile->GetSizeY();i++)
      { 
        if ((*geometryWarpFile)(j,i)!=(*geometryWarpFileIn)(j,i))
          {
            os << "     FAILED GeometryWarpFile first differs at (x,y) (" 
               << j << "," << i << ")" << std::endl;
            return mpcdi::MPCDI_FAILURE;
          }
      }
  os << "     in and out GeometryWarpFile are the same" << std::endl;

  return err;
}

/* ====================================================================== */
