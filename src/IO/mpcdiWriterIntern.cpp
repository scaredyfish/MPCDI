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
#include "mpcdiWriterIntern.h"
#include "mpcdiFileUtils.h"
#include "mpcdiUtils.h"
#include "mpcdiPNGReadWrite.h"
#include "mpcdiPfmIO.h"
#include "mpcdiProfile.h"

using namespace mpcdi;

/* ====================================================================== */
Writer *Writer::CreateWriter()
{
  return (Writer*)(new WriterIntern());
}

/* ====================================================================== */

WriterIntern::WriterIntern()
{
  m_OverwriteExistingFile = true;
  m_DoProfileValidation = true;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::Write(std::string FileName, Profile & profile)
{
  MPCDI_Error err;
  if (m_DoProfileValidation)
  {
    err = profile.ValidateProfile();
    if MPCDI_FAILED(err)
      return err;
  }

  std::string archiveName = FileName;
  if (exist_file(FileName.c_str()))
  {
    if(!m_OverwriteExistingFile)
      return MPCDI_FILE_ALREADY_EXISTS;

    archiveName += ".temp";
  }

  m_XmlIO = new XmlIO();
  m_Zipper = new ZipWriter(archiveName);
  m_Zipper->OpenArchive();
  
  err = WriteMPCD(profile);
  if (err != MPCDI_SUCCESS)
    return err;

  tinyxml2::XMLPrinter printer;
  m_XmlIO->GetDoc().Accept(&printer);
  std::string documentString = std::string(printer.CStr());
  
  void* buf=NULL;
  int size_buf = printer.CStrSize();

#pragma warning(disable : 4996)
  buf = (void*)malloc(size_buf);

  sprintf((char*)buf,"%s",documentString.c_str());
  err = m_Zipper->AddFile("mpcdi.xml", buf, size_buf-1);
  if MPCDI_FAILED(err) return err;
  
  m_Zipper->WriteArchive();

  if (FileName!=archiveName)
  {
    if(delete_file(FileName.c_str()))
      copy_file(archiveName.c_str(), FileName.c_str(), false);
    else
      err = MPCDI_FILE_PERMISSION_ERROR;

    delete_file(archiveName.c_str());
  }

  //clean up
  free(buf);
#pragma warning(default : 4996)
  buf = NULL;
  m_XmlIO->GetDoc().Clear();

  return err;
}

/*=======================================================================*/

MPCDI_Error WriterIntern::WriteMPCD(Profile & profile)
{
  m_XmlIO->GetDoc().Clear();

  tinyxml2::XMLDeclaration* decl = m_XmlIO->GetDoc().NewDeclaration();
  m_XmlIO->GetDoc().LinkEndChild( decl );
  
  mpcdi::ProfileVersion version = profile.GetVersion();
  if (version.MajorVersion == 1 && version.MinorVersion == 0)
     return WriteMPCDVersion1_0(profile);
  else
      return WriteMPCDVersion1_0(profile); // default drop down to version 1.0
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteMPCDVersion1_0(Profile & profile)
{
  tinyxml2::XMLElement* root = m_XmlIO->GetDoc().NewElement( XML_NODES::MPCDI.c_str() );
  m_XmlIO->GetDoc().LinkEndChild( root );

  // Requirement:  XML 1.0 and UTF-8. Not bothering to check for it.
  MPCDI_FAIL_RET(WriteProfile1_0(root, profile));
  MPCDI_FAIL_RET(WriteDisplay1_0(root, *profile.GetDisplay()));
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteProfile1_0(tinyxml2::XMLElement *element, 
                                          Profile & profile)
{
  std::string profileType=GetProfileType(profile.GetProfileType());
  element->SetAttribute(XML_ATTR::profile.c_str(),profileType.c_str()); 

  time_t now;
  time (&now);
  char time_buffer [sizeof "2011-10-08T07:07:09Z"];
#pragma warning(disable : 4996)
  strftime(time_buffer, sizeof time_buffer, "%Y-%m-%dT%H:%M:%SZ",localtime(&now)); //GIJSTODO CHECK IF THIS SHOULD BE gmtime(..)
#pragma warning(default : 4996)

  element->SetAttribute(XML_ATTR::date.c_str(),time_buffer); // must be ISO8601 fomat: yyyy-MM-dd HH:mm:ss
  element->SetAttribute(XML_ATTR::level.c_str(),profile.GetLevel()); 
  element->SetAttribute(XML_ATTR::version.c_str(),mpcdi::Utils::ProfileVersionToString(profile.GetVersion()).c_str());
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteDisplay1_0(tinyxml2::XMLElement *parent, 
                                          Display & display)
{
  tinyxml2::XMLElement *displayElement = 
      m_XmlIO->AddNewXMLElement(XML_NODES::display, parent);

  for(Display::BufferIterator it=display.GetBufferBegin(); 
      it!=display.GetBufferEnd(); it++)
  { 
    MPCDI_FAIL_RET(WriteBuffer1_0(displayElement, *it->second)); 
  } 
  
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteBuffer1_0(tinyxml2::XMLElement *parent, Buffer &buffer)
{
  tinyxml2::XMLElement *bufferElement = m_XmlIO->AddNewXMLElement(XML_NODES::buffer, parent);

  bufferElement->SetAttribute(XML_ATTR::id.c_str(),buffer.GetId().c_str());
  if (buffer.GetXresolution()>0)
    bufferElement->SetAttribute(XML_ATTR::Xresolution.c_str(),buffer.GetXresolution()); // if failure, it is OK.
  if (buffer.GetYresolution()>0)
    bufferElement->SetAttribute(XML_ATTR::Yresolution.c_str(),buffer.GetYresolution()); // if failure, it is OK.
  
  for(Buffer::RegionIterator it=buffer.GetRegionBegin(); it != buffer.GetRegionEnd(); it++) 
  { 
    MPCDI_FAIL_RET(WriteRegion(bufferElement, *it->second)); 
  } 

  //if (a3d or sl  && NumRegions != 1) return ERROR_OF_SOME_SORT;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteRegion(tinyxml2::XMLElement *parent, Region &region)
{
  tinyxml2::XMLElement *regionElement = m_XmlIO->AddNewXMLElement(XML_NODES::region,parent);

  regionElement->SetAttribute(XML_ATTR::id.c_str(),region.GetId().c_str());
  regionElement->SetAttribute(XML_ATTR::Xresolution.c_str(),region.GetXresolution()); 
  regionElement->SetAttribute(XML_ATTR::Yresolution.c_str(),region.GetYresolution());
  regionElement->SetAttribute(XML_ATTR::x.c_str(),region.GetX());
  regionElement->SetAttribute(XML_ATTR::y.c_str(),region.GetY());
  regionElement->SetAttribute(XML_ATTR::xsize.c_str(),region.GetXsize());
  regionElement->SetAttribute(XML_ATTR::ysize.c_str(),region.GetYsize());

  Frustum *frustum = region.GetFrustum();
  if (frustum != NULL)
    MPCDI_FAIL_RET(WriteFrustum1_0(regionElement, *frustum)); 

  CoordinateFrame *coordinateFrame = region.GetCoordinateFrame();
  if (coordinateFrame != NULL)
    MPCDI_FAIL_RET(WriteCoordinateFrame1_0(regionElement, *coordinateFrame)); 

  MPCDI_FAIL_RET(WriteFileSet1_0(region.GetId(), *region.GetFileSet()));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteFrustum1_0(tinyxml2::XMLElement *parent, Frustum & frustum)
{
  tinyxml2::XMLElement *frustumElement = m_XmlIO->AddNewXMLElement(XML_NODES::frustum,parent);

  //if(LeftAngle > RightAngle) 
  //  return ERROR_SOME_ERROR;
  //if(DownAngle < UpAngle)    
  //  return ERROR_SOME_ERROR;

  m_XmlIO->AddNewXMLText(XML_NODES::yaw,frustum.GetYaw(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::pitch,frustum.GetPitch(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::roll,frustum.GetRoll(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::rightAngle,frustum.GetRightAngle(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::leftAngle,frustum.GetLeftAngle(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::upAngle,frustum.GetUpAngle(),frustumElement);
  m_XmlIO->AddNewXMLText(XML_NODES::downAngle,frustum.GetDownAngle(),frustumElement);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteCoordinateFrame1_0(
                                        tinyxml2::XMLElement *parent, 
                                        CoordinateFrame &coordinateFrame)
{
  tinyxml2::XMLElement *coordinateFrameElement =
      m_XmlIO->AddNewXMLElement(XML_NODES::coordinateFrame,parent);

  m_XmlIO->AddNewXMLText(XML_NODES::posx, coordinateFrame.GetPosx(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::posy, coordinateFrame.GetPosy(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::posz, coordinateFrame.GetPosz(), coordinateFrameElement);

  m_XmlIO->AddNewXMLText(XML_NODES::yawx, coordinateFrame.GetYawx(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::yawy, coordinateFrame.GetYawy(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::yawz, coordinateFrame.GetYawz(), coordinateFrameElement);

  m_XmlIO->AddNewXMLText(XML_NODES::pitchx, coordinateFrame.GetPitchx(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::pitchy, coordinateFrame.GetPitchy(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::pitchz, coordinateFrame.GetPitchz(), coordinateFrameElement);

  m_XmlIO->AddNewXMLText(XML_NODES::rollx, coordinateFrame.GetRollx(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::rolly, coordinateFrame.GetRolly(), coordinateFrameElement);
  m_XmlIO->AddNewXMLText(XML_NODES::rollz, coordinateFrame.GetRollz(), coordinateFrameElement);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteFileSet1_0(std::string regionid, 
                                          FileSet & fileSet)
{
  tinyxml2::XMLElement *parent;
  parent = m_XmlIO->GetDoc().RootElement()->FirstChildElement(XML_NODES::files.c_str());
  if (!parent)
    parent = m_XmlIO->AddNewXMLElement(XML_NODES::files,m_XmlIO->GetDoc().RootElement());
  
  assert(parent != NULL);

  tinyxml2::XMLElement *filesetElement = m_XmlIO->AddNewXMLElement(XML_NODES::fileset,parent);
  filesetElement->SetAttribute(XML_ATTR::region.c_str(),regionid.c_str());

  MPCDI_FAIL_RET(WriteGeometryWarpFile1_0(filesetElement, *fileSet.GetGeometryWarpFile()));
  MPCDI_FAIL_RET(WriteAlphaMap1_0(filesetElement, *fileSet.GetAlphaMap()));

  //// Only fail on this guys if there is an error Writeing in the structure.
  //// if the tags are not there, don't worry about it.
  //// CAREFUL! Only allowed to miss the high level tag, nothing more.
  if (fileSet.GetBetaMap() != NULL)
    MPCDI_FAIL_RET(WriteBetaMap1_0(filesetElement, *fileSet.GetBetaMap()));

  if (fileSet.GetDistortionMap() != NULL)
    MPCDI_FAIL_RET(WriteDistortionMapFile1_0(filesetElement, *fileSet.GetDistortionMap()));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteAlphaMap1_0(tinyxml2::XMLElement *parent, 
                                           AlphaMap & alphaMap)
{
  assert(parent!=NULL);

  std::string regiondID;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::region,parent, regiondID));
  std::string fullPath = "AlphaMap-" + regiondID + ".png";

  tinyxml2::XMLElement * amElement = m_XmlIO->AddNewXMLElement(XML_NODES::alphaMap, parent);
  m_XmlIO->AddNewXMLText(XML_NODES::path, fullPath, amElement);
  m_XmlIO->AddNewXMLText(XML_NODES::componentDepth, alphaMap.GetComponentDepth(), amElement);
  m_XmlIO->AddNewXMLText(XML_NODES::bitDepth, alphaMap.GetBitDepth(), amElement);
  m_XmlIO->AddNewXMLText(XML_NODES::gammaEmbedded, alphaMap.GetGammaEmbedded(), amElement);

  MPCDI_FAIL_RET(WriteDataMap(fullPath, alphaMap));
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteBetaMap1_0(tinyxml2::XMLElement *parent, 
                                          BetaMap &betaMap)
{
  assert(parent!=NULL);

  std::string regiondID;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::region,parent, regiondID));
  std::string fullPath = "BetaMap-" + regiondID + ".png";

  tinyxml2::XMLElement * bmElement = m_XmlIO->AddNewXMLElement(XML_NODES::betaMap, parent);
  m_XmlIO->AddNewXMLText(XML_NODES::path, fullPath, bmElement);
  m_XmlIO->AddNewXMLText(XML_NODES::bitDepth, betaMap.GetBitDepth(), bmElement);
  m_XmlIO->AddNewXMLText(XML_NODES::componentDepth, betaMap.GetComponentDepth(), bmElement);
  
  MPCDI_FAIL_RET(WriteDataMap(fullPath, betaMap));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteGeometryWarpFile1_0(tinyxml2::XMLElement *parent, 
                                                   GeometryWarpFile & geometryWarpFile)
{
  assert(parent!=NULL);

  std::string regiondID;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::region,parent, regiondID));
  std::string fullPath = "GeometryWarpFile-" + regiondID + ".pfm";

  tinyxml2::XMLElement * gwfElement = m_XmlIO->AddNewXMLElement(XML_NODES::geometryWarpFile, parent);
  m_XmlIO->AddNewXMLText(XML_NODES::path, fullPath, gwfElement);
  
  std::string value = GetGeometricUnit(geometryWarpFile.GetGeometricUnit());
  m_XmlIO->AddNewXMLText(XML_NODES::geometricUnit, value , gwfElement);
  
  value = GetInterpolation(geometryWarpFile.GetInterpolation());
  m_XmlIO->AddNewXMLText(XML_NODES::interpolation, value, gwfElement);
  
  geometryWarpFile.GetOriginOf3DData();
  m_XmlIO->AddNewXMLText(XML_NODES::originof3DData, value, gwfElement);

  MPCDI_FAIL_RET(WritePFM(fullPath, geometryWarpFile));

  //if (sl && origin == IdealeyePoint)  return ERROR_OF_SOME_SORT;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteDistortionMapFile1_0(tinyxml2::XMLElement *parent, 
                                                    DistortionMap & distortionMap)
{
  assert(parent!=NULL);

  std::string regiondID;
  MPCDI_FAIL_RET(XmlIO::QueryStringAttribute(XML_ATTR::region,parent, regiondID));
  std::string fullPath = "DistortionMapFile-" + regiondID + ".pfm";
  
  tinyxml2::XMLElement * dmElement = m_XmlIO->AddNewXMLElement(XML_NODES::distortionMap, parent); 
  m_XmlIO->AddNewXMLText(XML_NODES::path, fullPath, dmElement);
  
  MPCDI_FAIL_RET(WritePFM(fullPath, distortionMap));

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WriteDataMap(std::string fileName, DataMap &dataMap)
{
  std::ostringstream membuf(std::ios::in | std::ios::binary);

  MPCDI_Error err = PNGReadWrite::Write(membuf,dataMap);

  // there has to be a better way! wrap a vector in a memory buffer 
  // and pre allocate should be better and fewer copies
  std::string continuosBuf = membuf.str();
  membuf.clear();

  m_Zipper->AddFile(fileName,(void*)continuosBuf.c_str(),continuosBuf.size());
  return err;
}

/* ====================================================================== */

MPCDI_Error WriterIntern::WritePFM(std::string fileName, PFM &pfm)
{
  std::ostringstream membuf(std::ios::in | std::ios::binary);

  MPCDI_FAIL_RET(PfmIO::Write(membuf,pfm));

  // there has to be a better way! wrap a vector in a memory buffer and pre allocate should be better and fewer copies
  std::string continuousBuf = membuf.str();
  membuf.clear();

  MPCDI_FAIL_RET(m_Zipper->AddFile(fileName,(void*)continuousBuf.c_str(),continuousBuf.size()));
  return MPCDI_SUCCESS;
}
