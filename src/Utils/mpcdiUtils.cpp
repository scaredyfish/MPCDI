/* =========================================================================

  Program:   MPCDI Library
  Language:  C++
  Date:      $Date: 2012-08-22 20:19:58 -0400 (Wed, 22 Aug 2012) $
  Version:   $Revision: 19513 $

  Copyright (c) 2013 Scalable Display Technologies, Inc.
  All Rights Reserved.
  The MPCDI Library is distributed under the BSD license.
  Please see License.txt distributed with this package.

===================================================================auto== */

#include "mpcdiUtils.h"

using namespace mpcdi;

std::string Utils::ProfileVersionToString(const ProfileVersion& pv)
{
  std::stringstream ss;
  ss << pv.MajorVersion <<  "." << pv.MinorVersion;
  return ss.str();
}

MPCDI_Error Utils::StringToProfileVersion(const std::string &text, ProfileVersion& pv)
{
  pv = ProfileVersion();
  
  size_t pos = text.find_first_of(".");
  if (pos==std::string::npos)
    return MPCDI_FAILURE;

  pv.MajorVersion = StringToNumber<int>(text.substr(0,pos));
  pv.MinorVersion = 0;
  if (text.size() >= pos+1)
    pv.MinorVersion = StringToNumber<int>(text.substr(pos+1,text.length()));

  return MPCDI_SUCCESS;
}
