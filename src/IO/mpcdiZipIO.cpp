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

#include "mpcdiZipIO.h"
#include "mpcdiErrorHelper.h"
#include "zip.h"

using namespace mpcdi;

#define WRITEBUFFERSIZE (16384)
#define CASESENSITIVITY (0)

/*=======================================================================*/

ZipWriter::ZipWriter(std::string archiveName)
: m_ArchiveName(archiveName),
  m_ZipFile(NULL)
{
}

/*=======================================================================*/

ZipWriter::~ZipWriter()
{
  //WriteArchive();
}

/*=======================================================================*/

MPCDI_Error ZipWriter::OpenArchive()
{
  m_ZipFile = zipOpen64(m_ArchiveName.c_str(),0);;
  return MPCDI_SUCCESS;
}

/*=======================================================================*/

MPCDI_Error ZipWriter::AddFile(std::string fileName,void* buffer, int bufferSize)
{
  if (m_ZipFile == NULL)
  {
    CREATE_ERROR_MSG(zipfileMsg,"OpenArchive() needs to be called first");
    ReturnCustomErrorMacro(MPCDI_FAILURE, zipfileMsg);
  }

  zip_fileinfo zi;
  unsigned long crcFile=0;
  int zip64 = 0;
  int opt_compress_level=Z_DEFAULT_COMPRESSION;
  const char* password=NULL;

  zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
  zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
  zi.dosDate = 0;
  zi.internal_fa = 0;
  zi.external_fa = 0;
  //filetime(filenameinzip,&zi.tmz_date,&zi.dosDate);
  
  //err = getFileCrc(filenameinzip,buf,size_buf,&crcFile);

  int zip_err = zipOpenNewFileInZip3_64(m_ZipFile,fileName.c_str(),&zi,
                                    NULL,0,NULL,0,NULL /* comment*/,
                                    (opt_compress_level != 0) ? Z_DEFLATED : 0,
                                    opt_compress_level,0,
                                    /* -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY, */
                                    -MAX_WBITS, DEF_MEM_LEVEL, Z_DEFAULT_STRATEGY,
                                    password,crcFile, zip64);
  if (zip_err != ZIP_OK)
  {
    CREATE_ERROR_MSG(openNewZipFileMsg,"Error opening " << fileName.c_str() << " in archive " << m_ArchiveName << ";");
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR,openNewZipFileMsg)
  }

  zip_err = zipWriteInFileInZip(m_ZipFile, buffer, bufferSize);
  if (zip_err != ZIP_OK)
  {
    CREATE_ERROR_MSG(openNewZipFileMsg,"Error writeing to " << fileName.c_str() << " in archive " << m_ArchiveName << ";");
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR,openNewZipFileMsg)
  }

  return MPCDI_SUCCESS;
}

/*=======================================================================*/

MPCDI_Error ZipWriter::WriteArchive()
{
  if (m_ZipFile != NULL)
  {
    zipClose(m_ZipFile, NULL);
    //delete m_ZipFile; no need taken care off inside zipClose
    m_ZipFile = NULL;
  }

  return MPCDI_SUCCESS;
}

/*=======================================================================*/

ZipReader::ZipReader(std::string archiveName):
  m_ArchiveName(archiveName),
  m_UnzFile(NULL)
{

}

/*=======================================================================*/

ZipReader::~ZipReader()
{

}

/*=======================================================================*/

MPCDI_Error ZipReader::OpenArchive()
{
  m_UnzFile = unzOpen64(m_ArchiveName.c_str());
  if (m_UnzFile == NULL)
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR, "Failed to open archive");

  return MPCDI_SUCCESS;
}

/*=======================================================================*/

MPCDI_Error ZipReader::GetFile(std::string fileName,void*& buffer, int & bufferSize)
{
  if (m_UnzFile == NULL)
  {
    return MPCDI_ARCHIVE_ERROR;
  }

  if (unzLocateFile(m_UnzFile,fileName.c_str(),CASESENSITIVITY)!=UNZ_OK)
  {
    CREATE_ERROR_MSG(noSuchFileMsg,"File with name " << fileName << " not found in archive " << m_ArchiveName);
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR, noSuchFileMsg);
  }

  char filename_inzip[256];
  unz_file_info64 file_info;
  uLong ratio=0;
  int unz_err = unzGetCurrentFileInfo64(m_UnzFile,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
  if (unz_err!=UNZ_OK)
  {
    CREATE_ERROR_MSG(getFileErrMsg, "Error getting info for " << fileName << "in archive " << m_ArchiveName); 
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR,getFileErrMsg);
  }

  bufferSize = (int)file_info.uncompressed_size;
  buffer = (void*)malloc(bufferSize);
  if (buffer==NULL)
  {
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR,"Error allocating memory for unzip file");
  }

  unz_err=unzOpenCurrentFilePassword(m_UnzFile,NULL);
  if (unz_err != UNZ_OK)
  {
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR,"Error on archive password");
  }

  unz_err = unzReadCurrentFile(m_UnzFile, buffer, bufferSize);
  if (unz_err<0) //if succesfull returns num read
  {
    CREATE_ERROR_MSG(readFileErrMsg, "Error reading " << fileName << "in archive " << m_ArchiveName); 
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR, readFileErrMsg);
  }

  unz_err = unzCloseCurrentFile (m_UnzFile);
  if (unz_err!=UNZ_OK)
  {
    CREATE_ERROR_MSG(closeFileErrMsg, "Error closing " << fileName << "in archive " << m_ArchiveName); 
    ReturnCustomErrorMacro(MPCDI_ARCHIVE_ERROR, closeFileErrMsg);
  }

  return MPCDI_SUCCESS;
}
