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

#include "mpcdiXmlIO.h"
#include "mpcdiXMLStrings.h"
#include "mpcdiErrorHelper.h"
#include "mpcdiMacros.h"

using namespace mpcdi;

#define TINYXML2_SUCCEEDED(num) ((num) == tinyxml2::XML_NO_ERROR)
#define TINYXML2_FAILED(num)    ((num) != tinyxml2::XML_NO_ERROR)

/* ====================================================================== */

MPCDI_Error XmlIO::QueryIntAttribute(std::string attrName,tinyxml2::XMLElement *xmlElement, int &value)
{
  assert(xmlElement != NULL);

  int tempValue;
  if(TINYXML2_FAILED(xmlElement->QueryIntAttribute(attrName.c_str(), &tempValue)))
  {
    CREATE_ERROR_MSG(msg, "Failed to read attribute (" << attrName << ") on node: " << xmlElement->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR, msg);
  }
  
  value = tempValue;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error XmlIO::QueryFloatAttribute(std::string attrName,tinyxml2::XMLElement *xmlElement, float &value)
{
  assert(xmlElement != NULL);

  float tempValue;
  if(TINYXML2_FAILED(xmlElement->QueryFloatAttribute(attrName.c_str(), &tempValue)))
  {
    CREATE_ERROR_MSG(msg, "Failed to read attribute (" << attrName << ") on node: " << xmlElement->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }
  
  value = tempValue;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error XmlIO::QueryStringAttribute(std::string attrName,tinyxml2::XMLElement *xmlElement, std::string &value)
{
  assert(xmlElement != NULL);

  const char *attrValue = xmlElement->Attribute( attrName.c_str() );  
  if (attrValue == NULL)
  {
    CREATE_ERROR_MSG(msg, "Failed to read attribute (" << attrName << ") on node: " << xmlElement->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  value = std::string(attrValue);
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error XmlIO::QueryDoubleText(std::string textName, tinyxml2::XMLElement *xmlParent, double &value)
{
  value = -1;
  assert(xmlParent != NULL);

  tinyxml2::XMLElement *element=xmlParent->FirstChildElement(textName.c_str());
  if (element==NULL)
  {
    CREATE_ERROR_MSG(msg, "Failed find child node (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  double temp;
  if(TINYXML2_FAILED(element->QueryDoubleText(&temp)))
  {
    CREATE_ERROR_MSG(msg, "Failed parse text as double from (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  value = temp;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error XmlIO::QueryIntText(std::string textName, tinyxml2::XMLElement *xmlParent, int &value)
{
  value = -1;
  assert(xmlParent != NULL);

  tinyxml2::XMLElement *element=xmlParent->FirstChildElement(textName.c_str());
  if (element==NULL)
  {
    CREATE_ERROR_MSG(msg, "Failed find child node (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  int temp;
  if(TINYXML2_FAILED(element->QueryIntText(&temp)))
  {
    CREATE_ERROR_MSG(msg, "Failed parse text as int from (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  value = temp;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error XmlIO::QueryStringText(std::string textName, tinyxml2::XMLElement *xmlParent, std::string &value)
{
  value = "";
  assert(xmlParent != NULL);

  tinyxml2::XMLPrinter printer2;
  xmlParent->Accept(&printer2);
  std::string elementStringNew = std::string(printer2.CStr());

  tinyxml2::XMLElement *element=xmlParent->FirstChildElement(textName.c_str());
  if (element==NULL)
  {
    CREATE_ERROR_MSG(msg, "Failed find child node (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  std::string temp;
  if ( element->FirstChild() && element->FirstChild()->ToText() ) 
  {
      const char* t = element->FirstChild()->ToText()->Value();
      temp = std::string(t);
  }
  else
  {
    CREATE_ERROR_MSG(msg, "Failed parse text as string from (" << textName << ") on parent node: " << xmlParent->Name());
    ReturnCustomErrorMacro(MPCDI_XML_FORMAT_ERROR,msg);
  }

  value = temp;
  return MPCDI_SUCCESS;
}

/* ====================================================================== */

tinyxml2::XMLElement *XmlIO::AddNewXMLElement(std::string nodeName, tinyxml2::XMLElement *parent)
{
  assert(m_MPCDxml.RootElement() != NULL);
  return (tinyxml2::XMLElement *)parent->LinkEndChild(m_MPCDxml.NewElement(nodeName.c_str()));
}

/* ====================================================================== */



