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

#include "mpcdiFileUtils.h"
#include <stdlib.h>
#include <string.h>
#include <string>
#include <cstdio>
#include <assert.h>
#  include <sys/types.h>
#  include <sys/stat.h>
#include <time.h>
#include <fstream>
#include <iostream>
#if (defined(WIN32))
#  include "Windows.h"
#  include <io.h>
#else
#  include <unistd.h>
#  include <stdio.h>
#  include <limits.h>
#  include <unistd.h>
#  include <dirent.h>
#  include <fnmatch.h>
#endif

namespace mpcdi
{

/* ================================================================ */

bool exist_file(const char * const filename)
{
#if (defined(WIN32))
  return (!_access(filename,0));
#else
  return (!access(filename,F_OK));
#endif
}

/* ================================================================ */

bool delete_file(const char * const filename)
{
  return std::remove(filename) == 0;
}

/* ================================================================ */

bool move_file(const char * const file_in,
               const char * const file_out,
               const bool &OverWrite)
{
  if (!exist_file(file_in)) return false;
#ifndef WIN32
  if (OverWrite && exist_file(file_out)) 
    delete_file(file_out);
  return std::rename(file_in,file_out) == 0;
#else
  return MoveFileExA(file_in, file_out,
                     OverWrite ? MOVEFILE_REPLACE_EXISTING : NULL) != 0;
#endif
}

/* ================================================================ */

bool copy_file(const char * const file_in,
               const char * const file_out,
               const bool &OverWrite)
{
#ifdef WIN32
  return CopyFileA(file_in, file_out, !OverWrite) != 0;
#else
  std::ifstream ifs(file_in, std::ios::binary);
  std::ofstream ofs(file_out, std::ios::binary);
  if (!ifs.good() || !ofs.good()) return false;
  ofs << ifs.rdbuf();
  return true;
#endif
}

int time_since_modification_file(const char * const filename)
{
  time_t now;
  time(&now);
  return time_since_modification_file(filename,now);
}

int time_since_modification_file(const char * const filename, const time_t &base_time)
{
#if (defined(WIN32))
  if (!exist_file(filename)) 
    return -1;
 
  WIN32_FILE_ATTRIBUTE_DATA fileInfo;
  if (GetFileAttributesExA( filename, GetFileExInfoStandard, &fileInfo))
  {
    LARGE_INTEGER li_end, li_begin;
    FILETIME ft;
    
    LONGLONG LL = Int32x32To64(base_time, 10000000) + 116444736000000000;
    ft.dwLowDateTime = (DWORD) LL;
    ft.dwHighDateTime = LL >>32;

    li_end.LowPart = ft.dwLowDateTime;
    li_end.HighPart = ft.dwHighDateTime;

    li_begin.LowPart = fileInfo.ftLastWriteTime.dwLowDateTime;
    li_begin.HighPart = fileInfo.ftLastWriteTime.dwHighDateTime;

    return (int)((li_end.QuadPart - li_begin.QuadPart)/10000000);
  }
  else
  {
    return -1; //there might be a lock on the file by a different thread
  }
#else
  struct stat st;
  if (stat(filename,&st) == 0)
  {
    return ((int)(difftime(base_time, st.st_mtime)));
  }

  return -1;
#endif
}

/* ================================================================ */

bool copy_directory_contents(const char * const InDir,
                             const char * const OutDir,
                             const char * const WildCardFileSelection,
                             const bool &Recursive,
                             const bool &OverWrite,
                             const bool &MoveInstead)
{
  if (!exist_directory(InDir)) return false;
  if (!exist_directory(OutDir)) return false;
  
#ifdef WIN32
  // Non-recursive copy.
  WIN32_FIND_DATAA FindData;
  std::string AllFiles = std::string(InDir) + WildCardFileSelection;
  HANDLE hFind = FindFirstFileA(AllFiles.c_str(), &FindData);
  if (hFind == INVALID_HANDLE_VALUE) return false;
  do
    {
      if(strcmp(FindData.cFileName,".") == 0 || 
         strcmp(FindData.cFileName,"..") == 0)
      {
        continue;
      }

      if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && Recursive)
      {
        std::string InPath = std::string(InDir) + 
          "\\" + FindData.cFileName + "\\";
        std::string OutPath = std::string(OutDir) + 
          "\\" + FindData.cFileName + "\\";

        if(!exist_directory(OutPath.c_str()))
        {
          if(!create_dir(OutPath.c_str()))
          {
            FindClose(hFind);
            return false;
          }
        }

        if(!copy_directory_contents(InPath.c_str(),OutPath.c_str(),"*",
                                    Recursive,OverWrite,MoveInstead))
        {
          FindClose(hFind);
          return false;
        }
      }
      else
      {
          //std::cout << FindData.cFileName << std::endl;
          std::string ExistingFile = std::string(InDir) + 
            "\\" + FindData.cFileName;
          std::string NewFile = std::string(OutDir) + 
            "\\" + FindData.cFileName;
          if (MoveInstead)
            move_file(ExistingFile.c_str(), NewFile.c_str(), OverWrite);
          else
            copy_file(ExistingFile.c_str(), NewFile.c_str(), OverWrite);
        }
    }
  while (FindNextFileA(hFind, &FindData));
  FindClose(hFind);
  return true;
#else
  DIR *dir = opendir(InDir);
  if (dir == NULL) 
    { 
      // mplErrorMacro(<< "Error Opening " << dirname << " for copying");
      return false;
    }
  struct dirent *entry;
  bool current_result = true;
  // mplProgrammerMessageMacro(<< "Warning fnmatch not yet tested!");
  while ((entry = readdir(dir)) != NULL && current_result)
    {
      if ((entry->d_type != DT_DIR) &&
          (!fnmatch(WildCardFileSelection,entry->d_name,FNM_CASEFOLD)))
        //     (!WildCardMatch(WildCardFileSelection,entry->d_name)))
        {
          if(strcmp(entry->d_name,".") == 0 || 
             strcmp(entry->d_name,"..") == 0)
            {
              continue;
            }

          std::string ExistingFile = std::string(InDir) + "/" + entry->d_name;
          std::string NewFile = std::string(OutDir) + "/" + entry->d_name;

          if (MoveInstead)
            current_result = current_result &&
              move_file(ExistingFile.c_str(), NewFile.c_str(), OverWrite);
          else
            current_result = current_result &&
              copy_file(ExistingFile.c_str(), NewFile.c_str(), OverWrite);
        }
    }
  closedir(dir);
  return(current_result);


#endif
}


/* ================================================================ */

bool exist_directory(const char * const directory)
{
#if (defined(WIN32))
  return (!_access(directory,0));
#else
  struct stat st;
  return (stat(directory,&st) == 0);
#endif
}

/* ================================================================ */

bool create_dir(const char * const directory)
{
#if (defined(WIN32))
    return (CreateDirectoryA(directory,NULL)) ? true : false;
#else
  struct stat st;
  return (mkdir(directory,0777) == 0);
#endif
}

/* ================================================================ */

bool delete_directory(const char * const directory)
{
#if (defined(WIN32))

  const std::string &DirPath = directory;
  if(DirPath.empty())  return false;

  WIN32_FIND_DATAA FindData;
  std::string AllFiles = DirPath + "\\*";
  HANDLE hFind = FindFirstFileA(AllFiles.c_str(), &FindData);
  if(hFind == INVALID_HANDLE_VALUE) 
    return false;

  do
  {
    if(strcmp(FindData.cFileName,".") == 0 || 
       strcmp(FindData.cFileName,"..") == 0)
    {
      continue;
    }

    std::string Path =
            DirPath + "\\" + FindData.cFileName;

    if(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      if(!delete_directory(Path.c_str()))
      {
        FindClose(hFind);
        return false;
      }
    }
    else 
    {
      if(FindData.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
        _chmod(Path.c_str(), _S_IWRITE); // change read-only file mode

      if(!DeleteFileA(Path.c_str())) {
        FindClose(hFind);
        return false;
      }
    }
  } while(FindNextFileA(hFind, &FindData));

  if(GetLastError() != ERROR_NO_MORE_FILES)
  {
    FindClose(hFind);
    return false;
  }

  FindClose(hFind);

  return RemoveDirectoryA(DirPath.c_str()) != 0;
#else
  const char * dirname = directory;
  struct dirent *entry;
  char path[PATH_MAX];

  DIR *dir = opendir(dirname);
  if (dir == NULL) 
    { 
      // mplErrorMacro(<< "Error Opening " << dirname << " for deletion");
      return false;
    }

  bool current_result = true;
  while ((entry = readdir(dir)) != NULL && current_result)
    {
      if (strcmp(entry->d_name, ".") && strcmp(entry->d_name, "..")) 
        {
          snprintf(path, (size_t) PATH_MAX, "%s/%s", dirname, entry->d_name);
          if (entry->d_type == DT_DIR) 
            {
              current_result = delete_directory(path);
            }
          current_result = current_result && (remove(path) != 0);
        }
    }
  closedir(dir);
  return(current_result && (remove(dirname) != 0));
#endif
}


/* ================================================================ */

#if 0
char can_write_to(const char * const filename)
{
  FILE *f = NULL;
  if(exist_file(filename)) /* If the file exists, just check permissions */
      return !access(filename,W_OK);
  else /* Otherwise try writing there... */
    {
      f = openfile(filename,"w");
      if (f==NULL) return(0);
      fclose(f);
      unlink(filename);
      return 1;
    }
}
#endif

} //namespace mpcdi
