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

/*! \mainpage The MPCDI Library 
 *
 * \section intro_sec Summary
 *
 * This software is an implementation of the Version 1.0 of the MPCDI
 * standard as released by VESA (Vesa.org) in early summer of 2013.
 * The standard is a free download from the website, currently under
 * the 'Purchase Standards' link on Vesa.org. In this software,
 * you will find an implementation of:
 * 
 * - A container to hold the MPCDI Profile
 * - A read/writer to read and write the Profile to disk.
 * - A mechanism to validate the MPCDI Profile.
 * - A set of classes to help you create valid MPCDI Profiles.
 * - An example to illustrate creating the 4 types of MPCDI Profiles.
 * - An example to illustrate a test to Read and Write the MPCDI Profiles.
 *
 * \section getting_started_sec Getting started.
 * - On Windows, Open MPCDI/MCDPI.sln in Visual Studio 2008 or newer.
 * - For all other operating systems, use cmake version 2.8.5 or newer. From the MPCDI directory, try:
 *   - cmake .
 *   - make all 
 *
 * You may choose to use cmake on windows, but you will over-write the
 * shipped visual studio files. That is, please use one of the above
 * mechanisms, not both.
 *   
 * \section contents_sec Key Contents
 * - The Container class is a tree structure found in MPCDI/src/Container. The highest level element in the tree is the Profile <b> mpcdiProfile.h </b>
 *
 * - If you want to validate a Profile, you will find the Validate()
 * command in <b> mpcdiProfile.h </b>. It's very difficult to know if
 * the validator checks for everything, but we believe it gets between
 * 90% and 100% of the requirements.
 * 
 * - If you want to create a new profile, you will find 4 Creator
 * classes for the 4 different types of MPCDI files. Look in src/Creators 
 *
 * - There is a Read/Write example programs. <b> ReadWriteExample.cpp </b> 
 * - There is a example to create all 4 kinds of Profiles. <b> CreateAll.cpp </b>
 * 
 * \section thirdparty_sec ThirdParty contents.  
 * 
 * We have tried to use the smallest, minimal number of 3rd Party
 * libraries possible.  You will find 
 *   - TinyXml (xml library)
 *   - Zlib(compression/archival),
 *   - MiniZip (zip) and 
 *   - Libpng (for images)
 * 
 * \section authorship_sec Authorship
 *
 * This package was written by Scalable Display Technologies, Inc. The entirety of the Scalable Display engineering team contributed. 
 * 
 */
