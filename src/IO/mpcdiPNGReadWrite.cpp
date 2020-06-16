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

#include "mpcdiPNGReadWrite.h"
#include "mpcdiErrorHelper.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#pragma warning(disable : 4005)
#include <png.h>
#pragma warning(default : 4005)

using namespace mpcdi;

#define PNG_SIGNATURE_SIZE 8

// Description:
// some helper functions, not defined in header to keep header easy to export
void PngReadFromStream(png_structp pngPtr, png_bytep data, png_size_t length);
void PngWriteFromStream(png_structp PngPointer, png_bytep Data, png_size_t Length);

/* ====================================================================== */

MPCDI_Error PNGReadWrite::Read(std::string fileName, mpcdi::DataMap *&dataMap)
{
  std::fstream source(fileName.c_str(), std::ios::in | std::ios::binary);
  if (!source.is_open())
  {
    source.close();
    CREATE_ERROR_MSG(msg,"Error opening file " << fileName << " for reading");
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR,msg);
  }

  MPCDI_Error err = Read(source,dataMap);

  source.close();

  return err;
}

/* ====================================================================== */

MPCDI_Error PNGReadWrite::Read(std::istream &source, mpcdi::DataMap *&dataMap)
{
  //validate stream
  if (!Validate(source)) {
    CREATE_ERROR_MSG(validateErrMsg, "Data is not valid PNG file" );
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR, validateErrMsg);
  }

  // attempt to create png read struct
  png_structp pngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngPtr) 
  {
    CREATE_ERROR_MSG(readStructErrMsg, "Failed initialize png read struct" );
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR, readStructErrMsg);
  }

  //create info struct
  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr) 
  {
    png_destroy_read_struct(&pngPtr, (png_infopp)0, (png_infopp)0);
    CREATE_ERROR_MSG(infoStructErrMsg, "Failed initialize png info struct" );
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR, infoStructErrMsg);
  }

  int number_of_passes;
  png_bytep * rowPtrs = NULL;

  if (setjmp(png_jmpbuf(pngPtr)))
  {
    png_destroy_read_struct(&pngPtr, (png_infopp)&infoPtr, (png_infopp)0);
    if (rowPtrs != NULL) delete [] rowPtrs;
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR, "Error occured while reading png file");
    return MPCDI_PNG_READ_ERROR;
  }

  // set custom reading function
  png_set_read_fn(pngPtr,(png_voidp)&source, PngReadFromStream);

  // tell png we already read the signature
  png_set_sig_bytes(pngPtr, PNG_SIGNATURE_SIZE);

  // read png info
  png_read_info(pngPtr, infoPtr);

  // get some inffo
  png_uint_32 width = png_get_image_width(pngPtr, infoPtr);
  png_uint_32 height = png_get_image_height(pngPtr, infoPtr);
  png_uint_32 color_type = png_get_color_type(pngPtr, infoPtr);
  png_uint_32 channels   = png_get_channels(pngPtr, infoPtr);
  png_uint_32 bit_depth = png_get_bit_depth(pngPtr, infoPtr);

  // if needed you can now set conversion functions to change the pallet
  // png_set_palette_to_rgb(), png_set_expand_gray_1_2_4_to_8(), png_get_valid(), png_set_tRNS_to_alpha() and png_set_strip_16()

  number_of_passes = png_set_interlace_handling(pngPtr);
  png_read_update_info(pngPtr, infoPtr);

  // allocate memory
  rowPtrs = (png_bytep*) malloc(sizeof(png_bytep) * height);
  for (unsigned int y=0; y<height; y++)
    rowPtrs[y] = (png_byte*) malloc(png_get_rowbytes(pngPtr,infoPtr));

  // actually read the image
  png_read_image(pngPtr, rowPtrs);

  // now move it over to the DataMap format
  mpcdi::ComponentDepth componentDepth=mpcdi::CD_THREE;
  switch (color_type)
  {
    case PNG_COLOR_TYPE_RGB:
      componentDepth = mpcdi::CD_THREE;
      break;
    case PNG_COLOR_TYPE_GRAY:
      componentDepth = mpcdi::CD_ONE;
      break;
  }
  dataMap = new mpcdi::DataMap(width,height,componentDepth);

  for (unsigned int y=0; y< height ; y++)
  {
    png_byte *row = rowPtrs[y];
    for (unsigned int x=0; x< width; x++)
    {
      for (unsigned int z=0; z< (unsigned int)componentDepth; z++)
        (*dataMap)(x,y,z) = row[(int)componentDepth*x+z];
    }
  }

  /* cleanup allocation */
  for (unsigned int y=0; y < height; y++)
          free(rowPtrs[y]);
  free(rowPtrs);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

MPCDI_Error PNGReadWrite::Write(std::string fileName, mpcdi::DataMap &dataMap)
{
  std::fstream source(fileName.c_str(), std::ios::out | std::ios::binary);
  if (!source.is_open())
  {
    source.close();
    CREATE_ERROR_MSG(msg,"Error opening file " << fileName << " for writing");
    ReturnCustomErrorMacro(MPCDI_PNG_WRITE_ERROR,msg);
  }

  MPCDI_Error err = Write(source,dataMap);

  source.close();

  return err;
}

/* ====================================================================== */

MPCDI_Error PNGReadWrite::Write( std::ostream &source, mpcdi::DataMap &dataMap)
{
  png_byte color_type = PNG_COLOR_TYPE_GRAY;
  switch (dataMap.GetComponentDepth())
  {
    case mpcdi::CD_ONE:
      color_type = PNG_COLOR_TYPE_GRAY;
      break;
    case mpcdi::CD_THREE:
      color_type = PNG_COLOR_TYPE_RGB;
      break;
  }
  png_byte bit_depth = dataMap.GetBitDepth();
  int width = dataMap.GetSizeX();
  int height = dataMap.GetSizeY();
  
  /* initialize stuff */
  png_structp pngPtr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!pngPtr)
  {
    CREATE_ERROR_MSG(readStructErrMsg, "Failed initialize png write struct" );
    ReturnCustomErrorMacro(MPCDI_PNG_WRITE_ERROR, readStructErrMsg);
  }

  png_infop infoPtr = png_create_info_struct(pngPtr);
  if (!infoPtr)
  {
    png_destroy_write_struct(&pngPtr, (png_infopp)0);
    CREATE_ERROR_MSG(infoStructErrMsg, "Failed initialize png info struct" );
    ReturnCustomErrorMacro(MPCDI_PNG_READ_ERROR, infoStructErrMsg);
  }

  png_byte ** rowPtrs = NULL;

  if (setjmp(png_jmpbuf(pngPtr)))
  {
    png_destroy_read_struct(&pngPtr, (png_infopp)&infoPtr, (png_infopp)0);
    if (rowPtrs != NULL)
    {
        for (unsigned int y=0; y<dataMap.GetSizeY(); y++)
          free(rowPtrs[y]);
        free(rowPtrs);
    }
    ReturnCustomErrorMacro(MPCDI_PNG_WRITE_ERROR, "Error occured while writing png file");
  }

  png_set_write_fn(pngPtr,(png_voidp)&source, PngWriteFromStream, NULL);

  png_set_IHDR(pngPtr, infoPtr, width , height ,
               bit_depth, color_type, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

  png_write_info(pngPtr, infoPtr);

  rowPtrs = (png_bytep*) png_malloc (pngPtr, height * sizeof (png_byte *));
  for (unsigned int y = 0; y < (unsigned int) height; ++y) {
      png_byte *row = (png_bytep) png_malloc (pngPtr, sizeof (unsigned char) * width * dataMap.GetComponentDepth());
      rowPtrs[y] = row;
      for (unsigned int x = 0; x < (unsigned int) width; ++x) 
      {
          for (unsigned int z =0 ; z< (unsigned int) dataMap.GetComponentDepth(); z++)
            *row++ = dataMap(x,y,z);
      }
  }  

  png_write_image(pngPtr,rowPtrs);

  png_write_end(pngPtr, NULL);

  /* cleanup heap allocation */
  for (unsigned int y=0; y<dataMap.GetSizeY(); y++)
          free(rowPtrs[y]);
  free(rowPtrs);

  png_destroy_write_struct(&pngPtr,&infoPtr);

  return MPCDI_SUCCESS;
}

/* ====================================================================== */

bool PNGReadWrite::Validate(std::istream& source) 
{
  unsigned char Header[PNG_SIGNATURE_SIZE] = {0};
  source.read(reinterpret_cast<char*>(&Header), sizeof(Header));

  if (png_sig_cmp(Header, 0, 8))
    return false;

  return true;
}

/* ====================================================================== */

void PngReadFromStream(png_structp PngPointer, png_bytep Data, png_size_t Length) 
{
    std::ifstream *Stream = (std::ifstream*)png_get_io_ptr(PngPointer);
    Stream->read((char*)Data, Length);
}

/* ====================================================================== */

void PngWriteFromStream(png_structp PngPointer, png_bytep Data, png_size_t Length)
{
    std::ofstream *Stream = (std::ofstream*)png_get_io_ptr(PngPointer);
    Stream->write((char*)Data, Length);
}
