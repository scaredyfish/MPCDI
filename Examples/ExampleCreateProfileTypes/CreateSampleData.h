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

#include "mpcdiReader.h"
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

mpcdi::MPCDI_Error DrawRectangle(mpcdi::DataMap *data,
                                      const unsigned char &intensity,
                                      const int &cx,
                                      const int &cy,
                                      const int &cc,
                                      const int &Sidex, 
                                      const int &Sidey)
{
  if (cc>=data->GetComponentDepth())   return mpcdi::MPCDI_FAILURE;

  for( int j=0;j<(int)data->GetSizeY();j++)
    for( int i=0;i<(int)data->GetSizeX();i++)
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
  for( int j=0;j<(int)data->GetSizeY();j++)
    for( int i=0;i<(int)data->GetSizeX();i++)
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
