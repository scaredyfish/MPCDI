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

#include <time.h>
#include <algorithm>
#include "mpcdiReaderIntern.h"
#include "mpcdiFileUtils.h"
#include "mpcdiUtils.h"
#include "mpcdiPNGReadWrite.h"
#include "mpcdiPfmIO.h"
#include "mpcdiErrorHelper.h"
#include "mpcdiMacros.h"

using namespace mpcdi;

const int ReaderIntern::s_MaxSupportedMajorVersion = 1;
const int ReaderIntern::s_MaxSupportedMinorVersions[] = {0};

/* ====================================================================== */

Reader * Reader::CreateReader()
{
  return (Reader*)(new ReaderIntern());
}

/* ====================================================================== */

ReaderIntern::ReaderIntern()
{
  m_DoProfileValidation = false;
  m_CheckVersionSupported = true;
}

/* ====================================================================== */

std::string ReaderIntern::GetSupportedVersions()
{
  std::stringstream ss;
  ss << "Supported versions: " << std::endl;
  for (int i=1;i<=s_MaxSupportedMajorVersion;i++)
  {
    ss << " MajorVersion: " << i << " Minor Versions";
    int supportedMinorVersions = s_MaxSupportedMinorVersions[i-1];
    for (int j=0;j<=supportedMinorVersions;j++)
      ss << "  ." << j;
    ss << std::endl;
  }

  return ss.str();
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::Read(std::istream &is, Profile * profile)
{
  return MPCDI_FAILURE;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::Read(std::string FileName, Profile * profile)
{
  if (!exist_file(FileName.c_str()))
    return MPCDI_FILE_ALREADY_EXISTS;

  m_Zipper = new ZipReader(FileName);
  m_Zipper->OpenArchive();

  void* buf=NULL;
  int size_buf;
  MPCDI_Error err;
  /* the standard specifies mpcdi.xml however mpacs.xml has been observed by other other implementations */
  if MPCDI_FAILED(err = m_Zipper->GetFile("mpcdi.xml", buf, size_buf))
    if MPCDI_FAILED(err = m_Zipper->GetFile("mpacs.xml", buf, size_buf))
      return err;

  // gzip layer needs to be included
  m_XmlIO = new XmlIO();
  m_XmlIO->GetDoc().Parse((const char*)buf,size_buf);
  mpcdi::ProfileVersion version;
  GetVersion(m_XmlIO->GetDoc(), version);

  err = ReadMPCD(*profile, version);

  //clean up
  free(buf);
  delete m_Zipper;
  m_Zipper = NULL;
  delete m_XmlIO;
  m_XmlIO = NULL;

  if (err != MPCDI_SUCCESS)
    return err;

  if (m_DoProfileValidation)
  {
    err = profile->ValidateProfile();
    if MPCDI_FAILED(err)
      return err;
  }

  return MPCDI_SUCCESS;
}

/*=======================================================================*/
MPCDI_Error ReaderIntern::GetVersion(tinyxml2::XMLDocument &doc, ProfileVersion & version)
{
  version = ProfileVersion();
  tinyxml2::XMLElement *root = m_XmlIO->GetDoc().RootElement();
  if ( std::string(root->Name()).compare(XML_NODES::MPCDI) != 0)
    return MPCDI_FAILURE;
    
  std::string versionstring;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::version, root, versionstring));
  return mpcdi::Utils::StringToProfileVersion(versionstring,version);
}

/*=======================================================================*/

MPCDI_Error ReaderIntern::ReadMPCD(Profile & profile, mpcdi::ProfileVersion version)
{
  if(m_CheckVersionSupported)
  {
    if((version.MajorVersion > s_MaxSupportedMajorVersion)) // fail on major version mismatch
    {
      CREATE_ERROR_MSG(msg, "MPACS Version: " << mpcdi::Utils::ProfileVersionToString(version) << " is not supported by the reader, you can turn CheckVersionSupported off at Reader will attempt to read it, some information might be skipped.")
      ReturnCustomErrorMacro(MPCDI_UNSPORTED_PROFILE_VERSION,msg);
    }

    if (version.MinorVersion > s_MaxSupportedMinorVersions[version.MajorVersion-1]) // drop down to highest minor version suppported
      version.MinorVersion = s_MaxSupportedMinorVersions[version.MajorVersion-1];
  }
  else
  {
    // drop down to highest supported version
    version.MajorVersion = std::min(version.MajorVersion,s_MaxSupportedMajorVersion);
    version.MinorVersion = s_MaxSupportedMinorVersions[version.MajorVersion-1];
  }

  assert(version.MajorVersion > 0);
  assert(version.MinorVersion >= 0);

  if (version.MajorVersion == 1 && version.MinorVersion == 0)
    return ReadMPCDVersion1_0(profile);

  // we should never actually make it here
  CREATE_ERROR_MSG(msg, "Failed to read profile");
  ReturnCustomErrorMacro(MPCDI_FAILURE, msg);
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadMPCDVersion1_0(Profile & profile)
{
  tinyxml2::XMLElement *root = m_XmlIO->GetDoc().RootElement();
  
  // Requirement:  XML 1.0 and UTF-8. Not bothering to check for it.
  MPCDI_FAIL_RET(ReadProfile1_0(root, profile));
  MPCDI_FAIL_RET(ReadDisplay1_0(root, *profile.GetDisplay()));
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadProfile1_0(tinyxml2::XMLElement *element, Profile & profile)
{
  std::string profileType;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::profile, element, profileType));
  ProfileType currentType = GetProfileType(profileType.c_str());
  profile.SetProfileType(currentType);

  std::string dateValue;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::date, element, dateValue));
  profile.SetDate(dateValue); // FIXME: do a ISO8601 format check

  int levelValue;
  MPCDI_FAIL_RET(XmlIO::QueryIntAttribute(XML_ATTR::level, element, levelValue)); 
  profile.SetLevel(levelValue);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadDisplay1_0(tinyxml2::XMLElement *parent, Display & display)
{
  tinyxml2::XMLElement *displayElement = parent->FirstChildElement(XML_NODES::display.c_str());
  if (displayElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,"Failed to find display node");

  tinyxml2::XMLElement *bufferElement = displayElement->FirstChildElement();
  for(bufferElement; bufferElement; bufferElement = bufferElement->NextSiblingElement())
  { 
    MPCDI_FAIL_RET(ReadBuffer1_0(bufferElement, display)); 
  } 
  
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadBuffer1_0(tinyxml2::XMLElement *bufferElement, Display &display)
{
  assert(bufferElement != NULL);
  MPCDI_Error err;

  // check if indeed is buffer element
  if(std::string(bufferElement->Name()) != XML_NODES::buffer)
  {
    std::string errMsg = std::string("Expected a buffer found: ") + std::string(bufferElement->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, errMsg);
  }

  const char *atrValue = bufferElement->Attribute(XML_ATTR::id.c_str());
  if (atrValue == NULL)
    return MPCDI_FAILED_TO_FIND_XML_ATTRIBUTE;

  std::string bufferId = std::string(atrValue);
  if MPCDI_FAILED(err = display.NewBuffer(bufferId))
    return err;

  Buffer *buffer = display.GetBuffer(bufferId);

  int value;
  buffer->SetXresolution(-1);
  if MPCDI_SUCCEEDED(XmlIO::QueryIntAttribute(XML_ATTR::Xresolution, bufferElement, value))
    buffer->SetXresolution(value);
  buffer->SetYresolution(-1);
  if MPCDI_SUCCEEDED(XmlIO::QueryIntAttribute(XML_ATTR::Yresolution, bufferElement, value))
    buffer->SetYresolution(value);
  
  tinyxml2::XMLElement *regionElement = bufferElement->FirstChildElement();
  for(regionElement; regionElement; regionElement = regionElement->NextSiblingElement())
  { 
    MPCDI_FAIL_RET(ReadRegion(regionElement, *buffer)); // should we delete buffer on fail?
  } 

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadRegion(tinyxml2::XMLElement *regionElement, Buffer &buffer)
{
  assert(regionElement != NULL);
  MPCDI_Error err;

  // check if indeed is buffer element
  if(std::string(regionElement->Name()) != XML_NODES::region)
    return MPCDI_XML_FORMAT_ERROR;

  const char *atrValue = regionElement->Attribute(XML_ATTR::id.c_str());
  if (atrValue == NULL)
    ReturnCustomErrorMacro(MPCDI_FAILED_TO_FIND_XML_ATTRIBUTE, "Failed to find buffer id attribute");

  std::string regionId = std::string(atrValue);
  if(MPCDI_FAILED(err=buffer.NewRegion(regionId)))
    return err;
    
  Region *region = buffer.GetRegion(regionId);
  
  MPCDI_FAIL_RET(XmlIO::QueryIntAttribute(XML_ATTR::Xresolution,regionElement,region->GetRefXresolution()));
  MPCDI_FAIL_RET(XmlIO::QueryIntAttribute(XML_ATTR::Yresolution,regionElement,region->GetRefYresolution()));
  MPCDI_FAIL_RET(XmlIO::QueryFloatAttribute(XML_ATTR::x,regionElement,region->GetRefX()));
  MPCDI_FAIL_RET(XmlIO::QueryFloatAttribute(XML_ATTR::y,regionElement,region->GetRefY()));
  MPCDI_FAIL_RET(XmlIO::QueryFloatAttribute(XML_ATTR::xsize,regionElement,region->GetRefXsize()));
  MPCDI_FAIL_RET(XmlIO::QueryFloatAttribute(XML_ATTR::ysize,regionElement,region->GetRefYsize()));


  tinyxml2::XMLElement *element = regionElement->FirstChildElement(XML_NODES::frustum.c_str());
  if (element != NULL)
  {
    region->SetFrustum();
    MPCDI_FAIL_RET(ReadFrustum1_0(element, *region->GetFrustum())); 
  }
  
  element = regionElement->FirstChildElement(XML_NODES::coordinateFrame.c_str());
  if (element != NULL)
  {
    region->SetCoordinateFrame();
    MPCDI_FAIL_RET(ReadCoordinateFrame1_0(element, *region->GetCoordinateFrame())); 
  }

  MPCDI_FAIL_RET(ReadFileSet1_0(region->GetId(), *region->GetFileSet()));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadFrustum1_0(tinyxml2::XMLElement *frustumElement, Frustum & frustum)
{
  if (frustumElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "Frustum node not found");
  
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::yaw,frustumElement,frustum.GetRefYaw()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::pitch,frustumElement,frustum.GetRefPitch()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::roll,frustumElement,frustum.GetRefRoll()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::rightAngle,frustumElement,frustum.GetRefRightAngle()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::leftAngle,frustumElement,frustum.GetRefLeftAngle()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::upAngle,frustumElement,frustum.GetRefUpAngle()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::downAngle,frustumElement,frustum.GetRefDownAngle()));

  //if(LeftAngle > RightAngle) 
  //  return ERROR_SOME_ERROR;
  //if(DownAngle < UpAngle)    
  //  return ERROR_SOME_ERROR;

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadCoordinateFrame1_0(tinyxml2::XMLElement *coordinateFrameElement, CoordinateFrame &coordinateFrame)
{
  if (coordinateFrameElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "CoordinateFrame node not found");
  
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::posx, coordinateFrameElement, coordinateFrame.GetRefPosx()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::posy, coordinateFrameElement, coordinateFrame.GetRefPosy()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::posz, coordinateFrameElement, coordinateFrame.GetRefPosz()));

  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::yawx, coordinateFrameElement, coordinateFrame.GetRefYawx()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::yawy, coordinateFrameElement, coordinateFrame.GetRefYawy()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::yawz, coordinateFrameElement, coordinateFrame.GetRefYawz()));

  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::pitchx, coordinateFrameElement, coordinateFrame.GetRefPitchx()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::pitchy, coordinateFrameElement, coordinateFrame.GetRefPitchy()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::pitchz, coordinateFrameElement, coordinateFrame.GetRefPitchz()));

  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::rollx, coordinateFrameElement, coordinateFrame.GetRefRollx()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::rolly, coordinateFrameElement, coordinateFrame.GetRefRolly()));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::rollz, coordinateFrameElement, coordinateFrame.GetRefRollz()));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadFileSet1_0(std::string regionid, FileSet & fileSet)
{
  tinyxml2::XMLElement *filesElement;
  filesElement = m_XmlIO->GetDoc().RootElement()->FirstChildElement(XML_NODES::files.c_str());

  if (filesElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "FilesElement node not found");

  tinyxml2::XMLElement *fileSetElement = filesElement->FirstChildElement();
  for(fileSetElement; fileSetElement; fileSetElement = fileSetElement->NextSiblingElement())
  { 
    std::string nodeRegionId;
    MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::region, fileSetElement, nodeRegionId));
    if (nodeRegionId==regionid)
    {
      MPCDI_FAIL_RET(ReadGeometryWarpFile1_0(fileSetElement->FirstChildElement(XML_NODES::geometryWarpFile.c_str()), fileSet));
      MPCDI_FAIL_RET(ReadAlphaMap1_0(fileSetElement->FirstChildElement(XML_NODES::alphaMap.c_str()), fileSet));

      //// Only fail on this guys if there is an error Reading in the structure.
      //// if the tags are not there, don't worry about it.
      //// CAREFUL! Only allowed to miss the high level tag, nothing more.
      tinyxml2::XMLElement *element = fileSetElement->FirstChildElement(XML_NODES::betaMap.c_str());
      if (element!=NULL)
        MPCDI_FAIL_RET(ReadBetaMap1_0(element, fileSet));
      
      element = fileSetElement->FirstChildElement(XML_NODES::distortionMap.c_str());
      if (element != NULL)
        MPCDI_FAIL_RET(ReadDistortionMapFile1_0(element, fileSet));

    }
  } 

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadAlphaMap1_0(tinyxml2::XMLElement *alphaMapElement, FileSet & fileSet)
{
  if (alphaMapElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "AlphaMapElement node not found");

  std::string alphaMapPath;
  int componentDepth,bitDepth;
  double gammaEmbedded;
  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::path, alphaMapElement, alphaMapPath));
  MPCDI_FAIL_RET(XmlIO::QueryIntText(XML_NODES::componentDepth, alphaMapElement, componentDepth));
  MPCDI_FAIL_RET(XmlIO::QueryIntText(XML_NODES::bitDepth, alphaMapElement, bitDepth));
  MPCDI_FAIL_RET(XmlIO::QueryDoubleText(XML_NODES::gammaEmbedded, alphaMapElement, gammaEmbedded));

  DataMap *dataMap = NULL;
  MPCDI_FAIL_RET(ReadDataMap(alphaMapPath, dataMap));

  fileSet.SetAlphaMap(dataMap->GetSizeX(),dataMap->GetSizeY(),dataMap->GetComponentDepth());
  fileSet.GetAlphaMap()->CopyData(*dataMap);
  //todo copy data!!

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadBetaMap1_0(tinyxml2::XMLElement *betaMapElement, FileSet & fileSet)
{
  if (betaMapElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "BetaMapElement node not found");

  std::string betaMapPath;
  int bitDepth,componentDepth;
  MPCDI_FAIL_RET(XmlIO::QueryIntText(XML_NODES::bitDepth, betaMapElement, bitDepth));
  MPCDI_FAIL_RET(XmlIO::QueryIntText(XML_NODES::componentDepth, betaMapElement, componentDepth));
  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::path, betaMapElement, betaMapPath));

  DataMap *dataMap = NULL;
  MPCDI_FAIL_RET(ReadDataMap(betaMapPath, dataMap));

  fileSet.SetBetaMap(dataMap->GetSizeX(),dataMap->GetSizeY(),dataMap->GetComponentDepth());
  fileSet.GetBetaMap()->CopyData(*dataMap);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadGeometryWarpFile1_0(tinyxml2::XMLElement *dmfElement, FileSet & fileSet)
{
  if (dmfElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "ReadGeometryWarpFile node not found");


  std::string path;
  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::path, dmfElement, path));
  
  PFM *pfm = NULL;
  MPCDI_FAIL_RET(ReadPFM(path, pfm));

  MPCDI_Error err;
  if (MPCDI_FAILED(err = fileSet.SetGeometryWarpFile(pfm->GetSizeX(),pfm->GetSizeY())))
    return err;
  GeometryWarpFile * warpFile = fileSet.GetGeometryWarpFile();
  warpFile->SetPath(path);
  warpFile->CopyData(*pfm);

  std::string value;
  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::geometricUnit, dmfElement, value));
  warpFile->SetGeometricUnit(GetGeometricUnit(value));

  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::interpolation, dmfElement, value));
  warpFile->SetInterpolation(GetInterpolation(value));

  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::originof3DData, dmfElement, value));
  warpFile->SetOriginOf3DData(GetOriginOf3DData(value));

  //if (sl && origin == IdealeyePoint)  return ERROR_OF_SOME_SORT;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadDistortionMapFile1_0(tinyxml2::XMLElement *dmfElement, FileSet & fileSet)
{
  if (dmfElement == NULL)
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, "DistortionMap node not found");

  std::string path;
  MPCDI_FAIL_RET(XmlIO::QueryStringText(XML_NODES::path, dmfElement, path));

  PFM *pfm = NULL;
  MPCDI_FAIL_RET(ReadPFM(path, pfm));

  MPCDI_Error err;
  if (MPCDI_FAILED(err = fileSet.SetDistortionMap(pfm->GetSizeX(),pfm->GetSizeY())))
    return err;
  DistortionMap * distortionMap = fileSet.GetDistortionMap();
  distortionMap->SetPath(path);
  distortionMap->CopyData(*pfm);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadDataMap(std::string FileName, DataMap *&dataMap)
{
  void* buf=NULL;
  int size_buf;
  MPCDI_Error err = m_Zipper->GetFile(FileName.c_str(), buf, size_buf);
  if MPCDI_FAILED(err)
    return err;

  std::string buffer;
  std::stringstream membuf;
  membuf.write((const char*)buf,size_buf);
  free(buf);

  err = PNGReadWrite::Read(membuf,dataMap);

  return err;
}

/* ====================================================================== */

MPCDI_Error ReaderIntern::ReadPFM(std::string FileName, PFM *&pfm)
{
  void* buf=NULL;
  int size_buf;
  MPCDI_Error err = m_Zipper->GetFile(FileName.c_str(), buf, size_buf);
  if MPCDI_FAILED(err)
    return err;

  std::string buffer;
  std::stringstream membuf;
  membuf.write((const char*)buf,size_buf);
  free(buf);

  err = PfmIO::Read(membuf,pfm);

  return err;
}
