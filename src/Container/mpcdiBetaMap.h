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
// .NAME BetaMap - An MPCDI Alpha Map 
// .SECTION Description
//
// 
// .AUTHOR Scalable Display Technologies, Inc.
//

#pragma once
#include <string>
#include "mpcdiHeader.h"
#include "mpcdiMacros.h"
#include "mpcdiDataMap.h"

namespace mpcdi {

  struct EXPORT_MPCDI BetaMap: DataMap {
    // Description:
    // Set Default Values
    inline 
    BetaMap(unsigned int sizeX, unsigned int sizeY,
            ComponentDepth componentDepth) :
      DataMap(sizeX,sizeY,componentDepth) {}

    // Description:
    // Get/set file path. should not be used
         mpcdiSetMacro(Path,std::string);
    mpcdiGetConstMacro(Path,std::string);

  protected:
    // Description:
    // member variables
    std::string m_Path;
  };

}; // end namespace mpcdi 


