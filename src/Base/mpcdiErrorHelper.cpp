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

#include "mpcdiErrorHelper.h"

using namespace mpcdi;

std::string ErrorHelper::m_LastErrorMsg;
MPCDI_Error ErrorHelper::m_Error;

std::string ErrorHelper::GetLastError(bool addLeadingSpace)
{ 
  std::string temp = m_LastErrorMsg; 
  m_LastErrorMsg = ""; 
  if (addLeadingSpace && (temp.size()>0) && temp[0] != ' ')
    temp = " " + temp;

  return temp;
}

MPCDI_Error ErrorHelper::SetLastError(MPCDI_Error error, std::string msg)
{ 
  m_Error = error;
  m_LastErrorMsg = msg; 

  return m_Error;
}